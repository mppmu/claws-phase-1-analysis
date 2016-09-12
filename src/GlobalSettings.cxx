//============================================================================
// Name        : GlobalSettings.hh
// Author      : Miroslav Gabriel
// Mail        : mgabriel@mpp.mpg.de
// Version     :
// Created on  : Sep 9, 2016
// Copyright   : GNU General Public License
// Description :
//=============================================================================

// c++ includes
#include <sstream>
// boost includes
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

//project includes
#include "GlobalSettings.hh"


claws::GlobalSettings * GS = new claws::GlobalSettings();

namespace claws {

        GlobalSettings::GlobalSettings() : hook_("")
        {

        };


        GlobalSettings::~GlobalSettings()
        {
            // TODO Auto-generated destructor stub
        };

        GlobalSettings& GlobalSettings::ResetHook()
        {
            hook_ = "";
            return *this;
        };

        boost::filesystem::path GlobalSettings::GetHook()
        {
            std::cout << hook_ << std::endl;
            return hook_;
        };

        GlobalSettings& GlobalSettings::SetData()
        {
            hook_ = hook_ / path_data_;
            return *this;
        };

        GlobalSettings& GlobalSettings::SetNtp()
        {
            hook_ = hook_ / path_ntp_;
            return *this;
        };

        GlobalSettings& GlobalSettings::SetRaw()
        {
            hook_ = hook_ / path_raw_data_;
            return *this;
        };

        GlobalSettings& GlobalSettings::SetDate(int day, int month, int year ){

            if (day <= 0 || day > 31 || month <= 0 || month > 12 || year != 16)
            {
                std::cerr << "/* error message, date incorrect. */" << std::endl; exit(1);
            }

            std::stringstream date;
            date << boost::format("%02i-%02i-%02i") % year % month % day;
            hook_ = hook_ / date.str();

            return *this;
        };

        vector <boost::filesystem::path> GlobalSettings::GetFiles(int tsMin, int tsMax)
        {
            vector <boost::filesystem::path> files;

            if(boost::is_regular_file(hook_))
            {
                files.push_back(hook_);
            }
            else if (boost::is_directory(hook_)) {
                /* code */
            }

            this->ResetHook();

            return files;
        }


}
