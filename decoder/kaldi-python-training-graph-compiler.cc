#include "kaldi-python-training-graph-compiler.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_training_graph_compiler
{
void *GetTrainingGraphCompiler( void    *i_transition_model
                              , void   *i_context_dependency
                              , void   *io_lexical_fst
                              , int    *i_disambiguation_symbols
                              , int     i_disambiguation_symbols_length
                              , double  i_option_transition_scale
                              , double  i_option_self_loop_scale
                              , bool    i_option_reorder
                              , int    *o_err_code
                              )
{
    *o_err_code = OK;
    kaldi::TrainingGraphCompilerOptions gopts(i_option_transition_scale, i_option_self_loop_scale, i_option_reorder);
    std::vector<int> disambiguation_symbols(i_disambiguation_symbols_length);
    for(int i = 0; i < i_disambiguation_symbols_length; i++)
    {
        disambiguation_symbols[i] = i_disambiguation_symbols[i];
    }
    kaldi::TrainingGraphCompiler *tgc = new kaldi::TrainingGraphCompiler(
                                          *static_cast<kaldi::TransitionModel*>(i_transition_model)
                                        , *static_cast<kaldi::ContextDependency*>(i_context_dependency)
                                        , static_cast<fst::VectorFst<fst::StdArc>*>(io_lexical_fst)
                                        , disambiguation_symbols
                                        , gopts
                                        );
    if(!tgc)
    {
        *o_err_code = ERROR_OPENING;
        return 0;
    }
    io_lexical_fst = 0;
    return tgc;
}

void DeleteTrainingGraphCompiler(void *o_graph_compiler)
{
    if(o_graph_compiler)
    {
        delete static_cast<kaldi::TrainingGraphCompiler *>(o_graph_compiler);
    }
    o_graph_compiler = 0;
}

void *CompilePhraseGraphFromText( void *i_graph_compiler
                                , int  *i_transcript
                                , int   i_transcript_length
                                , int  *o_err_code
                                )
{
    *o_err_code = OK;
    std::vector<int> transcript(i_transcript_length);
    for(int i = 0; i < i_transcript_length; i++)
    {
        transcript[i] = i_transcript[i];
    }
    fst::VectorFst<fst::StdArc> *decode_fst = new fst::VectorFst<fst::StdArc>();
    static_cast<kaldi::TrainingGraphCompiler *>(i_graph_compiler)->CompileGraphFromText(transcript, decode_fst);
    if(!decode_fst)
    {
        *o_err_code = INTERNAL_KALDI_ERROR;
        return 0;
    }
    return decode_fst;
}
} //namespace kaldi_python_training_graph_compiler
} //extern "C"