//============================================================================
// Name        : calibration.xx
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================

// --- C++ includes ---
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <typeinfo>
#include <fstream>
#include <iomanip>

// --- boost includes ---
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

// --- root includes ---
#include "TFile.h"
#include "TSystem.h"
#include "TKey.h"
#include "TTree.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TApplication.h"
#include "TCanvas.h"
#include <TStyle.h>

// --- project includes ---
#include "run.hh"
#include "globalsettings.hh"


int main(int argc, char* argv[]) {

		cout << "---------------------------------------------------------" << endl;
		cout << "|            CLAWS phase I particle reconstruction       |" << endl;
		cout << "---------------------------------------------------------" << endl;

		boost::program_options::options_description options("Generic options");
		options.add_options()
		        ("help", "Displays this help message.")
		        ("config-file,c",   boost::program_options::value<boost::filesystem::path>()->default_value("./config/calibration_phase1.ini"), "Config file to get parameters from.")
		        ("pe-file",         boost::program_options::value<boost::filesystem::path>()->default_value("./config/pe_to_mip.ini"), "Config file to get conversion from pe to MIP.")
		        ("data.input",      boost::program_options::value<boost::filesystem::path>()->default_value("./"), "Data directory containing runs meant to be analysed.")
		        ("write-ntp",       boost::program_options::value<bool>()->default_value(false), "Data directory containing runs meant to be analysed.")
		        ("profile-timing",  boost::program_options::value<bool>()->default_value(false), "Displays timing info for the individual steps.")
		        ("tasks",
		        boost::program_options::value<std::string>()->default_value("0"),
		        "Task to be processed:\n0: run the complete calibration cahin\
									  \n1: pedestal subtraction of the calibration waveforms\
									  \n2: gain determination\n3: calculation of the average 1 pe waveform\
									  \n4: pedestal subtraction on physics waveforms\
									  \n5: overshoot compensation\
									  \n6: "                                                                                                            )
		;

		boost::program_options::variables_map config_map;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), config_map);
		boost::program_options::notify(config_map);

		if (config_map.count("help")) {
				// std::cout << generic_options << "\n";
				std::cout << options << "\n";
				return 1;
		}

		std::ifstream ifs(config_map["config-file"].as<boost::filesystem::path>().c_str());
		if (!ifs)
		{
				cout << "Can not open config file: " << config_map["config-file"].as<boost::filesystem::path>() << "\n";
				return 0;
		}
		else
		{
				std::cout << "Using config file: " << config_map["config-file"].as<boost::filesystem::path>() << ".\n";
				GS->LoadCalibrationConfig(config_map["config-file"].as<boost::filesystem::path>());
		}

		std::ifstream ifs2(config_map["pe-file"].as<boost::filesystem::path>().c_str());
		if (!ifs2)
		{
				cout << "Can not open pe file: " << config_map["pe-file"].as<boost::filesystem::path>() << "\n";
				return 0;
		}
		else
		{
				std::cout << "Using pe file: " << config_map["pe-file"].as<boost::filesystem::path>() << ".\n";
				GS->LoadPeToMip(config_map["pe-file"].as<boost::filesystem::path>());
		}

		std::cout << config_map["data.input"].as<boost::filesystem::path>() << "\n";

		/**		Determine the tasks to be processed
		 *		\todo adapt number of tasks
		 *		@param i [ 0: run the complete calibration cahin
		 *				   1: pedestal subtraction of the calibration waveforms
		 *				   2: gain determination
		 *                 3: calculation of the average 1 pe waveform
		 *                 4: pedestal subtraction on physics waveforms
		 *                 5: overshoot compensation
		 *                 6: Waveform decomposition
		 *				   7: Waveform reconstruction ]
		 */

		bool tasks[10] = {false};
		std::string tasks_names[10] = {"Calibration waveform pedestal subtraction", "Gain determination", "Averaging of 1 pe waveforms", "Physics waveform pedestal subtraction", "Overshoot correction", "Signal tagging", "Waveform decomposition", "Waveform reconstruction", "MIP time retrieval", "Systematics study"};

		std::string tasks_tmp = config_map["tasks"].as<std::string>();
		bool profile_timing   = config_map["profile-timing"].as<bool>();

		if(tasks_tmp.size() == 1 || tasks_tmp.size() == 2)
		{
				if(std::stoi(tasks_tmp) == 0)
				{
						for(int i = 0; i<10; i++) tasks[i]=true;
				}
				else
				{
						tasks[std::stoi( tasks_tmp ) -1 ] = true;
				}
		}
		else if (tasks_tmp.size() == 3)
		{
				for(int i = std::stoi(tasks_tmp.substr(0,1)) -1; i<std::stoi( tasks_tmp.substr(2,1) ); i++) tasks[i]=true;
		}

		else if (tasks_tmp.size() == 4)
		{
				int found = tasks_tmp.find("-");

				for(int i = std::stoi(tasks_tmp.substr(0,found)) -1; i<std::stoi( tasks_tmp.substr(found+1) ); i++) tasks[i]=true;
		}

		else
		{
				assert(false);
		}

		std::cout << "\033[1;31mRunning following tasks: \n";

		for(int i = 0; i<10; ++i)
		{
				if(tasks[i]) std::cout << tasks_names[i] << "\n";
		}

		std::cout << "\033[0m" << std::endl;

		std::vector <CalibrationRun*> runs;

		for(auto run_name : GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>()) )
		{
				CalibrationRun* run = new CalibrationRun(run_name);

				run->SynchronizePhysicsEvents();
				run->SynchronizeCalibrationEvents();

				run->LoadRunSettings();

				/**	Pedestal subtraction on the calibration waveforms
				 *
				 */
				if(tasks[0])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->PDS_Calibration();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Gain determination on the calibration waveforms
				 *
				 */
				if(tasks[1])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->GainDetermination();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Calculation of average 1 pe calibration waveform
				 *
				 */
				if(tasks[2])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->Average1PE();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Pedestal subtraction on the physics waveforms
				 *
				 */
				if(tasks[3])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->PDS_Physics();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Correct for overshoots in the physics waveforms
				 *
				 */
				if(tasks[4])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->OverShootCorrection();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Tag all signals in the physics waveforms
				 *
				 */
				if(tasks[5])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->SignalTagging();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Decompose the physics waveforms
				 *
				 */
				if(tasks[6])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->WaveformDecomposition();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Reconstruct the physics waveforms to make sure the decomposition worked
				 *
				 */
				if(tasks[7])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->WaveformReconstruction();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Determine the arrival time of the MIPs
				 *
				 */
				if(tasks[8])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->MipTimeRetrieval();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				/**	Extract benchmark quantities for the systematic study
				 *
				 */
				if(tasks[9])
				{
						double wall0 = claws::get_wall_time();
						double cpu0  = claws::get_cpu_time();

						run->SystematicsStudy();

						double wall1 = claws::get_wall_time();
						double cpu1  = claws::get_cpu_time();

						if(profile_timing)
						{
								cout << "Wall Time = " << wall1 - wall0 << endl;
								cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						}
				}

				delete run;

		}

		return 0;

} // File done
