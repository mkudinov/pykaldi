#include "kaldi-python-feat.h"
#include "feat/wave-reader.h"
#include <iostream>

namespace kaldi_python_feat
{
void *GetMatrixOfDeltaFeatures(void *i_feature_matrix
                               , int i_order
                               , int i_window
                               , int *o_n_rows
                               , int *o_n_columns
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

void *GetWavData(void *io_matrix_wav_data, int i_sampling_rate, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::Matrix<float>* mat_wav_data = static_cast<kaldi::Matrix<float>* >(io_matrix_wav_data);
    kaldi::WaveData* wav_data = new kaldi::WaveData(static_cast<float>(i_sampling_rate), *mat_wav_data);
    return wav_data;
}

void DeleteWavData(void *o_matrix_wav_data)
{
    if(o_matrix_wav_data)
    {
        delete static_cast<kaldi::WaveData* >(o_matrix_wav_data);
    }
    o_matrix_wav_data = 0;
}

void *GetMfccComputer(float i_frame_length_ms
                    , float i_frame_shift_ms
                    , float i_sampling_rate

                    , int i_n_mel_banks
                    , int i_n_cepstral_coefficients

                    , float i_dither_scale
                    , float i_preemph_coeff
                    , bool i_remove_dc_offset

                    , bool i_use_energy
                    , bool i_raw_energy
                    , float i_cepstral_lifter_coefficient
                    , float i_high_freq
                    , float i_low_freq
                    , int *o_err_code)
{
    *o_err_code = OK;
    kaldi::MfccOptions mfcc_opts;

    kaldi::FrameExtractionOptions frame_options;
    frame_options.samp_freq = i_sampling_rate;
    frame_options.frame_shift_ms = i_frame_shift_ms;
    frame_options.frame_length_ms = i_frame_length_ms;
    frame_options.dither = i_dither_scale;
    frame_options.preemph_coeff = i_preemph_coeff;
    frame_options.remove_dc_offset = i_remove_dc_offset;
    frame_options.window_type = "povey";
    frame_options.round_to_power_of_two = true;

    kaldi::MelBanksOptions mel_bank_options;
    mel_bank_options.num_bins = i_n_mel_banks;
    mel_bank_options.low_freq = i_low_freq;
    mel_bank_options.high_freq = i_high_freq;

    mfcc_opts.num_ceps = i_n_cepstral_coefficients;
    mfcc_opts.use_energy = i_use_energy;
    mfcc_opts.raw_energy = i_raw_energy;

    mfcc_opts.mel_opts = mel_bank_options;
    mfcc_opts.frame_opts = frame_options;

    kaldi::Mfcc *mfcc_computer = new kaldi::Mfcc(mfcc_opts);
    return mfcc_computer;
}

void *ComputeMfcc(void *i_waveform
                , int i_channel
                , bool i_subtract_mean
                , float i_vtln_warp_local
                , void *i_mfcc_computer
                , int *o_n_features
                , int *o_n_frames
                , int *o_err_code)
{
    *o_err_code = OK;
    kaldi::WaveData *wave_data = static_cast<kaldi::WaveData *>(i_waveform);
    kaldi::Mfcc *mfcc_computer = static_cast<kaldi::Mfcc *>(i_mfcc_computer);
    kaldi::SubVector<kaldi::BaseFloat> waveform(wave_data->Data(), i_channel);
    kaldi::Matrix<kaldi::BaseFloat> *features = new kaldi::Matrix<kaldi::BaseFloat>();
    try
    {
        mfcc_computer->Compute(waveform, i_vtln_warp_local, features, NULL);
        *o_n_features = features->NumCols();
        *o_n_frames = features->NumRows();
        if (i_subtract_mean) {
        kaldi::Vector<kaldi::BaseFloat> mean(features->NumCols());
        mean.AddRowSumMat(1.0, *features);
        mean.Scale(1.0 / features->NumRows());
        for (int32 i = 0; i < features->NumRows(); i++)
          features->Row(i).AddVec(-1.0, mean);
      }
    }
    catch (...)
    {
        KALDI_WARN << "Failed to compute features for utterance.";
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    return features;
}

} //namespace kaldi_python_feat