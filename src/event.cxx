//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

// OpenMP
#include <omp.h>

#include "event.hh"


using namespace std;
using namespace boost;

//----------------------------------------------------------------------------------------------
// Definition of the Event base class.
//----------------------------------------------------------------------------------------------

int Event::id_ = 0;

int Event::GetId()
{
    return id_;
}

Event::Event(const path &file_root, const path &file_ini)
{
    ++id_;

    path_file_ini_       = file_ini;
    path_file_root_      = file_root;

};
void Event::LoadRootFile()
{
    TFile *file=NULL;
    file = new TFile(path_file_root_.string().c_str(), "open");

    if(file->IsZombie())
    {
        cout << "Error openning file" << endl;
        exit(-1);
    }

    for (auto &itr : channels_)
    {
        itr.second->LoadHistogram(file);
    }

    file->Close("R");
    delete file;
    file = NULL;

};

void Event::LoadWaveform()
{

    for (auto &itr : channels_)
    {
        itr.second->LoadWaveform();
    }

};

void Event::DeleteHistograms()
{

    for (auto &itr : channels_)
    {
        itr.second->DeleteHistogram();
    }

};

void Event::LoadPedestal()
{
    for (auto &itr : channels_)
    {
        itr.second->LoadPedestal();
    }
};

void Event::SubtractPedestal(map<string, double> ped)
{
    for(auto &ch : channels_)
    {
        ch.second->Subtract( ped[ch.first] );
    }
}

void Event::SetBaseline(map<std::string, float> baseline)
{
    for(auto & itr : channels_)
    {
        itr.second->SetBaseline( baseline[itr.first] );
    }
};


int Event::GetNr()     const
{
    return nr_;
}

std::string Event::GetNrStr()     const
{
    return nr_str_;
}

int Event::getCh(string ch){

    return 0;
}

void Event::Draw(){

    cout << "Drawing PhysicsEvent: "<< nr_ << endl;

    TCanvas * c = new TCanvas(to_string(nr_).c_str(),to_string(nr_).c_str(), 1600, 1200);
    c->Divide(2, channels_.size()/2);
    unsigned int pad=0;

    for(auto i : channels_)
    {
        pad+=+2;
        if(pad > channels_.size()) pad =1;
        c->cd(pad);
        i.second->GetWaveformHist()->Draw();
    }

    string ped_can_name = to_string(nr_) + " Pedestal";
    TCanvas * c_ped = new TCanvas(ped_can_name.c_str(), ped_can_name.c_str(), 1600, 1200);
    c_ped->Divide(2, channels_.size()/2);
    pad=0;
    for(auto i : channels_)
    {
        pad+=+2;
        if(pad > channels_.size() ) pad =1;
        c_ped->cd(pad);
        i.second->GetPedestal()->Draw();
    }


}

std::map<std::string, TH1I*> Event::GetPedestal()
{
    std::map<std::string, TH1I*> rtn;
    for(auto & itr : channels_)
    {
        rtn[itr.first]  =  itr.second->GetPedestal();
    }

    return rtn;
};

std::map<std::string, Channel*> Event::GetChannels()
{
    return channels_;
}

Event::~Event() {
	// TODO Auto-generated destructor stub
}

//----------------------------------------------------------------------------------------------
// Definition of the PhysicsEvent class derived from Event.
//----------------------------------------------------------------------------------------------
PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{

    fill_n(rate_online_, 6, -1);
    fill_n(rate_offline_, 8, -1);

    nr_     = atoi(file_root.filename().string().substr(6,15).c_str());
    nr_str_ = file_root.filename().string().substr(6,9);


    // cout << "Listing gDirectory in PhysicsEvent::PhysicsEvent!" << endl;
    // gDirectory->ls();

    channels_["FWD1"] = new PhysicsChannel("FWD1");
    channels_["FWD2"] = new PhysicsChannel("FWD2");
    channels_["FWD3"] = new PhysicsChannel("FWD3");
    channels_["FWD4"] = new PhysicsChannel("FWD4");

    channels_["BWD1"] = new PhysicsChannel("BWD1");
    channels_["BWD2"] = new PhysicsChannel("BWD2");
    channels_["BWD3"] = new PhysicsChannel("BWD3");
    channels_["BWD4"] = new PhysicsChannel("BWD4");

};

PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate): PhysicsEvent(file_root, file_ini)
{
    path_online_rate_ = file_online_rate;
};

PhysicsEvent::~PhysicsEvent() {
	// TODO Auto-generated destructor stub
};

