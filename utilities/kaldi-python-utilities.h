// python/kaldi-python-readers.h
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

/* ContextTree*/
void *GetContextTree(char *i_specifier
                   , int *o_err_code);
void DeleteContextTree(void *o_context_tree);

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
