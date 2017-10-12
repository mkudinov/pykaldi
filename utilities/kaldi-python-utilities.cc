#include "kaldi-python-utilities.h"
#include "../hmm/hmm-utils.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_readers 
{

void *GetAlignmentReader(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    std::string specifier(i_specifier);
    kaldi::RandomAccessInt32VectorReader *alignment_reader = new kaldi::RandomAccessInt32VectorReader(specifier);
    alignment_reader->HasKey("abc");
    return alignment_reader;
}

void DeleteAlignmentReader(void *o_alignment_reader)
{
    if(o_alignment_reader)
    {
        delete static_cast<kaldi::RandomAccessInt32VectorReader*>(o_alignment_reader); 
        o_alignment_reader = 0;
    }
}

Alignment *ReadAlignment(char *i_key, void *i_transition_model, void *i_alignment_reader, int *o_err_code)
{
    *o_err_code = OK;
    if(!i_alignment_reader || !i_transition_model)
    {
        *o_err_code = NO_READER_EXISTS;
        return 0;
    }
    kaldi::RandomAccessInt32VectorReader* alignment_reader = static_cast<kaldi::RandomAccessInt32VectorReader*>(i_alignment_reader); 
    kaldi::TransitionModel *transition_model = static_cast<kaldi::TransitionModel*>(i_transition_model);
    std::string key(i_key);
    if(!alignment_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    std::vector<vector<int32> > split;
    std::vector<int32> alignment = alignment_reader->Value(key);
    kaldi::SplitToPhones(*transition_model, alignment, &split);
    Alignment *result = kaldi_python_common::CreateAlignmentBuffer(split.size(), o_err_code);
    if(*o_err_code != OK)
        return 0;
    try 
    {
        for(size_t i = 0; i < result->number_of_phones; ++i)
        {
            int phone = transition_model->TransitionIdToPhone(split[i][0]);
            int num_repeats = split[i].size();
            result->phones[i] = phone;
            result->num_repeats_per_phone[i] = num_repeats;
        }
    }
    catch(...)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    return result;
}

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
