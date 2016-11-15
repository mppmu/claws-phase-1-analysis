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
// my includes
#include "Event.hh"
#include "GlobalSettings.hh"

using namespace std;
using namespace boost::filesystem;


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

        void Pedestal();
        void LoadPedestal();
        void FitPedestal();
        void SubtractPedestal();
        void DrawPedestal();

        void LoadEventFiles(path path_data);
        void LoadIntFiles(path path_int);
        void LoadRunSettings();

        int    GetRunNr();
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

        property_tree::ptree settings_;

        vector<PhysicsEvent*>   events_;
        vector<IntEvent*>       int_events_;

        map<string, double> ped_;
        map<string, double> ped_int_;

        map<string, TH1I*> h_ped_;
        map<string, TH1I*> h_ped_int_;
};

#endif /* CLAWS_RUN_H_ */
