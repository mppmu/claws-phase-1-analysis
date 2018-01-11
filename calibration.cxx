//============================================================================
// Name        : main.cpp
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
//#include "event.hh"
#include "run.hh"
#include "globalsettings.hh"

using namespace std;
using namespace boost;
using namespace boost::filesystem;
// using namespace po = boost::program_options;
namespace pt = boost::property_tree;


int error(){
		throw "DeiMudder";
		return 0;
}



int main(int argc, char* argv[]) {

		cout << "---------------------------------------------------------" << endl;
		cout << "|            CLAWS phase I data calibration             |" << endl;
		cout << "---------------------------------------------------------" << endl;

		boost::program_options::options_description options("Generic options");
		options.add_options()
		        ("help", "Displays this help message.")
		        ("config-file,c",   boost::program_options::value<boost::filesystem::path>()->default_value("./config/calibration_phase1.ini"), "Config file to get parameters from.")
		        ("pe-file",         boost::program_options::value<boost::filesystem::path>()->default_value("./config/pe_to_mip.ini"), "Config file to get conversion from pe to MIP.")
		        ("data.input",      boost::program_options::value<boost::filesystem::path>()->default_value("./"), "Data directory containing runs meant to be analysed.")
		        ("write-ntp",       boost::program_options::value<bool>()->default_value(false), "Data directory containing runs meant to be analysed.")
		        ("tasks",
		        boost::program_options::value<std::string>()->default_value("0"),
		        "Task to be processed:\n0: run the complete calibration cahin\n1: pedestal subtraction of the calibration waveforms\n1: gain determination\n2: calculation of the average 1 pe waveform\n3: pedestal subtraction on physics waveforms\n4: overshoot compensation\n5: ")
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
		 *		@param i [0: run the complete calibration cahin
		 *							1: pedestal subtraction of the calibration waveforms
		 *							2: gain determination
		 *              3: calculation of the average 1 pe waveform
		 *              4: pedestal subtraction on physics waveforms
		 *              5: overshoot compensation]
		 */

		bool tasks[10] = {false};

		std::string tasks_tmp = config_map["tasks"].as<std::string>();

		if(tasks_tmp.size() == 1)
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
		else
		{
				assert(false);
		}


		//std::vector <boost::filesystem::path> runs = GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>());

		std::vector <CalibrationRun*> runs;

		for(auto run_name : GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>()) )
		{
				CalibrationRun* run = new CalibrationRun(run_name);

				run->SynchronizePhysicsEvents();
				run->SynchronizeCalibrationEvents();

				run->LoadRunSettings();

				runs.emplace_back( run );
		}

		/**	Pedestal subtraction on the calibration waveforms
		 *
		 */
		if(tasks[0])
		{
		    for(auto run: runs)
				{
					run->PDS_Calibration();
				}
		}

		/**	Gain determination on the calibration waveforms
		 *
		 */
		if(tasks[1])
		{
			for(auto run: runs)
			{
				run->GainDetermination();
			}
		}

		/**	Calculation of average 1 pe calibration waveform
		 *
		 */
		if(tasks[2])
		{
			for(auto run: runs)
			{
				run->Average1PE();
			}
		}

		/**	Pedestal subtraction on the physics waveforms
		 *
		 */
		if(tasks[3])
		{

		}

		for(auto run : runs)
		{
			delete run;
		}
//----------------------------------------------------------------------------------------------
// Now search for the right claws runs & events corresponding to the timestamp of the beast run
//----------------------------------------------------------------------------------------------

//		std::vector <boost::filesystem::path> runs = GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>());


//      std::vector <boost::filesystem::path> runs = GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>());
//
//      for(unsigned i = 0; i<runs.size(); i++)
//      {
//
//              //	std::cout << runs.at(i) << std::endl;
//              CalibrationRun* myrun = new CalibrationRun(runs.at(i));
//
//              myrun->SynchronizeFiles();
//
//              myrun->LoadData();
//              myrun->SubtractPedestal2();
// //		myrun->DeletePhysicsData();
//              myrun->GainCalibration();
//              myrun->Average1PE();
//              myrun->WaveformDecompositionV2();
//              myrun->SaveRates();
//
//              if(config_map["write-ntp"].as<bool>())
//              {
//                      std::string day = runs.at(i).parent_path().filename().string();
//                      myrun->WriteNTuple(path(GS->ResetHook()->SetData()->SetNtp()->SetDetector(claws::CLW)->GetHook()/day));
//              }
//
//              delete myrun;
//      }






//	hendrik_file.close();
//	app->Run();

		return 0;

}

















// cycle through the directory
//  for (directory_iterator itr(path_to_day); itr != end_itr; ++itr)
//  {
//      if (is_regular_file(itr->path())) {
//          // If it is a file do not do anything!
//
// //           string current_file = itr->path().string();
// // //			cout <<  itr->path()<< " is directory!" << endl;
// //           cout <<  itr->path().filename().string() << " is file!" << endl;
//          // fileFromPath(itr->path());
//      }
//
//      else if(is_directory(itr->path()) && starts_with(itr->path().filename().string(), "Run-")){
//          // If it is a directory check if it is a Run folder and proceed.
//
//          path datafolder = itr->path() / path("data_root");
//          directory_iterator end_itr;
//          // Cycle through the event files.
//          for (directory_iterator itr(datafolder); itr != end_itr; ++itr){
//
//          }
//
//
//      }
//  }


// pt::ptree pt;
// pt::ini_parser::read_ini(p.string(), pt);
// printTree(pt, 0);
//
//
// std::ifstream ratefile(path_to_rate.string());
//
// double rate_fwd1, rate_fwd2, rate_fwd3, rate_fwd4, rate_bwd1, rate_bwd2, rate_bwd3, rate_bwd4;
//
// if (!ratefile){
//  cerr << "not file" << endl;
//  exit(1);
// }
// while(!ratefile.eof())
// {
//  ratefile >> rate_fwd1 >> rate_fwd2 >> rate_fwd3 >> rate_fwd4 >> rate_bwd1 >> rate_bwd2 >> rate_bwd3 >> rate_bwd4;
//
// }
// cout << rate_fwd1 << endl;
// cout << rate_fwd2 << endl;
// cout << rate_fwd3 << endl;
// cout << rate_fwd4 << endl;
// cout << rate_bwd1 << endl;
// cout << rate_bwd2 << endl;
// cout << rate_bwd3 << endl;
// cout << rate_bwd4 << endl;
//





//	Event e(p);
//	e.draw();











//    //asdadsasdlcasca
//
//      // Path and file name.
//      // TODO: commandline args
//      TString path = "/Users/mgabriel/workspace/claws/claws_analysis/claws_analysis/Run-300022.root";
//
//      TFile * rootfile  = new TFile(path, "OPEN");
//
//       if (rootfile->IsZombie()){
//           cout<<"File: " << path << " was not found!"<<endl;
//           return -1;
//       }
//   //	rootfile->GetListOfKeys()->Print();
//
//      TDirectory* dir_data;
//      TDirectory* dir_scope1;
//      TDirectory* dir_scope2;
//
//      //TODO Make a try and catch block with exceptions, in case folders are not present in the file.
//
//      dir_data   = (TDirectory*)rootfile->GetDirectory("data", true);
//
//      dir_scope1 = (TDirectory*)rootfile->GetDirectory("Scope1", true);
//
//      dir_scope2 = (TDirectory*)rootfile->GetDirectory("Scope2", true);
//
//
//
//
//
//       Data* data=new Data(dir_data);
//
//
//
//
//      rootfile->Close();
