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
#include <string>
#include <map>
// root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>


// mixed
#include<boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

class Event{

    public:
        Event(path p);
        virtual ~Event();

        int loadRoot();
        int loadIni();
        int subtract();

        int getCh(string ch);
        int draw();

    private:
        TFile *file;
        map<string, TH1I*> channels;
};


class Run{

    public:
        Run(string dir);
        virtual ~Run();
        int buildNTulples();
        
    protected:

    private:


};

#endif /* EVENT_H_ */
// class Event {
//
// public:
// 	Event(TTree* meta, TTree* data);
// 	virtual ~Event();
//
//     long int getEvtnr() {return evt_nr;};
//     double getTimestamp() {return unixtime;}; // In principl there are several different timestamps in the file, for now we just work with the unixone
//
//     TGraph* getChannel(std::string channel); // FWD1-4, BWD1-4
//     double getRate(std::string channel); // Input: FWD1-4, BWD1-4, COMB
//     int calcRate(std::string channel);
//     int eventToPdf(std::string file);
//
//
//
// protected:
//
// 	// These graphs contain the actual scope data. Spoiler alert, it might be, that not every channel is used/instantiated
// 	std::vector<TGraph*> graphs;
//
//     TGraph* FWD1;
//     TGraph* FWD2;
//     TGraph* FWD3;
//     TGraph* FWD4;
//
//     TGraph* BWD1;
//     TGraph* BWD2;
//     TGraph* BWD3;
//     TGraph* BWD4;
//
//     // meta data
//     UInt_t evt_nr;
//     double unixtime;
//
// };
//
// class Data {
//
// public:
//     Data(TDirectory* data);
//     virtual ~Data();
//
//     Event* getEvent(int evt_nr);
//     int appendEvent(Event* event);
//
// protected:

//};
