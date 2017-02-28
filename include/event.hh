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
// #include <gperftools/heap-profiler.h>
// #include <gperftools/profiler.h>

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
        void SetPedestal(std::map<std::string, float> ped = std::map<std::string, float>(), bool backup = false);


        void             LoadPedestal();

        static int GetId();

        int getCh(std::string ch);
  //      virtual void Draw();


        // Methods for loading data from disk
        virtual void LoadRootFile();
        virtual void LoadWaveform();
        virtual void DeleteHistograms();
        virtual void DeleteWaveforms();
        virtual void LoadIniFile() = 0;

        void                                CalculateIntegral();

        std::map<std::string, double>       GetIntegral();
        std::vector< double>                GetIntegralVec();


        void                                SetBaseline(std::map<std::string, float> baseline);

        int                                 GetNr()   const;
        std::string                                     GetNrStr() const;
        std::map<std::string, TH1I*>                    GetPedestal();
        virtual Channel*                                GetChannel(std::string name);
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
        virtual ~PhysicsEvent();

//        void                   LoadRootFile();
        void                   LoadIniFile();
        void                   LoadOnlineRate();

        void                   SetUpWaveforms();
        void                   FastRate(std::map<std::string, std::vector<float>*> avg_waveforms, std::map<std::string, double> pe_to_mips);
        void                   Decompose(std::map<std::string, std::vector<float>*> avg_waveforms);
        void                   Reconstruct(std::map<std::string, std::vector<float>*> avg_waveforms);
        void                   CalculateChi2();
        void                   SaveEvent(boost::filesystem::path result_folder);

        // Type O: online       double[6]
        // Type 1: fast offline double[3]
        // Type 2: offline      double[3]
        double*                GetRate( int type = 0);

        int                    GetLerBg()     const;
        int                    GetHerBg()     const;
        bool                   GetInjection() const;
        int                    GetScrubbing() const;
        double                 GetUnixtime()  const;

        template<typename T>
        T GetPV(std::string pv)
        {
            return pt_.get<T>( "SuperKEKBData." + pv );
        };

//        std::map<std::string, double> GetIntegral(); // Pure Placeholder fo far

        boost::property_tree::ptree pt_;
        path path_online_rate_;

        double unixtime_        = -1;
        int lerbg_              = -1;
        int herbg_              = -1;
        bool injection_         = false;
        std::string kekb_status_    = "";
        std::string ler_status_ ="";
        std::string her_status_ = "";
        int scrubbing_        = 0;

        double online_rate_[6];
        double fast_rate_[3];
        double rate_[3];


};

class IntEvent : public Event{

    public:

        // PhysicsEvent();
        IntEvent(const path &file_root, const path &file_ini);

        virtual ~IntEvent();

        void                   LoadIniFile();
//        IntChannel*                                GetChannel(std::string name);
//        std::map<std::string, double> GetIntegral();

        double mean_online_[8];
        double accepted_online_[8];
//        std::map<std::string, IntChannel*> channels_;


};


#endif /* CLAWS_EVENT_H_ */
