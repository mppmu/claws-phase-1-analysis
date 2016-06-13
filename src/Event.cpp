/*
 * Event.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#include <iostream>
#include "Event.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TSystem.h"
#include "TKey.h"
#include "TTree.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TApplication.h"
#include "TCanvas.h"

using namespace std;

int setStyle(TGraph* graph1,TGraph* graph2,TGraph* graph3,TGraph* graph4){
    double markersize=0.1;
    
    graph1->SetMarkerColor(kAzure-3);
    graph1->SetMarkerStyle(20);
    graph1->SetMarkerSize(markersize);
    graph1->GetYaxis()->SetRangeUser(-32512., 32512);
    
    graph2->SetMarkerColor(kOrange+7);
    graph2->SetMarkerStyle(20);
    graph2->SetMarkerSize(markersize);
    
    graph3->SetMarkerColor(kGreen+2);
    graph3->SetMarkerStyle(20);
    graph3->SetMarkerSize(markersize);
    
    graph4->SetMarkerColor(kRed);
    graph4->SetMarkerStyle(20);
    graph4->SetMarkerSize(markersize);
    
    return 0;
}

Event::Event(TTree* meta, TTree* data) {
    
    // Check if everything is there.
    if (meta == NULL || data == NULL){
        std::cout << "Meta tree or data tree not found" << std::endl;
    }

    // Extract the meta data aka timestamp and evt number
    meta->SetBranchAddress("evt_nr", &evt_nr);
    meta->SetBranchAddress("unixtime", &unixtime);
    meta->GetEntry(0);
    
    delete meta;
    meta = NULL;
    
    //TODO make this here dynamic, now only 8 channels are possible
    int16_t fwd1;
    data->SetBranchAddress("FWD1", &fwd1);
    FWD1=new TGraph();
    
    int16_t fwd2;
    data->SetBranchAddress("FWD2", &fwd2);
    FWD2=new TGraph();
    
    int16_t fwd3;
    data->SetBranchAddress("FWD3", &fwd3);
    FWD3=new TGraph();
    
    int16_t fwd4;
    data->SetBranchAddress("FWD4", &fwd4);
    FWD4=new TGraph();
    
    int16_t bwd1;
    data->SetBranchAddress("BWD1", &bwd1);
    BWD1=new TGraph();
    
    int16_t bwd2;
    data->SetBranchAddress("BWD2", &bwd2);
    BWD2=new TGraph();
    
    int16_t bwd3;
    data->SetBranchAddress("BWD3", &bwd3);
    BWD3=new TGraph();
    
    int16_t bwd4;
    data->SetBranchAddress("BWD4", &bwd4);
    BWD4=new TGraph();
    
    Long64_t n_entries = data->GetEntries();

    //TODO Instead of i*0.8 use the appropriate timebase from the scope.
    for (Long64_t i = 0 ; i < n_entries ; i++) {
        data->GetEntry(i);
        FWD1->SetPoint(i, i * 0.8, fwd1);
        FWD2->SetPoint(i, i * 0.8, fwd2);
        FWD3->SetPoint(i, i * 0.8, fwd3);
        FWD4->SetPoint(i, i * 0.8, fwd4);
        BWD1->SetPoint(i, i * 0.8, bwd1);
        BWD2->SetPoint(i, i * 0.8, bwd2);
        BWD3->SetPoint(i, i * 0.8, bwd3);
        BWD4->SetPoint(i, i * 0.8, bwd4);
    }

    delete data;
    data = NULL;
    
    // TODO Auto-generated constructor stub
	// TODO Autoflag on TGraph to check if is_clock=true, maybe GetYAxis->GetMean ~ (max - min)/2 + min ~ Wert

}

Event::~Event() {
	// TODO Auto-generated destructor stub
}


TGraph* Event::getChannel(std::string channel){
    
    if(channel == "FWD1"){
        return FWD1;
    }else if (channel == "FWD2"){
        return FWD2;
    }else if (channel == "FWD3"){
        return FWD3;
    }else if (channel == "FWD4"){
        return FWD4;
    }else if (channel == "BWD1"){
        return BWD1;
    }else if (channel == "BWD2"){
        return BWD2;
    }else if (channel == "BWD3"){
        return BWD3;
    }else if (channel == "BWD4"){
        return BWD4;
    }else{
        std::cout << "Channel " << channel << " not found! " << std::endl;
        return NULL;
    }
 
}

int Event::eventToPdf(std::string file){
   
    setStyle(FWD1, FWD2, FWD3, FWD4);
    setStyle(BWD1, BWD2, BWD3, BWD4);
    
    TCanvas *c1 = new TCanvas("c1","multipads",1200,750);
    c1->Divide(1,2,0,0);

    c1->cd(1);
    FWD1->Draw("AP");
    FWD2->Draw("P");
    FWD3->Draw("P");
    FWD4->Draw("P");
    
    c1->cd(2);
    BWD1->Draw("AP");
    BWD2->Draw("P");
    BWD3->Draw("P");
    BWD4->Draw("P");
    
    c1->SaveAs(file.c_str());
    
    return 0;

}

int Event::calcRate(std::string channel){
    
    
    return 0;
}


double Event::getRate(std::string channel){
    //TODO implementation
    return 0;
}



Data::Data(TDirectory* dir){
    

    
//    Event* event = new Event(meta, data);
    map<int,Event*> events;
    map<int,Event*>::iterator it;

    
    TIter next(dir->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())){
    		int evt_nr = std::stoi(string(key->GetName()).substr(0,9));
    		it = events.find(evt_nr);
    		if (it == events.end()){
                TTree* meta = (TTree*)dir->Get((to_string(evt_nr)+"-meta").c_str());
                TTree* data = (TTree*)dir->Get((to_string(evt_nr)+"-data").c_str());
                Event* event=new Event(meta,data);
    			events.insert(pair<int,Event*>(evt_nr,event));
    		}
    
    	}
    

//    it = events.begin();
//    while(it !=events.end()){
////        data = (TTree*)dir->Get((to_string(it->first)+"-data").c_str());
////        meta = (TTree*)dir->Get((to_string(it->first)+"-meta").c_str());
////        it->second=new Event(meta, data);
//        cout << it->first << ", " << it->second->getEvtnr() <<endl;
//        ++it;
//    	}
    
//    cout << events.size()<< endl;
}

Data::~Data() {
    // TODO Auto-generated destructor stub
}

Event* getEvent(int evt_nr){
    return NULL;
};

int Data::appendEvent(Event* event){
    
    
    return 0;
}

