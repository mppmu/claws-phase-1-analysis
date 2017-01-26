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
#include "channel.hh"


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

//        void SubtractPedestal();


        void SubtractPedestal(std::map<std::string, float> ped = std::map<std::string, float>(), bool backup = false);



        void             LoadPedestal();

        static int GetId();

        int getCh(std::string ch);
        virtual void Draw();


        // Methods for loading data from disk
        virtual void LoadRootFile();
        virtual void LoadWaveform();
        virtual void DeleteHistograms();
        virtual void LoadIniFile() = 0;

        void                                CalculateIntegral();
        std::map<std::string, double>       GetIntegral();
        std::vector< double>                GetIntegralVec();


        void                                SetBaseline(std::map<std::string, float> baseline);

        int                                 GetNr()   const;
        std::string                                     GetNrStr() const;
        std::map<std::string, TH1I*>                    GetPedestal();
        Channel*                                        GetChannel(std::string name);
        std::map<std::string, Channel*>                 GetChannels();
        std::map<std::string, std::vector<float>*>      GetWaveforms();
    // protected:

    //    static int id_;

        // An event relies on data/information in three different files. The .root, .ini & the online monitor.
        path path_file_root_;
        path path_file_ini_;



	    int nr_        = -1;
        std::string nr_str_;

        std::map<std::string, Channel*> channels_;
};

class PhysicsEvent : public Event{

    public:

        // PhysicsEvent();
        PhysicsEvent(const path &file_root, const path &file_ini);
        PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate);
        ~PhysicsEvent();

//        void                   LoadRootFile();
        void                   LoadIniFile();
        void                   LoadOnlineRate();

        double*                GetRateOnline();
        int                    GetLerBg()     const;
        int                    GetHerBg()     const;
        bool                   GetInjection() const;
        int                    GetScrubbing() const;
        double                 GetUnixtime()  const;

//        std::map<std::string, double> GetIntegral(); // Pure Placeholder fo far

        path path_online_rate_;

        double unixtime_        = -1;
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

        void                   LoadIniFile();

//        std::map<std::string, double> GetIntegral();

        double mean_online_[8];
        double accepted_online_[8];

};


#endif /* CLAWS_EVENT_H_ */
