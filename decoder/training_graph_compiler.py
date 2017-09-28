import cffi
import numpy as np
import os
import pdb
import sys

if not os.path.isfile('common/constants.py'):
    print "No import module found in the PYTHONPATH. Please, add current directory to your PYTHON_PATH"
    exit(1)

from common.constants import KALDI_ERR_CODES as ked
from fst.fst import KaldiFst
from common.constants import print_error as print_error

ffi = None
kaldi_lib = None

LIB_PATH = 'libpython-kaldi-decoder.so'

def initialize_cffi():
    src = """
    void *GetTrainingGraphCompiler( void   *i_transition_model
                                  , void   *i_context_dependency
                                  , void   *io_lexical_fst
                                  , int    *i_disambiguation_symbols
                                  , int     i_disambiguation_symbols_length
                                  , double  i_option_transition_scale
                                  , double  i_option_self_loop_scale
                                  , bool    i_option_reorder
                                  , int    *o_err_code
                                  );

    void *CompilePhraseGraphFromText( void *i_graph_compiler
                                    , int  *i_transcript
                                    , int   i_transcript_length
                                    , int  *o_err_code
                                    );

    void DeleteTrainingGraphCompiler(void *o_graph_compiler);
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


class TrainingGraphCompiler(object):
    def __init__(self, asr_model,
                       context_dependency,
                       lexical_fst,
                       disambiguation_symbols,
                       transition_scale=1.0,
                       self_loop_scale=1.0,
                       reorder=True):
        self.kaldi_lib = kaldi_lib
        ptr_last_err_code = ffi.new("int *")
        self._ptr_graph_compiler = self.kaldi_lib.GetTrainingGraphCompiler(asr_model.transition_model_handle,
                                                context_dependency.handle,
                                                lexical_fst.handle,
                                                disambiguation_symbols.handle,
                                                len(disambiguation_symbols),
                                                transition_scale,
                                                self_loop_scale,
                                                reorder)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Graph compiler creation failed')
        lexical_fst._ptr_fst = 0

    def compile_phrase_graph(self, transcript):
        ptr_last_err_code = ffi.new("int *")
        decoder_fst_ptr = kaldi_lib.CompilePhraseGraphFromText(self._ptr_graph_compiler, transcript.handle, len(transcript),
                                                             ptr_last_err_code)
        decoder_fst = KaldiFST(fst_handle=decoder_fst_ptr)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Graph compilation failed')
        return decoder_fst


    def __del__(self):
        self._ptr_fst = self.kaldi_lib.DeleteTrainingGraphCompiler(self._ptr_graph_compiler)

    @property
    def handle(self):
        return self._ptr_graph_compiler


initialize_cffi()
