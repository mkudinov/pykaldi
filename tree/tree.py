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

LIB_PATH = 'libpython-kaldi-tree.so'

def initialize_cffi():
    src = """
        void *GetContextDependency(char *i_path_to_tree, int *o_err_code);
        int GetContextWidth(void *i_tree, int *o_err_code);
        int GetCentralPosition(void *i_tree, int *o_err_code);
        void DeleteContextDependency(void *o_tree);
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


class ContextDependency(object):
    def __init__(self, path_to_tree_file):
        self.kaldi_lib = kaldi_lib
        if path_to_tree_file is None:
            raise RuntimeError('Only reading from file is currently supported!')
        ptr_last_err_code = ffi.new("int *")
        self._ptr_tree = self.kaldi_lib.GetContextDependency(path_to_tree_file, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Tree reading failed')
        self.context_width = self.kaldi_lib.GetContextWidth(self._ptr_tree, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Object reference crashed')
        self.central_position = self.kaldi_lib.GetCentralPosition(self._ptr_tree, ptr_last_err_code)
        err_code = ptr_last_err_code[0]
        if err_code != ked.OK:
            raise RuntimeError('Object reference crashed')

    def __del__(self):
        self.kaldi_lib.DeleteContextDependency(self._ptr_tree)

    def __str__(self):
        return "ContextDependency: width: %s; %s central position " % (self.context_width, self.central_position)

    @property
    def handle(self):
        return self._ptr_tree


initialize_cffi()
if __name__ == '__main__':
    KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
    RUSPEECH_EXP_PATH = 'egs/ruspeech/s1/'
    PATH_TO_CONTEXT_TREE = KALDI_PATH + RUSPEECH_EXP_PATH + 'exp/tri1/tree'
    context_dependency = ContextDependency(PATH_TO_CONTEXT_TREE)
    print context_dependency
