#include "kaldi-python-feat.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_asr_model
{
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
    const kaldi::Matrix<float>* features = new kaldi::Matrix<float>(feature_reader->Value(key));
    *o_n_rows = features->NumRows();
    *o_n_columns = features->NumCols();
    return features;
}

void DeleteFeatureMatrix(void *o_feature_matrix)
{
    if(o_feature_matrix)
    {
        delete static_cast<kaldi::Matrix<float>*>(o_feature_matrix);
        o_feature_matrix = 0;
    }
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

void *GetMatrixOfDeltaFeatures(void *i_feature_matrix,
                      int i_order,
                      int i_window,
                      int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        kaldi::Matrix<float>* input_features = static_cast<kaldi::Matrix<float>* >(i_feature_matrix);
        kaldi::DeltaFeaturesOptions options(i_order, i_window);
        kaldi::Matrix<float>* new_features = new kaldi::Matrix<float>();
        kaldi::ComputeDeltas(options, *input_features, new_features);
        return new_features;
    }
    catch(...)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
}

} //namespace kaldi_python_asr_model
} //extern "C"