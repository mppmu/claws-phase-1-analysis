//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

#include "Event.hh"

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

void Event::SubtractPedestal()
{

    for(auto i : channels_)
    {
        i.second->SubtractPedestal( 5);
    }
}


double Event::GetUnixtime() const
{
    return unixtime_;
}

int Event::GetEventNr()     const
{
    return event_number;
}

int Event::getCh(string ch){

    return 0;
}



void Event::Draw(){

    cout << "Drawing PhysicsEvent: "<< event_number << endl;

    TCanvas * c = new TCanvas(to_string(event_number).c_str(),to_string(event_number).c_str(), 1600, 1200);
    c->Divide(2, channels_.size()/2);
    unsigned int pad=0;

    for(auto i : channels_)
    {
        pad+=+2;
        if(pad > channels_.size()) pad =1;
        c->cd(pad);
        i.second->GetWaveformHist()->Draw();
    }

    string ped_can_name = to_string(event_number) + " Pedestal";
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

map<string, TH1I*> Event::GetPedestal()
{
    map<string, TH1I*> rtn;
    for(auto & itr : channels_)
    {
        rtn[itr.first]  =  itr.second->GetPedestal();
    }

    return rtn;
};

Event::~Event() {
	// TODO Auto-generated destructor stub
}

//----------------------------------------------------------------------------------------------
// Definition of the PhysicsEvent class derived from Event.
//----------------------------------------------------------------------------------------------
PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{
    cout << "Loading PhysicsEvent: " << file_root.string() << endl;

    fill_n(rate_online_, 8, -1);
    fill_n(rate_offline_, 8, -1);

    this->LoadRootFile();
    this->LoadIniFile();
};

PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate): PhysicsEvent(file_root, file_ini)
{
    path_online_rate_ = file_online_rate;

    this->LoadOnlineRate();
};

PhysicsEvent::~PhysicsEvent() {
	// TODO Auto-generated destructor stub
};

void PhysicsEvent::LoadRootFile()
{
    file = new TFile(path_file_root_.string().c_str(), "open");

    channels_["FWD1"] = new PhysicsChannel("FWD1");
    channels_["FWD2"] = new PhysicsChannel("FWD2");
    channels_["FWD3"] = new PhysicsChannel("FWD3");
    channels_["FWD4"] = new PhysicsChannel("FWD4");

    channels_["BWD1"] = new PhysicsChannel("BWD1");
    channels_["BWD2"] = new PhysicsChannel("BWD2");
    channels_["BWD3"] = new PhysicsChannel("BWD3");
    channels_["BWD4"] = new PhysicsChannel("BWD4");

    for (auto &itr : channels_)
    {
        itr.second->LoadWaveform(file);
        itr.second->LoadPedestal();
    }

    file->Close("R");
    delete file;

}

void PhysicsEvent::LoadIniFile(){

	property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);

    unixtime_   = pt_.get<double>("Properties.UnixTime");
	lerbg_      = pt_.get<int>("SuperKEKBData.LERBg");
	herbg_      = pt_.get<int>("SuperKEKBData.HERBg");

    if (lerbg_ || herbg_ ) injection_ = true;
    else                   injection_ = false;

    //TODO load the rest that is written in the .ini file.
};

void PhysicsEvent::LoadOnlineRate(){

    std::ifstream ratefile(path_online_rate_.string());

    if (!ratefile){
        cerr << "not file" << endl;
        exit(1);
    }

    ratefile >> rate_online_[0] >> rate_online_[1] >> rate_online_[2] >> rate_online_[3] >> rate_online_[4] >> rate_online_[5] >> rate_online_[6] >> rate_online_[7];

    ratefile.close();
};

double* PhysicsEvent::GetRateOnline(){
    return rate_online_;
}

bool PhysicsEvent::GetInjection()  const
{
    return injection_;
}

int PhysicsEvent::GetLerBg()       const
{
    return lerbg_;
}

int PhysicsEvent::GetHerBg()       const
{
    return herbg_;
}

//----------------------------------------------------------------------------------------------
// Definition of the IntEvent class derived from Event.
//----------------------------------------------------------------------------------------------
IntEvent::IntEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{
    cout << "Loading Intermediate Event: " << file_root.string() << endl;
    this->LoadRootFile();

};

void IntEvent::LoadRootFile()
{
    file = new TFile(path_file_root_.string().c_str(), "open");

    channels_["FWD1-INT"] = new IntChannel("FWD1");
    channels_["FWD2-INT"] = new IntChannel("FWD2");
    channels_["FWD3-INT"] = new IntChannel("FWD3");

    channels_["BWD1-INT"] = new IntChannel("BWD1");
    channels_["BWD2-INT"] = new IntChannel("BWD2");
    channels_["BWD3-INT"] = new IntChannel("BWD3");

    for (auto &itr : channels_)
    {
        itr.second->LoadWaveform(file);
        itr.second->LoadPedestal();
    }

    file->Close("R");
    delete file;

}

void IntEvent::LoadIniFile()
{
    property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);

    unixtime_   = pt_.get<double>("Properties.UnixTime");

    //TODO load the rest that is written in the .ini file.
}

IntEvent::~IntEvent() {
	// TODO Auto-generated destructor stub
};
