import cffi
import numpy as np
import os
import pdb

ffi = None
kaldi_lib = None

#enum
OK,\
ERROR_OPENING, \
NO_KEY, \
INDEX_OUT_OF_BOUNDS, \
NO_READER_EXISTS, \
COPY_ERROR, \
MEMORY_ALLOCATION_ERROR, \
INTERNAL_KALDI_ERROR = \
range(8)

#int error code to string error code
string_error_codes = ["OK","ERROR_OPENING", "NO_KEY", "INDEX_OUT_OF_BOUNDS", "NO_READER_EXISTS", "COPY_ERROR", "MEMORY_ALLOCATION_ERROR", "INTERNAL_KALDI_ERROR"]

def print_error(error_message):
    print "ERROR: code {}".format(string_error_codes[error_message])

KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
ALIGNS_PATH = 'exp/tri1/ali.1.gz'
FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
LIB_PATH = KALDI_PATH + 'src/python/libpython-kaldi-data-read.so'
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
    /* INTERFACE FUNCTION */
    void* GetAlignmentReader(char *i_specifier,  int *o_err_code);
    void* GetTransitionModel(char *i_transition_model_filename, int *o_err_code);
    size_t ReadAlignment(char *i_key, void *i_transition_model, void* i_alignment_reader, Alignment *o_alignment_buffer, int *o_err_code);
    void* GetFeatureReader(char *i_specifier, int *o_err_code);
    void* ReadFeatureMatrix(char *i_key, void *i_feature_reader, int *o_n_rows, int *o_n_columns, int *o_err_code);
    /* TRANSFER TO PYTHON */
    Alignment *GetResultBuffer();
    void CopyFeatureMatrix(void *i_source, void *o_destination, int *o_err_code);
    /* CLEAR MEMORY */
    void DeleteTransitionModel(void *o_transition_model);
    void DeleteAlignmentReader(void *o_alignment_reader);
    void DeleteResultBuffer(Alignment *o_alignment_buffer);
    void DeleteFeatureReader(void *o_feature_reader);
    """
    global ffi
    global kaldi_lib
    ffi = cffi.FFI()
    ffi.cdef(src)
    kaldi_lib = ffi.dlopen(LIB_PATH)


class KaldiAlignmentReader(object):
    def __init__(self, path_to_transition_model, path_to_phones_table):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int*")
        self._open = False
        self._result_buffer = self._kaldi_lib.GetResultBuffer()
        self._transition_model = self._kaldi_lib.GetTransitionModel(path_to_transition_model, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != OK:
            print_error(err_code)
            raise RuntimeError('Error trying to read transition model from file {}'.format(path_to_transition_model))
        self._phone_table = []
        for line in open(path_to_phones_table, 'r'):
            phone, phone_id = line.split(' ')
            self._phone_table.append(phone)
        if len(self._phone_table) == 0:
            raise RuntimeError('Empty phone table read from file {}'.format(path_to_phones_table))

    def __del__(self):
        if self._open:
            self.close_archive()
        self._kaldi_lib.DeleteTransitionModel(self._transition_model)
        self._kaldi_lib.DeleteResultBuffer(self._result_buffer)

    def get_alignment(self, record_descriptor):
        if not self._open:
            return None
        alignment_size = self._kaldi_lib.ReadAlignment(record_descriptor, self._transition_model, self._alignment_reader, self._result_buffer, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != OK:
            print_error(err_code)
            raise RuntimeError('Error trying to get alignment for descriptor {}'.format(record_descriptor))
        alignment = []
        for phone_num in range(alignment_size):
            phone_id = self._result_buffer.phones[phone_num] 
            phone_length = self._result_buffer.num_repeats_per_phone[phone_num]
            alignment.append((phone_id, self._phone_table[phone_id], phone_length))
        return alignment

    def open_archive(self, path_to_archive):
        if self._open:
            self.close_archive()
        if not os.path.isfile(path_to_archive):
            raise RuntimeError('Error trying to open archive {}. No such file or directory'.format(path_to_archive))
        specifier = "ark,s:gunzip -c {}|".format(path_to_archive)
        self._alignment_reader = self._kaldi_lib.GetAlignmentReader(specifier, self._ptr_last_err_code)
        self._open = True

    def close_archive(self):
        if self._open:
            self._kaldi_lib.DeleteAlignmentReader(self._alignment_reader)
        self._open = False


class KaldiMatrixProxy(object):
    def __init__(self, ptr_to_matrix, shape):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_to_matrix = ptr_to_matrix
        self.shape = shape
        self.valid = True
        print shape
        self._ptr_last_err_code = self._ffi.new("int *")

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
        if err_code != OK:
            print_error(err_code)
            raise RuntimeError('Error trying to get feature matrix for descriptor {}'.format(record_descriptor))
        num_rows = self._ptr_return_by_reference1[0]
        num_columns = self._ptr_return_by_reference2[0]
        feature_matrix = KaldiMatrixProxy(result_ptr, [num_rows, num_columns])  
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
        if err_code != OK:
            print_error(err_code)
            raise RuntimeError('Error trying to open archive {}'.format(path_to_archive))
        self._open = True

    def close_archive(self):
        if self._open:
            self._destroy_all_matrices()
            self._kaldi_lib.DeleteFeatureReader(self._feature_reader)
        self._open = False

    def _destroy_all_matrices(self):
        for matrix in self._kaldi_matrices:
            matrix.valid = False
        self._kaldi_matrices = []


initialize_cffi()
alignment_reader = KaldiAlignmentReader(PATH_TO_TRANSITION_MODEL, PATH_TO_PHONES_TABLE)
path_to_archive = KALDI_PATH + RUSPEECH_EXP_PATH + ALIGNS_PATH
alignment_reader.open_archive(path_to_archive)
alignment = alignment_reader.get_alignment("TRAIN-FCT018-018R0070")
print alignment
feature_matrix_reader = KaldiFeatureReader()
path_to_feature_archive = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
feature_matrix_reader.open_archive(path_to_feature_archive)
feature_matrix = feature_matrix_reader.get_feature_matrix("TRAIN-FCT002-002B0181")
print feature_matrix.numpy_array()
