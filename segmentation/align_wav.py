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

ffi = None
kaldi_lib = None

KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
LIB_PATH = 'libpython-kaldi-segmentation.so'
PATH_TO_TRANSITION_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/final.mdl'
PATH_TO_PHONES_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones.txt'

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
            self.handler = vec_handler
            self.n_elements = n_elements
        elif filename is not None:
            self.handler, self.n_elements = kaldi_reader.read_integer_vector(filename)
        else:
            raise RuntimeError("Constructor arguments are missing")

    def __del__(self):
        kaldi_reader.delete_integer_vector(self.handler)


class ContextTree(object):
    def __init__(self, ctx_handler=None, filename=None):
        if ctx_handler is not None:
            self.handler = ctx_handler
        elif filename is not None:
            self.handler = kaldi_reader.read_context_tree(filename)
        else:
            raise RuntimeError("Constructor arguments are missing")

    def __del__(self):
        kaldi_reader.delete_context_tree(self.handler)

class TransitionModel(object):
    def __init__(selfi, model_handler=None, filename=None):
        if model_handler is not None:
            self.handler = model_handler
        elif filename is not None:
            self.handler = kaldi_reader.read_transition_model(filename)
        else:
            raise RuntimeError("Constructor arguments are missing")

    def __del__(self):
        kaldi_reader.read_transition_model(filename)


class KaldiWavAligner(object):
    def __init__(self):
        self.kaldi_lib = kaldi_lib
        self.ffi = ffi
        self.kaldi_lib.GetAligner(path_to_context_tree, path_to_transition_model, path_to_lexicon_fst)


initialize_cffi()
context_tree = ContextTree(filename=PATH_TO_TRIPHONE_MODEL)
disambiguation_symbols = IntegerVector(filename=PATH_TO_DISAMBIGUATION_SYMBOLS) 
transition_model = TransitionModel(filename=PATH_TO_TRANSITION_MODEL)
text_fst_compiler = TextFstCompiler(PATH_TO_LEXICAL_FST, context_tree, disambiguation_symbols, transition_model)
