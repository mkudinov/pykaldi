#include "kaldi-segmentation.h"


extern "C"
{
namespace python_segmentation
{
void GetAligner(char *i_tree_rxfilename, char *i_disambig_rxfilename, char *i_model_rxfilename, char *i_lex_rxfilename, char *i_fsts_wspecifier, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::TrainingGraphCompilerOptions gopts;
    gopts.transition_scale = 0.0; 
    gopts.self_loop_scale = 0.0; 
    kaldi::ContextDependency ctx_dep;  // the tree.
    kaldi::TransitionModel trans_model;
    try
    {
        kaldi::ReadKaldiObject(i_tree_rxfilename, &ctx_dep);
        kaldi::ReadKaldiObject(i_model_rxfilename, &trans_model);
    }
    catch(...)
    {
        *o_err_code = ERROR_OPENING;
        return;
    }
    // need VectorFst because we will change it by adding subseq symbol.
    fst::VectorFst<fst::StdArc> *lex_fst = fst::ReadFstKaldi(i_lex_rxfilename);
    if(!lex_fst)
    {
        *o_err_code = ERROR_OPENING;
        return;
    }
    vector<int32> disambig_syms;
    if (!kaldi::ReadIntegerVectorSimple(i_disambig_rxfilename, &disambig_syms))
    {   
        KALDI_ERR << "fstcomposecontext: Could not read disambiguation symbols from "
                  << i_disambig_rxfilename;
        *o_err_code = ERROR_OPENING;
        return;
    }
    try
    {
        kaldi::TrainingGraphCompiler gc(trans_model, ctx_dep, lex_fst, disambig_syms, gopts);
    }
    catch(...)
    {
        *o_err_code = COMPILER_CREATION_FAILED;
    }
}

int CreateDecodingGraph(const kaldi::TrainingGraphCompiler& i_compiler, const vector<int32>& i_transcript, fst::VectorFst<StdArc>& o_decode_fst)
{
    if (!i_compiler.CompileGraphFromText(i_transcript, &o_decode_fst)) 
    {
        decode_fst.DeleteStates();  // Just make it empty.
    }
    if (decode_fst.Start() == fst::kNoStateId) 
    {
        KALDI_ERR << "Empty decoding graph for utterance ";
        return GRAPH_CREATION_FAILED;
    }
    return OK;
}

void Align(const Matrix<BaseFloat>& i_features, int *i_transicript, int i_transcript_len, int *o_err_code)
{
    *o_err_code = OK;
    int num_succeed = 0, num_fail = 0;
    vector<int32> transcript(i_transcript_len);
    for(int i = 0; i < i_transcript_len; i++) transcript = i_transicript[i];
    VectorFst<StdArc> decode_fst;
    if(kaldi::CreateDecodingGraph(graph_compiler, transcript, decode_fst) != OK)
    {
        *o_err_code = GRAPH_CREATION_FAILED;
        return;
    }
      
  if ((config.retry_beam != 0 && config.retry_beam <= config.beam) ||
      config.beam <= 0.0) {
    KALDI_ERR << "Beams do not make sense: beam " << config.beam
              << ", retry-beam " << config.retry_beam;
  }

  if (fst->Start() == fst::kNoStateId) {
      KALDI_WARN << "Empty decoding graph for " << utt;
      if (num_error != NULL) (*num_error)++;
      return;
  }


  fst::StdArc::Label special_symbol = 0;
  if (config.careful)
      ModifyGraphForCarefulAlignment(fst);

  FasterDecoderOptions decode_opts;
  decode_opts.beam = config.beam;

  FasterDecoder decoder(*fst, decode_opts);
  decoder.Decode(decodable);

  bool ans = decoder.ReachedFinal();  // consider only final states.
  
  if (!ans && config.retry_beam != 0.0) {
      if (num_retried != NULL) (*num_retried)++;
      KALDI_WARN << "Retrying utterance " << utt << " with beam "
               << config.retry_beam;
      decode_opts.beam = config.retry_beam;
      decoder.SetOptions(decode_opts);
      decoder.Decode(decodable);
      ans = decoder.ReachedFinal();
  }

  if (!ans) {  // Still did not reach final state.
      KALDI_WARN << "Did not successfully decode file " << utt << ", len = "
               << decodable->NumFramesReady();
      if (num_error != NULL) (*num_error)++;
      return;
  }
  
  fst::VectorFst<LatticeArc> decoded;  // linear FST.
  decoder.GetBestPath(&decoded);
  if (decoded.NumStates() == 0) {
      KALDI_WARN << "Error getting best path from decoder (likely a bug)";
      if (num_error != NULL) (*num_error)++;
      return;
  }
    
  std::vector<int32> alignment;
  std::vector<int32> words;
  LatticeWeight weight;

  GetLinearSymbolSequence(decoded, &alignment, &words, &weight);
}

} //namespace python_segmentation
} //extern "C"
