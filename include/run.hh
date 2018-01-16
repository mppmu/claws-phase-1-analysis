/*
 * run.hh
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_RUN_H_
#define CLAWS_RUN_H_


// std includes
// #include <vector>
// #include <string>
// #include <map>
// root includes
// #include <TFile.h>
// #include <TH1D.h>
// #include <TH1I.h>
// #include <TTree.h>
// #include <TBranch.h>
// // boost
// #include <boost/filesystem.hpp>


//std includes
// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <tuple>
// #include <map>
// #include <string>
// #include <cstdlib>
// #include <typeinfo>
// #include <math.h>
// #include <stdlib.h>

// boost
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
// #include <boost/lexical_cast.hpp>
// #include <boost/algorithm/string/predicate.hpp>
// #include <boost/algorithm/string/replace.hpp>
// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/ini_parser.hpp>
// #include <boost/program_options.hpp>
// #include <boost/filesystem/fstream.hpp>
// #include <boost/algorithm/string/predicate.hpp>
// #include <boost/foreach.hpp>

// root includes
// #include <TFile.h>
// #include <TH1D.h>
// #include <TH1F.h>
// #include <TH1I.h>
// #include <TApplication.h>
// #include <TCanvas.h>
// #include <TF1.h>
// #include <TThread.h>
// // OpenMP
//#include <omp.h>

// google performance tools
// #include <gperftools/heap-profiler.h>
// #include <gperftools/profiler.h>

// Project includes
// #include "pedestal.hh"
#include "gain.hh"
#include "event.hh"

// #include "globalsettings.hh"


//----------------------------------------------------------------------------------------------
// Definition of the Run class. This class is supposed to do all gthe organization of a run.
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Run
{
  public:
    Run(boost::filesystem::path p);
    virtual ~Run();

    virtual void LoadRunSettings();


    //virtual void SynchronizeFiles() = 0;

  protected:
    boost::filesystem::path path_;
    int nr_;

    boost::property_tree::ptree settings_;

// int int_nr_;


//
// virtual void SynchronizeFiles() = 0;
//
//
//
// // double GetStartTime();
// // double GetStopTime();
//
// std::tuple<double, double> GetTime();
// int    GetRunNr();
//
// boost::filesystem::path path_run_;
// double tsMin;
// double tsMax;
//
// int run_nr_;
// int int_nr_;
// std::string run_nr_str_;
//
};

class CalibrationRun : public Run
{
  public:
    CalibrationRun(boost::filesystem::path p);
    virtual ~CalibrationRun();

    void SynchronizeCalibrationEvents();
    void PDS_Calibration();
    void GainDetermination();
    void Average1PE();
    //    Gain* LoadGain();
    //    void FitGain();

//    void DeleteHistograms()
    //void Average1PE_Creation();

    void SynchronizePhysicsEvents();
    void PDS_Physics();

    //void LoadH
    //void DeleteHistograms();
    void DeleteCalibrationHistograms();
  private:
    int cal_nr_;

    std::vector<PhysicsEvent*>           evts_;
    std::vector<CalibrationEvent*>       cal_evts_;

    // std::vector<TH1I*> gainhist_;
    // std::vector<double> gain_;
    // std::vector<TH1D*> 1pe_;

//         int BuildOnlineTree();
//         int BuildOfflineTree();
//
//         int MapOnlineRates();
//
//         int WriteOnlineTree(TFile* file);
//         int WriteTimeStamp(TFile* file);
//         int WriteTree(TFile* file, std::string type = "-1" );
//         int WriteNTuple(boost::filesystem::path path_ntuple = "" );
//
//         void Subtract();
//
//         void DrawPedestal();
//
// //        void SaveEvents(boost::filesystem::path fname);
//
//
//         // These methods are meant to be public in the end and used by the user
//         void SynchronizeFiles();
//
//         void LoadData();
//
//             void LoadIntermediate();
//             void LoadMetaData();
//                 void LoadRunSettings();
//             void LoadPhysicsData();
//
//         void SubtractPedestal();
//
//             void LoadPedestal();
//             void CalculatePedestal(); // Outdated, needs to be removed
//             void SavePedestal();
//
//         void SubtractPedestal2();
//
//         void DeletePhysicsData();
//
//         void GainCalibration();
//
//         void Average1PE();
//
//         void WaveformDecomposition();
//
//             void SetUpWaveforms();
//             void FastRate();
//             void Decompose();
//             void Reconstruct();
//             void CalculateChi2();
//
//         void WaveformDecompositionV2();
//
//         void SaveEvents();
//         void SaveRates();
//
//             void LoadEventFiles();
//             void LoadWaveforms();
//
//
//
//
//         TTree *GetOnlineTree();
//         TTree *GetOfflineTree();
//
//         std::vector<IntChannel*> GetIntChannel(std::string name);
//         std::vector<PhysicsChannel*> GetPhysicsChannel(std::string name);
//
//     protected:
//
//
//     private:
//
//         int PathToRunNumber(boost::filesystem::path p);
//
//
//
//         TTree *tree_online;
//         TTree *tree_offline;
//         TTree *tree_skb;
//
//         std::vector<PhysicsEvent*>   events_;
//         std::vector<IntEvent*>       int_events_;
//
//         Pedestal* pedestal_;
//         Gain*     gain_;
//
//         // std::map<std::string, TH1F*>   gain_;
//         std::map<std::string, TH1F*>   average_1pe_;
//
//         int pre_samples_;//         int BuildOnlineTree();
//         int BuildOfflineTree();
//
//         int MapOnlineRates();
//
//         int WriteOnlineTree(TFile* file);
//         int WriteTimeStamp(TFile* file);
//         int WriteTree(TFile* file, std::string type = "-1" );
//         int WriteNTuple(boost::filesystem::path path_ntuple = "" );
//
//         void Subtract();
//
//         void DrawPedestal();
//
// //        void SaveEvents(boost::filesystem::path fname);
//
//
//         // These methods are meant to be public in the end and used by the user
//         void SynchronizeFiles();
//
//         void LoadData();
//
//             void LoadIntermediate();
//             void LoadMetaData();
//                 void LoadRunSettings();
//             void LoadPhysicsData();
//
//         void SubtractPedestal();
//
//             void LoadPedestal();
//             void CalculatePedestal(); // Outdated, needs to be removed
//             void SavePedestal();
//
//         void SubtractPedestal2();
//
//         void DeletePhysicsData();
//
//         void GainCalibration();
//
//         void Average1PE();
//
//         void WaveformDecomposition();
//
//             void SetUpWaveforms();
//             void FastRate();
//             void Decompose();
//             void Reconstruct();
//             void CalculateChi2();
//
//         void WaveformDecompositionV2();
//
//         void SaveEvents();
//         void SaveRates();
//
//             void LoadEventFiles();
//             void LoadWaveforms();
//
//
//
//
//         TTree *GetOnlineTree();
//         TTree *GetOfflineTree();
//
//         std::vector<IntChannel*> GetIntChannel(std::string name);
//         std::vector<PhysicsChannel*> GetPhysicsChannel(std::string name);
//
//     protected:
//
//
//     private:
//
//         int PathToRunNumber(boost::filesystem::path p);
//
//
//
//         TTree *tree_online;
//         TTree *tree_offline;
//         TTree *tree_skb;
//
//         std::vector<PhysicsEvent*>   events_;
//         std::vector<IntEvent*>       int_events_;
//
//         Pedestal* pedestal_;
//         Gain*     gain_;
//
//         // std::map<std::string, TH1F*>   gain_;
//         std::map<std::string, TH1F*>   average_1pe_;
//
//         int pre_samples_;
//         int post_samples_;
//         int int_pre_samples_;
//         int int_post_samples_;
//         int post_samples_;
//         int int_pre_samples_;
//         int int_post_samples_;
};

// class AnalysisRun : public Run
// {
//     public:
//         AnalysisRun(boost::filesystem::path p);
//         virtual ~AnalysisRun();
//
//         void SynchronizeFiles();
//         void LoadMetaData();
//         void LoadPhysicsData();
//
//         void DeleteEvent(int nr = -1);
//         void EraseElement(std::vector<AnalysisEvent*>::iterator itr_vec);
//         void SetCurrentLimit(std::string ring = "none", double low = -1, double high = 10000);
//         void SetInjectionLimit( int type = -1);
//
//         std::vector<AnalysisEvent*> GetEvents();
//
//             // void FilterRuns();
//             // void LoadEvents();
//
//         int NEvents();
//
//         std::vector<AnalysisEvent*>   events_;
//
//         std::map<std::string, Channel*> channels_;
// };

#endif /* CLAWS_RUN_H_ */
