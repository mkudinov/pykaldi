#include "kaldi-python-transform.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_transform
{
void CmvnTransform(void *i_cmvn_stats_matrix, void *io_feature_matrix, bool i_var_norm, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::Matrix<double>* cmvn_stats = static_cast<kaldi::Matrix<double>* >(i_cmvn_stats_matrix);
    kaldi::Matrix<float>* features = static_cast<kaldi::Matrix<float>* >(io_feature_matrix);
    kaldi::ApplyCmvn(*cmvn_stats, i_var_norm, features);
}

} //namespace kaldi_python_asr_model
} //extern "C"