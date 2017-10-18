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

LIB_PATH = 'libpython-kaldi-fst.so'

def initialize_cffi():
    src = """
    void *GetFst(char *i_path_to_fst, int *o_err_code);
    int GetNumberOfArcs(void *i_fst, int *o_err_code);
    void DeleteFst(void *o_fst);
    void *GetFstReader(char *i_specifier, int *o_err_code);
    void *ReadFst(char *i_key, void *i_fst_reader, int *o_err_code);
    void DeleteFstReader(void* o_fst_reader);
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


class KaldiFST(object):
    def __init__(self, path_to_fst_file=None, fst_handle=None):
        self.kaldi_lib = kaldi_lib
        ptr_last_err_code = ffi.new("int *")
        self._ptr_fst = None
        if path_to_fst_file is not None:
            self._ptr_fst = self.kaldi_lib.GetFst(path_to_fst_file, ptr_last_err_code)
            err_code = ptr_last_err_code[0]
            if err_code != ked.OK:
                raise RuntimeError('FST reading failed')
        elif fst_handle is not None:
            self._ptr_fst = fst_handle
        else:
            raise RuntimeError('Only reading from file and initialization from existing pointer is currently supported!')
        self.n_arcs = self.kaldi_lib.GetNumberOfArcs(self._ptr_fst, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('FST reference crashed')

    def __del__(self):
        self.kaldi_lib.DeleteFst(self._ptr_fst)

    def __str__(self):
        return "FST: %s arcs" % (self.n_arcs)

    @property
    def handle(self):
        return self._ptr_fst


class KaldiFstReader(object):
    def __init__(self):
        self._kaldi_lib = kaldi_lib
        self._ffi = ffi
        self._ptr_last_err_code = self._ffi.new("int *")
        self._open = False

    def __del__(self):
        if self._open:
            self.close_archive()

    def get_fst(self, record_descriptor):
        result_ptr = self._kaldi_lib.ReadFst(record_descriptor, self._fst_reader, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to get fst for descriptor {}'.format(record_descriptor))
        fst = KaldiFST(fst_handle=result_ptr)
        return fst

    def open_archive(self, path_to_archive):
        if self._open:
            self.close_archive()
        if not os.path.isfile(path_to_archive):
            raise RuntimeError('Error trying to open archive {}. No such file or directory'.format(path_to_archive))
        specifier = "ark:{}".format(path_to_archive)
        self._fst_reader = self._kaldi_lib.GetFstReader(specifier, self._ptr_last_err_code)
        err_code = self._ptr_last_err_code[0]
        if err_code != ked.OK:
            print_error(err_code)
            raise RuntimeError('Error trying to open archive {}'.format(path_to_archive))
        self._open = True

    def close_archive(self):
        if self._open:
            self._kaldi_lib.DeleteFstReader(self._fst_reader)
        self._open = False


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_LEXICAL_FST = KALDI_PATH + RUSPEECH_EXP_PATH + 'data/lang/L.fst'
    PATH_TO_FST_ARCHIVE = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/fsts.1'
    fst = KaldiFST(PATH_TO_LEXICAL_FST)
    print fst
    fst_reader = KaldiFstReader()
    fst_reader.open_archive(PATH_TO_FST_ARCHIVE)
    fst2 = fst_reader.get_fst("TRAIN-FCT002-002B0181")
    print fst2