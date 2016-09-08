/*
 * Event.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

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
#include "Event.h"

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
//----------------------------------------------------------------------------------------------
// Definition of the Run class.
//----------------------------------------------------------------------------------------------

Run::Run(path dir)
{
    // Extract the runnumer from the path to the folder and convert it to int.
    run_number_ = atoi(dir.filename().string().substr(4,20).c_str());

    cout << "Start Loading Run:  " << run_number_ << endl;

    path path_data = dir / path("data_root");

    // Look into the data folder of the run land get a list/vector of all the files inside
    vector<path> folder_content;
    copy(directory_iterator(dir / path("data_root")), directory_iterator(), back_inserter(folder_content));

    for (vector<path>::const_iterator itr = folder_content.begin(); itr != folder_content.end(); ++itr){

	    if(    is_regular_file(*itr)
            && starts_with((*itr).filename().string(), "Event-")
            && ends_with((*itr).filename().string(), ".root")){

                // Get the paths to the .root file of the event.
                path path_file_root = path_data / (*itr);

                // Get the path to the .ini file.
                string tmp          = (*itr).filename().string();
                replace_last( tmp, ".root" , ".ini");
                path path_file_ini  = path_data / path(tmp);

                // Get the path to the file from the online monitor
                replace_first(tmp, "Event-","");
                replace_last(tmp, ".ini", "");
                string ratefile = "Rate-Run--" + to_string( atoi(tmp.substr(2,4).c_str())) + to_string( atoi(tmp.substr(6,10).c_str())-1 );
                path path_online_rate = dir / ratefile;

                // Check if the .ini & online monitor exist for the event.
                if( exists( path_file_ini) && exists( path_online_rate)){
                    events.push_back(new PhysicsEvent(path_file_root, path_file_ini, path_online_rate));
                }
                else{
                    //TODO put in some mechanism in case the ini or the online rate files do not exist.
                }

        };
    };

    tsMin = events.front()->GetUnixtime();
    tsMax = events.back()->GetUnixtime();

    cout << "Done  Loading Run!" << endl;
};

Run::~Run() {
	// TODO Auto-generated destructor stub
};

// int PathToRunNumber(path p)
// {
//     // p
//     //
//     // run_number_ =
//     return run_number_
// }

double Run::GetStartTime(){
    return tsMin;
};
double Run::GetStopTime(){
    return tsMax;
};

int Run::BuildOnlineTree(){
    // TODO Implentation
    return 0;
};
int Run::BuildOfflineTree(){
    // TODO Implentation
    return 0;
};
TTree *Run::GetOnlineTree(){
    this->BuildOnlineTree();
    return tree_online;
};
TTree *Run::GetOfflineTree(){
    this->BuildOfflineTree();
    return tree_offline;
};

int Run::WriteOnlineTree(TFile* file)
{
    TTree *tout = new TTree("rates_online","rates_online");

    double ts, rate_fwd1, rate_fwd2, rate_fwd3, rate_fwd4, rate_bwd1, rate_bwd2, rate_bwd3, rate_bwd4;

    tout->Branch("ts", &ts,     "ts/D");
    tout->Branch("fwd1", &rate_fwd1,     "fwd1/D");
    tout->Branch("fwd2", &rate_fwd2,     "fwd2/D");
    tout->Branch("fwd3", &rate_fwd3,     "fwd3/D");
    tout->Branch("fwd4", &rate_fwd4,     "fwd4/D");
    tout->Branch("bwd1", &rate_bwd1,     "fbwd1/D");
    tout->Branch("bwd2", &rate_bwd2,     "fbwd2/D");
    tout->Branch("bwd3", &rate_bwd3,     "fbwd3/D");
    tout->Branch("bwd4", &rate_bwd4,     "fbwd4/D");

    for(unsigned int i=0; i < events.size(); i++){

    	ts = events.at(i)->GetUnixtime();

    	rate_fwd1 = events.at(i)->GetRateOnline()[0];
    	rate_fwd2 = events.at(i)->GetRateOnline()[1];
    	rate_fwd3 = events.at(i)->GetRateOnline()[2];
    	rate_fwd4 = events.at(i)->GetRateOnline()[3];
    	rate_bwd1 = events.at(i)->GetRateOnline()[4];
    	rate_bwd2 = events.at(i)->GetRateOnline()[5];
    	rate_bwd3 = events.at(i)->GetRateOnline()[6];
    	rate_bwd4 = events.at(i)->GetRateOnline()[7];

    	tout->Fill();
    }

    file->cd();
    tout->Write();
    return 0;
};
int Run::WriteNTuple(path path_ntuple){

    path_ntuple = path_ntuple / ("CLAWS" + to_string((int)tsMin)+ ".root");
    TFile * root_file  = new TFile(path_ntuple.string().c_str(), "RECREATE");

    this->WriteOnlineTree(root_file);

    root_file->Close();

    return 0;
};
// int setStyle(TGraph* graph1,TGraph* graph2,TGraph* graph3,TGraph* graph4){
//     double markersize=0.1;
//
//     graph1->SetMarkerColor(kAzure-3);
//     graph1->SetMarkerStyle(20);
//     graph1->SetMarkerSize(markersize);
//     graph1->GetYaxis()->SetRangeUser(-32512., 32512);
//
//     graph2->SetMarkerColor(kOrange+7);
//     graph2->SetMarkerStyle(20);
//     graph2->SetMarkerSize(markersize);
//
//     graph3->SetMarkerColor(kGreen+2);
//     graph3->SetMarkerStyle(20);
//     graph3->SetMarkerSize(markersize);
//
//     graph4->SetMarkerColor(kRed);
//     graph4->SetMarkerStyle(20);
//     graph4->SetMarkerSize(markersize);
//
//     return 0;
// }
//
// Event::Event(TTree* meta, TTree* data) {
//
//     // Check if everything is there.
//     if (meta == NULL || data == NULL){
//         std::cout << "Meta tree or data tree not found" << std::endl;
//     }
//
//     // Extract the meta data aka timestamp and evt number
//     meta->SetBranchAddress("evt_nr", &evt_nr);
//     meta->SetBranchAddress("unixtime", &unixtime);
//     meta->GetEntry(0);
//
//     delete meta;
//     meta = NULL;
//
//     //TODO make this here dynamic, now only 8 channels are possible
//     int16_t fwd1;
//     data->SetBranchAddress("FWD1", &fwd1);
//     FWD1=new TGraph();
//
//     int16_t fwd2;
//     data->SetBranchAddress("FWD2", &fwd2);
//     FWD2=new TGraph();
//
//     int16_t fwd3;
//     data->SetBranchAddress("FWD3", &fwd3);
//     FWD3=new TGraph();
//
//     int16_t fwd4;
//     data->SetBranchAddress("FWD4", &fwd4);
//     FWD4=new TGraph();
//
//     int16_t bwd1;
//     data->SetBranchAddress("BWD1", &bwd1);
//     BWD1=new TGraph();
//
//     int16_t bwd2;
//     data->SetBranchAddress("BWD2", &bwd2);
//     BWD2=new TGraph();
//
//     int16_t bwd3;
//     data->SetBranchAddress("BWD3", &bwd3);
//     BWD3=new TGraph();
//
//     int16_t bwd4;
//     data->SetBranchAddress("BWD4", &bwd4);
//     BWD4=new TGraph();
//
//     Long64_t n_entries = data->GetEntries();
//
//     //TODO Instead of i*0.8 use the appropriate timebase from the scope.
//     for (Long64_t i = 0 ; i < n_entries ; i++) {
//         data->GetEntry(i);
//         FWD1->SetPoint(i, i * 0.8, fwd1);
//         FWD2->SetPoint(i, i * 0.8, fwd2);
//         FWD3->SetPoint(i, i * 0.8, fwd3);
//         FWD4->SetPoint(i, i * 0.8, fwd4);
//         BWD1->SetPoint(i, i * 0.8, bwd1);
//         BWD2->SetPoint(i, i * 0.8, bwd2);
//         BWD3->SetPoint(i, i * 0.8, bwd3);
//         BWD4->SetPoint(i, i * 0.8, bwd4);
//     }
//
//     delete data;
//     data = NULL;
//
//     // TODO Auto-generated constructor stub
// 	// TODO Autoflag on TGraph to check if is_clock=true, maybe GetYAxis->GetMean ~ (max - min)/2 + min ~ Wert
//
// }
//
// Event::~Event() {
// 	// TODO Auto-generated destructor stub
// }
//
//
// TGraph* Event::getChannel(std::string channel){
//
//     if(channel == "FWD1"){
//         return FWD1;
//     }else if (channel == "FWD2"){
//         return FWD2;
//     }else if (channel == "FWD3"){
//         return FWD3;
//     }else if (channel == "FWD4"){
//         return FWD4;
//     }else if (channel == "BWD1"){
//         return BWD1;
//     }else if (channel == "BWD2"){
//         return BWD2;
//     }else if (channel == "BWD3"){
//         return BWD3;
//     }else if (channel == "BWD4"){
//         return BWD4;
//     }else{
//         std::cout << "Channel " << channel << " not found! " << std::endl;
//         return NULL;
//     }
//
// }
//
// int Event::eventToPdf(std::string file){
//
//     setStyle(FWD1, FWD2, FWD3, FWD4);
//     setStyle(BWD1, BWD2, BWD3, BWD4);
//
//     TCanvas *c1 = new TCanvas("c1","multipads",1200,750);
//     c1->Divide(1,2,0,0);
//
//     c1->cd(1);
//     FWD1->Draw("AP");
//     FWD2->Draw("P");
//     FWD3->Draw("P");
//     FWD4->Draw("P");
//
//     c1->cd(2);
//     BWD1->Draw("AP");
//     BWD2->Draw("P");
//     BWD3->Draw("P");
//     BWD4->Draw("P");
//
//     c1->SaveAs(file.c_str());
//
//     return 0;
//
// }
//
// int Event::calcRate(std::string channel){
//
//
//     return 0;
// }
//
//
// double Event::getRate(std::string channel){
//     //TODO implementation
//     return 0;
// }
//
//
//
// Data::Data(TDirectory* dir){
//
//
//
// //    Event* event = new Event(meta, data);
//     map<int,Event*> events;
//     map<int,Event*>::iterator it;
//
//
//     TIter next(dir->GetListOfKeys());
//     TKey* key;
//     while ((key = (TKey*)next())){
//     		int evt_nr = std::stoi(string(key->GetName()).substr(0,9));
//     		it = events.find(evt_nr);
//     		if (it == events.end()){
//                 TTree* meta = (TTree*)dir->Get((to_string(evt_nr)+"-meta").c_str());
//                 TTree* data = (TTree*)dir->Get((to_string(evt_nr)+"-data").c_str());
//                 Event* event=new Event(meta,data);
//     			events.insert(pair<int,Event*>(evt_nr,event));
//     		}
//
//     	}
//
//
// //    it = events.begin();
// //    while(it !=events.end()){
// ////        data = (TTree*)dir->Get((to_string(it->first)+"-data").c_str());
// ////        meta = (TTree*)dir->Get((to_string(it->first)+"-meta").c_str());
// ////        it->second=new Event(meta, data);
// //        cout << it->first << ", " << it->second->getEvtnr() <<endl;
// //        ++it;
// //    	}
//
// //    cout << events.size()<< endl;
// }
//
// Data::~Data() {
//     // TODO Auto-generated destructor stub
// }
//
// Event* getEvent(int evt_nr){
//     return NULL;
// };
//
// int Data::appendEvent(Event* event){
//
//
//     return 0;
// }
