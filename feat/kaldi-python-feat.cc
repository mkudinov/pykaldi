#include "kaldi-python-feat.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_asr_model
{
kaldi::RandomAccessBaseFloatMatrixReader* getFeatureReaderFloat(char *i_specifier, int *o_err_code)
{
    kaldi::RandomAccessBaseFloatMatrixReader* feature_reader = new kaldi::RandomAccessBaseFloatMatrixReader(i_specifier);
    feature_reader->HasKey("abc");
    return feature_reader;
}

kaldi::RandomAccessDoubleMatrixReader* getFeatureReaderDouble(char *i_specifier, int *o_err_code)
{
    kaldi::RandomAccessDoubleMatrixReader* feature_reader = new kaldi::RandomAccessDoubleMatrixReader(i_specifier);
    feature_reader->HasKey("abc");
    return feature_reader;
}

void *GetFeatureReader(char *i_specifier, char *i_data_type, int *o_err_code)
{
    *o_err_code = OK;
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        return getFeatureReaderFloat(i_specifier, o_err_code);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        return getFeatureReaderDouble(i_specifier, o_err_code);
    }
    *o_err_code = NO_READER_EXISTS;
    return 0;
}

void DeleteFeatureReader(void *o_feature_reader, char *i_data_type)
{
    if(!o_feature_reader)
    {
        return;
    }
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        delete static_cast<kaldi::RandomAccessBaseFloatMatrixReader*>(o_feature_reader);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        delete static_cast<kaldi::RandomAccessDoubleMatrixReader*>(o_feature_reader);
    }
    o_feature_reader = 0;
}

const void* readFeatureMatrixFloat(char* i_key, void *i_feature_reader, int* o_n_rows, int* o_n_columns, int *o_err_code)
{
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

const void* readFeatureMatrixDouble(char* i_key, void *i_feature_reader, int* o_n_rows, int* o_n_columns, int *o_err_code)
{
    kaldi::RandomAccessDoubleMatrixReader* feature_reader = static_cast<kaldi::RandomAccessDoubleMatrixReader*>(i_feature_reader);
    std::string key(i_key);
    if(!feature_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    const kaldi::Matrix<double>* features = new kaldi::Matrix<double>(feature_reader->Value(key));
    *o_n_rows = features->NumRows();
    *o_n_columns = features->NumCols();
    return features;
}

const void* ReadFeatureMatrix(char* i_key, void *i_feature_reader, char* i_data_type, int* o_n_rows, int* o_n_columns, int *o_err_code)
{
    *o_err_code = OK;
    if(!i_feature_reader)
    {
        *o_err_code = NO_READER_EXISTS;
        return 0;
    }
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        return readFeatureMatrixFloat(i_key, i_feature_reader, o_n_rows, o_n_columns, o_err_code);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        return readFeatureMatrixDouble(i_key, i_feature_reader, o_n_rows, o_n_columns, o_err_code);
    }

    *o_err_code = WRONG_INPUT;
    return 0;
}

void DeleteFeatureMatrix(void *o_feature_matrix, char* i_data_type)
{
    if(!o_feature_matrix)
    {
        return;
    }
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        delete static_cast<kaldi::Matrix<float>*>(o_feature_matrix);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        delete static_cast<kaldi::Matrix<double>*>(o_feature_matrix);
    }
    o_feature_matrix = 0;
}

void copyFeatureMatrixFloat(void *i_source, void *o_destination, int *o_err_code)
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

void copyFeatureMatrixDouble(void *i_source, void *o_destination, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        const kaldi::Matrix<double>* source = static_cast<const kaldi::Matrix<double>*>(i_source);
        int nRows = source->NumRows();
        int nColumns = source->NumCols();
        for(int i = 0; i < nRows; i++)
        {
            std::memcpy(static_cast<double*>(o_destination) + i * nColumns, source->RowData(i), nColumns * sizeof(double));
        }
    }
    catch(...)
    {
        *o_err_code = COPY_ERROR;
    }
}

void CopyFeatureMatrix(void *i_source, void *o_destination, char *i_data_type, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
        {
            copyFeatureMatrixFloat(i_source, o_destination, o_err_code);
        }
        else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
        {
            copyFeatureMatrixDouble(i_source, o_destination, o_err_code);
        }
        else
        {
            *o_err_code = WRONG_INPUT;
        }
    }
    catch(...)
    {
        *o_err_code = COPY_ERROR;
    }
}

void *GetMatrixOfDeltaFeatures(void *i_feature_matrix
                               , int i_order
                               , int i_window
                               , int* o_n_rows
                               , int* o_n_columns
                               , int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        kaldi::Matrix<float>* input_features = static_cast<kaldi::Matrix<float>* >(i_feature_matrix);
        kaldi::DeltaFeaturesOptions options(i_order, i_window);
        kaldi::Matrix<float>* new_features = new kaldi::Matrix<float>();
        kaldi::ComputeDeltas(options, *input_features, new_features);
        *o_n_rows = new_features->NumRows();
        *o_n_columns = new_features->NumCols();
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