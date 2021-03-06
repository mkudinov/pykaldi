// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_FEAT
#define KALDI_PYTHON_FEAT

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/kaldi-io.h"
#include "../util/common-utils.h"
#include "../feat/feature-functions.h"
#include "../feat/feature-mfcc.h"
#include "../feat/feature-fbank.h"

using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_feat
{
void *GetMatrixOfDeltaFeatures(void *i_feature_matrix
                               , int i_order
                               , int i_window
                               , int* o_n_rows
                               , int* o_n_columns
                               , int *o_err_code);

void *GetWavData(void *io_matrix_wav_data
               , int i_sampling_rate
               , int *o_err_code);

void DeleteWavData(void *o_matrix_wav_data);

void *ComputeMfcc(void *i_waveform
                , int i_channel
                , bool i_subtract_mean
                , float i_vtln_warp_factor
                , void *i_mfcc_computer
                , int *o_n_features
                , int *o_n_frames
                , int *o_err_code);

void *GetMfccComputer(float i_frame_length_ms
                    , float i_frame_shift_ms
                    , float i_sampling_rate

                    , float i_dither_scale
                    , float i_preemph_coeff
                    , bool  i_remove_dc_offset

                    , int i_n_mel_banks
                    , float i_high_freq
                    , float i_low_freq

                    , bool i_use_energy
                    , bool i_raw_energy
                    , int i_n_cepstral_coefficients
                    , float i_cepstral_lifter_coefficient
                    , int *o_err_code);

void DeleteMfccComputer(void *o_mfcc_computer);

void *GetFbankFeaturesComputer( float i_frame_length_ms
                              , float i_frame_shift_ms
                              , float i_sampling_rate

                              , float i_dither_scale
                              , float i_preemph_coeff
                              , bool i_remove_dc_offset

                              , int i_n_mel_banks
                              , float i_high_freq
                              , float i_low_freq

                              , bool i_use_energy
                              , bool i_raw_energy
                              , bool i_use_log_fbank
                              , bool i_use_power
                              , int *o_err_code);

void *ComputeFbankFeatures(void *i_waveform
                , int i_channel
                , bool i_subtract_mean
                , float i_vtln_warp_local
                , void *i_fbank_features_computer
                , int *o_n_features
                , int *o_n_frames
                , int *o_err_code);

void DeleteFbankFeaturesComputer(void *o_fbank_computer);
} //namespace kaldi_python_feat
} //extern "C"
#endif