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

KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
ALIGNS_PATH = 'exp/tri1/ali.1.gz'
LIB_PATH = 'libpython-kaldi-utilities.so'
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
    /* AlignmentReader */
    void *GetAlignmentReader(char *i_specifier
                      ,  int *o_err_code);
    void DeleteAlignmentReader(void *o_alignment_reader);
    Alignment *ReadAlignment(char *i_key
                       , void *i_transition_model
                       , void *i_alignment_reader
                       , int *o_err_code);

    /* IntegerVector*/
    int *ReadIntegerVector(char *i_specifier
                     , int *o_n_elements
                     , int *o_err_code);
    void DeleteIntegerVector(int *o_vector);
    void CopyIntegerVector(int *i_source
                     , int i_size
                     , void *o_destination
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
    def __init__(self, path_to_transition_model, path_to_phones_table):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int*")
        self._open = False
        self._result_buffer = None 
        self._transition_model = self._kaldi_lib.GetTransitionModel(path_to_transition_model, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
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


class KaldiIntegerVector(object):
    def __init__(self):
        self._ptr_integer_vector = None
        self._n_elements = 0

    def load(self, read_specifier=None):
        ptr_last_err_code = ffi.new("int *")
        ptr_n_elements=ffi.new("int *")
        self._ptr_integer_vector = kaldi_lib.ReadIntegerVector(read_specifier, ptr_n_elements, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Error trying to load vector')
        self._n_elements = ptr_n_elements[0]

    def __del__(self):
        if self._ptr_integer_vector is not None:
            kaldi_lib.DeleteIntegerVector(self._ptr_integer_vector)

    @property
    def handle(self):
        return self._ptr_integer_vector

    def __len__(self):
        return self._n_elements


initialize_cffi()
if __name__ == '__main__':
    alignment_reader = KaldiAlignmentReader(PATH_TO_TRANSITION_MODEL, PATH_TO_PHONES_TABLE)
    path_to_archive = KALDI_PATH + RUSPEECH_EXP_PATH + ALIGNS_PATH
    alignment_reader.open_archive(path_to_archive)
    alignment = alignment_reader.get_alignment("TRAIN-FCT018-018R0070")
    print alignment