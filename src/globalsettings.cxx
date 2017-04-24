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
#include <stdio.h>
#include <sys/time.h>


//project includes
#include "globalsettings.hh"


std::unique_ptr<claws::GlobalSettings>  GS = std::unique_ptr<claws::GlobalSettings>(new claws::GlobalSettings);

namespace claws {

    bool CheckIntFolder(boost::filesystem::path p)
    {
        // Function returns true if folder exists and is not empty.
        bool int_folder_good = false;
        if( exists(p) )
        {
            if( !boost::filesystem::is_empty(p) ) int_folder_good = true;
        }
        return int_folder_good;
    };

    void handler(int sig) {
        void *array[10];
        size_t size;

        // get void*'s for all entries on the stack
        size = backtrace(array, 10);

        // print out all the frames to stderr
        fprintf(stderr, "Error: signal %d:\n", sig);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        exit(1);
    }

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

    void print_local_time()
    {
        time_t rawtime;
        struct tm*timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        printf("Date: %s", asctime(timeinfo));
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
          boost::property_tree::ini_parser::read_ini(boost::filesystem::path("./config/pe_to_mip.ini").string(), pe_to_mip_);
          boost::property_tree::ini_parser::read_ini(boost::filesystem::path("./config/config_architecture.ini").string(), config_architecture_);
          boost::property_tree::ini_parser::read_ini(boost::filesystem::path("./config/config_calibration.ini").string(), config_calibration_);
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

        float GlobalSettings::GetXLow()
        {
            return x_low_;
        }

        float GlobalSettings::GetXUp()
        {
            return x_up_;
        }

        std::vector <std::string> GlobalSettings::GetChannels(int type)
        {
            if(type == 0){
                std::vector<std::string> tmp = channels_;
                tmp.insert(std::end(tmp),std::begin(int_channels_),std::end(int_channels_));
                return tmp;
            }
            else if (type == 1) return channels_;
            else if (type == 2) return int_channels_;
            else
            {
              std::cout << "Wrong value for type given in GlobalSettings::GetChannels(int type)~!" << std::endl;
            }
        };

        double GlobalSettings::GetAcceptedGain()
        {
            return accepted_gain_;
        };

        std::pair<double, double> GlobalSettings::GetPEtoMIP(std::string detector, unsigned int time)
        {
            if(detector == "FWD1")
            {
                if(time < 1463716800)
                {
                    detector += "/1";
                }
                else
                {
                    detector += "/2";
                }
            }
            else if (detector == "BWD1")
            {
                if(time < 1463716800)
                {
                    detector += "/1";
                }
                else
                {
                    detector += "/2";
                }
            }
            else if (detector == "BWD2")
            {
                if(time < 1463716800)
                {
                    detector += "/1";
                }
                else
                {
                    detector += "/2";
                }
            }
            return std::make_pair(pe_to_mip_.get<double>( detector + ".Value" ), pe_to_mip_.get<double>( detector + ".Error" ) );
        };

        std::map<std::string, double> GlobalSettings::GetPEtoMIPs( unsigned int time )
        {
            std::map<std::string, double> rtn;
            for(auto & ivec : channels_)
            {
                if(!boost::algorithm::ends_with(ivec, "4"))
                {
                    std::string ini_key = ivec;
                    if(ivec == "FWD1")
                    {
                        if(time < 1463716800)
                        {
                            ini_key += "/1";
                        }
                        else
                        {
                            ini_key += "/2";
                        }
                    }
                    else if (ivec == "BWD1")
                    {
                        if(time < 1463716800)
                        {
                            ini_key += "/1";
                        }
                        else
                        {
                            ini_key += "/2";
                        }
                    }
                    else if (ivec == "BWD2")
                    {
                        if(time < 1463716800)
                        {
                            ini_key += "/1";
                        }
                        else
                        {
                            ini_key += "/2";
                        }
                    }
                    rtn[ivec] = pe_to_mip_.get<double>( ini_key + ".Value" );
                }
            }
            return rtn;
        }

        int GlobalSettings::GetNThreads()
        {
            return n_threads_;
        };

        boost::filesystem::path GlobalSettings::GetHook()
        {
            //std::cout << hook_ << std::endl;
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
        };

        std::vector <boost::filesystem::path> GlobalSettings::GetRuns(boost::filesystem::path p)
        {
            std::vector <boost::filesystem::path> runs;
            // if(boost::filesystem::is_regular_file(p))
            // {
            //     files.push_back(p);
            // }
            if ( boost::filesystem::is_directory(p) && boost::starts_with(p.filename().string(), "Run-") )
            {
                std::cout << p.string() <<std::endl;
                runs.push_back(p);
            }
            else if (boost::filesystem::is_directory(p))
            {
                std::vector <boost::filesystem::path> folder_content;
                copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(folder_content));
                std::sort(folder_content.begin(), folder_content.end());

                boost::filesystem::directory_iterator end_itr;
                // cycle through the directory
            //	for (boost::filesystem::directory_iterator itr(p); itr != end_itr; ++itr)
                for (std::vector <boost::filesystem::path>::const_iterator itr(folder_content.begin()), it_end(folder_content.end()); itr != it_end; ++itr)
            	{
            		if (boost::filesystem::is_regular_file(*itr))
                    {
            			// If it is a file do not do anything!
            		}
            		else if(boost::filesystem::is_directory(*itr) && boost::starts_with((*itr).filename().string(), "Run-"))
                    {
            			// If it is a directory check if it is a Run folder and proceed.
                        runs.push_back(*itr);
            		}
            	}
            }
            return runs;
        };

        void GlobalSettings::SaveConfigFiles(boost::filesystem::path folder)
        {
            boost::property_tree::write_ini((folder/"pe_to_mip.ini").string(), pe_to_mip_);
            boost::property_tree::write_ini((folder/"config_architecture.ini").string(), config_architecture_);
            boost::property_tree::write_ini((folder/"config_calibration.ini").string(), config_calibration_);
        }
}
