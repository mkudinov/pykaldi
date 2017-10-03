// python/kaldi-python-readers.h
// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_FEAT
#define KALDI_PYTHON_FEAT

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/kaldi-io.h"
#include "../util/common-utils.h"
#include "../feat/feature-functions.h"


using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_feat
{
void *GetFeatureReader(char *i_specifier, int *o_err_code);

void DeleteFeatureReader(void *o_feature_reader);

//Non-possessive pointer to feature matrix
const void* ReadFeatureMatrix(char* i_key, void *i_feature_reader, int* o_n_rows, int* o_n_columns, int *o_err_code);

void CopyFeatureMatrix(void *i_source, void *o_destination, int *o_err_code);

void *GetMatrixOfDeltaFeatures(void *i_feature_matrix, int i_order, int i_window, int *o_err_code);
} //namespace kaldi_python_feat
} //extern "C"
#endif