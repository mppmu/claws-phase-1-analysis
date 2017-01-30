

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>

//root
#include <TFile.h>
#include <TF1.h>


#include "gain.hh"
#include "globalsettings.hh"


Gain::Gain(int run_nr):run_nr_(run_nr)
{
    for(auto & ivec : GS->GetChannels(2))
    {
        std::string title    = "run_" + std::to_string(run_nr_) + "_" + ivec + "_gain";
        std::replace(title.begin(), title.end(), '-','_');
        channels_.push_back(new GainChannel(ivec, new TH1I(title.c_str(), title.c_str(),140,-210 ,2610),0, new std::vector<float>(230,0)));
    }

    // for(auto & v : channel_list_)
    // {
    //     // TODO make the vector initialization proper.
    //     std::string title    = "run_" + std::to_string(run_nr_) + "_" + v + "_gain";
    //     channels_.push_back(new TH1I(title.c_str(), title.c_str(),140,-210 ,2610));
    //     gain_.push_back(0);
    // }

};

void Gain::AddValue(std::map<std::string, double> values)
{
    for(auto & ivec : channels_)
    {
        ivec->gain_hist->Fill(values[ivec->name]);
    }
        // for(unsigned i =0; i<channels_.size();i++)
        // {
        //     channels_.at(i).hist->Fill(-values[channel_list_.at(i)]);
        // }
};

// void Gain::AddValue(std::vector<double> values)
// {
//         if(values.size() != channels_.size())
//         {
//             std::cout << "\033[32;1mGain::AddValue: number of values and channels does not agree! \033[0m done!     "<< std::endl;
//         }
//
//         for(unsigned i =0; i<channels_.size();i++)
//         {
//             channels_.at(i)->gain_hist->Fill(-values.at(i));
//         }
//         // for(unsigned i =0; i<channels_.size();i++)
//         // {
//         //     channels_.at(i)->Fill(-values.at(i));
//         // }
// };

void Gain::FitGain()
{
    std::ofstream hendrik_file("/home/iwsatlas1/mgabriel/Plots/forHendyDany.txt", ios::app);

    for(auto & ivec : channels_)
    {

        TF1* gaussian=new TF1( (ivec->name + "_gaussian").c_str(),"gaus",-200, 2600);

        ivec->gain_hist->Fit(gaussian,"Q");
        ivec->gain_hist->Fit(gaussian,"Q");

        TF1* double_gaussian=new TF1( (ivec->name + "_double_gaussian").c_str(),"gaus(220)+gaus(420)",0,3*gaussian->GetParameter(1) );

        double_gaussian->SetParameter(0,gaussian->GetParameter(0));
        double_gaussian->SetParameter(1,gaussian->GetParameter(1));
        double_gaussian->SetParameter(2,gaussian->GetParameter(2));

        double_gaussian->SetParameter(3,gaussian->GetParameter(0)*0.1);
        double_gaussian->SetParameter(4,gaussian->GetParameter(1)*2);
        double_gaussian->SetParameter(5,gaussian->GetParameter(2));

        double_gaussian->SetParLimits(4,gaussian->GetParameter(1)*1.65, gaussian->GetParameter(1)*2.8);
        double_gaussian->SetParLimits(5,gaussian->GetParameter(2)*0.5, gaussian->GetParameter(2)*2.);

        // ivec->gain_hist->Fit(double_gaussian,"WWQ");
        // ivec->gain = double_gaussian->GetParameter(4) - double_gaussian->GetParameter(1);
        // hendrik_file<< " "<< ivec->name << " " << ivec->gain;
    }
    hendrik_file << std::endl;
    hendrik_file.close();
};

void Gain::SaveGain(boost::filesystem::path path_run)
{
    if(!boost::filesystem::is_directory(path_run/boost::filesystem::path("Calibration")) )
    {
        boost::filesystem::create_directory(path_run/boost::filesystem::path("Calibration"));
    }

    std::string fname = path_run.string()+"/Calibration/run_"+std::to_string(run_nr_)+"_gain_v1.root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto & ivec : channels_)
    {
        ivec->gain_hist->Write();
    }

    rfile->Close();
    delete rfile;
};

void Gain::AddIntWf(std::map<std::string, std::vector<float>*> wfs, std::map<std::string, double>integral)
{
    for(unsigned i=0; i < GS->GetChannels(2).size(); i++ )
    {
        std::string ch_name = GS->GetChannels(2).at(i);
        if(integral[ch_name]>= channels_.at(i)->gain*0.75 && integral[ch_name] <= channels_.at(i)->gain*1.25)
        {
            std::transform(channels_.at(i)->avg_wf->begin(), channels_.at(i)->avg_wf->end(),wfs[ch_name]->begin(), channels_.at(i)->avg_wf->begin(), std::plus<float>());
        }
    }
};

void  Gain::AddIntWfs(std::vector<std::vector<IntChannel*>> int_channels)
{
    for(unsigned i = 0;  i < channels_.size(); i++)
    {
        int counter = 0;
        for(auto &ivec : int_channels.at(i))
        {
            if(ivec->GetIntegral() >= channels_.at(i)->gain*(1-GS->GetAcceptedGain()) && ivec->GetIntegral() <= channels_.at(i)->gain*(1+GS->GetAcceptedGain()))
            {
                std::vector<float>* wf= ivec->GetWaveform();
                std::transform(channels_.at(i)->avg_wf->begin(), channels_.at(i)->avg_wf->end(),wf->begin(), channels_.at(i)->avg_wf->begin(), std::plus<float>());
                counter ++;
            }
        }
        this->NormalizeWaveform(i, counter);
    //channels_.at(i)->avg_hist->SetEntries(channels_.at(i)->avg_hist->GetEntries()*counter);

    }
};

