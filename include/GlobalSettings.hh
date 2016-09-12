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

// boost includes
#include <boost/filesystem.hpp>


namespace claws {

class GlobalSettings
{
    /*
        Basic interface class to managed everything related to the location of the
        data on the individual system.
    */

    public:

        GlobalSettings();
        virtual ~GlobalSettings();

        GlobalSettings& ResetHook();
        boost::filesystem::path GetHook();

        GlobalSettings& SetData();
        GlobalSettings& SetNtp();
        GlobalSettings& SetRaw();
        GlobalSettings& SetDate(int day, int month, int year = 16);

        vector <boost::filesystem::path> GetFiles(int tsMin = 0, tsMax = 0);

    private:

        boost::filesystem::path hook_;

        const boost::filesystem::path path_data_     = "/remote/ceph/group/ilc/claws/data";
        const boost::filesystem::path path_ntp_      = "/NTP";
        const boost::filesystem::path path_raw_data_ = "/RAW";

};

};

extern claws::GlobalSettings * GS;

#endif /* GLOBAL_SETTINGS_H_ */
