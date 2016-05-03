/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef EVENT_H_
#define EVENT_H_

// std includes
#include <vector>
// root includes
#include "TGraph.h"
#include "TTree.h"



class Event {
    
public:
	Event(TTree* meta, TTree* data);
	virtual ~Event();

    TGraph* getChannel(std::string channel); // FWD1-4, BWD1-4
    
    long int getEvtnr() {return evt_nr;};
    double getTimestamp() {return timestamp;};
    
    double getRate(std::string channel); // Input: FWD1-4, BWD1-4, COMB
    
    
protected:

	// These graphs contain the actual scope data. Spoiler alert, it might be, that not every channel is used/instantiated
	std::vector<TGraph*> graphs;
    
    // lower level data
    long int evt_nr;
    double timestamp;
    
};



#endif /* EVENT_H_ */
