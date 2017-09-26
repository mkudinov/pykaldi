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