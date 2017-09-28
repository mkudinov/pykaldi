// python/kaldi-python-readers.h
// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_ASR_MODEL
#define KALDI_PYTHON_ASR_MODEL

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/kaldi-io.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../gmm/am-diag-gmm.h"

using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_asr_model
{
/* TransitionModel*/
void *GetTransitionModel(char *i_transition_model_filename
                        , int *o_err_code);

void DeleteTransitionModel(void *o_transition_model);

int GetNumberOfTransitionIds(void *i_transition_model
                            , int *o_err_code);

int GetNumberOfPdfsTM(void *i_transition_model
                     , int *o_err_code);

int GetNumberOfPhones(void *i_transition_model
                     , int *o_err_code);

/* Acoustic model*/
void *GetAcousticModel(char *i_transition_model_filename
                      , int *o_err_code);

void DeleteAcousticModel(void *o_acoustic_model);

int GetNumberOfPdfsAM(void *i_acoustic_model
                     , int *o_err_code);

int GetNumberOfGauss(void *i_acoustic_model
                    , int *o_err_code);

int GetNumberOfGaussInPdf(void *i_acoustic_model
                         , int i_pdf_id
                         , int *o_err_code);

void BoostSilence(void *i_transition_model
                , void *io_acoustic_model
                , int *i_silence_phones
                , int silence_phones_size
                , double i_boost);
} //namespace kaldi_python_asr_model
} //extern "C"
#endif