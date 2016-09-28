//============================================================================
// Name        : GlobalSettings.hh
// Author      : Miroslav Gabriel
// Mail        : mgabriel@mpp.mpg.de
// Version     :
// Created on  : Sep 9, 2016
// Copyright   : GNU General Public License
// Description :
//=============================================================================


//project includes
#include "GlobalSettings.hh"


claws::GlobalSettings * GS = new claws::GlobalSettings();

namespace claws {

        std::string StringRunMode(RunMode mode)
        {
            if     (mode == DEV)        return "dev";
            else if(mode == SCOPE)      return "scope";
            else if(mode == FIRST_BEAM) return "first-beam";
            else if(mode == LLAMA)      return "llama";
            else if(mode == CONNECTICUT)return "connecticut";
            else if(mode == ORION)      return "orion";
            else                        return "Who knows!";
        };

        GlobalSettings::GlobalSettings() : hook_("")
        {

        };

        GlobalSettings::~GlobalSettings()
        {
            // TODO Auto-generated destructor stub
        };

        int GlobalSettings::GetNBits()
        {
            return n_bits_;
        }

        int GlobalSettings::GetNBitsScope()
        {
            return n_bits_scope_;
        }

        int GlobalSettings::GetXLow()
        {
            return x_low_;
        }

        int GlobalSettings::GetXUp()
        {
            return x_up_;
        }

        boost::filesystem::path GlobalSettings::GetHook()
        {
            std::cout << hook_ << std::endl;
            return hook_;
        };

        GlobalSettings* GlobalSettings::SetData()
        {
            hook_ = hook_ / path_data_;
            return this;
        };

        GlobalSettings* GlobalSettings::ResetHook()
        {
            // data_type_ = NONE;
            hook_ = "";
            return this;
        };

        GlobalSettings* GlobalSettings::SetNtp()
        {
            // data_type_ = NTP;
            hook_ = hook_ / path_ntp_;
            return this;
        };

        GlobalSettings* GlobalSettings::SetDetector(Detector det)
        {
            if(det == CLW){
                hook_ = hook_ / "CLW";
            }
            else if(det == BEAST)
            {
                hook_ = hook_ / "BEAST";
            }
            else if(det == TPC)
            {
                hook_ = hook_ / "TPC";
            }
            else if(det == BGO)
            {
                hook_ = hook_ / "BGO";
            }
            else if(det == PIN)
            {
                hook_ = hook_ / "PIN";
            }
            else if(det == SKB)
            {
                hook_ = hook_ / "SKB";
            }
            else if(det == SuperKEKB)
            {
                hook_ = hook_ / "SuperKEKB";
            }
            else if(det == SuperKEKB_MachineStudies)
            {
                hook_ = hook_ / "SuperKEKB_MachineStudies";
            }
            else
            {
                hook_ = hook_ / "Who knows!";
            }
            return this;
        };

        GlobalSettings* GlobalSettings::SetRaw()
        {
            // data_type_ = RAW;
            hook_ = hook_ / path_raw_data_;
            return this;
        };

        GlobalSettings* GlobalSettings::SetMode(RunMode mode)
        {
    //        std::cout << mode << std::endl;
            hook_ = hook_ / StringRunMode(mode);
            return this;
        };

        GlobalSettings* GlobalSettings::SetDate(int day, int month, int year ){

            if (day <= 0 || day > 31 || month <= 0 || month > 12 || year != 16)
            {
                std::cerr << "/* error message, date incorrect. */" << std::endl; exit(1);
            }

            std::stringstream date;
            date << boost::format("%02i-%02i-%02i") % year % month % day;
            hook_ = hook_ / ("20" + date.str());

            return this;
        };


        std::vector <boost::filesystem::path> GlobalSettings::GetNtpFiles(float tsMin, float tsMax)
        {
            std::vector <boost::filesystem::path> files;

            if(boost::filesystem::is_regular_file(hook_))
            {
                files.push_back(hook_);
            }
            else if (boost::filesystem::is_directory(hook_)) {
                /* code */
            }

            this->ResetHook();

            return files;
        }

        std::vector <boost::filesystem::path> GlobalSettings::GetRawFiles()
        {
            std::vector <boost::filesystem::path> files;

            if(boost::filesystem::is_regular_file(hook_))
            {
                files.push_back(hook_);
            }
            else if (boost::filesystem::is_directory(hook_)) {


                boost::filesystem::directory_iterator end_itr;
                // cycle through the directory
            	for (boost::filesystem::directory_iterator itr(hook_); itr != end_itr; ++itr)
            	{
            		if (boost::filesystem::is_regular_file(itr->path())) {
            			// If it is a file do not do anything!
            		}

            		else if(boost::filesystem::is_directory(itr->path()) && boost::starts_with(itr->path().filename().string(), "Run-")){
            			// If it is a directory check if it is a Run folder and proceed.
                        files.push_back(itr->path());
            		}
            	}

            }

            this->ResetHook();

            return files;
        }
}