void Gain::NormalizeWaveform(int ch, double norm)
{
        for(unsigned i = 0; i<channels_.at(ch)->avg_wf->size(); i++)
        {
            channels_.at(ch)->avg_wf->at(i) /= norm;
        }
};

void Gain::FitAvg()
{
    for(auto & ivec : channels_)
    {
        std::string name = ivec->name + "_exponential";
        std::replace(name.begin(), name.end(), '-','_');
        TF1* expo=new TF1( name.c_str(),"[0]*exp([1]*(x-[2]))", 1, ivec->avg_hist->GetNbinsX());
        expo->SetParameter(0,-33);
        expo->SetParameter(1,-0.040);
        expo->SetParameter(2,97);
        // expo->SetParameter(3,-0.11);
        ivec->avg_hist->Fit(expo, "Q", "", ivec->avg_hist->GetMinimumBin()+10, ivec->avg_wf->size());
        ivec->end = round(expo->GetX(0.015));
        for(signed i = ivec->avg_wf->size(); i < ivec->end + 1; i++)
        {
            ivec->avg_hist->SetBinContent(i, expo->Eval(i));
        }
        std::cout<< expo->GetX(-0.015) << std::endl;
        // std::cout<< expo->GetX(0.015) << std::endl;
    }
}

void Gain::SaveAvg(boost::filesystem::path path_run)
{
    if(!boost::filesystem::is_directory(path_run/boost::filesystem::path("Calibration")) )
    {
        boost::filesystem::create_directory(path_run/boost::filesystem::path("Calibration"));
    }

    std::string fname = path_run.string()+"/Calibration/run_"+std::to_string(run_nr_)+"_avg1pe_v1.root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto & ivec : channels_)
    {
        // std::string title = "run_" + std::to_string(run_nr_) + "_" + ivec->name + "_avg1pe";
        //
        // TH1F* tmp = new TH1F(title.c_str(), title.c_str(),ivec->avg_wf->size() ,-0.5 ,ivec->avg_wf->size()+0.5 );
        // for(unsigned i=0; i < ivec->avg_wf->size(); i++)
        // {
        //     tmp->SetBinContent(i+1, ivec->avg_wf->at(i));
        // }
        ivec->avg_hist->Write();
        // delete tmp;
        // tmp = NULL;
    }

    rfile->Close();
    delete rfile;
};
void Gain::WfToHist()
{
    for(auto & ivec : channels_)
    {
        delete ivec->avg_hist;

        std::string title = "run_" + std::to_string(run_nr_) + "_" + ivec->name + "_avg1pe";
        std::replace(title.begin(), title.end(), '-','_');
        ivec->avg_hist = new TH1F(title.c_str(), title.c_str(),ivec->avg_wf->size() + 150 ,-0.5 ,ivec->avg_wf->size()+0.5 + 150 );
        for(unsigned i=0; i < ivec->avg_wf->size(); i++)
        {
            ivec->avg_hist->SetBinContent(i+1, ivec->avg_wf->at(i));
        }
    }
}

void Gain::HistToWf()
{

    for(auto & ivec : channels_)
    {
        ivec->avg_wf->clear();
        std::cout<< "1 Size: " << ivec->avg_wf->size() << ", Capacity: " << ivec->avg_wf->capacity()<< std::endl;
        //if(ivec->end-129 > ivec->avg_wf->capacity()) ivec->avg_wf->reserve(ivec->end-129);
        ivec->avg_wf->reserve(ivec->end-129);
        std::cout<< "2 Size: " << ivec->avg_wf->size() << ", Capacity: " << ivec->avg_wf->capacity()<< std::endl;
        for(unsigned i=129; i < ivec->end; i++)
        {
            ivec->avg_wf->push_back(ivec->avg_hist->GetBinContent(i));
        }
        std::cout<< "3 Size: " << ivec->avg_wf->size() << ", Capacity: " << ivec->avg_wf->capacity()<< std::endl;
    }
};

double Gain::GetGain(std::string channel)
{
    std::vector<GainChannel*>::iterator ivec = std::find_if(channels_.begin(), channels_.end(), boost::bind(&GainChannel::name, _1) == channel);
    // std::vector<std::string>::iterator ivec = std::find(channel_list_.begin(), channel_list_.end(), channel);
    // if(ivec != channels_.end()) return ain_[std::distance(channels_.begin(),ivec)];
    if(ivec != channels_.end()) return (*ivec)->gain;
    else                            return -1;
};

std::map<std::string, double> Gain::GetGain()
{
    std::map<std::string, double> rtn;
    for(auto & ivec : channels_)
    {
        rtn[ivec->name] = ivec->gain;
    }

    // for(unsigned i = 0; i < channel_list_.size(); i++)
    // {
    //     rtn[channel_list_.at(i)] = gain_.at(i);
    // }
    return rtn;
};

std::vector<float>* Gain::GetWaveform(std::string channel)
{
    std::vector<GainChannel*>::iterator ivec = std::find_if(channels_.begin(), channels_.end(), boost::bind(&GainChannel::name, _1) == channel);
    return (*ivec)->avg_wf;
};

std::map<std::string, std::vector<float>*> Gain::GetWaveform()
{
    std::map<std::string, std::vector<float>*> rtn;
    for(auto & ivec : channels_)
    {
        rtn[ivec->name] = ivec->avg_wf;
    }
    return rtn;
};



Gain::~Gain() {
	// TODO Auto-generated destructor stub
    for(auto & ivec : channels_)
    {
        delete ivec;
    }
};
