#include "kaldi-python-fst.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_tree
{
void *GetContextDependency(char *i_path_to_tree, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::ContextDependency *tree = kaldi::ReadKaldiObject(i_path_to_tree);
    if(!tree)
    {
        *o_err_code = ERROR_OPENING;
        return 0;
    }
    return tree;
}

void DeleteContextDependency(void *o_tree)
{
    if(o_tree)
    {
        delete static_cast<kaldi::ContextDependency*>(o_tree);
        o_tree = 0;
    }
}

int GetNumberOfArcs(void *i_fst, int *o_err_code)
{
    if(!i_fst)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int number_of_arcs = 0;
    try
    {
        fst::VectorFst<fst::StdArc> *fst = static_cast<fst::VectorFst<fst::StdArc>*>(i_fst);
        for (int s = 0; s < fst->NumStates(); s++)
        {
            number_of_arcs += fst->NumArcs(s);
        }
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return number_of_arcs;
}
} //namespace kaldi_python_tree
} //extern "C"