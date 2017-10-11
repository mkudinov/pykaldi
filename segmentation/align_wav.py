#coding: utf-8
import cffi
import numpy as np
import os
import pdb
import sys

#if not os.path.isfile('common/constants.py'):
#    print "No import module found in the PYTHONPATH. Please, add current directory to your PYTHON_PATH"
#    exit(1)

from common.constants import KALDI_ERR_CODES as ked
from common.constants import print_error as print_error
from utilities.utilities import KaldiIntegerVector
from asr_model.asr_model import ASR_model
from fst.fst import KaldiFST
from tree.tree import ContextDependency
from decoder.training_graph_compiler import TrainingGraphCompiler
from feat.feat import KaldiFeatureReader, KaldiMatrix, get_delta_features
from transform.transform import cmvn_transform

ffi = None
kaldi_lib = None

LIB_PATH = 'libpython-kaldi-segmentation.so'

def initialize_cffi():
    src = """
    typedef struct 
    {
        int number_of_phones;
        int *phones;
        int *num_repeats_per_phone;
    } Alignment;  
    void* GetTextFstCompiler(void *i_context_tree
                       , int *i_disambiguation_symbols
                       , int i_size_of_disambiguation_symbols
                       , void *i_transition_model
                       , char *i_lex_in_filename
                       , int *o_err_code);
    void DeleteTextFstCompiler(void *o_compiler);

    /*Fst for the phrase*/
    void *GetAlignerFst(void *i_text_fst_compiler
                  , int *i_transcript
                  , int i_transcript_len
                  , int *o_err_code);
    void DeleteAlignerFst(void *o_aligner_fst);
    /* Main function*/
    Alignment *Align(void *i_features
               , void *i_transition_model
               , void *i_acoustic_model
               , void *i_aligner_fst
               , float i_acoustic_scale
               , float i_transition_scale
               , float i_self_loop_scale
               , float i_beam
               , float i_retry_beam
               , bool i_careful
               , int *o_err_code);

    void DeleteAlignment(Alignment *o_alignment_buffer);
    """
    global ffi
    global kaldi_lib
    ffi = cffi.FFI()
    ffi.cdef(src)
    try:
        kaldi_lib = ffi.dlopen(LIB_PATH)
    except ImportError:
        print "Library {} is not found in the LD_LIBRARY_PATH. Please, add ./lib to your LD_LIBRARY_PATH".format(LIB_PATH)
        exit(1)


