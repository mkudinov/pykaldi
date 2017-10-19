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

ffi = None
kaldi_lib = None

LIB_PATH = 'libpython-kaldi-utilities.so'

def initialize_cffi():
    src = """
    /* IntegerVector*/
    void *InitIntegerVector(int *i_source
                         , int i_size
                         , int *o_err_code);
    int *ReadIntegerVector(char *i_specifier
                         , int *o_n_elements
                         , int *o_err_code);
    void DeleteIntegerVector(int *o_vector);
    void CopyIntegerVector(int *i_source
                         , int i_size
                         ,  int *o_destination
                         , int *o_err_code);
    /* Kaldi Matrix */
    void *initMatrixFloat(void *i_source
                        , int i_nRows
                        , int i_nColumns
                        , int *o_err_code);
    void *GetMatrixReader(char *i_specifier, char *i_data_type, int *o_err_code);
    void DeleteMatrixReader(void *o_matrix_reader, char *i_data_type);
    const void* ReadMatrix(char* i_key
                                 , void *i_matrix_reader
                                 , char* i_data_type
                                 , int* o_n_rows
                                 , int* o_n_columns
                                 , int *o_err_code);
    void DeleteMatrix(void *o_matrix
                           , char* i_data_type);
    void CopyMatrix(void *i_source
                  , void *o_destination
                  , char* i_data_type
                  , int *o_err_code);
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


class KaldiIntegerVector(object):
    def __init__(self, python_list=None):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_integer_vector = None
        self._n_elements = 0
        if python_list is not None:
            self._initialize_from_python_list(python_list)

    def load(self, read_specifier=None):
        ptr_last_err_code = self._ffi.new("int *")
        ptr_n_elements= self._ffi.new("int *")
        self._ptr_integer_vector = self._kaldi_lib.ReadIntegerVector(read_specifier, ptr_n_elements, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Error trying to load vector')
        self._n_elements = ptr_n_elements[0]

    def __del__(self):
        if self._ptr_integer_vector is not None:
            self._kaldi_lib.DeleteIntegerVector(self._ptr_integer_vector)

    def _initialize_from_python_list(self, python_list):
        if  len(python_list) == 0:
            raise RuntimeError("ERROR: kaldi vectors of zero length are not supported")
        if not isinstance(python_list[0], int):
            raise RuntimeError("ERROR: python_list algument must be a vector of integers. type %s is given." % type(python_list[0]))
        ptr_last_err_code = self._ffi.new("int *")
        self._n_elements = len(python_list)
        ffi_python_list = self._ffi.new("int[]", python_list)
        self._ptr_integer_vector = self._kaldi_lib.InitIntegerVector(ffi_python_list, self._n_elements, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Error trying to init vector')

    def numpy_array(self):
        numpy_vector = np.zeros([1, self._n_elements], dtype=np.int32)
        ptr_last_err_code = self._ffi.new("int *")
        self._kaldi_lib.CopyIntegerVector(self._ptr_integer_vector,
                                          self._n_elements,
                                          self._ffi.cast('int *', numpy_vector.__array_interface__['data'][0]),
                                          ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Error trying to copy vector')
        return numpy_vector[0]

    @property
    def handle(self):
        return self._ptr_integer_vector

    def __len__(self):
        return self._n_elements


class KaldiMatrix(object):
    def __init__(self, ptr_to_matrix, shape, dtype):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_to_matrix = ptr_to_matrix
        self.shape = shape
        self.valid = True
        self._ptr_last_err_code = self._ffi.new("int *")
        self._dtype = np.dtype(dtype).name

    def __del__(self):
        self._kaldi_lib.DeleteMatrix(self._ptr_to_matrix, self._dtype)

    @property
    def handle(self):
        return self._ptr_to_matrix

    @property
    def dtype(self):
        return np.dtype(self._dtype)

    def numpy_array(self):
        if not self.valid:
            raise RuntimeError("Matrix proxy for object {} is not longer valid!".format(self._ptr_to_matrix[0]))
        numpy_matrix = np.zeros(self.shape, dtype=np.dtype(self._dtype))
        self._kaldi_lib.CopyMatrix(self._ptr_to_matrix,
                                          self._ffi.cast('void *', numpy_matrix.__array_interface__['data'][0]),
                                          self._dtype, self._ptr_last_err_code)
        return numpy_matrix


class KaldiMatrixReader(object):
    def __init__(self):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int *")
        self._ptr_return_by_reference1 = self._ffi.new("int *")
        self._ptr_return_by_reference2 = self._ffi.new("int *")
        self._open = False
        self._dtype = None

    def __del__(self):
        if self._open:
            self.close_archive()

    def get_matrix(self, record_descriptor):
        result_ptr = self._kaldi_lib.ReadMatrix(record_descriptor, self._matrix_reader, np.dtype(self._dtype).name, self._ptr_return_by_reference1, self._ptr_return_by_reference2, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to get matrix for descriptor {}'.format(record_descriptor))
        num_rows = self._ptr_return_by_reference1[0]
        num_columns = self._ptr_return_by_reference2[0]
        matrix = KaldiMatrix(result_ptr, [num_rows, num_columns], dtype=np.dtype(self._dtype))
        return matrix

    def open_archive(self, path_to_archive, dtype):
        if self._open:
            self.close_archive()
        self._dtype = np.dtype(dtype).name
        if not os.path.isfile(path_to_archive):
            raise RuntimeError('Error trying to open archive {}. No such file or directory'.format(path_to_archive))
        specifier = "ark:{}".format(path_to_archive)
        self._matrix_reader = self._kaldi_lib.GetMatrixReader(specifier, self._dtype, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to open archive {}'.format(path_to_archive))
        self._open = True

    def close_archive(self):
        if self._open:
            self._kaldi_lib.DeleteMatrixReader(self._matrix_reader, self._dtype)
        self._open = False
        self._dtype = None

    @property
    def dtype(self):
        return np.dtype(self._dtype)


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_DISAMBIGUATION_SYMBOLS = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones/disambig.int'
    disambiguation_symbols = KaldiIntegerVector()
    disambiguation_symbols.load(PATH_TO_DISAMBIGUATION_SYMBOLS)
    print "Disamb.symbols: ", len(disambiguation_symbols)
    print disambiguation_symbols.numpy_array()
    test_vector = KaldiIntegerVector(range(10))
    print test_vector.numpy_array()
    FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
    FILE_CODE = "TRAIN-FCT002-002B0181"
    feature_matrix_reader = KaldiMatrixReader()
    path_to_feature_archive = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
    feature_matrix_reader.open_archive(path_to_feature_archive, np.float32)
    feature_matrix = feature_matrix_reader.get_matrix(FILE_CODE)
    print feature_matrix.numpy_array()