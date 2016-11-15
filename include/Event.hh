/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_EVENT_H_
#define CLAWS_EVENT_H_

//std includes
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <typeinfo>
// boost
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// #include <boost/program_options.hpp>
// #include <boost/filesystem/fstream.hpp>
// #include <boost/algorithm/string/predicate.hpp>
// #include <boost/foreach.hpp>


// gperf
#include <gperftools/heap-profiler.h>
#include <gperftools/profiler.h>

// root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include "TApplication.h"
#include <TCanvas.h>
#include <TF1.h>
// Project includes
#include "Channel.hh"


using namespace std;
using namespace boost;
//needed for all the paths
using namespace boost::filesystem;


//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Event{

    public:
        //TODO -fix event name und numbger

        Event(const path &file_root, const path &file_ini);

        virtual ~Event();

        virtual void                   LoadRootFile()   = 0;
        virtual void                   LoadIniFile()    = 0;

        void SubtractPedestal(map<string, double> ped);

        void             LoadPedestal();

        double                 GetUnixtime()  const;

	    int                    GetEventNr()   const;
        map<string, TH1I*>     GetPedestal();

        static int GetId();

        int getCh(string ch);
        virtual void Draw();

    // protected:

        static int id_;

        // An event relies on data/information in three different files. The .root, .ini & the online monitor.
        path path_file_root_;
        path path_file_ini_;

        property_tree::ptree pt_;

	    int event_number        = -1;
        double unixtime_        = -1;




        TFile *file;

        map<string, Channel*> channels_;
};

class PhysicsEvent : public Event{

    public:

        // PhysicsEvent();
        PhysicsEvent(const path &file_root, const path &file_ini);
        PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate);
        ~PhysicsEvent();

        void                   LoadRootFile();
        void                   LoadIniFile();
        void                   LoadOnlineRate();

        double*                GetRateOnline();
        int                    GetLerBg()     const;
        int                    GetHerBg()     const;
        bool                   GetInjection() const;
        int                    GetScrubbing() const;

        path path_online_rate_;

        int lerbg_              = -1;
        int herbg_              = -1;
        bool injection_         = false;
        std::string kekb_status_    = "";
        std::string ler_status_ ="";
        std::string her_status_ = "";
        int scrubbing_        = 0;

        double rate_online_[6];
        double rate_offline_[8];

        // map<string, PhysicsChannel*> channels_;
        //map<string, PhysicsChannel*> phy_chs_;

};

class IntEvent : public Event{

    public:

        // PhysicsEvent();
        IntEvent(const path &file_root, const path &file_ini);

        ~IntEvent();

        void                   LoadRootFile();
        void                   LoadIniFile();






};


#endif /* CLAWS_EVENT_H_ */
