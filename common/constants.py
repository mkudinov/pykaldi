
class KALDI_ERR_CODES(object):
    OK,\
    ERROR_OPENING,\
    NO_KEY,\
    INDEX_OUT_OF_BOUNDS,\
    NO_READER_EXISTS,\
    COPY_ERROR,\
    MEMORY_ALLOCATION_ERROR,\
    INTERNAL_KALDI_ERROR,\
    WRONG_INPUT,\
    KALDI_WRONG_CONFIGURATION, \
    CORRUPTED_POINTER = range(11)

#int error code to string error code
string_error_codes = ["OK", "ERROR_OPENING", "NO_KEY", "INDEX_OUT_OF_BOUNDS", "NO_READER_EXISTS", "COPY_ERROR", "MEMORY_ALLOCATION_ERROR", "INTERNAL_KALDI_ERROR", "WRONG_INPUT", "KALDI_WRONG_CONFIGURATION", "CORRUPTED_POINTER"]

def print_error(error_message):
    print "ERROR: code {}".format(string_error_codes[error_message])

if __name__ == '__main__':
    print KALDI_ERR_CODES
