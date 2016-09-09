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
    public:

        GloablSettings() {std::cout << "asdad" << std::endl;}
    //    virtual ~GloablSettings()
        // {
        //     // TODO Auto-generated destructor stub
        // };


    private:
        boost::filesystem::path path_ntp      = "/remote/ceph/group/ilc/claws/data/NTP";
        boost::filesystem::path path_raw_data = "/remote/ceph/group/ilc/claws/data/RAW";

};

};

claws::GlobalSettings * GS = new claws::GlobalSettings();

#endif /* GLOBAL_SETTINGS_H_ */
