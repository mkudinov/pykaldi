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

LIB_PATH = 'libpython-kaldi-asr-model.so'

def initialize_cffi():
    src = """
       /* TransitionModel*/
        void *GetTransitionModel(char *i_transition_model_filename
                                , int *o_err_code);

        void DeleteTransitionModel(void *o_transition_model);

        int GetNumberOfTransitionIds(void *i_transition_model
                                    , int *o_err_code);

        int GetNumberOfPdfsTM(void *i_transition_model
                             , int *o_err_code);

        int GetNumberOfPhones(void *i_transition_model
                             , int *o_err_code);

        /* Acoustic model*/
        void *GetAcousticModel(char *i_transition_model_filename
                              , int *o_err_code);

        void DeleteAcousticModel(void *o_amm_gmm);

        int GetNumberOfPdfsAM(void *i_acoustic_model
                             , int *o_err_code);

        int GetNumberOfGauss(void *i_acoustic_model
                            , int *o_err_code);

        int GetNumberOfGaussInPdf(void *i_acoustic_model
                                 , int i_pdf_id
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

class ASR_model(object):
    def __init__(self, path_to_model=None):
        self.kaldi_lib = kaldi_lib
        if path_to_model is None:
            raise RuntimeError('Only reading from file is currently supported!')
        ptr_last_err_code = ffi.new("int *")
        self._ptr_transition_model = self.kaldi_lib.GetTransitionModel(path_to_model, ptr_last_err_code)
        self._ptr_acoustic_model = self.kaldi_lib.GetAcousticModel(path_to_model, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Transition model reading failed')
        self.n_transitions = self.kaldi_lib.GetNumberOfTransitionIds(self._ptr_transition_model, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Model reference crashed')
        self.n_pdfs = self.kaldi_lib.GetNumberOfPdfsAM(self._ptr_acoustic_model, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Model reference crashed')
        self.n_gauss = self.kaldi_lib.GetNumberOfGauss(self._ptr_acoustic_model, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Model reference crashed')

    def __del__(self):
        self.kaldi_lib.DeleteTransitionModel(self._ptr_transition_model)
        self.kaldi_lib.DeleteAcousticModel(self._ptr_acoustic_model)

    def __str__(self):
        return "Model: %s transitions; %s pdfs; %s gauss" % (self.n_transitions, self.n_pdfs, self.n_gauss)


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_MODEL = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/final.mdl'
    asr_model = ASR_model(PATH_TO_MODEL)
    print asr_model
