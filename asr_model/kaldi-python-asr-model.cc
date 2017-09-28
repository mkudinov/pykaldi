#include "kaldi-python-asr-model.h"
#include <iostream>
extern "C"
{
namespace kaldi_python_asr_model
{
void *GetTransitionModel(char *i_asr_filename, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::TransitionModel* transition_model = new kaldi::TransitionModel();
    kaldi::ReadKaldiObject(i_asr_filename, transition_model);
    if(!transition_model)
    {
        *o_err_code = ERROR_OPENING;
        return 0;
    }
    return transition_model;
}

void DeleteTransitionModel(void *o_transition_model)
{
    if(o_transition_model)
    {
        delete static_cast<kaldi::TransitionModel*>(o_transition_model);
        o_transition_model = 0;
    }
}

int GetNumberOfTransitionIds(void *i_transition_model, int *o_err_code)
{
    if(!i_transition_model)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int numbert_of_transition_ids = 0;
    try
    {
        numbert_of_transition_ids = static_cast<kaldi::TransitionModel*>(i_transition_model)->NumTransitionIds();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return numbert_of_transition_ids;
}

int GetNumberOfPdfsTM(void *i_transition_model, int *o_err_code)
{
    if(!i_transition_model)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int number_of_pdfs = 0;
    try
    {
        number_of_pdfs = static_cast<kaldi::TransitionModel*>(i_transition_model)->NumPdfs();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return number_of_pdfs;
}

int GetNumberOfPhones(void *i_transition_model, int *o_err_code)
{
    if(!i_transition_model)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int number_of_phones = 0;
    try
    {
        number_of_phones = static_cast<kaldi::TransitionModel*>(i_transition_model)->NumPhones();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return number_of_phones;
}

void *GetAcousticModel(char *i_asr_model_filename, int *o_err_code)
{
    *o_err_code = OK;
    kaldi::TransitionModel transition_model;
    std::string model_filename(i_asr_model_filename);
    kaldi::AmDiagGmm* am_gmm = new kaldi::AmDiagGmm();
    {
        bool binary;
        kaldi::Input ki(model_filename, &binary);
        transition_model.Read(ki.Stream(), binary);
        am_gmm->Read(ki.Stream(), binary);
    }
    if(!am_gmm)
    {
        *o_err_code = ERROR_OPENING;
        return 0;
    }
    return am_gmm;
}

void DeleteAcousticModel(void *o_am_gmm)
{
    if(o_am_gmm )
    {
        delete static_cast<kaldi::AmDiagGmm*>(o_am_gmm);
        o_am_gmm = 0;
    }
}

int GetNumberOfPdfsAM(void *i_acoustic_model, int *o_err_code)
{
    if(!i_acoustic_model)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int number_of_pdfs = 0;
    try
    {
        number_of_pdfs = static_cast<kaldi::AmDiagGmm*>(i_acoustic_model)->NumPdfs();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return number_of_pdfs;
}

int GetNumberOfGauss(void *i_acoustic_model, int *o_err_code)
{
    if(!i_acoustic_model)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int number_of_gaussian = 0;
    try
    {
        number_of_gaussian = static_cast<kaldi::AmDiagGmm*>(i_acoustic_model)->NumGauss();
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return number_of_gaussian;
}

int GetNumberOfGaussInPdf(void *i_acoustic_model, int i_pdf_id, int *o_err_code)
{
    if(!i_acoustic_model)
    {
        *o_err_code = CORRUPTED_POINTER;
        return 0;
    }
    *o_err_code = OK;
    int number_of_gaussian = 0;
    try
    {
        number_of_gaussian = static_cast<kaldi::AmDiagGmm*>(i_acoustic_model)->NumGaussInPdf(i_pdf_id);
    }
    catch(...)
    {
        *o_err_code = CORRUPTED_POINTER;
    }
    return number_of_gaussian;
}
} //namespace kaldi_python_asr_model
} //extern "C"