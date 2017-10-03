// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_TRANSFORM
#define KALDI_PYTHON_TRANSFORM

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/kaldi-io.h"
#include "../util/common-utils.h"
#include "../feat/feature-functions.h"
#include "../transform/cmvn.h"


using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_transform
{
void CmvnTransform(void *i_cmvn_stats_matrix, void *io_feature_matrix, bool i_var_norm, int *o_err_code);
} //namespace kaldi_python_transform
} //extern "C"
#endif