class PhraseMatcher(object):
    def __init__(self, text_fst_compiler, text_codes):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._text_codes = self._ffi.new("int[]", text_codes)
        self._ptr_last_err_code = self._ffi.new("int *")
        self._aligner = self._kaldi_lib.GetAlignerFst(text_fst_compiler.handler, self._text_codes, len(text_codes), self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError("Error creating aligner")

    @property
    def handler(self):
        return self._aligner

    @property
    def text(self):
        return self._text

    def __del__(self):
        self._kaldi_lib.DeleteAligner(self._aligner)


class WavAligner(object):
    def __init__(self, text_fst_compiler, transition_model, acoustic_model, char_codes):
        self._fst_compiler = text_fst_compiler
        self._transition_model = transition_model
        self._acoustic_model = acoustic_model
        self._phrases = {}
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._char_codes = char_codes
        self._ptr_last_err_code = self._ffi.new("int *")

    def add_phrase(self, text, transcription):
        self._phrases[text] = PhraseMatcher(self._fst_compiler, transcription)

    def align(self, mfcc_features, text, acoustic_scale=1.0, transition_scale=1.0, self_loop_scale=1.0, beam=10, retry_beam=40):
        alignment = self._kaldi_lib.Align(mfcc_features.handler, self._transition_model.handler, self._acoustic_model.handler, self._phrases[text].handler, acoustic_scale, transition_scale, self_loop_scale, beam, retry_beam, False, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError("Error while aligning")
        return alignment

    @property
    def n_phrases(self):
        return len(self._phrases)


class Transcriber(object):
    def __init__(self, word_table, unknown_word_code, phone_table, inv_phone_table, encoding='default'):
        self._word_table = word_table
        assert(unknown_word_code in self._word_table)
        self._unk_code = self._word_table[unknown_word_code]
        self._code_to_char = phone_table
        self._char_to_int = inv_phone_table
        self._encoding = encoding

    def transcribe(self, phrase):
        if self._encoding != 'default':
            phrase = phrase.encode(self._encoding)
        return [self._word_table[word] if word in self._word_table else self._unk_code for word in phrase.split(' ') ]


def load_phones_table(path_to_phones_table):
    phone_table = []
    inv_phone_table = {}
    for line in open(path_to_phones_table, 'r'):
        phone, phone_id = unicode(line.strip()).split(' ')
        phone_table.append(phone)
        inv_phone_table[phone] = phone_id
    if len(phone_table) == 0:
        raise RuntimeError('Empty phone table read from file {}'.format(path_to_phones_table))
    return phone_table, inv_phone_table


def load_word_table(path_to_word_table):
    word_table = {}
    for line in open(path_to_word_table, 'r'):
        word_code = line.split(' ')
        word = word_code[0]
        code = int(word_code[1])
        word_table[word] = code
    return word_table


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/final.mdl'
    FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
    FILE_CODE = "TRAIN-FCT002-002B0181"
    CMVN_STATS_PATH = "mfcc/cmvn_train.ark"
    CMVN_CODE = "FCT002"
    PATH_FO_FEATURE_ARCHIVE = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
    PATH_TO_CMVN_ARCHIVE = KALDI_PATH + RUSPEECH_EXP_PATH + CMVN_STATS_PATH
    PATH_TO_LEXICAL_FST = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/L.fst'
    PATH_TO_CONTEXT_TREE = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/tree'
    PATH_TO_DISAMBIGUATION_SYMBOLS = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones/disambig.int'
    PATH_TO_PHONES_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones.txt'
    PATH_TO_WORD_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/words.txt'
    UNK_CODE = '<UNK>'
    TEST_PHRASE = u'уха обычно готовится из пресноводных рыб'

    #ASR model
    asr_model = ASR_model(PATH_TO_MODEL)
    # asr_model.boost_silence(1.0)

    #Graph Compiler
    fst = KaldiFST(PATH_TO_LEXICAL_FST)
    context_dependency = ContextDependency(PATH_TO_CONTEXT_TREE)
    disambiguation_symbols = KaldiIntegerVector()
    disambiguation_symbols.load(PATH_TO_DISAMBIGUATION_SYMBOLS)
    graph_compiler = TrainingGraphCompiler(asr_model, context_dependency, fst, disambiguation_symbols)

    #Reading MFCC features
    feature_matrix_reader = KaldiFeatureReader()
    feature_matrix_reader.open_archive(PATH_FO_FEATURE_ARCHIVE, np.float32)
    mfcc_feature_matrix = feature_matrix_reader.get_matrix(FILE_CODE)

    #CMVN transform
    feature_matrix_reader.open_archive(PATH_TO_CMVN_ARCHIVE, np.float64)
    cmvn_matrix = feature_matrix_reader.get_matrix(CMVN_CODE)
    cmvn_transform(cmvn_matrix, mfcc_feature_matrix, True)

    #Delta + delta delta features
    delta_feature_matrix = get_delta_features(mfcc_feature_matrix, 2, 2)

    aligner = SpeechToTextAligner(asr_model, transition_scale=1.0, acoustic_scale=1.0, self_loop_scale=0.1, beam=10, retry_beam=40, careful=False)
    phrase_fst = compile_fst(TEST_PHRASE)
    alignment = aligner.get_alignment(delta_feature_matrix, phrase_fst)