// void PhysicsEvent::LoadRootFile()
// {
//
//     // TFile and TTree classes are not thread save, therefore, you have to look them.
//     #pragma omp critical
//     {
//         file = new TFile(path_file_root_.string().c_str(), "open");
//
//         if(file->IsZombie())
//         {
//             cout << "Error openning file" << endl;
//             exit(-1);
//         }
//     }
//
//     for (auto &itr : channels_)
//
//     #pragma omp critical
//     {
//         file->Close("R");
//     }
//
//     delete file;
//     file = NULL;
//     // cout << "Listing gDirectory in PhysicsEvent::LoadRootFile() after closing the file!" << endl;
//     // gDirectory->ls();
// }

void PhysicsEvent::LoadIniFile(){

    property_tree::ptree pt_;
	property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);

    unixtime_       = pt_.get<double>("Properties.UnixTime");
	lerbg_          = pt_.get<int>("SuperKEKBData.LERBg");
	herbg_          = pt_.get<int>("SuperKEKBData.HERBg");
    kekb_status_    = pt_.get<string>("SuperKEKBData.SuperKEKBStatus");
    ler_status_     = pt_.get<string>("SuperKEKBData.LERSTatus");
    her_status_     = pt_.get<string>("SuperKEKBData.HERStatus");

    if (lerbg_ || herbg_ )                      injection_ = true;
    else                                        injection_ = false;
    if(kekb_status_ == "Vacuum Scrubbing"
     && ler_status_ == "Vacuum Scrubbing"
     && her_status_ == "Vacuum Scrubbing")      scrubbing_ = 3;
    else if(ler_status_ == "Vacuum Scrubbing")  scrubbing_ = 1;
    else if(her_status_ == "Vacuum Scrubbing")  scrubbing_ = 2;
    else                                        scrubbing_ = 0;

    //TODO load the rest that is written in the .ini file.
};

void PhysicsEvent::LoadOnlineRate(){

    std::ifstream ratefile(path_online_rate_.string());

    if (!ratefile){
        cerr << "not file" << endl;
        exit(1);
    }
    double dummy;
    ratefile >> rate_online_[0] >> rate_online_[1] >> rate_online_[2] >> dummy >> rate_online_[3] >> rate_online_[4] >> rate_online_[5] >> dummy;

    ratefile.close();
};

double* PhysicsEvent::GetRateOnline(){
    return rate_online_;
}

bool PhysicsEvent::GetInjection()  const
{
    return injection_;
}

int PhysicsEvent::GetScrubbing()  const
{
    return scrubbing_;
}

int PhysicsEvent::GetLerBg()       const
{
    return lerbg_;
}

int PhysicsEvent::GetHerBg()       const
{
    return herbg_;
}
double PhysicsEvent::GetUnixtime() const
{
    return unixtime_;
}
//----------------------------------------------------------------------------------------------
// Definition of the IntEvent class derived from Event.
//----------------------------------------------------------------------------------------------
IntEvent::IntEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{

    nr_str_ = file_root.filename().string().substr(14,3);
    nr_     = atoi(nr_str_.c_str());

    channels_["FWD1-INT"] = new IntChannel("FWD1");
    channels_["FWD2-INT"] = new IntChannel("FWD2");
    channels_["FWD3-INT"] = new IntChannel("FWD3");

    channels_["BWD1-INT"] = new IntChannel("BWD1");
    channels_["BWD2-INT"] = new IntChannel("BWD2");
    channels_["BWD3-INT"] = new IntChannel("BWD3");

};

// void IntEvent::LoadRootFile()
// {
//     file = new TFile(path_file_root_.string().c_str(), "open");
//
//
//
//     for (auto &itr : channels_)
//     {
//         itr.second->LoadWaveform(file);
//         itr.second->LoadPedestal();
//     }
//
//     file->Close("R");
//     delete file;
//
// }

void IntEvent::LoadIniFile()
{
    property_tree::ptree pt_;
    property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);

    for(int i=1;i<5;i++)
    {
        mean_online_[i] = pt_.get<double>("FWD."+std::to_string(i)+"-Mean");
        accepted_online_[i] = pt_.get<double>("FWD."+std::to_string(i)+"-Accepted");
    }
    for(int i=1;i<5;i++)
    {
        mean_online_[i+4] = pt_.get<double>("BWD."+std::to_string(i)+"-Mean");
        accepted_online_[i+4] = pt_.get<double>("BWD."+std::to_string(i)+"-Accepted");
    }
}

IntEvent::~IntEvent() {
	// TODO Auto-generated destructor stub
};
