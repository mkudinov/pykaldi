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

LIB_PATH = 'libpython-kaldi-transform.so'

def initialize_cffi():
    src = """
    void CmvnTransform(void *i_cmvn_stats_matrix, void *io_feature_matrix, bool i_var_norm, int *o_err_code);
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


def cmvn_transform(cmvn_matrix, feature_matrix_to_transform, vars_norm):
    ptr_to_result = ffi.new("int *")
    kaldi_lib.CmvnTransform(cmvn_matrix.handle, feature_matrix_to_transform.handle, vars_norm, ptr_to_result)
    err_code = ptr_to_result[0]
    if err_code != ked.OK:
        print_error(err_code)
        raise RuntimeError("Error during CMVN transform")


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/final.mdl'
    asr_model = ASR_model(PATH_TO_MODEL)
    print asr_model
