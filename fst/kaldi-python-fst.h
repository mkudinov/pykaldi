// python/kaldi-python-fst.h
// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_ASR_MODEL
#define KALDI_PYTHON_ASR_MODEL

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../fstext/kaldi-fst-io.h"

using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_fst
{
    void *GetFst(char *i_path_to_fst, int *o_err_code);
    int GetNumberOfArcs(void *i_fst, int *o_err_code);
    void DeleteFst(void *o_fst);
} //namespace kaldi_python_fst
} //extern "C"
#endif