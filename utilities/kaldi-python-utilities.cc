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

void *GetFeatureReader(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::RandomAccessBaseFloatMatrixReader* feature_reader = new kaldi::RandomAccessBaseFloatMatrixReader(i_specifier);
    feature_reader->HasKey("abc");
    return feature_reader;
}

void DeleteFeatureReader(void *o_feature_reader)
{
    if(o_feature_reader)
    {
        delete static_cast<kaldi::RandomAccessBaseFloatMatrixReader*>(o_feature_reader); 
        o_feature_reader = 0;
    }
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
    std::string key(i_key);
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

//void AddDeltaFeatures(void *i_feature_matrix
//                    , int i_n_rows
//                    , int i_n_columns
//                    , void *o_delta_feature_matrix
//                    , int *o_n_rows
//                    , int *o_n_columns
//                    , int *o_err_code)
//{
//    try
//    {
//        *o_err_code = OK;
//        kaldi::DelatFeaturesOptions opts;
//        int truncate = 0;
//        const kaldi::Matrix<float>* source = static_cast<const kaldi::Matrix<float>*>(i_feature_matrix);
//        int nRows = source->NumRows();
//        int nColumns = source->NumCols();
//        for()
//        {
//            if (source->NumRows() == 0)
//            {
//                KALDI_WARN << "Empty feature matrix for key " << key;
//            }
//        }
//        Matrix<float> new_feats;
//        if(truncate > 0)
//        {
//            if(truncate > feats.NumCols())
//            {
//                KALDI_ERR << "Cannot truncate features as dimension " << feats.NumCols()
//                    << " is smaller than truncation dimension.";
//            }
//            SubMatrix<BaseFloat> feats_sub(feats, 0, feats.NumRows(), 0, truncate);
//            ComputeDeltas(opts, feats_sub, &new_feats);
//        }
//        else
//        {
//            ComputeDeltas(opts, feats, &new_feats);
//            *o_delta_feature_matrix = &
//        }
//
//    }
//    catch(const std::exception &e)
//    {
//        *o_err_code = INTERNAL_KALDI_ERROR;
//        std::cerr << e.what();
//    }
//}

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

void CopyIntegerVector(int *i_source, int i_size, void *o_destination, int *o_err_code)
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

void DeleteAlignment(Alignment *o_alignment_buffer)
{
    kaldi_python_common::DeleteAlignmentBuffer(o_alignment_buffer);
}
} //namespace kaldi_python_readers 
} //extern "C"
