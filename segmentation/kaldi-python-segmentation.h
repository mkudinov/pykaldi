// python/segmentation/kaldi-python-segmentation.h
// SRR
// Mikhail Kudinov 2017

#ifndef PYTHON_KALDI_SEGMENTATION
#define PYTHON_KALDI_SEGMENTATION

#include "../common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../tree/context-dep.h"
#include "../fstext/fstext-lib.h"
#include "../decoder/training-graph-compiler.h"
#include "../decoder/decoder-wrappers.h"
#include "../decoder/faster-decoder.h"
#include "../tree/context-dep.h"
#include "../gmm/am-diag-gmm.h"
#include "../hmm/hmm-utils.h"
#include "../gmm/decodable-am-diag-gmm.h"
#include "../fstext/fstext-utils.h"
using namespace kaldi_python_common_errors;

extern "C"
{
namespace python_segmentation
{
typedef struct
{
    int number_of_phones;
    int *phones;
    int *num_repeats_per_phone;
} Alignment;

/* AlignmentReader */
void *GetAlignmentReader(char *i_specifier
                      ,  int *o_err_code);
void DeleteAlignmentReader(void *o_alignment_reader);
Alignment *ReadAlignment(char *i_key
                       , void *i_transition_model
                       , void *i_alignment_reader
                       , int *o_err_code);

/* Main function*/
Alignment *Align(void *i_features
               , void *i_transition_model
               , void *i_acoustic_model
               , void *i_aligner_fst
               , float *o_likelihood
               , int   *o_n_retries
               , int   *o_n_frames_ready
               , float i_acoustic_scale
               , float i_transition_scale
               , float i_self_loop_scale
               , float i_beam
               , float i_retry_beam
               , bool i_careful
               , int *o_err_code);

void DeleteAlignment(Alignment *o_alignment_buffer);
} //namespace python_segmantation
} //extern "C"
#endif
