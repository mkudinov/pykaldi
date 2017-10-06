#include "kaldi-python-common.h"
#include <cstdio>

namespace kaldi_python_common
{
Alignment *CreateAlignmentBuffer(int i_number_of_phones, int *o_err_code)
{
    *o_err_code = OK;
    Alignment *alignment_buffer = new Alignment;
    if(!alignment_buffer) 
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
        return 0;
    }
    alignment_buffer->number_of_phones = i_number_of_phones;
    try
    {
        alignment_buffer->num_repeats_per_phone = new int[i_number_of_phones];
        alignment_buffer->phones = new int[i_number_of_phones];
    }
    catch(...)
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
    }
    return alignment_buffer;
} 

void DeleteAlignmentBuffer(Alignment *o_alignment_buffer)
{
    if(o_alignment_buffer)
    {
        if(o_alignment_buffer->phones) 
        {
            delete[] o_alignment_buffer->phones;
        }
        if(o_alignment_buffer->num_repeats_per_phone) 
        {
            delete[] o_alignment_buffer->num_repeats_per_phone;
        }
    }
    o_alignment_buffer->number_of_phones = 0;
    delete o_alignment_buffer;
}
} //namespace kaldi_python_common

namespace numpy_constants
{
const char* NP_FLOAT_32 = "float32";
const char* NP_FLOAT_64 = "float64";
} //namespace numpy_constants