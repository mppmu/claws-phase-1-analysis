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
    if (meta == NULL || data == NULL){
        std::cout << "Meta tree or data tree not found" << std::endl;
    }
	
    // TODO Auto-generated constructor stub
	// TODO Autoflag on TGraph to check if is_clock=true, maybe GetYAxis->GetMean ~ (max - min)/2 + min ~ Wert

}

Event::~Event() {
	// TODO Auto-generated destructor stub
}


TGraph* getChannel(std::string channel){
	return NULL;
}


