#include "kaldi-python-utilities.h"
#include "../hmm/hmm-utils.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_readers 
{
int *ReadIntegerVector(char *i_specifier, int *o_n_elements, int *o_err_code)
{
    *o_err_code = OK;
    vector<int32> tmp;
    if (!kaldi::ReadIntegerVectorSimple(i_specifier, &tmp))
    {   
        KALDI_ERR << "fstcomposecontext: Could not read disambiguation symbols from "
                  << i_specifier;
        *o_err_code = ERROR_OPENING;
        *o_n_elements = 0;
        return 0;
    }
    int size = *o_n_elements = tmp.size(); 
    int *destination;
    try
    {
        destination = new int[size]; 
        std::memcpy(destination, &tmp[0], (*o_n_elements) * sizeof(int));
    }
    catch(...)
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
    }
    return destination; 
}

void DeleteIntegerVector(int *o_vector)
{
    if(o_vector)
    {
        delete[] o_vector;
    }
}

void CopyIntegerVector(int *i_source, int i_size,  int *o_destination, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        std::memcpy(o_destination, i_source, i_size * sizeof(int));
    }
    catch(...)
    {
       *o_err_code = MEMORY_ALLOCATION_ERROR; 
    }
}

void *InitIntegerVector(int *i_source, int i_size, int *o_err_code)
{
    *o_err_code = OK;
    int *result = new int[i_size];
    try
    {
        std::memcpy(result, i_source, i_size * sizeof(int));
    }
    catch(...)
    {
       *o_err_code = MEMORY_ALLOCATION_ERROR;
    }
    return result;
}

void DeleteAlignment(Alignment *o_alignment_buffer)
{
    kaldi_python_common::DeleteAlignmentBuffer(o_alignment_buffer);
}
} //namespace kaldi_python_readers 
} //extern "C"
