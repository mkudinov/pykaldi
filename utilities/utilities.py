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

KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
ALIGNS_PATH = 'exp/tri1/ali.1.gz'
LIB_PATH = 'libpython-kaldi-utilities.so'
PATH_TO_TRANSITION_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/final.mdl'
PATH_TO_PHONES_TABLE = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones.txt'

def initialize_cffi():
    src = """


    /* IntegerVector*/
    int *ReadIntegerVector(char *i_specifier
                         , int *o_n_elements
                         , int *o_err_code);
    void DeleteIntegerVector(int *o_vector);
    void CopyIntegerVector(int *i_source
                         , int i_size
                         ,  int *o_destination
                         , int *o_err_code);
    void *InitIntegerVector(int *i_source
                         , int i_size
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


initialize_cffi()
if __name__ == '__main__':
    #alignment_reader = KaldiAlignmentReader(PATH_TO_TRANSITION_MODEL, PATH_TO_PHONES_TABLE)
    #path_to_archive = KALDI_PATH + RUSPEECH_EXP_PATH + ALIGNS_PATH
    #alignment_reader.open_archive(path_to_archive)
    #alignment = alignment_reader.get_alignment("TRAIN-FCT018-018R0070")
    #print alignment
    PATH_TO_DISAMBIGUATION_SYMBOLS = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/phones/disambig.int'
    disambiguation_symbols = KaldiIntegerVector()
    disambiguation_symbols.load(PATH_TO_DISAMBIGUATION_SYMBOLS)
    print "Disamb.symbols: ", len(disambiguation_symbols)
    print disambiguation_symbols.numpy_array()
    test_vector = KaldiIntegerVector(range(10))
    print test_vector.numpy_array()