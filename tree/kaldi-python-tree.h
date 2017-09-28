// python/kaldi-python-fst.h
// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_TREE
#define KALDI_PYTHON_TREE

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../util/common-utils.h"
#include "../fstext/kaldi-fst-io.h"

using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_tree
{
    void *GetContextDependency(char *i_path_to_tree, int *o_err_code);
    int GetNumberOfArcs(void *i_fst, int *o_err_code);
    void DeleteContextDependency(void *o_tree);
} //namespace kaldi_python_tree
} //extern "C"
#endif