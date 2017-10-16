#include "kaldi-python-fst.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_fst
{
void *GetFst(char *i_path_to_fst, int *o_err_code)
{
    *o_err_code = OK;
    fst::VectorFst<fst::StdArc> *fst = fst::ReadFstKaldi(i_path_to_fst);
    if(!fst)
    {
        *o_err_code = ERROR_OPENING;
        return 0;
    }
    return fst;
}

void DeleteFst(void *o_fst)
{
    if(o_fst)
    {
        delete static_cast<fst::VectorFst<fst::StdArc>*>(o_fst);
        o_fst = 0;
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

void *GetFstReader(char *i_specifier, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::RandomAccessTableReader<fst::VectorFstHolder> *fst_reader = new kaldi::RandomAccessTableReader<fst::VectorFstHolder> (i_specifier);
    fst_reader->HasKey("abc");
    return fst_reader;
}

void *ReadFst(char *i_key, void *i_fst_reader, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::RandomAccessTableReader<fst::VectorFstHolder> *fst_reader = static_cast<kaldi::RandomAccessTableReader<fst::VectorFstHolder> *>(i_fst_reader);
    std::string key(i_key);
    if(!fst_reader->HasKey(key))
    {
        *o_err_code = NO_KEY;
        return 0;
    }
    fst::VectorFst<fst::StdArc> *fst = new fst::VectorFst<fst::StdArc>();
    *fst = fst_reader->Value(key);
    return fst;
}

void DeleteFstReader(void* o_fst_reader)
{
    if(o_fst_reader)
    {
        delete static_cast<kaldi::RandomAccessTableReader<fst::VectorFstHolder> *>(o_fst_reader);
    }
}

} //namespace kaldi_python_fst
} //extern "C"