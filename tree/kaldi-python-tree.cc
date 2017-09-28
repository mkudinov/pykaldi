#include "kaldi-python-tree.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_tree
{
void *GetContextDependency(char *i_path_to_tree, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::ContextDependency *tree = new kaldi::ContextDependency();
    kaldi::ReadKaldiObject(i_path_to_tree, tree);
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

int GetContextWidth(void *i_tree, int *o_err_code)
{
    if(!i_tree)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int context_length = 0;
    try
    {
        context_length = static_cast<kaldi::ContextDependency*>(i_tree)->ContextWidth();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return context_length;
}

int GetCentralPosition(void *i_tree, int *o_err_code)
{
    if(!i_tree)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int central_position = 0;
    try
    {
        central_position = static_cast<kaldi::ContextDependency*>(i_tree)->CentralPosition();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return central_position;
}
} //namespace kaldi_python_tree
} //extern "C"