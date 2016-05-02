/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <vector>
//root includes
#include "TGraph.h"
#include "TTree.h"

namespace claws {

class Event {
public:
	Event(TTree* meta, TTree* data);
	virtual ~Event();

	TGraph* GetChannel(int channel); // fwd = 1-6, bwd 7-12

protected:

	// These graphs contain the actual scope data. Spoiler alert, it might be, that not every channel is used/instantiated
	std::vector<TGraph*> graphs;

};

} /* namespace claws */

#endif /* EVENT_H_ */
