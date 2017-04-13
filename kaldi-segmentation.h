// python/kaldi-segmentation.h
// SRR
// Mikhail Kudinov 2017

#ifndef PYTHON_KALDI_SEGMENTATION
#define PYTHON_KALDI_SEGMENTATION

#include "kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../tree/context-dep.h"
#include "../fstext/fstext-lib.h"
#include "../decoder/training-graph-compiler.h"
#include "../tree/context-dep.h"
using namespace kaldi_python_common;

extern "C"
{
namespace python_segmentation
{
/*Fst compiler to assemble aligner for phrase*/
void* GetTextFstCompiler(void *i_context_tree
                       , int *i_disambiguation_symbols
                       , int i_size_of_disambiguation_symbols
                       , void *i_transition_model
                       , char *i_lex_in_filename
                       , int *o_err_code);
void DeleteTextFstCompiler(void *o_compiler);

/*Fst for the phrase*/
void *GetAlignerFst(void *i_text_fst_compiler
                  , int *i_transcript
                  , int i_transcript_len
                  , int *o_err_code);
void DeleteAlignerFst(void *o_aligner_fst);
/* Main function*/
Alignment *Align(void *i_features
               , void *i_transition_model
               , void* i_acoustic_model
               , void *i_aligner_fst
               , float i_acoustic_scale
               , float i_transition_scale
               , float i_self_loop_scale
               , float i_beam
               , float i_retry
               , int *o_err_code);
}
}
#endif
