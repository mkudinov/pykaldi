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

ffi = None
kaldi_lib = None

LIB_PATH = 'libpython-kaldi-fst.so'

def initialize_cffi():
    src = """
        void *GetFst(char *i_path_to_fst, int *o_err_code);
        int GetNumberOfArcs(void *i_fst, int *o_err_code);
        void DeleteFst(void *o_fst);
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


class KaldiFST(object):
    def __init__(self, path_to_fst_file):
        self.kaldi_lib = kaldi_lib
        if path_to_fst_file is None:
            raise RuntimeError('Only reading from file is currently supported!')
        ptr_last_err_code = ffi.new("int *")
        self._ptr_fst = self.kaldi_lib.GetFst(path_to_fst_file, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('FST reading failed')
        self.n_arcs = self.kaldi_lib.GetNumberOfArcs(self._ptr_fst, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('FST reference crashed')

    def __del__(self):
        self._ptr_fst = self.kaldi_lib.DeleteFst(self._ptr_fst)

    def __str__(self):
        return "FST: %s arcs" % (self.n_arcs)


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_LEXICAL_FST = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/L.fst'
    fst = KaldiFST(PATH_TO_LEXICAL_FST)
    print fst
