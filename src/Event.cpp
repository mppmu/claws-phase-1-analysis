/*
 * Event.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

 // root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include "Event.h"
#include "TApplication.h"
#include <TCanvas.h>
#include <TF1.h>

//std includes
#include <typeinfo>
#include <map>

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
 //

 // #include <boost/foreach.hpp>


using namespace std;
using namespace boost;


Event::Event(path file_root, path file_ini): unixtime(-1.), lerbg(0), herbg(0), injection(false)
{

    path_file_root  = file_root;
    path_file_ini   = file_ini;

    this->LoadIniFile();

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

    // TF1 * fit = new TF1("fit", "[0]", 0 ,115);
    // FWD1->Fit(fit, "WRV");

    // TH1D *fwd_values=new TH1D("values","values", 1000000, -32512,32512);
    //
    // for (int i = 0; i < FWD1->GetNbinsX(); i++) {
    //     cout << FWD1->GetBinContent(i) << endl;
    //     fwd_values->Fill(FWD1->GetBinContent(i));
    // }

    //
    // FWD1->Draw();
    //
    // file->Close();
    // cout << channels["FWD1-INT"] << endl;
};


double Event::LoadIniFile(){

    property_tree::ptree pt;
	property_tree::ini_parser::read_ini(path_file_ini.string(), pt);

    unixtime   = pt.get<double>("Properties.UnixTime");
	lerbg      = pt.get<int>("SuperKEKBData.LERBg");
	herbg      = pt.get<int>("SuperKEKBData.HERBg");

    if (lerbg || herbg) injection = true;
    else                injection = false;

    return unixtime;
};
double Event::GetUnixtime(){
    return unixtime;
}
int Event::GetLerBg(){
    return lerbg;
}
int Event::GetHerBg(){
    return herbg;
}

bool Event::GetInjection(){
    return injection;
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
int Event::subtract(){
    return 0;
};

Event::~Event() {
	// TODO Auto-generated destructor stub
}

//----------------------------------------------------------------------------------------------
// Definition of the Run class.
//----------------------------------------------------------------------------------------------

Run::Run(path dir){
    cout << "Do something" << endl;
    cout << dir.string() << endl;

    path path_data = dir / path("data_root");

    vector<path> folder_content;
    copy(directory_iterator(dir / path("data_root")), directory_iterator(), back_inserter(folder_content));

    for (vector<path>::const_iterator itr = folder_content.begin(); itr != folder_content.end(); ++itr){

	    if( is_regular_file(*itr)
            && starts_with((*itr).filename().string(), "Event-")
            && ends_with((*itr).filename().string(), ".root")){

                // Get the paths to the .root and .ini file of the evnet.
                path path_file_root = path_data / (*itr);
                string tmp          = (*itr).filename().string();
                replace_last( tmp, ".root" , ".ini");
                path path_file_ini  = path_data / path(tmp);

                if( exists( path_file_ini) ){
                    events.push_back(new Event(path_file_root, path_file_ini));
                }

        };
    };

    for(int i =0; i< events.size(); i++){
        events.at(i)->LoadIniFile();
    }



};

Run::~Run() {
	// TODO Auto-generated destructor stub
}

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
