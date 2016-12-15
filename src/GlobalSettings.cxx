//============================================================================
// Name        : GlobalSettings.hh
// Author      : Miroslav Gabriel
// Mail        : mgabriel@mpp.mpg.de
// Version     :
// Created on  : Sep 9, 2016
// Copyright   : GNU General Public License
// Description :
//=============================================================================

// std includes
#include <time.h>
#include <sys/time.h>

//project includes
#include "GlobalSettings.hh"


claws::GlobalSettings * GS = new claws::GlobalSettings();

namespace claws {

    double get_wall_time()
    {
        struct timeval time;
        if (gettimeofday(&time,NULL))
        {
            //  Handle error
            return 0;
        }
        return (double)time.tv_sec + (double)time.tv_usec * .000001;
    }

    double get_cpu_time(){
        return (double)clock() / CLOCKS_PER_SEC;
    }

        void ProgressBar(float progress, int show_step_size){
            if(progress <= 1.0){
                if(int(progress*100)%show_step_size==0){
                    int barWidth = 80;
                    std::cout << "[";
                    int pos = barWidth * progress;
                    for(int i = 0; i < barWidth; ++i){
                        if(i< pos) std::cout << "=";
                        else if (i == pos) std::cout << ">";
                        else std::cout << " ";
                    }
                    std::cout << "] " << int(progress * 100.0) << "%\r";
                    std::cout.flush();
                }

            }
            if (progress == 1.0) std::cout << "\n" << std::endl;
        };


        double ConvertRange(int in){
            double ranges[] = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};
            return ranges[in];
        };

        int8_t ConvertOffset(double offset, int rn)
        {
            double range = ConvertRange(rn);

            if(offset >= 0){
                return int8_t(offset*1000/range * 127);
            }
            else if(offset < 0)
            {
                return int8_t(offset*1000/range * 128);
            }
            else
            {
                std::cout << "Couldn't convert Offset!" << std::endl;
                return 0;
            }
        };

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

        std::string indent(int level) {
          std::string s;
          for (int i=0; i<level; i++) s += "  ";
          return s;
        }

        void printTree (pt::ptree &pt, int level) {
          if (pt.empty()) {
            std::cerr << "\""<< pt.data()<< "\"";
          } else {
            if (level) std::cerr << std::endl;
            std::cerr << indent(level) << "{" << std::endl;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end();) {
              std::cerr << indent(level+1) << "\"" << pos->first << "\": ";
              printTree(pos->second, level + 1);
              ++pos;
              if (pos != pt.end()) {
                std::cerr << ",";
              }
              std::cerr << std::endl;
            }
            std::cerr << indent(level) << " }";
          }
          return;
        }

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
