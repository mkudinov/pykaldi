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
from fst.fst import KaldiFST, KaldiFstReader
from tree.tree import ContextDependency
from decoder.training_graph_compiler import TrainingGraphCompiler
from feat.feat import KaldiFeatureReader, KaldiMatrix, get_delta_features
from transform.transform import cmvn_transform
from transcriber.transcriber import Transcriber, load_word_table, load_phone_table

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

    /* AlignmentReader */
    void *GetAlignmentReader(char *i_specifier
                          ,  int *o_err_code);
    void DeleteAlignmentReader(void *o_alignment_reader);
    Alignment *ReadAlignment(char *i_key
                           , void *i_transition_model
                           , void *i_alignment_reader
                           , int *o_err_code);

    /* Main function*/
    Alignment *Align(void *i_features
                   , void *i_transition_model
                   , void *i_acoustic_model
                   , void *i_aligner_fst
                   , float *o_likelihood
                   , int   *o_n_retries
                   , int   *o_n_frames_ready
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


class KaldiAlignmentReader(object):
    def __init__(self, asr_model, phone_table):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int*")
        self._open = False
        self._result_buffer = None
        self._transition_model = asr_model.transition_model_handle
        self._phone_table = phone_table

    def __del__(self):
        if self._open:
            self.close_archive()

    def get_alignment(self, record_descriptor):
        if not self._open:
            return None
        self._result_buffer = self._kaldi_lib.ReadAlignment(record_descriptor, self._transition_model, self._alignment_reader, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to get alignment for descriptor {}'.format(record_descriptor))
        alignment_size = self._result_buffer.number_of_phones
        alignment = []
        for phone_num in range(alignment_size):
            phone_id = self._result_buffer.phones[phone_num]
            phone_length = self._result_buffer.num_repeats_per_phone[phone_num]
            alignment.append((phone_id, self._phone_table[phone_id], phone_length))
        self._kaldi_lib.DeleteAlignment(self._result_buffer)
        self._result_buffer = None
        return alignment

    def open_archive(self, path_to_archive, format=None):
        if self._open:
            self.close_archive()
        if not os.path.isfile(path_to_archive):
            raise RuntimeError('Error trying to open archive {}. No such file or directory'.format(path_to_archive))
        if format == 'gzip':
            specifier = "ark,s:gunzip -c {}|".format(path_to_archive)
        else:
            specifier = "ark:{}".format(path_to_archive)
        self._alignment_reader = self._kaldi_lib.GetAlignmentReader(specifier, self._ptr_last_err_code)
        self._open = True

    def close_archive(self):
        if self._open:
            self._kaldi_lib.DeleteAlignmentReader(self._alignment_reader)
        self._open = False


class SpeechToTextAligner(object):
    def __init__(self, asr_model, phone_table, transition_scale=1.0, acoustic_scale=1.0, self_loop_scale=0.1, beam=10,
                                  retry_beam=40, careful=False):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._asr_model = asr_model
        self._transition_scale = transition_scale
        self._acoustic_scale = acoustic_scale
        self._self_loop_scale = self_loop_scale
        self._beam = beam
        self._retry_beam = retry_beam
        self._careful = careful
        self._phone_table = phone_table

    def get_alignment(self, features, fst):
        ptr_last_err_code = self._ffi.new("int*")
        ptr_likelihood = self._ffi.new("float*")
        ptr_n_retries = self._ffi.new("int*")
        ptr_n_frames = self._ffi.new("int*")
        result_buffer = self._kaldi_lib.Align(features.handle, self._asr_model.transition_model_handle,
                                       self._asr_model.acoustic_model_handle, fst.handle,
                                       ptr_likelihood, ptr_n_retries, ptr_n_frames,
                                       self._acoustic_scale,
                                       self._transition_scale, self._self_loop_scale, self._beam, self._retry_beam,
                                       self._careful, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to make alignment')
        likelihood = ptr_likelihood[0]
        n_retries = ptr_n_retries[0]
        n_frames = ptr_n_frames[0]
        alignment_size = result_buffer.number_of_phones
        alignment = []
        for phone_num in range(alignment_size):
            phone_id = result_buffer.phones[phone_num]
            phone_length = result_buffer.num_repeats_per_phone[phone_num]
            alignment.append((phone_id, self._phone_table[phone_id], phone_length))
        self._kaldi_lib.DeleteAlignment(result_buffer)
        return  alignment, likelihood, n_retries, n_frames


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
    PATH_TO_WORD_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/words.txt'
    UNK_CODE = '<UNK>'
    TEST_PHRASE = u'уха обычно готовится из пресноводных рыб'
    ALIGNS_PATH = 'exp/tri1/ali.1.gz'
    PATH_TO_PHONES_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones.txt'

    #ASR model
    asr_model = ASR_model(PATH_TO_MODEL)
    # asr_model.boost_silence(1.0)

    #Aligner
    phone_table = load_phone_table(PATH_TO_PHONES_TABLE)
    aligner = SpeechToTextAligner(asr_model, phone_table, transition_scale=1.0, acoustic_scale=1.0,
                                  self_loop_scale=0.1, beam=10, retry_beam=40, careful=False)

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
    cmvn_transform(cmvn_matrix, mfcc_feature_matrix, False)

    #Delta + delta delta features
    delta_feature_matrix = get_delta_features(mfcc_feature_matrix, 2, 2)

    #Transcription
    word2int = load_word_table(PATH_TO_WORD_TABLE)
    transcriber = Transcriber(word2int, UNK_CODE, encoding='cp1251')
    transcription = transcriber.transcribe(TEST_PHRASE)

    #Phrase graph
    decoder_fst = graph_compiler.compile_phrase_graph(transcription)

    #align!
    alignment, likelihood, _, _ = aligner.get_alignment(delta_feature_matrix, decoder_fst)
    print alignment

    #read the same alignment from the archive
    alignment_reader = KaldiAlignmentReader(asr_model, phone_table)
    path_to_archive = KALDI_PATH + RUSPEECH_EXP_PATH + ALIGNS_PATH
    print path_to_archive
    alignment_reader.open_archive(path_to_archive, format='gzip')
    alignment = alignment_reader.get_alignment(FILE_CODE)
    print alignment

