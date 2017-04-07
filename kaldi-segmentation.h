// python/kaldi-segmentation.h
// SRR
// Mikhail Kudinov 2017

#ifndef PYTHON_KALDI_SEGMENTATION
#define PYTHON_KALDI_SEGMENTATION

#include <vector>
#include <string>
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../hmm/transition-model.h"
#include "../tree/context-dep.h"
#include "../fstext/fstext-lib.h"
#include "../decoder/training-graph-compiler.h"

using std::vector;
using std::string;
typedef void* p_ContextTree;
typedef int* p_DisambiguationSymbols;
typedef void* p_TransitionModel;
typedef void* p_VectorFst;

extern "C"
{
namespace python_segmentation
{
enum
{
    OK,
    ERROR_OPENING,
    NO_KEY,
    INDEX_OUT_OF_BOUNDS,
    NO_READER_EXIST
};
/*VARIABLES TO STORE TEMPORARY RESULTS*/

/* INTERFACE FUNCTION */
void GetAligner(p_ContextTree i_context_tree, 
                p_DisambiguationSymbols i_disambiguation_symbols, 
                int i_size_of_disambiguation_symbols,
                p_TransitionModel i_transition_model, 
                p_VectorFst i_lex_fst, 
                int *o_err_code);
//size_t GetAlignment(char * i_key, int* o_err_code);
/* TRANSFER TO PYTHON */
//int GetNextPhone(int i_index, int* o_err_code);
//int GetNextLength(int i_index, int* o_err_code);

/* CLEAR MEMORY */
//void DeleteAligner();
//void ClearTempVariables();
}
}
#endif
