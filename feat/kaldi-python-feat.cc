#include "kaldi-python-feat.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_feat
{
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

} //namespace kaldi_python_feat
} //extern "C"