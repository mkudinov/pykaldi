import cffi

ffi = None
kaldi_lib = None

KALDI_PATH = '/home/mkudinov/KALDI/kaldi_new/kaldi/'
LIB_PATH = KALDI_PATH + 'src/python/libpython-kaldi-aligner.so'


def initialize_cffi():
    src="""
    void GetAligner(char *i_tree_rxfilename,  char *i_model_rxfilename, char *i_lex_rxfilename);
    """
    global ffi
    global kaldi_lib
    ffi = cffi.FFI()
    ffi.cdef(src)
    kaldi_lib = ffi.dlopen(LIB_PATH)


class KaldiWavAligner(object):
    def __init__(self):
        self.kaldi_lib = kaldi_lib
        self.ffi = ffi
        self.kaldi_lib.GetAligner(path_to_context_tree, path_to_transition_model, path_to_lexicon_fst)


initialize_cffi()
aligner = KaldiWavAligner(PATH_TO_CONTEXT_TREE, PATH_TO_TRANSITION_MODEL. PATH_TO_LEXICON_FST)
