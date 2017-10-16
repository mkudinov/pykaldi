#include "kaldi-python-segmentation.h"

extern "C"
{
namespace python_segmentation
{
void *GetAlignmentReader(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    std::string specifier(i_specifier);
    kaldi::RandomAccessInt32VectorReader *alignment_reader = new kaldi::RandomAccessInt32VectorReader(specifier);
    alignment_reader->HasKey("abc");
    return alignment_reader;
}

void DeleteAlignmentReader(void *o_alignment_reader)
{
    if(o_alignment_reader)
    {
        delete static_cast<kaldi::RandomAccessInt32VectorReader*>(o_alignment_reader);
        o_alignment_reader = 0;
    }
}

Alignment *CreateAlignmentBuffer(int i_number_of_phones, int *o_err_code)
{
    *o_err_code = OK;
    Alignment *alignment_buffer = new Alignment;
    if(!alignment_buffer)
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
        return 0;
    }
    alignment_buffer->number_of_phones = i_number_of_phones;
    try
    {
        alignment_buffer->num_repeats_per_phone = new int[i_number_of_phones];
        alignment_buffer->phones = new int[i_number_of_phones];
    }
    catch(...)
    {
        *o_err_code = MEMORY_ALLOCATION_ERROR;
    }
    return alignment_buffer;
}

void DeleteAlignmentBuffer(Alignment *o_alignment_buffer)
{
    if(o_alignment_buffer)
    {
        if(o_alignment_buffer->phones)
        {
            delete[] o_alignment_buffer->phones;
        }
        if(o_alignment_buffer->num_repeats_per_phone)
        {
            delete[] o_alignment_buffer->num_repeats_per_phone;
        }
    }
    o_alignment_buffer->number_of_phones = 0;
    delete o_alignment_buffer;
}

Alignment *ReadAlignment(char *i_key, void *i_transition_model, void *i_alignment_reader, int *o_err_code)
{
    *o_err_code = OK;
    if(!i_alignment_reader || !i_transition_model)
    {
        *o_err_code = NO_READER_EXISTS;
        return 0;
    }
    kaldi::RandomAccessInt32VectorReader* alignment_reader = static_cast<kaldi::RandomAccessInt32VectorReader*>(i_alignment_reader);
    kaldi::TransitionModel *transition_model = static_cast<kaldi::TransitionModel*>(i_transition_model);
    std::string key(i_key);
    if(!alignment_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    std::vector<vector<int32> > split;
    std::vector<int32> alignment = alignment_reader->Value(key);
    kaldi::SplitToPhones(*transition_model, alignment, &split);
    Alignment *result = CreateAlignmentBuffer(split.size(), o_err_code);
    if(*o_err_code != OK)
        return 0;
    try
    {
        for(size_t i = 0; i < result->number_of_phones; ++i)
        {
            int phone = transition_model->TransitionIdToPhone(split[i][0]);
            int num_repeats = split[i].size();
            result->phones[i] = phone;
            result->num_repeats_per_phone[i] = num_repeats;
        }
    }
    catch(...)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    return result;
}

void DeleteAlignment(Alignment *o_alignment_buffer)
{
    DeleteAlignmentBuffer(o_alignment_buffer);
}

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
               , int *o_err_code)
{ 
    *o_err_code = OK;
    kaldi::Matrix<kaldi::BaseFloat> *features = static_cast<kaldi::Matrix<kaldi::BaseFloat>*>(i_features);
    fst::VectorFst<fst::StdArc> *aligner_fst = static_cast<fst::VectorFst<fst::StdArc>*>(i_aligner_fst);
    kaldi::TransitionModel *transition_model = static_cast<kaldi::TransitionModel *>(i_transition_model); 
    kaldi::AmDiagGmm* acoustic_model = static_cast<kaldi::AmDiagGmm*>(i_acoustic_model);
    *o_n_retries = 0;
    if (features->NumRows() == 0) 
    {
        KALDI_WARN << "Zero-length utterance";
        *o_err_code = WRONG_INPUT;
        return 0;
    }
    {  // Add transition-probs to the FST.
        std::vector<int32> disambig_syms;  // empty.
        kaldi::AddTransitionProbs(*transition_model, disambig_syms, i_transition_scale, i_self_loop_scale, aligner_fst);
    }
    kaldi::DecodableAmDiagGmmScaled decodable(*acoustic_model, *transition_model, *features,
                                               i_acoustic_scale);
    fst::StdArc::Label special_symbol = 0;

    if (i_careful)
    {
        kaldi::ModifyGraphForCarefulAlignment(aligner_fst);
    }
    kaldi::FasterDecoderOptions decode_opts;
    decode_opts.beam = i_beam;

    kaldi::FasterDecoder decoder(*aligner_fst, decode_opts);
    decoder.Decode(&decodable);

    bool ans = decoder.ReachedFinal();  // consider only final states.

    if (!ans && i_retry_beam != 0.0)
    {
        (*o_n_retries)++;
        KALDI_WARN << "Retrying utterance with beam "
                   << i_retry_beam;
        decode_opts.beam = i_retry_beam;
        decoder.SetOptions(decode_opts);
        decoder.Decode(&decodable);
        ans = decoder.ReachedFinal();
    }

    fst::VectorFst<kaldi::LatticeArc> decoded;  // linear FST.
    decoder.GetBestPath(&decoded);
    if (decoded.NumStates() == 0)
    {
        KALDI_WARN << "Error getting best path from decoder (likely a bug)";
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }

    std::vector<int32> alignment;
    std::vector<int32> words;
    kaldi::LatticeWeight weight;
    GetLinearSymbolSequence(decoded, &alignment, &words, &weight);
    std::vector<vector<int32> > split;
    kaldi::SplitToPhones(*transition_model, alignment, &split);
    Alignment *result = CreateAlignmentBuffer(split.size(), o_err_code);
    if(*o_err_code != OK)
        return 0;
    try
    {
        for(size_t i = 0; i < result->number_of_phones; ++i)
        {
            int phone = transition_model->TransitionIdToPhone(split[i][0]);
            int num_repeats = split[i].size();
            result->phones[i] = phone;
            result->num_repeats_per_phone[i] = num_repeats;
        }
    }
    catch(...)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    *o_likelihood = -(weight.Value1()+weight.Value2()) / i_acoustic_scale;
    *o_n_frames_ready = decodable.NumFramesReady();
    return result;
}

} //namespace python_segmentation
} //extern "C"
