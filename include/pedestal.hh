/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */
 #ifndef CLAWS_PEDESTAL_H_
 #define CLAWS_PEDESTAL_H_


#include <TFile.h>
#include <TH1I.h>
#include <TGraphErrors.h>
#include <vector>
#include <map>




// class Pedestal {
//
// public:
// Pedestal(int run_nr, int int_nr);
// virtual ~Pedestal();
//
// void                                AddEvent(std::map<std::string, TH1I*> event);
// void                                SavePedestal(TFile* file);
//
//
// std::map<std::string, float>             GetPedestal(int type=0);
// std::map<std::string, float>             GetError();
//
// std::map<std::string, TH1I*>             GetHistograms();
// std::map<std::string, TGraphErrors*>     GetGraphs();
//
// std::map<std::string, TH1I*>             h_;
// std::map<std::string, TGraphErrors*>     g_;
// int run_nr_;
// int int_nr_;
//
//
// };
#endif /* CLAWS_PEDESTAL_H_ */
