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
// boost includes
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>


namespace claws {

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


std::string StringRunMode(RunMode mode);

class GlobalSettings
{
    /*
        Basic interface class to managed everything related to the location of the
        data on the individual system.
    */

    public:

        GlobalSettings();
        virtual ~GlobalSettings();

        boost::filesystem::path GetHook();
        boost::filesystem::path GetDetector();

        GlobalSettings* ResetHook();
        GlobalSettings* SetData();
        GlobalSettings* SetNtp();
        GlobalSettings* SetDetector(Detector det);
        GlobalSettings* SetRaw();
        GlobalSettings* SetMode(RunMode mode);
        GlobalSettings* SetDate(int day, int month, int year = 16);

        std::vector <boost::filesystem::path> GetNtpFiles(float tsMin = 0, float tsMax = 0);
        std::vector <boost::filesystem::path> GetRawFiles();

    private:

        boost::filesystem::path hook_;

        // enum data_type_ {NONE, RAW, NTP}             = NONE;

        const boost::filesystem::path path_data_     = "/remote/ceph/group/ilc/claws/data";
        const boost::filesystem::path path_ntp_      = "/NTP";
        const boost::filesystem::path path_raw_data_ = "/RAW";

};

};

extern claws::GlobalSettings * GS;

#endif /* GLOBAL_SETTINGS_H_ */
