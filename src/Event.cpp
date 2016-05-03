/*
 * Event.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#include "Event.h"

namespace claws {

inline int conChName(std::string channel){
    int out = 0;
    if (channel == "FWD1") out = 1;
        return out;
    }
    
Event::Event(TTree* meta, TTree* data) {
	// TODO Auto-generated constructor stub
	// TODO Autoflag on TGraph to check if is_clock=true, maybe GetYAxis->GetMean ~ (max - min)/2 + min ~ Wert

}

Event::~Event() {
	// TODO Auto-generated destructor stub
}


TGraph* Event::GetChannel(int channel){
	return graphs.at(channel);
}

} /* namespace claws */
