// python/read-kaldi-data.h
// SRR
// Mikhail Kudinov 2017

#ifndef PYTHON_READ_KALDI_DATA
#define PYTHON_READ_KALDI_DATA

#include <vector>
#include <string>
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../matrix/kaldi-matrix.h"

using std::vector;
using std::string;

extern "C"
{
namespace python_data_readers
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
    INTERNAL_KALDI_ERROR
};
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

/* class AlignmentReader */
void *GetAlignmentReader(char *i_specifier,  int *o_err_code);
void DeleteAlignmentReader(void *o_alignment_reader);
size_t ReadAlignment(char *i_key, void *i_transition_model, void *i_alignment_reader, Alignment *o_alignment_buffer, int *o_err_code);

/* class TransitionModel*/
void *GetTransitionModel(char *i_transition_model_filename, int *o_err_code);
void DeleteTransitionModel(void *o_transition_model);

/* class FeatureReader*/
void *GetFeatureReader(char *i_specifier, int *o_err_code);
void DeleteFeatureReader(void *o_feature_reader);

/* class FeatureMatrix */
//Non-possessive pointer to feature matrix
const void *ReadFeatureMatrix(char *i_key, void *i_feature_reader, int *o_n_rows, int *o_n_columns, int *o_err_code);
void CopyFeatureMatrix(void *i_source, void *o_destination, int *o_err_code);

/* class ResultBuffer*/
Alignment *GetResultBuffer(int *o_err_code);
void DeleteResultBuffer(Alignment *o_alignment_buffer);

/* class ContextTree*/
void *GetContextTree(char *i_specifier, int *o_err_code);
void DeleteContextTree(void *o_context_tree);

/* class IntegerVector*/
int *ReadIntegerVector(char *i_specifier, int *o_n_elements, int *o_err_code);
void DeleteIntegerVector(int *o_vector);
void CopyIntegerVector(int *i_source, int i_size, void *o_destination, int *o_err_code);

void *GetKaldiFst(char *i_specifier, int *o_err_code);   
void DeleteKaldiFst(void *fst);
} //namespace python_data_readers
} //extern "C"
#endif
