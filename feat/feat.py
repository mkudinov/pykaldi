import cffi
import numpy as np
import os
import pdb
import scipy.io.wavfile

# if not os.path.isfile('common/constants.py'):
#     print "No import module found in the PYTHONPATH. Please, add current directory to your PYTHON_PATH"
#     exit(1)

from common.constants import KALDI_ERR_CODES as ked
from common.constants import print_error as print_error
from utilities.utilities import KaldiMatrix, KaldiMatrixReader

ffi = None
kaldi_lib = None

LIB_PATH = 'libpython-kaldi-feat.so'

def initialize_cffi():
    src = """
    void *GetMatrixOfDeltaFeatures(void *i_feature_matrix
                                   , int i_order
                                   , int i_window
                                   , int* o_n_rows
                                   , int* o_n_columns
                                   , int *o_err_code);
    void *GetWavData(void *i_matrix_wav_data
                   , int i_sampling_rate
                   , int *o_err_code);
    void DeleteWavData(void *o_matrix_wav_data);
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


class KaldiWavData(object):
    def __init__(self, wav_data, sampling_rate):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._sampling_rate = sampling_rate
        if len(wav_data.shape) > 1:
            self._n_channels = wav_data.shape[0]
            self._n_samples = wav_data.shape[1]
        else:
            self._n_channels = 1
            self._n_samples = wav_data.shape[0]
        self._kaldi_data_matrix = self._initialize_kaldi_object(wav_data)

    def _initialize_kaldi_object(self, wav_data):
        if len(wav_data) == 0:
            raise RuntimeError("ERROR: wav file is empty")
        if not isinstance(wav_data[0], int):
            raise RuntimeError("ERROR: wav data must be a list of integers. type %s is given." % type(wav_data[0]))
        ptr_last_err_code = self._ffi.new("int *")
        wav_data_float = np.array(wav_data, dtype=np.float32)
        kaldi_data_matrix = KaldiMatrix(wav_data, numpy_matrix=wav_data_float)
        self._ptr_wav_data = self._kaldi_lib.GetWavData(kaldi_data_matrix.handle, self._sampling_rate, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Error trying to create object WaveData')

    def __del__(self):
        self._kaldi_lib.DeleteWavData(self._ptr_wav_data)

    @property
    def handle(self):
        return self._ptr_wav_data


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
    return KaldiMatrix(ptr_delta_matrix, [num_rows, num_columns], np.float32)


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    FEATURE_PATH = 'mfcc/raw_mfcc_train.1.ark'
    FILE_CODE = "TRAIN-FCT002-002B0181"
    feature_matrix_reader = KaldiMatrixReader()
    path_to_feature_archive = KALDI_PATH + RUSPEECH_EXP_PATH + FEATURE_PATH
    feature_matrix_reader.open_archive(path_to_feature_archive, np.float32)
    feature_matrix = feature_matrix_reader.get_matrix(FILE_CODE)
    print feature_matrix.numpy_array()
    delta_matrix = get_delta_features(feature_matrix, 2, 2)
    print delta_matrix.numpy_array()



