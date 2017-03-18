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
/* INTERFACE FUNCTION */
void *GetAlignmentReader(char *i_specifier,  int *o_err_code);
void *GetTransitionModel(char *i_transition_model_filename, int *o_err_code);
size_t ReadAlignment(char *i_key, void *i_transition_model, void *i_alignment_reader, Alignment *o_alignment_buffer, int *o_err_code);
void *GetFeatureReader(char *i_specifier, int *o_err_code);
const void *ReadFeatureMatrix(char *i_key, void *i_feature_reader, int *o_n_rows, int *o_n_columns, int *o_err_code);
/* TRANSFER TO PYTHON */
Alignment *GetResultBuffer();
void CopyFeatureMatrix(void *i_source, void *o_destination, int *o_err_code);
/* CLEAR MEMORY */
void DeleteTransitionModel(void *o_transition_model);
void DeleteAlignmentReader(void *o_alignment_reader);
void DeleteResultBuffer(Alignment *o_alignment_buffer);
void DeleteFeatureReader(void *o_feature_reader);
}
}
#endif
