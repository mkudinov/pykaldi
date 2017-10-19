// SRR
// Mikhail Kudinov 2017

#ifndef KALDI_PYTHON_FST
#define KALDI_PYTHON_FST

#include "common/kaldi-python-common.h"
#include "../base/kaldi-common.h"
#include "../fstext/kaldi-fst-io.h"
#include "../util/kaldi-table.h"

using namespace kaldi_python_common_errors;
extern "C"
{
namespace kaldi_python_fst
{
    void *GetFst(char *i_path_to_fst, int *o_err_code);
    int GetNumberOfArcs(void *i_fst, int *o_err_code);
    void DeleteFst(void *o_fst);
    void *GetFstReader(char *i_specifier, int *o_err_code);
    void *ReadFst(char *i_key, void *i_fst_reader, int *o_err_code);
    void WriteFst(void *i_fst, char *i_filename, int *o_err_code);
    void DeleteFstReader(void* o_fst_reader);
} //namespace kaldi_python_fst
} //extern "C"
#endif