/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_EVENT_H_
#define CLAWS_EVENT_H_

//std includes
// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <map>
// #include <string>
// #include <cstdlib>
// #include <typeinfo>

// --- BOOST includes ---
#include <boost/property_tree/ptree.hpp>
// #define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
// #undef BOOST_NO_CXX11_SCOPED_ENUMS
//
// #include <boost/lexical_cast.hpp>
// #include <boost/algorithm/string/predicate.hpp>
// #include <boost/algorithm/string/replace.hpp>

// // #include <boost/program_options.hpp>
// // #include <boost/filesystem/fstream.hpp>
// // #include <boost/algorithm/string/predicate.hpp>
// // #include <boost/foreach.hpp>
//
//
// // gperf
// // #include <gperftools/heap-profiler.h>
// // #include <gperftools/profiler.h>
//
// // root includes
// #include <TFile.h>
// #include <TH1D.h>
// #include <TH1I.h>
// #include "TApplication.h"
// #include <TCanvas.h>
// #include <TF1.h>

// Project includes
#include "channel.hh"
#include "gain.hh"


// using namespace boost;
// //needed for all the paths
// using namespace boost::filesystem;


//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

enum EventState
{
    EVENTSTATE_INIT,
    EVENTSTATE_RAW,
    EVENTSTATE_PREP,
    EVENTSTATE_PDFILLED,
    EVENTSTATE_PDSUBTRACTED,
    EVENTSTATE_OSCORRECTED,
    EVENTSTATE_PDFAILED,
    EVENTSTATE_OSFAILED,
    EVENTSTATE_WFDECOMPOSED,
    EVENTSTATE_WFDFAILED,
    EVENTSTATE_WFRECONSTRUCTED,
    EVENTSTATE_WFRFAILED,
    // COLOR_GREEN, // assigned 3
    // COLOR_WHITE, // assigned 4
    // COLOR_CYAN, // assigned 5
    // COLOR_YELLOW, // assigned 6
    // COLOR_MAGENTA // assigned 7
};

/** Usually definitions should go into the .cxx file because otherwise
*  the complier complains about multiple definitions. To avoid this
*  the definition is inlined.
*/
inline std::string printEventState(EventState state)
{
  switch(state)
  {
    case EVENTSTATE_INIT:
      return "init";
    case EVENTSTATE_RAW:
      return "raw";
    case EVENTSTATE_PREP:
        return "prepared";
    case EVENTSTATE_PDFILLED:
      return "pd_filled";
    case EVENTSTATE_PDSUBTRACTED:
      return "pd_subtracted";
    case EVENTSTATE_OSCORRECTED:
      return "os_corrected";
    case EVENTSTATE_OSFAILED:
      return "os_failed";
    case EVENTSTATE_PDFAILED:
      return "pd_failed";
    case EVENTSTATE_WFDECOMPOSED:
      return "wf_decomposed";
    case EVENTSTATE_WFDFAILED:
      return "wfd_failed";
    case EVENTSTATE_WFRECONSTRUCTED:
      return "wf_reconstructed";
    case EVENTSTATE_WFRFAILED:
      return "wfr_failed";
    default:
      return "Invalid Selection";
  }
}

// enum EventType
// {
//     EVENTTYPE_EVENT
//     EVENTTYPE_CALIBRATION,
//     EVENTTYPE_PHYSICS,
//     EVENTTYPE_ANALYSIS,
//     // COLOR_GREEN, // assigned 3
//     // COLOR_WHITE, // assigned 4
//     // COLOR_CYAN, // assigned 5
//     // COLOR_YELLOW, // assigned 6
//     // COLOR_MAGENTA // assigned 7
// };
//
// inline std::string printEventType(EventType type)
// {
//   switch(type)
//   {
//     case EVENTTYPE_EVENT:
//       return "event";
//     case EVENTTYPE_CALIBRATION:
//       return "calibration";
//     case EVENTTYPE_PHYSICS:
//       return "physics";
//     case EVENTTYPE_ANALYSIS:
//       return "analysis";
//     default:
//       return "Invalid Selection";
//   }
// }

class Event{

    public:
        Event();
        Event( boost::filesystem::path file, boost::filesystem::path ini_file );

        virtual ~Event();

        virtual void LoadFiles();
            virtual void LoadRaw();

                virtual void LoadHistograms(boost::filesystem::path file);


        virtual void PrepHistograms();
        virtual void SaveEvent(boost::filesystem::path dst, bool save_pd = false);

        virtual void DeleteHistograms();

        virtual void FillPedestals();
        virtual void SubtractPedestals(std::vector<double> pd = {});

        virtual void   SetTime(double unixtime);
        virtual double GetTime();
        virtual int    GetNumber();
        virtual std::vector<Channel*> GetChannels();
        virtual EventState GetState();
        template<typename T>
        T GetParameter(std::string pv)
        {
            return pt_.get<T>( pv );
        };

    protected:
       int nr_;
       EventState state_;
    //   EventType type_;
       double unixtime_        = -1;
       boost::filesystem::path path_;
       boost::filesystem::path file_;
       boost::filesystem::path ini_file_;
       boost::property_tree::ptree pt_;

       std::vector<Channel*> channels_;

//         virtual void LoadIniFile() = 0;
//
//         void                                CalculateIntegral();
//
//         std::map<std::string, double>       GetIntegral();
//         std::vector< double>                GetIntegralVec();
//
//
//         void                                SetBaseline(std::map<std::string, float> baseline);
//
//         int                                 GetNr()   const;

//         std::map<std::string, TH1I*>                    GetPedestal();
//         virtual Channel*                                GetChannel(std::string name);
//         std::map<std::string, Channel*>                 GetChannels();
//         std::map<std::string, std::vector<float>*>      GetWaveforms();
//

//         virtual std::map<std::string, TH1*> GetHistograms();

};

