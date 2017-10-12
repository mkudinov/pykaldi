// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_UTILITIES
#define KALDI_PYTHON_UTILITIES

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../matrix/kaldi-matrix.h"
#include "../gmm/am-diag-gmm.h"
#include "../feat/feature-functions.h"

using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_readers 
{
/* IntegerVector*/
int *ReadIntegerVector(char *i_specifier
                     , int *o_n_elements
                     , int *o_err_code);
void DeleteIntegerVector(int *o_vector);
void CopyIntegerVector(int *i_source
                     , int i_size
                     ,  int *o_destination
                     , int *o_err_code);
void *InitIntegerVector(int *i_source
                     , int i_size
                     , int *o_err_code);
} //namespace kaldi_python_readers
} //extern "C"
#endif
