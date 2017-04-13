#include "kaldi-segmentation.h"

extern "C"
{
namespace python_segmentation
{
void* GetTextFstCompiler(void *i_context_tree, 
                int *i_disambiguation_symbols, 
                int i_size_of_disambiguation_symbols,
                void *i_transition_model, 
                char *i_lex_in_filename, 
                int *o_err_code)
{
    *o_err_code = OK;
    kaldi::ContextDependency *context_tree = static_cast<kaldi::ContextDependency*>(i_context_tree);  
    kaldi::TransitionModel *transition_model = static_cast<kaldi::TransitionModel *>(i_transition_model); 
    kaldi::TrainingGraphCompiler *gc;
    std::vector<int32> disambig_syms(i_disambiguation_symbols, i_disambiguation_symbols + i_size_of_disambiguation_symbols);     
    kaldi::TrainingGraphCompilerOptions gopts;
    gopts.transition_scale = 0.0; 
    gopts.self_loop_scale = 0.0; 
    try
    {
        // ownership will be taken by gc.
        fst::VectorFst<fst::StdArc> *lex_fst = fst::ReadFstKaldi(i_lex_in_filename);
        gc = new kaldi::TrainingGraphCompiler(i_transition_model, i_context_tree, lex_fst, disambig_syms, gopts);
    }
    catch(...)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    return gc;
}

void DeleteTextFstCompiler(void *o_compiler)
{
    if(o_compiler)
    {
        delete static_cast<kaldi::TrainingGraphCompiler*>(o_compiler);
        o_compiler = 0;
    }
}

void *GetAlignerFst(void *i_text_fst_compiler, int *i_transcript, int i_transcript_len, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::TrainingGraphCompiler* compiler = static_cast<kaldi::TrainingGraphCompiler*>(i_text_fst_compiler);
    fst::VectorFst<StdArc> *aligner_fst = new fst::VectorFst<StdArc>(); 
    vector<int32> transcript(i_transcript, i_transcript + i_transcript_len);
    if (!compiler->CompileGraphFromText(transcript, aligner_fst)) 
    {
        aligner_fst->DeleteStates();  // Just make it empty.
    }
    if (aligner_fst->Start() == fst::kNoStateId) 
    {
        KALDI_ERR << "Empty decoding graph for utterance ";
        delete aligner_fst;
        *o_err_code = WRONG_INPUT;
        return 0;
    }
    return aligner_fst;
}

void DeleteAlignerFst(void *o_aligner_fst)
{
    if (o_aligner_fst)
    {
        delete static_cast<fst::VectorFst<StdArc>*>(o_aligner_fst); 
        o_aligner_fst = 0;
    }
}

Alignment *Align(void *i_features
               , void *i_transition_model
               , void* i_acoustic_model
               , void *i_aligner_fst
               , float i_acoustic_scale
               , float i_transition_scale
               , float i_self_loop_scale
               , float i_beam,
               , float i_retry
               , int *o_err_code)
{ 
    *o_err_code = OK;
    Matrix<BaseFloat> *features = static_cast<Matrix<BaseFloat>*>(i_features);
    fst::VectorFst<StdArc> *aligner_fst = static_cast<fst::VectorFst<StdArc>*>(i_aligner_fst);
    kaldi::TransitionModel *transition_model = static_cast<kaldi::TransitionModel *>(i_transition_model); 
    kaldi::AmDiagGmm* acoustic_model = static_cast<kaldi::AmDiagGmm*>(i_acoustic_model);
    if (features->NumRows() == 0) 
    {
        KALDI_WARN << "Zero-length utterance: " << utt;
        num_err++;
        *o_err_code = WRONG_INPUT;
        return 0;
    }

   // BaseFloat acoustic_scale = 1.0;
   // BaseFloat transition_scale = 1.0;
  //  BaseFloat self_loop_scale = 1.0;
//TODO: Maybe AddTransitionProbs is not necessary
    {// Add transition-probs to the FST.
     std::vector<int32> disambig_syms;  // empty.
     AddTransitionProbs(*transition_model, disambig_syms,
                             i_transition_scale, i_self_loop_scale,
                             *aligner_fst);
    }
    DecodableAmDiagGmmScaled gmm_decodable(*acoustic_model, *transition_model, *features,
                                               i_acoustic_scale);
    o_num_error = 0;
    if ((i_retry_beam != 0 && i_retry_beam <= i_beam) || i_beam <= 0.0) 
    {
        KALDI_ERR << "Beams do not make sense: beam " << i_config.beam
                                  << ", retry-beam " << i_config.retry_beam;
        *o_err_code = KALDI_WRONG_CONFIGURATION;
        return 0;
    }
      
    if (fst->Start() == fst::kNoStateId) 
    {
        KALDI_WARN << "Empty decoding graph";
        *o_err_code = WRONG_INPUT;
        return 0;
    }

    fst::StdArc::Label special_symbol = 0;
       if (i_careful)
                ModifyGraphForCarefulAlignment(fst);
        
    FasterDecoderOptions decode_opts;
    decode_opts.beam = i_beam;
    FasterDecoder decoder(*fst, decode_opts);
    decoder.Decode(decodable);                
    bool ans = decoder.ReachedFinal();  // consider only final states.
                         
    if (!ans && i_retry_beam != 0.0) 
    {
        if (num_retried != NULL) (*num_retried)++;
             KALDI_WARN << "Retrying utterance " << utt << " with beam " << config.retry_beam;
        decode_opts.beam = i_retry_beam;
        decoder.SetOptions(decode_opts);
        decoder.Decode(decodable);
        ans = decoder.ReachedFinal();
    }
                          
    if (!ans) 
    {  // Still did not reach final state.
        KALDI_WARN << "Did not successfully decode file " << utt << ", len = "
                            << decodable->NumFramesReady();
        if (num_error != NULL) (*num_error)++;
            return;
    }
                               
    fst::VectorFst<LatticeArc> decoded;  // linear FST.
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
    BaseFloat like = -(weight.Value1()+weight.Value2()) / acoustic_scale;
    std::vector<std::vector<int32> > split;
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

} //namespace python_segmentation
} //extern "C"