class CalibrationEvent : public Event{

    public:

        // PhysicsEvent();
        CalibrationEvent(boost::filesystem::path file, boost::filesystem::path ini_file, double unixtime = -1);

        virtual void LoadFiles(EventState state = EVENTSTATE_RAW);
            virtual void LoadSubtracted();

        virtual ~CalibrationEvent();
        virtual void PrepHistograms();


    protected:

    //    std::vector<CalibrationChannel*> channels_;
//
//         void                   LoadIniFile();
// //        IntChannel*                                GetChannel(std::string name);
// //        std::map<std::string, double> GetIntegral();/remote/ceph/group/ilc/claws/data/RAW/connecticut/2016-05-23/Run-401141
//
//         double mean_online_[8];
//         double accepted_online_[8];
// //        std::map<std::string, IntChannel*> channels_;
//

};

/* This is called a "forward declaration".  We use it to tell the compiler that the
   identifier "B" will from now on stand for a class, and this class will be defined
   later.  We will not be able to make any use of "B" before it has been defined, but
   we will at least be able to declare pointers to it. */
class Gain;


/**
*   Description of the PhysicsEvent class.
*/
class PhysicsEvent : public Event{

    public:

        // PhysicsEvent();
        PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file);
        PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file, boost::filesystem::path rate_file);
        virtual ~PhysicsEvent();

        virtual void LoadFiles(EventState state = EVENTSTATE_RAW);
            virtual void LoadSubtracted();
            virtual void LoadOSCorrected();
            virtual void LoadWFDecomposed();

        virtual void SaveEvent(boost::filesystem::path dst);

        virtual void PrepHistograms( boost::property_tree::ptree &settings );

        virtual std::vector<std::vector<OverShootResult>> OverShootCorrection();

        virtual void PrepareDecomposition();
        virtual void WaveformDecomposition(Gain* gain);
        virtual void WaveformReconstruction(Gain* gain);
    //    virtual void PrepHistograms();
    private:
        boost::filesystem::path rate_file_;
};
//
// //        void                   LoadRootFile();
//         void                   LoadIniFile();
//         void                   LoadOnlineRate();
//
//         void                   SetUpWaveforms();
//         void                   SetUpWaveformsV2();
//         void                   DeleteWaveforms();
//         void                   FastRate(std::map<std::string, std::vector<float>*> avg_waveforms, std::map<std::string, double> pe_to_mips);
//         void                   Rate(std::map<std::string, double> pe_to_mips);
//
//         void                   Decompose(std::map<std::string, std::vector<float>*> avg_waveforms);
//         void                   Reconstruct(std::map<std::string, std::vector<float>*> avg_waveforms);
//         void                   CalculateChi2();
//         void                   SaveEvent(boost::filesystem::path result_folder, std::string type = "raw");
//
//
//         // Type O: online       double[6]
//         // Type 1: fast offline double[3]
//         // Type 2: offline      double[3]
//         double*                GetRate( int type = 0);
//
//         int                    GetLerBg()     const;
//         int                    GetHerBg()     const;
//         bool                   GetInjection() const;
//         int                    GetScrubbing() const;
//
//
//         virtual void CreateHistograms(std::string type = "raw");
//         virtual std::map<std::string, TH1*> GetHistograms(std::string type = "raw");
//

//
// //        std::map<std::string, double> GetIntegral(); // Pure Placeholder fo far
//
//         boost::property_tree::ptree pt_;
//         path path_online_rate_;
//
//
//         int lerbg_              = -1;
//         int herbg_              = -1;
//         bool injection_         = false;
//         std::string kekb_status_    = "";
//         std::string ler_status_ ="";
//         std::string her_status_ = "";
//         int scrubbing_        = 0;
//
//         double online_rate_[6];
//         double fast_rate_[3];
//         double rate_[3];
//
//
// };



// struct Rate
// {
//     double rate_online[6] = {};
//     double rate_fast[3] = {};
//     double decomposition[3] = {};
//     double rate[3] = {};
//     int count = 0;
//
//     Rate& operator+(const Rate& rhs){
//         for(int i =0; i < 6; i++) rate_online[i] += rhs.rate_online[i];
//         for(int i =0; i < 3; i++) rate_fast[i] += rhs.rate_fast[i];
//         for(int i =0; i < 3; i++) decomposition[i] += rhs.decomposition[i];
//         for(int i =0; i < 3; i++) rate[i] += rhs.rate[i];
//         count ++;
//         return *this;
//     }
// };
//
//
// class AnalysisEvent : public Event
// {
//     public:
//         AnalysisEvent();
//         AnalysisEvent(const path &file_root, const path &file_ini);
//         virtual ~AnalysisEvent();
//
//         virtual void LoadIniFile();
//         void AddEvent(AnalysisEvent* evt);
//         void Normalize();
//         void SetErrors(double err = 0);
//         void RunPeak();
//         void RunFFT();
//
//         std::tuple<double, double>                GetCurrent();
//         std::tuple<bool, double, bool, double >   GetInjection();
//         std::map<std::string, TH1*> GetHistograms();
//         Rate& GetRates();
//
//         void                   SaveEvent(boost::filesystem::path result_folder);
//
//         int n_evts_                         = 0;
//         double unixtime_                    = -1;
//         boost::property_tree::ptree pt_;
//
//         Rate rates;
//
// };


#endif /* CLAWS_EVENT_H_ */
