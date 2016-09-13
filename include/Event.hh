/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_ANALYSIS_EVENT_H_
#define CLAWS_ANALYSIS_EVENT_H_

// std includes
#include <vector>
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

        // Event();
        Event(const path &file_root, const path &file_ini);
        Event(const path &file_root, const path &file_ini, const path &file_online_rate);

        virtual ~Event();

        virtual int     LoadRootFile()   = 0;
        virtual int     LoadIniFile()    = 0;
        virtual int     LoadOnlineRate() = 0;

        int     Subtract();

        bool    GetInjection() const;
        double  GetUnixtime()  const;
        int     GetLerBg()     const;
        int     GetHerBg()     const;
	    int     GetEventNr()   const;

        static int GetId();
        double* GetRateOnline();

        int getCh(string ch);
        int draw();


    // protected:

        static int id_;

        // An event relies on data/information in three different files. The .root, .ini & the online monitor.
        path path_file_root_;
        path path_file_ini_;
        path path_online_rate_;

	    int event_number        = -1;
        double unixtime_        = -1;
        int lerbg_              = -1;
        int herbg_              = -1;
        bool injection_         = false;
	    double rate_online_[8]  = {};
        double rate_offline_[8] = {};

        TFile *file;
        map<string, TH1I*> channels;
};

class PhysicsEvent : public Event{

    public:

        // PhysicsEvent();
        PhysicsEvent(const path &file_root, const path &file_ini);
        PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate);
        ~PhysicsEvent();

        int     LoadRootFile();
        int     LoadIniFile();
        int     LoadOnlineRate();

    protected:

    private:

};
//
// class IntEvent : public Event{
//
// };

//----------------------------------------------------------------------------------------------
// Definition of the Run class. This class is supposed to do all gthe organization of a run.
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Run{

    public:

        Run(path p);
        virtual ~Run();

        int BuildOnlineTree();
        int BuildOfflineTree();

        int MapOnlineRates();

        int WriteOnlineTree(TFile* file);
        int WriteTimeStamp(TFile* file);
        int WriteNTuple(path path_ntuple);


        double GetStartTime();
        double GetStopTime();
        TTree *GetOnlineTree();
        TTree *GetOfflineTree();

    private:

        int PathToRunNumber(path p);

        path path_run_;
        double tsMin;
        double tsMax;

        int run_number_;

        TTree *tree_online;
        TTree *tree_offline;
        TTree *tree_skb;
        vector<Event*> events;

};

#endif /* CLAWS_ANALYSIS_EVENT_H_ */
