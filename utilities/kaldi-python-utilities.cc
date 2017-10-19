#include "kaldi-python-utilities.h"
#include "../hmm/hmm-utils.h"
#include <iostream>
namespace kaldi_python_readers 
{
int *ReadIntegerVector(char *i_specifier, int *o_n_elements, int *o_err_code)
{
    *o_err_code = OK;
    vector<int32> tmp;
    if (!kaldi::ReadIntegerVectorSimple(i_specifier, &tmp))
    {   
        KALDI_ERR << "Could not read disambiguation symbols from "
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

kaldi::RandomAccessBaseFloatMatrixReader* getMatrixReaderFloat(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::RandomAccessBaseFloatMatrixReader* matrix_reader = new kaldi::RandomAccessBaseFloatMatrixReader(i_specifier);
    matrix_reader->HasKey("abc");
    return matrix_reader;
}

kaldi::RandomAccessDoubleMatrixReader* getMatrixReaderDouble(char *i_specifier, int *o_err_code)
{
    kaldi::RandomAccessDoubleMatrixReader* matrix_reader = new kaldi::RandomAccessDoubleMatrixReader(i_specifier);
    matrix_reader->HasKey("abc");
    return matrix_reader;
}

void *GetMatrixReader(char *i_specifier, char *i_data_type, int *o_err_code)
{
    *o_err_code = OK;
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        return getMatrixReaderFloat(i_specifier, o_err_code);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        return getMatrixReaderDouble(i_specifier, o_err_code);
    }
    *o_err_code = NO_READER_EXISTS;
    return 0;
}

void DeleteMatrixReader(void *o_matrix_reader, char *i_data_type)
{
    if(!o_matrix_reader)
    {
        return;
    }
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        delete static_cast<kaldi::RandomAccessBaseFloatMatrixReader*>(o_matrix_reader);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        delete static_cast<kaldi::RandomAccessDoubleMatrixReader*>(o_matrix_reader);
    }
    o_matrix_reader = 0;
}

template<typename T>
const void* readMatrix(char* i_key, void *i_matrix_reader, int* o_n_rows, int* o_n_columns, int *o_err_code)
{
    kaldi::RandomAccessTableReader<kaldi::KaldiObjectHolder<kaldi::Matrix<T> > > * matrix_reader =
           static_cast<kaldi::RandomAccessTableReader<kaldi::KaldiObjectHolder<kaldi::Matrix<T> > > *>(i_matrix_reader);
    std::string key(i_key);
    if(!matrix_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    const kaldi::Matrix<T>* matrix = new kaldi::Matrix<T>(matrix_reader->Value(key));
    *o_n_rows = matrix->NumRows();
    *o_n_columns = matrix->NumCols();
    return matrix;
}

const void* ReadMatrix(char* i_key, void *i_matrix_reader, char* i_data_type, int* o_n_rows, int* o_n_columns, int *o_err_code)
{
    *o_err_code = OK;
    if(!i_matrix_reader)
    {
        *o_err_code = NO_READER_EXISTS;
        return 0;
    }
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        return readMatrix<float>(i_key, i_matrix_reader, o_n_rows, o_n_columns, o_err_code);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        return readMatrix<double>(i_key, i_matrix_reader, o_n_rows, o_n_columns, o_err_code);
    }

    *o_err_code = WRONG_INPUT;
    return 0;
}

void DeleteMatrix(void *o_matrix, char* i_data_type)
{
    if(!o_matrix)
    {
        return;
    }
    if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
    {
        delete static_cast<kaldi::Matrix<float>*>(o_matrix);
    }
    else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
    {
        delete static_cast<kaldi::Matrix<double>*>(o_matrix);
    }
    o_matrix = 0;
}

template<typename T>
void copyMatrix(void *i_source, void *o_destination, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        const kaldi::Matrix<T>* source = static_cast<const kaldi::Matrix<T>*>(i_source);
        int nRows = source->NumRows();
        int nColumns = source->NumCols();
        for(int i = 0; i < nRows; i++)
        {
            std::memcpy(static_cast<T*>(o_destination) + i * nColumns, source->RowData(i), nColumns * sizeof(T));
        }
    }
    catch(...)
    {
        *o_err_code = COPY_ERROR;
    }
}

void CopyMatrix(void *i_source, void *o_destination, char *i_data_type, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
        {
            copyMatrix<float>(i_source, o_destination, o_err_code);
        }
        else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
        {
            copyMatrix<double>(i_source, o_destination, o_err_code);
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

template<typename T>
void *initMatrix(void *i_source, int i_nRows, int i_nColumns, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        kaldi::Matrix<T>* result = new kaldi::Matrix<T>(i_nRows, i_nColumns);
        for(int i = 0; i < i_nRows; i++)
        {
            std::memcpy(result->RowData(i), static_cast<T*>(i_source) + i * i_nColumns, i_nColumns * sizeof(T));
        }
        return result;
    }
    catch(...)
    {
        *o_err_code = COPY_ERROR;
    }
    return 0;
}

void *InitMatrix(void *i_source, int i_nRows, int i_nColumns, char *i_data_type, int *o_err_code)
{
    *o_err_code = OK;
    try
    {
        if (!strcmp(i_data_type, numpy_constants::NP_FLOAT_32))
        {
            return initMatrix<float>(i_source, i_nRows, i_nColumns, o_err_code);
        }
        else if(!strcmp(i_data_type, numpy_constants::NP_FLOAT_64))
        {
            return initMatrix<double>(i_source, i_nRows, i_nColumns, o_err_code);
        }
        else
        {
            *o_err_code = WRONG_INPUT;
        }
    }
    catch(...)
    {
       *o_err_code = MEMORY_ALLOCATION_ERROR;
    }
    return 0;
}
} //namespace kaldi_python_readers
