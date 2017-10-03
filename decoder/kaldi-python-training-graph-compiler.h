// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_TRAINING_GRAPH_COMPILER
#define KALDI_PYTHON_TRAINING_GRAPH_COMPILER

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../decoder/training-graph-compiler.h"
using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_training_graph_compiler
{
    void *GetTrainingGraphCompiler( void   *i_transition_model
                                  , void   *i_context_dependency
                                  , void   *io_lexical_fst
                                  , int    *i_disambiguation_symbols
                                  , int     i_disambiguation_symbols_length
                                  , double  i_option_transition_scale
                                  , double  i_option_self_loop_scale
                                  , bool    i_option_reorder
                                  , int    *o_err_code
                                  );

    void *CompilePhraseGraphFromText( void *i_graph_compiler
                                    , int  *i_transcript
                                    , int   i_transcript_length
                                    , int  *o_err_code
                                    );

    void DeleteTrainingGraphCompiler(void *o_graph_compiler);
} //namespace kaldi_python_training_graph_compiler
} //extern "C"
#endif