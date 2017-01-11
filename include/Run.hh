/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_RUN_H_
#define CLAWS_RUN_H_

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
// boost
#include<boost/filesystem.hpp>


//std includes
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <typeinfo>
#include <math.h>
#include <stdlib.h>



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

// root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TThread.h>
// OpenMP
#include <omp.h>

// google performance tools
#include <gperftools/heap-profiler.h>
#include <gperftools/profiler.h>


// Project includes
#include "Run.hh"
#include "Event.hh"
#include "Pedestal.hh"
#include "GlobalSettings.hh"


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
        int WriteNTuple(boost::filesystem::path path_ntuple);




        void Subtract();


        void DrawPedestal();

        void SaveEvents(boost::filesystem::path fname);

        // These methods are meant to be public in the end and used by the user
        void SynchronizeFiles();

        void LoadRawData();

            void LoadEventFiles();
            void LoadIntFiles();
            void LoadWaveforms();
            void LoadRunSettings();

        void SubtractPedestal();

            void LoadPedestal();
            void FitPedestal();
            void SavePedestal();

        int    GetRunNr();
        double GetStartTime();
        double GetStopTime();
        TTree *GetOnlineTree();
        TTree *GetOfflineTree();

    protected:


    private:

        int PathToRunNumber(boost::filesystem::path p);

        boost::filesystem::path path_run_;
        double tsMin;
        double tsMax;

        int run_nr_;
        std::string run_nr_str_;

        TTree *tree_online;
        TTree *tree_offline;
        TTree *tree_skb;

        boost::property_tree::ptree settings_;

        std::vector<PhysicsEvent*>   events_;
        std::vector<IntEvent*>       int_events_;

        Pedestal* pedestal_;
        // std::map<string, double> ped_;
        // std::map<string, double> ped_int_;
        //
        // std::map<string, TH1I*> h_ped_;
        // std::map<string, TH1I*> h_ped_int_;
};

#endif /* CLAWS_RUN_H_ */
