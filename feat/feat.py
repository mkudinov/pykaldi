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

LIB_PATH = 'libpython-kaldi-feat.so'

def initialize_cffi():
    src = """
    void *GetFeatureReader(char *i_specifier, int *o_err_code);

    void DeleteFeatureReader(void *o_feature_reader);

    const void* ReadFeatureMatrix(char* i_key, void *i_feature_reader, int* o_n_rows, int* o_n_columns, int *o_err_code);

    void DeleteFeatureMatrix(void *o_feature_matrix);

    void CopyFeatureMatrix(void *i_source, void *o_destination, int *o_err_code);

    void *GetMatrixOfDeltaFeatures(void *i_feature_matrix, int i_order, int i_window, int *o_err_code);
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


class KaldiMatrix(object):
    def __init__(self, ptr_to_matrix, shape):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_to_matrix = ptr_to_matrix
        self.shape = shape
        self.valid = True
        self._ptr_last_err_code = self._ffi.new("int *")

    def __del__(self):
        self._kaldi_lib.DeleteFeatureMatrix(self._ptr_to_matrix)

    @property
    def handle(self):
        return self._ptr_to_matrix

    def numpy_array(self):
        if not self.valid:
            raise RuntimeError("Matrix proxy for object {} is not longer valid!".format(self._ptr_to_matrix[0]))
        numpy_matrix = np.zeros(self.shape, dtype=np.float32)
        self._kaldi_lib.CopyFeatureMatrix(self._ptr_to_matrix, self._ffi.cast('void *', numpy_matrix.__array_interface__['data'][0]), self._ptr_last_err_code)
        return numpy_matrix


class KaldiFeatureReader(object):
    def __init__(self):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int *")
        self._ptr_return_by_reference1 = self._ffi.new("int *")
        self._ptr_return_by_reference2 = self._ffi.new("int *")
        self._open = False
        self._kaldi_matrices = []

    def __del__(self):
        if self._open:
            self.close_archive()

    def get_feature_matrix(self, record_descriptor):
        result_ptr = self._kaldi_lib.ReadFeatureMatrix(record_descriptor, self._feature_reader, self._ptr_return_by_reference1, self._ptr_return_by_reference2, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to get feature matrix for descriptor {}'.format(record_descriptor))
        num_rows = self._ptr_return_by_reference1[0]
        num_columns = self._ptr_return_by_reference2[0]
        feature_matrix = KaldiMatrix(result_ptr, [num_rows, num_columns])
        self._kaldi_matrices.append(feature_matrix)
        return feature_matrix

    def open_archive(self, path_to_archive):
        if self._open:
            self.close_archive()
        if not os.path.isfile(path_to_archive):
            raise RuntimeError('Error trying to open archive {}. No such file or directory'.format(path_to_archive))
        specifier = "ark:{}".format(path_to_archive)
        self._feature_reader = self._kaldi_lib.GetFeatureReader(specifier, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to open archive {}'.format(path_to_archive))
        self._open = True

    def close_archive(self):
        if self._open:
            self._kaldi_lib.DeleteFeatureReader(self._feature_reader)
        self._open = False


def get_delta_features(feature_matrix, order, window):
    ptr_to_result = ffi.new("int *")
    ptr_return_by_reference1 = ffi.new("int *")
    ptr_return_by_reference2 = ffi.new("int *")
    ptr_delta_matrix = kaldi_lib.GetMatrixOfDeltaFeatures(feature_matrix.handle, order, window,
                                                          ptr_return_by_reference1, ptr_return_by_reference2,
                                                          ptr_to_result)
    err_code = ptr_to_result[0]
    if err_code != ked.OK:
        print_error(err_code)
        raise RuntimeError('Error trying to compute delta features')
    num_rows = ptr_return_by_reference1[0]
    num_columns = ptr_return_by_reference2[0]
    return KaldiMatrix(ptr_delta_matrix, [num_rows, num_columns])


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
    FILE_CODE = "TRAIN-FCT002-002B0181"
    feature_matrix_reader = KaldiFeatureReader()
    path_to_feature_archive = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
    feature_matrix_reader.open_archive(path_to_feature_archive)
    feature_matrix = feature_matrix_reader.get_feature_matrix(FILE_CODE)
    print feature_matrix.numpy_array()

