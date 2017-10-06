// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_COMMON_
#define KALDI_PYTHON_COMMON_

#include <vector>
#include <string>

extern "C"
{
namespace kaldi_python_common_errors
{
enum
{
    OK,
    ERROR_OPENING,
    NO_KEY,
    INDEX_OUT_OF_BOUNDS,
    NO_READER_EXISTS,
    COPY_ERROR,
    MEMORY_ALLOCATION_ERROR,
    INTERNAL_KALDI_ERROR,
    WRONG_INPUT,
    KALDI_WRONG_CONFIGURATION,
    CORRUPTED_POINTER
};
} //namespace kaldi_python_common_errors

namespace numpy_constants
{
    extern const char *NP_FLOAT_32;
    extern const char *NP_FLOAT_64;
}

namespace kaldi_python_common
{
using namespace kaldi_python_common_errors;
typedef struct 
{
    int number_of_phones;
    int *phones;
    int *num_repeats_per_phone;
} Alignment;

typedef struct
{
    int number_of_elements;
    int *elements;
} IntegerVector;

/* AlignmentBuffer*/
Alignment *CreateAlignmentBuffer(int i_number_of_phones, int *o_err_code);
void DeleteAlignmentBuffer(Alignment *o_alignment_buffer);

} //namespace kaldi_python_common
} //extern "C"

#endif
