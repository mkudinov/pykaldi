import cffi
import numpy as np
import os
import pdb
import sys

# if not os.path.isfile('common/constants.py'):
#     print "No import module found in the PYTHONPATH. Please, add current directory to your PYTHON_PATH"
#     exit(1)

from common.constants import KALDI_ERR_CODES as ked
from common.constants import print_error as print_error
import feat.feat as features

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
    FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
    FILE_CODE = "TRAIN-FCT002-002B0181"
    CMVN_STATS_PATH = "mfcc/cmvn_train.ark"
    CMVN_CODE = "FCT002"
    feature_matrix_reader = features.KaldiFeatureReader()
    path_to_feature_archive = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
    path_to_cmvn_archive = KALDI_PATH + RUSPEECH_EXP_PATH + CMVN_STATS_PATH
    feature_matrix_reader.open_archive(path_to_feature_archive, np.float32)
    feature_matrix = feature_matrix_reader.get_matrix(FILE_CODE)
    feature_matrix_reader.open_archive(path_to_cmvn_archive, np.float64)
    cmvn_matrix = feature_matrix_reader.get_matrix(CMVN_CODE)
    print feature_matrix.numpy_array()
    print ""
    print cmvn_matrix.numpy_array()
    print ""
    cmvn_transform(cmvn_matrix, feature_matrix, True)
    print feature_matrix.numpy_array()
