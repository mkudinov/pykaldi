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
    kaldi::AmDiagGmm* acoustic_model = new kaldi::AmDiagGmm();
    {
        bool binary;
        kaldi::Input ki(model_filename, &binary);
        transition_model.Read(ki.Stream(), binary);
        acoustic_model->Read(ki.Stream(), binary);
    }
    if(!acoustic_model)
    {
        *o_err_code = ERROR_OPENING;
        return 0;
    }
    return acoustic_model;
}

void DeleteAcousticModel(void *o_acoustic_model)
{
    if(o_acoustic_model )
    {
        delete static_cast<kaldi::AmDiagGmm*>(o_acoustic_model);
        o_acoustic_model = 0;
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

void BoostSilence(void *i_transition_model, void *io_acoustic_model, int *i_silence_phones, int silence_phones_size, double i_boost, int *o_err_code)
{
    *o_err_code = OK;
    std::vector<int> silence_phones(silence_phones_size);
    for(size_t i = 0; i < silence_phones_size; i++)
    {
        silence_phones[i] = i_silence_phones[i];
    }

    std::sort(silence_phones.begin(), silence_phones.end());
    std::vector<int> pdfs;
    bool ans = GetPdfsForPhones(*static_cast<kaldi::TransitionModel*>(i_transition_model), silence_phones, &pdfs);
    if (!ans)
    {
        KALDI_WARN << "The pdfs for the silence phones may be shared by other phones "
                   << "(note: this probably does not matter.)";
    }
    for (size_t i = 0; i < pdfs.size(); i++)
    {
        int pdf = pdfs[i];
        kaldi::DiagGmm &gmm = static_cast<kaldi::AmDiagGmm*>(io_acoustic_model)->GetPdf(pdf);
        kaldi::Vector<kaldi::BaseFloat> weights(gmm.weights());
        weights.Scale(i_boost);
        gmm.SetWeights(weights);
        gmm.ComputeGconsts();
    }
}
} //namespace kaldi_python_asr_model
} //extern "C"