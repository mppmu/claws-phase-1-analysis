//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================


 //std includes
 #include <iostream>
 #include <fstream>
 #include <vector>
 #include <map>
 #include <string>
 #include <cstdlib>
 #include <typeinfo>

 // boost
 #include <boost/filesystem.hpp>
 #include <boost/lexical_cast.hpp>
 #include <boost/algorithm/string/predicate.hpp>
 #include <boost/algorithm/string/replace.hpp>
 #include <boost/property_tree/ptree.hpp>
 #include <boost/property_tree/ini_parser.hpp>
 // #include <boost/program_options.hpp>
 // #include <boost/filesystem/fstream.hpp>
 // #include <boost/algorithm/string/predicate.hpp>
 // #include <boost/foreach.hpp>

 // root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include "TApplication.h"
#include <TCanvas.h>
#include <TF1.h>

// Project includes
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

Event::Event(const path &file_root, const path &file_ini, const path &file_online_rate):Event(file_root, file_ini)
{

    path_online_rate_    =   file_online_rate;

};

bool Event::GetInjection()  const
{
    return injection_;
}

double Event::GetUnixtime() const
{
    return unixtime_;
}

int Event::GetLerBg()       const
{
    return lerbg_;
}

int Event::GetHerBg()       const
{
    return herbg_;
}

int Event::GetEventNr()     const
{
    return event_number;
}

int Event::getCh(string ch){

    return 0;
}

int Event::draw(){

    TApplication *app=new TApplication("app",0,0);
    cout << "Drawing now!" << endl;
    TCanvas * fwd1 = new TCanvas("fwd1", "fwd1", 600, 600);

    map<string, TH1I*>::iterator it = channels.find("BWD3-INT");

    if(it != channels.end()){
        cout << it->first << endl;
        it->second->Draw();
    }

    app->Run();
    return 0;
}

int Event::Subtract(){
    return 0;
};

double* Event::GetRateOnline(){
    return rate_online_;
}

Event::~Event() {
	// TODO Auto-generated destructor stub
}

//----------------------------------------------------------------------------------------------
// Definition of the PhysicsEvent class derived from Event.
//----------------------------------------------------------------------------------------------
PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{
    this->LoadRootFile();
    this->LoadIniFile();
};

PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate): Event(file_root, file_ini, file_online_rate)
{
    this->LoadRootFile();
    this->LoadIniFile();
    this->LoadOnlineRate();

};

PhysicsEvent::~PhysicsEvent() {
	// TODO Auto-generated destructor stub
};

int PhysicsEvent::LoadRootFile()
{

    // cout << typeid(p.string()).name() << endl;
    //
    // file = new TFile(p.string().c_str(), "open");
    // cout << (TH1I*)file->Get("FWD1-INT") << endl;
    // channels["FWD1-INT"] = (TH1I*)file->Get("FWD1-INT")->Clone();
    // cout << channels["FWD1-INT"] << endl;
    // channels["FWD2-INT"] = (TH1I*)file->Get("FWD2-INT")->Clone();
    // channels["FWD3-INT"] = (TH1I*)file->Get("FWD3-INT")->Clone();
    // channels["BWD1-INT"] = (TH1I*)file->Get("BWD1-INT")->Clone();
    // channels["BWD2-INT"] = (TH1I*)file->Get("BWD2-INT")->Clone();
    // channels["BWD3-INT"] = (TH1I*)file->Get("BWD3-INT")->Clone();
    //
    //
    // TF1 * fit = new TF1("fit", "[0]", 0 ,115);
    // FWD1->Fit(fit, "WRV");
    //
    // TH1D *fwd_values=new TH1D("values","values", 1000000, -32512,32512);
    //
    // for (int i = 0; i < FWD1->GetNbinsX(); i++) {
    //     cout << FWD1->GetBinContent(i) << endl;
    //     fwd_values->Fill(FWD1->GetBinContent(i));
    // }
    //
    //
    // FWD1->Draw();
    //
    // file->Close();
    // cout << channels["FWD1-INT"] << endl;

    return 0;
}

int PhysicsEvent::LoadIniFile(){

    property_tree::ptree pt;
	property_tree::ini_parser::read_ini(path_file_ini_.string(), pt);

    unixtime_   = pt.get<double>("Properties.UnixTime");
	lerbg_      = pt.get<int>("SuperKEKBData.LERBg");
	herbg_      = pt.get<int>("SuperKEKBData.HERBg");

    if (lerbg_ || herbg_ ) injection_ = true;
    else                   injection_ = false;

    //TODO load the rest that is written in the .ini file.

    return unixtime_;
};

int PhysicsEvent::LoadOnlineRate(){

    std::ifstream ratefile(path_online_rate_.string());

    if (!ratefile){
        cerr << "not file" << endl;
        exit(1);
    }

    ratefile >> rate_online_[0] >> rate_online_[1] >> rate_online_[2] >> rate_online_[3] >> rate_online_[4] >> rate_online_[5] >> rate_online_[6] >> rate_online_[7];

    ratefile.close();

    return 0;
};
