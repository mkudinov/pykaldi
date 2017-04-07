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

void DeleteAlignmentReader(void *o_alignment_reader)
{
    kaldi::RandomAccessInt32VectorReader* alignment_reader = static_cast<kaldi::RandomAccessInt32VectorReader*>(o_alignment_reader);  
    if(alignment_reader)
        delete alignment_reader;
    alignment_reader = 0;
}


Alignment *GetResultBuffer(int *o_err_code)
{
    *o_err_code = OK;
    Alignment *alignment_buffer = new Alignment();
    if(!alignment_buffer) 
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
        return 0;
    }
    alignment_buffer->phones = 0;
    alignment_buffer->num_repeats_per_phone = 0;
    alignment_buffer->number_of_phones = 0;
    return alignment_buffer;
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

void DeleteTransitionModel(void *o_transition_model)
{
    kaldi::TransitionModel* transition_model = static_cast<kaldi::TransitionModel*>(o_transition_model);
    if(transition_model)
        delete transition_model;
    transition_model = 0;
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

void *GetContextTree(char *i_specifier, int *o_err_code);
{
    *o_err_code = OK;
    kaldi::ContextDependency* ctx_dep; // the tree.
    try
    {
        ctx_dep = new ContextDependency(); 
        kaldi::ReadKaldiObject(i_specifier, ctx_dep);
    }
    catch(...)
    {
        *o_err_code = ERROR_OPENING;
        return;
    }
    return ctx_dep;
}

void DeleteContextTree(void *o_context_tree)
{
    if(o_context_tree)
        delete o_context_tree;
}

int *ReadIntegerVector(char *i_specifier, int *o_n_elements, int *o_err_code)
{
    *o_err_code = OK;
    vector<int32> tmp;
    if (!kaldi::ReadIntegerVectorSimple(i_specifier, &disambig_syms))
    {   
        KALDI_ERR << "fstcomposecontext: Could not read disambiguation symbols from "
                  << i_specifier;
        *o_err_code = ERROR_OPENING;
        *o_n_elements = 0;
        return 0;
    }
    *o_n_elements = tmp.size(); 
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

void *GetKaldiFst(char *i_specifier, int *o_err_code)
{ 
    *o_err_code = OK;
    fst::VectorFst<fst::StdArc> *fst = fst::ReadFstKaldi(i_lex_rxfilename);
    if(!fst)
    {
        *o_err_code = ERROR_OPENING;
        return;
    }

    return fst;
}

} //namespace python_data_readers
} //extern "C"
