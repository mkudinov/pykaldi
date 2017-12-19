import cffi
import numpy as np
import os
import pdb
import scipy.io.wavfile as wavfile

# if not os.path.isfile('common/constants.py'):
#     print "No import module found in the PYTHONPATH. Please, add current directory to your PYTHON_PATH"
#     exit(1)

from common.constants import KALDI_ERR_CODES as ked
from common.constants import print_error as print_error
from utilities.utilities import KaldiMatrix, KaldiMatrixReader

CH_MONO = -1
NO_LIMIT = 0.0

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

    void *GetWavData(void *io_matrix_wav_data
                   , int i_sampling_rate
                   , int *o_err_code);

    void DeleteWavData(void *o_matrix_wav_data);

    void *ComputeMfcc(void *i_waveform
                    , int i_channel
                    , bool i_subtract_mean
                    , float i_vtln_warp_factor
                    , void *i_mfcc_computer
                    , int *o_n_features
                    , int *o_n_frames
                    , int *o_err_code);

    void *GetMfccComputer(float i_frame_length_ms
                        , float i_frame_shift_ms
                        , float i_sampling_rate

                        , float i_dither_scale
                        , float i_preemph_coeff
                        , bool  i_remove_dc_offset

                        , int i_n_mel_banks
                        , float i_high_freq
                        , float i_low_freq

                        , bool i_use_energy
                        , bool i_raw_energy
                        , int i_n_cepstral_coefficients
                        , float i_cepstral_lifter_coefficient
                        , int *o_err_code);

    void DeleteMfccComputer(void *o_mfcc_computer);

    void *GetFbankFeaturesComputer( float i_frame_length_ms
                                  , float i_frame_shift_ms
                                  , float i_sampling_rate

                                  , float i_dither_scale
                                  , float i_preemph_coeff
                                  , bool i_remove_dc_offset

                                  , int i_n_mel_banks
                                  , float i_high_freq
                                  , float i_low_freq

                                  , bool i_use_energy
                                  , bool i_raw_energy
                                  , bool i_use_log_fbank
                                  , bool i_use_power
                                  , int *o_err_code);

    void *ComputeFbankFeatures(void *i_waveform
                    , int i_channel
                    , bool i_subtract_mean
                    , float i_vtln_warp_local
                    , void *i_fbank_features_computer
                    , int *o_n_features
                    , int *o_n_frames
                    , int *o_err_code);

    void DeleteFbankFeaturesComputer(void *o_fbank_computer);
    """
    global ffi
    global kaldi_lib
    ffi = cffi.FFI()
    ffi.cdef(src)
    try:
        kaldi_lib = ffi.dlopen(LIB_PATH)
    except ImportError:
        print "Library {} is not found in the LD_LIBRARY_PATH. Please, add ./lib to your LD_LIBRARY_PATH".format(
            LIB_PATH)
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
        if not isinstance(wav_data[0], np.int16) and not isinstance(wav_data[0], np.int32):
            raise RuntimeError("ERROR: wav data must be a list of integers. type %s is given." % type(wav_data[0]))
        ptr_last_err_code = self._ffi.new("int *")
        wav_data_float = np.array(wav_data, dtype=np.float32)
        kaldi_data_matrix = KaldiMatrix(numpy_matrix=wav_data_float)
        self._ptr_wav_data = self._kaldi_lib.GetWavData(kaldi_data_matrix.handle, self._sampling_rate,
                                                        ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Error trying to create object WaveData')

    def __del__(self):
        self._kaldi_lib.DeleteWavData(self._ptr_wav_data)

    @property
    def handle(self):
        return self._ptr_wav_data

    @property
    def sampling_rate(self):
        return self._sampling_rate


class KaldiFeatureExtractor(object):
    def __init__(self):
        self._extractor_function = None
        self._ptr_feature_extractor = None
        self._sampling_rate = 0.0

    def compute(self, wav_data, channel=CH_MONO, subtract_mean=False, local_vtln_factor=1.0):
        ptr_to_result = ffi.new("int *")
        ptr_return_by_reference1 = ffi.new("int *")
        ptr_return_by_reference2 = ffi.new("int *")
        if wav_data.sampling_rate != self._sampling_rate:
            raise ValueError("Sampling rate of the target file is different from that of MFCC extractor")
        if channel == CH_MONO:
            channel = 0
        ptr_to_feature_data = self._extractor_function(wav_data.handle, channel, subtract_mean, local_vtln_factor,
                                                          self._ptr_feature_extractor, ptr_return_by_reference1,
                                                          ptr_return_by_reference2,
                                                          ptr_to_result)
        err_code = ptr_to_result[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to compute MFCC features')
        n_cols = ptr_return_by_reference1[0]
        n_rows = ptr_return_by_reference2[0]
        feature_matrix = KaldiMatrix(ptr_to_feature_data, [n_rows, n_cols], dtype=np.float32)
        return feature_matrix


class KaldiMfccFeatureExtractor(KaldiFeatureExtractor):
    def __init__(self, frame_length_msec=25.0, frame_shift_msec=25.0, sampling_rate=16e3, dithering_coefficient=1.0,
                 preemphasis_coefficient=0.97, remove_dc_offset=True, number_of_mel_banks=23, high_frequency=NO_LIMIT,
                 low_frequency=20.0, use_energy=False, raw_energy=True, number_of_cepstral_coefficients=13,
                 cepstral_lifter_base_coefficient=22.0):
        super(KaldiMfccFeatureExtractor, self).__init__()
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._frame_length_msec = frame_length_msec
        self._frame_shift_msec = frame_shift_msec
        self._sampling_rate = sampling_rate
        self._number_of_mel_banks = number_of_mel_banks
        self._number_of_cepstral_coefficients = number_of_cepstral_coefficients
        self._dithering_coefficient = dithering_coefficient
        self._preemphasis_coefficient = preemphasis_coefficient
        self._remove_dc_offset = remove_dc_offset
        self._use_energy = use_energy
        self._raw_energy = raw_energy
        self._cepstral_lifter_base_coefficient = cepstral_lifter_base_coefficient
        self._low_frequency = low_frequency
        self._high_frequency = high_frequency
        ptr_to_result = ffi.new("int *")
        self._ptr_feature_extractor = self._kaldi_lib.GetMfccComputer(self._frame_length_msec,
                                                                           self._frame_shift_msec,
                                                                           self._sampling_rate,
                                                                           self._dithering_coefficient,
                                                                           self._preemphasis_coefficient,
                                                                           self._remove_dc_offset,
                                                                           self._number_of_mel_banks,
                                                                           self._high_frequency,
                                                                           self._low_frequency,
                                                                           self._use_energy,
                                                                           self._raw_energy,
                                                                           self._number_of_cepstral_coefficients,
                                                                           self._cepstral_lifter_base_coefficient,
                                                                           ptr_to_result)
        err_code = ptr_to_result[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to compute delta features')
        self._extractor_function = self._kaldi_lib.ComputeMfcc

    def __del__(self):
        self._kaldi_lib.DeleteMfccComputer(self._ptr_feature_extractor)


class KaldiFbankFeatureExtractor(KaldiFeatureExtractor):
    def __init__(self, frame_length_msec=25.0, frame_shift_msec=25.0, sampling_rate=16e3, dithering_coefficient=1.0,
                 preemphasis_coefficient=0.97, remove_dc_offset=True, number_of_mel_banks=23, high_frequency=NO_LIMIT,
                 low_frequency=20.0, use_energy=False, raw_energy=True, use_log_fbank=True, use_power=True):
        super(KaldiFbankFeatureExtractor, self).__init__()
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._frame_length_msec = frame_length_msec
        self._frame_shift_msec = frame_shift_msec
        self._sampling_rate = sampling_rate
        self._number_of_mel_banks = number_of_mel_banks
        self._dithering_coefficient = dithering_coefficient
        self._preemphasis_coefficient = preemphasis_coefficient
        self._remove_dc_offset = remove_dc_offset
        self._use_energy = use_energy
        self._raw_energy = raw_energy
        self._low_frequency = low_frequency
        self._high_frequency = high_frequency
        self._use_log_fbank = use_log_fbank
        self._use_power = use_power
        ptr_to_result = ffi.new("int *")
        self._ptr_feature_extractor = self._kaldi_lib.GetFbankFeaturesComputer(self._frame_length_msec,
                                                                           self._frame_shift_msec,
                                                                           self._sampling_rate,
                                                                           self._dithering_coefficient,
                                                                           self._preemphasis_coefficient,
                                                                           self._remove_dc_offset,
                                                                           self._number_of_mel_banks,
                                                                           self._high_frequency,
                                                                           self._low_frequency,
                                                                           self._use_energy,
                                                                           self._raw_energy,
                                                                           self._use_log_fbank,
                                                                           self._use_power,
                                                                           ptr_to_result)
        err_code = ptr_to_result[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to compute delta features')
        self._extractor_function = self._kaldi_lib.ComputeFbankFeatures

    def __del__(self):
        self._kaldi_lib.DeleteFbankFeaturesComputer(self._ptr_feature_extractor)


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

#copy-matrix ark:/home/mkudinov/KALDI/kaldi_new/kaldi/egs/ruspeech/s1/mfcc/raw_mfcc_train.1.ark  'scp,t:echo TRAIN-FCT002-002B0181 -|'
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
    sr, wav_record = wavfile.read('/home/mkudinov/Data/RUSPEECH/TRAIN/FCT002/002B0181.WAV')
    kaldi_wav = KaldiWavData(wav_record, sr)
    mfcc_feature_extractor = KaldiMfccFeatureExtractor(frame_length_msec=25.0, frame_shift_msec=10.0, sampling_rate=22050)
    feature_matrix2 = mfcc_feature_extractor.compute(kaldi_wav)
    #print feature_matrix2.numpy_array()
    a = feature_matrix.numpy_array()
    b = feature_matrix2.numpy_array()
    print np.linalg.norm(a-b)


