/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_ANALYSIS_EVENT_H_
#define CLAWS_ANALYSIS_EVENT_H_

// std includes
#include <vector>assss
#include <string>
#include <map>
// root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TTree.h>
#include <TBranch.h>

// mixed
#include<boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;


//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Event{

    public:

        Event(path file_root, path file_ini);
        Event(path file_root, path file_ini, path file_online_rate);
        virtual ~Event();

        int     LoadRootFile();
        double  LoadIniFile();
	    int     LoadOnlineRate();

        int     Subtract();

        bool    GetInjection();
        double  GetUnixtime();
        int     GetLerBg();
        int     GetHerBg();
	    int     GetEventNr();
        double* GetRateOnline();

        int getCh(string ch);
        int draw();

    private:
        // An event relies on data/information in three different files. The .root, .ini & the online monitor.
        path path_file_root;
        path path_file_ini;
        path path_online_rate;

	    int event_number;
        double unixtime_;
        int lerbg_;
        int herbg_;
        bool injection_;
	    double rate_online_[8];
        double rate_offline_[8];

        TFile *file;
        map<string, TH1I*> channels;
};


//----------------------------------------------------------------------------------------------
// Definition of the Run class. This class is supposed to do all gthe organization of a run.
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Run{

    public:

        Run(path dir);
        virtual ~Run();

        int BuildOnlineTree();
        int BuildOfflineTree();

        int MapOnlineRates();
        int WriteNTuple(path path_ntuple);

        double GetStartTime();
        double GetStopTime();
        TTree *GetOnlineTree();
        TTree *GetOfflineTree();

    private:

        double tsMin;
        double tsMax;

        TTree *tree_online;
        TTree *tree_offline;
        TTree *tree_skb;
        vector<Event*> events;

};

#endif /* CLAWS_ANALYSIS_EVENT_H_ */
