//============================================================================
// Name        : GlobalSettings.hh
// Author      : Miroslav Gabriel
// Mail        : mgabriel@mpp.mpg.de
// Version     :
// Created on  : Sep 9, 2016
// Copyright   : GNU General Public License
// Description :
//=============================================================================

#ifndef GLOBAL_SETTINGS_H_
#define GLOBAL_SETTINGS_H_


// c++ includes
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <utility>
#include <tuple>
// boost includes
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <TF1.h>

namespace pt = boost::property_tree;



namespace claws {

    enum enPS6000Range
    {
        PS6000_10MV,
        PS6000_20MV,
        PS6000_50MV,
        PS6000_100MV,
        PS6000_200MV,
        PS6000_500MV,
        PS6000_1V,
        PS6000_2V,
        PS6000_5V,
        PS6000_10V,
        PS6000_20V,
        PS6000_50V,
        PS6000_MAX_RANGES
    };

   inline double RangeToVoltage(enPS6000Range range)
    {
        switch(range)
        {
            case PS6000_10MV:
                return 10;
            case PS6000_20MV:
                return 20;
            case PS6000_50MV:
                return 50;
            case PS6000_100MV:
                return 100;
            case PS6000_200MV:
                return 200;
            case PS6000_500MV:
                return 500;
            case PS6000_1V:
                return 1000;
            case PS6000_2V:
                return 2000;
            case PS6000_5V:
                return 5000;
            case PS6000_10V:
                return 10000;
            case PS6000_20V:
                return 20000;
            case PS6000_50V:
                return 50000;
            case PS6000_MAX_RANGES:
                return 0;
            default:
                return 0;
        }
    }

    enum Detector
    {
        CLW,                      // assigned 0
        BEAST,                      // assigned 1
        TPC,                        // assigned 2
        BGO,                        // assigned 3
        PIN,                        // assigned 4
        SKB,                        // assigned 5
        SuperKEKB,
        SuperKEKB_MachineStudies
    };

    enum RunMode
    {
        DEV,            // assigned 0
        SCOPE,          // assigned 1
        FIRST_BEAM,     // assigned 2
        LLAMA,          // assigned 3
        CONNECTICUT,    // assigned 4
        ORION           // assigned 5
    };

    bool CheckIntFolder(boost::filesystem::path p);
    double get_wall_time();
    double get_cpu_time();
    void print_local_time();

    void ProgressBar(float progress, int show_step_size = 1);
    double ConvertRange(int in);
    int8_t ConvertOffset(double offset, int rn);
    std::string StringRunMode(RunMode mode);
    std::string indent(int level);
    void printTree (boost::property_tree::ptree &pt, int level=0) ;
    void handler(int sig);

    class GlobalSettings
    {
        /*
            Basic interface class to managed everything related to the location of the
            data on the individual system.
        */

        public:

            GlobalSettings();
            virtual ~GlobalSettings();

            std::vector<std::string> GetChannelList(std::string type = "Physics");
            void LoadCalibrationConfig(boost::filesystem::path p);
            void LoadPeToMip(boost::filesystem::path p);
            int GetNBits();
            int GetNBitsScope();
            float GetXLow();
            float GetXUp();
            // std::vector <std::string> GetChannels(int type=0);
            boost::property_tree::ptree GetChannels(std::string type);
            double GetAcceptedGain();
            std::pair<double, double> GetPEtoMIP(std::string detector, unsigned int time = 0);
            std::map<std::string, double> GetPEtoMIPs( unsigned int time = 0);
            int GetNThreads();
            boost::filesystem::path GetHook();
            boost::filesystem::path GetDetector();
            TF1* GetOverShootFunction();

            GlobalSettings* ResetHook();
            GlobalSettings* SetData();
            GlobalSettings* SetNtp();
            GlobalSettings* SetDetector(Detector det);
            GlobalSettings* SetRaw();
            GlobalSettings* SetMode(RunMode mode);
            GlobalSettings* SetDate(int day, int month, int year = 16);




            std::vector <boost::filesystem::path> GetNtpFiles(float tsMin = 0, float tsMax = 0);
            std::vector <boost::filesystem::path> GetRuns(boost::filesystem::path p);

            template<typename T>
            T GetParameter(std::string par_name)
            {
                return config_calibration_.get<T>( par_name );
            };


            void SaveConfigFiles(boost::filesystem::path folder);


        private:

            const int n_threads_     = 7;
            const int n_bits_       = 65536;
            const int n_bits_scope_  = 256;

            const float x_low_        = - n_bits_scope_/2 -0.5;
            const float x_up_         =  x_low_ +  n_bits_scope_;

            std::vector <std::string>   channels_ = {"FWD1", "FWD2", "FWD3", "FWD4"
						                              ,"BWD1", "BWD2", "BWD3", "BWD4"
                                                    };

            std::vector <std::string>   int_channels_ = {"FWD1-INT", "FWD2-INT", "FWD3-INT"
                                                          ,"BWD1-INT", "BWD2-INT", "BWD3-INT"
                                                        };

            // pttree containing settings from ini files
            boost::property_tree::ptree pe_to_mip_;
            boost::property_tree::ptree config_architecture_;
            boost::property_tree::ptree config_calibration_;

    //        std::string intermediate_suffix_          = GS->GetIntSuffix();
            double accepted_gain_             = 0.35;  // +- from mean gain accepted for avg wfs.

            boost::filesystem::path hook_;

        // enum data_type_ {NONE, RAW, NTP}             = NONE;

            const boost::filesystem::path path_data_     = "/remote/ceph/group/ilc/claws/data";
            const boost::filesystem::path path_ntp_      = "/NTP";
            const boost::filesystem::path path_raw_data_ = "/RAW";

            TF1* overshoot_function;

    };

};

extern std::unique_ptr<claws::GlobalSettings>  GS;

#endif /* GLOBAL_SETTINGS_H_ */
