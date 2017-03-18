#include "read-kaldi-data.h"
#include "../hmm/hmm-utils.h"
#include <iostream>
extern "C"
{
namespace python_data_readers
{
kaldi::RandomAccessBaseFloatMatrixReader* feature_reader;

void *GetAlignmentReader(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    string specifier(i_specifier);
    kaldi::RandomAccessInt32VectorReader *alignment_reader = new kaldi::RandomAccessInt32VectorReader(specifier);
    alignment_reader->HasKey("abc");
    return alignment_reader;
}

Alignment *GetResultBuffer()
{
    Alignment *alignment_buffer = new Alignment();
    alignment_buffer->phones = 0;
    alignment_buffer->num_repeats_per_phone = 0;
    alignment_buffer->number_of_phones = 0;
    return alignment_buffer;
} 

void *GetTransitionModel(char *i_transition_model_filename, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::TransitionModel* transition_model = new kaldi::TransitionModel();
    kaldi::ReadKaldiObject(i_transition_model_filename, transition_model);
    if(!transition_model)
    {
        *o_err_code = ERROR_OPENING;
    }
    return transition_model;
}

size_t ReadAlignment(char *i_key, void *i_transition_model, void *i_alignment_reader, Alignment *o_alignment_buffer, int *o_err_code)
{
    *o_err_code = OK;
    if(!i_alignment_reader || !i_transition_model)
    {
        *o_err_code = NO_READER_EXISTS;
        return 0;
    }
    kaldi::RandomAccessInt32VectorReader* alignment_reader = static_cast<kaldi::RandomAccessInt32VectorReader*>(i_alignment_reader); 
    kaldi::TransitionModel *transition_model = static_cast<kaldi::TransitionModel*>(i_transition_model);
    string key(i_key);
    if(!alignment_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    vector<int32> alignment;
    vector<vector<int32> > split;
    alignment = alignment_reader->Value(key);
    DeleteResultBuffer(o_alignment_buffer);
    kaldi::SplitToPhones(*transition_model, alignment, &split);
    o_alignment_buffer->number_of_phones = split.size();
    try
    {
        o_alignment_buffer->phones = new int[o_alignment_buffer->number_of_phones];
        o_alignment_buffer->num_repeats_per_phone = new int[o_alignment_buffer->number_of_phones];
    }
    catch(...)
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
        return 0;
    }
    try 
    {
        for(size_t i = 0; i < o_alignment_buffer->number_of_phones; ++i)
        {
            int phone = transition_model->TransitionIdToPhone(split[i][0]);
            int num_repeats = split[i].size();
            o_alignment_buffer->phones[i] = phone;
            o_alignment_buffer->num_repeats_per_phone[i] = num_repeats;
        }
    }
    catch(...)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    return o_alignment_buffer->number_of_phones;
}

void DeleteTransitionModel(void *o_transition_model)
{
    kaldi::TransitionModel* transition_model = static_cast<kaldi::TransitionModel*>(o_transition_model);
    if(transition_model)
        delete transition_model;
    transition_model = 0;
}

void DeleteAlignmentReader(void *o_alignment_reader)
{
    kaldi::RandomAccessInt32VectorReader* alignment_reader = static_cast<kaldi::RandomAccessInt32VectorReader*>(o_alignment_reader);  
    if(alignment_reader)
        delete alignment_reader;
    alignment_reader = 0;
}

void DeleteResultBuffer(Alignment *o_alignment_buffer)
{
    if(o_alignment_buffer->phones) 
    {
        delete[] o_alignment_buffer->phones;
    }
    if(o_alignment_buffer->num_repeats_per_phone) 
    {
        delete[] o_alignment_buffer->num_repeats_per_phone;
    }
    o_alignment_buffer->number_of_phones = 0;
}

void *GetFeatureReader(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::RandomAccessBaseFloatMatrixReader* feature_reader = new kaldi::RandomAccessBaseFloatMatrixReader(i_specifier);
    feature_reader->HasKey("abc");
    return feature_reader;
}

const void* ReadFeatureMatrix(char* i_key, void *i_feature_reader, int* o_n_rows, int* o_n_columns, int *o_err_code)
{
    *o_err_code = OK;
    if(!i_feature_reader)
    {
        *o_err_code = NO_READER_EXISTS;
        return 0;
    }
    kaldi::RandomAccessBaseFloatMatrixReader* feature_reader = static_cast<kaldi::RandomAccessBaseFloatMatrixReader*>(i_feature_reader); 
    string key(i_key);
    if(!feature_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    const kaldi::Matrix<float> &features = feature_reader->Value(key);
    *o_n_rows = features.NumRows();
    *o_n_columns = features.NumCols();
    return &features;
}

void CopyFeatureMatrix(void *i_source, void *o_destination, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        const kaldi::Matrix<float>* source = static_cast<const kaldi::Matrix<float>*>(i_source);
        int nRows = source->NumRows();
        int nColumns = source->NumCols();
        for(int i = 0; i < nRows; i++)
        {
            std::memcpy(static_cast<float*>(o_destination) + i * nColumns, source->RowData(i), nColumns * sizeof(float));
        }
    }
    catch(...)
    {
        *o_err_code = COPY_ERROR;
    }
}

void DeleteFeatureReader(void *o_feature_reader)
{
    kaldi::RandomAccessBaseFloatMatrixReader* feature_reader = static_cast<kaldi::RandomAccessBaseFloatMatrixReader*>(o_feature_reader); 
    if(feature_reader)
        delete feature_reader;
    feature_reader = 0;
}

} //namespace python_data_readers
}
