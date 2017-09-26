// python/kaldi-python-readers.h
// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_READERS
#define KALDI_PYTHON_READERS

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../matrix/kaldi-matrix.h"
#include "../gmm/am-diag-gmm.h"
#include "../feat/feature-functions.h"


using kaldi_python_common::Alignment;
using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_readers 
{
/* AlignmentReader */
void *GetAlignmentReader(char *i_specifier
                      ,  int *o_err_code);
void DeleteAlignmentReader(void *o_alignment_reader);
Alignment *ReadAlignment(char *i_key
                       , void *i_transition_model
                       , void *i_alignment_reader
                       , int *o_err_code);

/* TransitionModel*/
void *GetTransitionModel(char *i_transition_model_filename
                       , int *o_err_code);
void DeleteTransitionModel(void *o_transition_model);

/* Acoustic model*/
void *GetAcousticModel(char *i_transition_model_filename
                     , int *o_err_code);
void DeleteAcousticModel(void *o_amm_gmm);

/* ContextTree*/
void *GetContextTree(char *i_specifier
                   , int *o_err_code);
void DeleteContextTree(void *o_context_tree);

/* FeatureReader*/
void *GetFeatureReader(char *i_specifier
                     , int *o_err_code);
void DeleteFeatureReader(void *o_feature_reader);

/* FeatureMatrix */
//Non-possessive pointer to feature matrix
const void *ReadFeatureMatrix(char *i_key
                            , void *i_feature_reader
                            , int *o_n_rows
                            , int *o_n_columns
                            , int *o_err_code);
void CopyFeatureMatrix(void *i_source
                     , void *o_destination
                     , int *o_err_code);

void AddDeltaFeatures(void *i_feature_matrix
                    , int i_n_rows
                    , int i_n_columns
                    , void *o_delta_feature_matrix
                    , int *o_n_rows
                    , int *o_n_columns
                    , int *o_err_code);

/* IntegerVector*/
int *ReadIntegerVector(char *i_specifier
                     , int *o_n_elements
                     , int *o_err_code);
void DeleteIntegerVector(int *o_vector);
void CopyIntegerVector(int *i_source
                     , int i_size
                     , void *o_destination
                     , int *o_err_code);
//from common.h
void DeleteAlignment(Alignment *o_alignment_buffer);
} //namespace kaldi_python_readers
} //extern "C"
#endif
