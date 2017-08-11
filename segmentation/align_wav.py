#coding: utf-8
import cffi
import numpy as np
import os
import pdb
import sys

if not os.path.isfile('common/constants.py'):
    print "No import module found in the PYTHONPATH. Please, add current directory to your PYTHON_PATH"
    exit(1)

from common.constants import KALDI_ERR_CODES as ked
from common.constants import print_error as print_error
from readers.utilities import ReaderUtilities as kaldi_reader
from readers.utilities import KaldiFeatureReader

ffi = None
kaldi_lib = None

KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
LIB_PATH = 'libpython-kaldi-segmentation.so'
PATH_TO_TRANSITION_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/final.mdl'
PATH_TO_PHONES_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones.txt'
PATH_TO_TRIPHONE_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/tree'
PATH_TO_DISAMBIGUATION_SYMBOLS = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones/disambig.int'
PATH_TO_LEXICAL_FST = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/L.fst'
TEST_PHRASE_CODE = 'TRAIN-FCT002-002B0181'
TEST_PHRASE = u'уха обычно готовится из пресноводных рыб'
PATH_TO_PHONES_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones.txt'
PATH_TO_WORD_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/words.txt'
UNK_CODE = '<UNK>'

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


class IntegerVector(object):
    def __init__(self, vec_handler=None, n_elements=None, filename=None):
        if vec_handler is not None and n_elements is not None:
            self._handler = vec_handler
            self._n_elements = n_elements
        elif filename is not None:
            self._handler, self._n_elements = kaldi_reader.read_integer_vector(filename)
        else:
            raise RuntimeError("Constructor arguments are missing")

    def __del__(self):
        kaldi_reader.delete_integer_vector(self._handler)

    @property
    def handler(self):
        return self._handler

    @property
    def n_elements(self):
        return self._n_elements



class ContextTree(object):
    def __init__(self, ctx_handler=None, filename=None):
        if ctx_handler is not None:
            self._handler = ctx_handler
        elif filename is not None:
            self._handler = kaldi_reader.read_context_tree(filename)
        else:
            raise RuntimeError("Constructor arguments are missing")

    def __del__(self):
        kaldi_reader.delete_context_tree(self._handler)

    @property
    def handler(self):
        return self._handler


class TransitionModel(object):
    def __init__(self, model_handler=None, filename=None):
        if model_handler is not None:
            self._handler = model_handler
        elif filename is not None:
            self._handler = kaldi_reader.read_transition_model(filename)
        else:
            raise RuntimeError("Constructor arguments are missing")

    def __del__(self):
        kaldi_reader.delete_transition_model(self._handler)

    @property
    def handler(self):
        return self._handler


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


class TextFstCompiler(object):
    def __init__(self, path_to_lexicon_fst, context_tree, disambiguation_symbols, transition_model):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int *")
        self.handler = self._kaldi_lib.GetTextFstCompiler(context_tree.handler, disambiguation_symbols.handler, disambiguation_symbols.n_elements, transition_model.handler, path_to_lexicon_fst, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError("Error creating text fst compiler")

    def __del__(self):
         self._kaldi_lib.DeleteTextFstCompiler(self.handler)


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
    context_tree = ContextTree(filename=PATH_TO_TRIPHONE_MODEL)
    disambiguation_symbols = IntegerVector(filename=PATH_TO_DISAMBIGUATION_SYMBOLS) 
    transition_model = TransitionModel(filename=PATH_TO_TRANSITION_MODEL)
    text_fst_compiler = TextFstCompiler(PATH_TO_LEXICAL_FST, context_tree, disambiguation_symbols, transition_model)
    code_to_char, char_to_code = load_phones_table(PATH_TO_PHONES_TABLE)
    word_to_code = load_word_table(PATH_TO_WORD_TABLE)
    aligner = WavAligner(text_fst_compiler, transition_model, context_tree, char_to_code)
    transcriber = Transcriber(word_to_code, UNK_CODE, code_to_char, char_to_code, encoding='cp1251')
    aligner.add_phrase(TEST_PHRASE, transcriber.transcribe(TEST_PHRASE))
    path_to_feature_archive = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
    feature_matrix_reader = KaldiFeatureReader()
    feature_matrix_reader.open_archive(path_to_feature_archive)
    feature_matrix = feature_matrix_reader.get_feature_matrix(TEST_PHRASE_CODE)
    aligner.align(feature_matrix, TEST_PHRASE)
