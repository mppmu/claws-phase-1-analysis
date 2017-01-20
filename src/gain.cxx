
#include <iostream>
#include <algorithm>

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
        channels_.push_back(GainChannel(ivec, new TH1I(title.c_str(), title.c_str(),140,-210 ,2610),0));
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
        ivec.hist->Fill(-values[ivec.name]);
    }
        // for(unsigned i =0; i<channels_.size();i++)
        // {
        //     channels_.at(i).hist->Fill(-values[channel_list_.at(i)]);
        // }
};

void Gain::AddValue(std::vector<double> values)
{
        if(values.size() != channels_.size())
        {
            std::cout << "\033[32;1mGain::AddValue: number of values and channels does not agree! \033[0m done!     "<< std::endl;
        }

        for(unsigned i =0; i<channels_.size();i++)
        {
            channels_.at(i).hist->Fill(-values.at(i));
        }
        // for(unsigned i =0; i<channels_.size();i++)
        // {
        //     channels_.at(i)->Fill(-values.at(i));
        // }
};

void Gain::Fit()
{
    for(auto & ivec : channels_)
    {

        TF1* gaussian=new TF1( (ivec.name + "_gaussian").c_str(),"gaus",-200, 2600);

        ivec.hist->Fit(gaussian,"Q");
        ivec.hist->Fit(gaussian,"Q");

        TF1* double_gaussian=new TF1( (ivec.name + "_double_gaussian").c_str(),"gaus(220)+gaus(420)",0,3*gaussian->GetParameter(1) );

        double_gaussian->SetParameter(0,gaussian->GetParameter(0));
        double_gaussian->SetParameter(1,gaussian->GetParameter(1));
        double_gaussian->SetParameter(2,gaussian->GetParameter(2));

        double_gaussian->SetParameter(3,gaussian->GetParameter(0)*0.1);
        double_gaussian->SetParameter(4,gaussian->GetParameter(1)*2);
        double_gaussian->SetParameter(5,gaussian->GetParameter(2));

        double_gaussian->SetParLimits(4,gaussian->GetParameter(1)*1.65, gaussian->GetParameter(1)*2.8);
        double_gaussian->SetParLimits(5,gaussian->GetParameter(2)*0.25, gaussian->GetParameter(2)*3.);

        ivec.hist->Fit(double_gaussian,"WWQ");
        ivec.gain = double_gaussian->GetParameter(4) - double_gaussian->GetParameter(1);
    }
};

void Gain::Save(boost::filesystem::path path_run)
{
    if(!boost::filesystem::is_directory(path_run/boost::filesystem::path("Calibration")) )
    {
        boost::filesystem::create_directory(path_run/boost::filesystem::path("Calibration"));
    }

    std::string fname = path_run.string()+"/Calibration/run_"+std::to_string(run_nr_)+"_gain_v1.root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto & ivec : channels_)
    {
        ivec.hist->Write();
    }

    rfile->Close();
    delete rfile;
}

double Gain::GetGain(std::string channel)
{
    std::vector<GainChannel>::iterator ivec = std::find_if(channels_.begin(), channels_.end(), boost::bind(&GainChannel::name, _1) == channel);
    // std::vector<std::string>::iterator ivec = std::find(channel_list_.begin(), channel_list_.end(), channel);
    // if(ivec != channels_.end()) return ain_[std::distance(channels_.begin(),ivec)];
    if(ivec != channels_.end()) return (*ivec).gain;
    else                            return -1;
};

std::map<std::string, double> Gain::GetGain()
{
    std::map<std::string, double> rtn;
    for(auto & ivec : channels_)
    {
        rtn[ivec.name] = ivec.gain;
    }

    // for(unsigned i = 0; i < channel_list_.size(); i++)
    // {
    //     rtn[channel_list_.at(i)] = gain_.at(i);
    // }
    return rtn;
}

Gain::~Gain() {
	// TODO Auto-generated destructor stub
};
