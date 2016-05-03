/*
 * Event.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#include <iostream>
#include "Event.h"



//inline int conChName(std::string channel){
//    int out = 0;
//    if (channel == "FWD1") out = 1;
//        return out;
//    }


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
    
    int16_t fwd1;
    data->SetBranchAddress("FWD1", &fwd1);
    
    FWD1=new TGraph();
    
    //TODO Instead of i*0.8 use the appropriate timebase from the scope.
    for (Long64_t i; i < data->GetEntries(); i++) {
        data->GetEntry(i);
        FWD1->SetPoint(i, i * 0.8, fwd1);
    }
    
    delete data;
    data = NULL;

    
    std::cout.precision(20);
    std::cout << evt_nr << " " << unixtime << std::endl;
    // TODO Auto-generated constructor stub
	// TODO Autoflag on TGraph to check if is_clock=true, maybe GetYAxis->GetMean ~ (max - min)/2 + min ~ Wert

}

Event::~Event() {
	// TODO Auto-generated destructor stub
}


TGraph* getChannel(std::string channel){
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


