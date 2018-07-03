//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================

//std includes
 #include <iostream>
// #include <fstream>
// #include <iterator>
// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// // project includes
// #include "run.hh"
#include "globalsettings.hh"

using namespace std;
using namespace boost;

int main(int argc, char* argv[])
{

	cout << "---------------------------------------------------------" << endl;
	cout << "|            CLAWS phase I anaysis                       |" << endl;
	cout << "---------------------------------------------------------" << endl;


	// Handle all the settings
	program_options::options_description options("Generic options");
	options.add_options()
		("help", "Displays this help message.")

		("config,c",
		program_options::value<filesystem::path>()->default_value(
				"/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_config.ini"),
		"Config file to get config from.")

		("selections,s",
		program_options::value<filesystem::path>()->default_value(
				"/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_selections.ini"),
		"Config file to date and parameter selection from.")

		("targets,t",
		program_options::value<filesystem::path>()->default_value(
				"/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_targets.ini"),
		"Config file to get targets from.")
	;

	// Define all options that are related to which files & runs are read in and
	// and where the output goes to.
	program_options::options_description data_options("Data");

	// data_options.add_options()
	// 		("data.test",
	// 		program_options::value<std::string>()->default_value("default_value"),
	// 		"test value")
	//
	// 		("data.input",
	// 		program_options::value<filesystem::path>(),
	// 		"Data directory containing runs meant to be analysed.")
	//
	// 		("data.output",
	// 		program_options::value<filesystem::path>(),
	// 		"Data directory containing results.")
	//
	// 		("data.output_prefix",
	// 		program_options::value<std::string>()->default_value(""),
	// 		"Data directory containing results.")
	//
	// 		("data.ts_min",
	// 		program_options::value<double>(),
	// 		"If selected, smallest timestamp of events used.")
	//
	// 		("data.ts_max",
	// 		program_options::value<double>(),
	// 		"If selected, largest timestamp of events used.")
	//
	// 		("data.event_min",
	// 		program_options::value<int>(),
	// 		"If selected, smallest event number of events used.")
	//
	// 		("data.event_max",
	// 		program_options::value<int>(),
	// 		"If selected, largest event number of events used.")
	//
	// 		("data.run_min",
	// 		program_options::value<int>(),
	// 		"If selected, smallest run number of events used.")
	//
	// 		("data.run_max",
	// 		program_options::value<int>(),
	// 		"If selected, largest run number of events used.")
	//
	// 		("data.day_min",
	// 		program_options::value<std::string>(),
	// 		"If selected, earliest day used.")
	//
	// 		("data.day_max",
	// 		program_options::value<std::string>(),
	// 		"If selected, latest used.")
	//
	// ;
	//
	// options.add(data_options);



	program_options::variables_map config_map;
	program_options::store(program_options::parse_command_line(argc, argv, options), config_map);
	program_options::notify(config_map);

	if (config_map.count("help")) {
			// std::cout << generic_options << "\n";
			std::cout << options << "\n";
			return 1;
	}

	// std::ifstream ifs(config_map["data"].as<filesystem::path>().c_str());
	// if (!ifs)
	// {
	// 		cout << "Can not open file for data information: " << config_map["data"].as<filesystem::path>() << "\n";
	// 		return 0;
	// }
	// else
	// {
	// 		std::cout << "File used for data selection: " << config_map["data"].as<filesystem::path>() << ".\n";
	// 		program_options::store(parse_config_file(ifs, options), config_map);
	// 		program_options::notify(config_map);
	// }

	property_tree::ptree selections;

	if(!filesystem::exists(config_map["selections"].as<filesystem::path>().c_str()))
	{
		cout << "Can not open file for data and parameter selection: " << config_map["selections"].as<filesystem::path>() << "\n";
		return 0;
	}
	else
	{
		std::cout << "File used for data and parameter selection: " << config_map["selections"].as<filesystem::path>().c_str() << ".\n";
		property_tree::ini_parser::read_ini(config_map["selections"].as<filesystem::path>().c_str(), selections);
	}

	GS->LoadCalibrationConfig(config_map["config"].as<filesystem::path>())
	// std::cout << "Input:  " << config_map["data.input"].as<filesystem::path>() << "\n";
	// std::cout << "Output: " << config_map["data.output"].as<filesystem::path>() << "\n";
	//
	// std::cout << "Day_min:" << config_map["data.day_min"].as<std::string>() << "\n";
	// std::cout << "Dat_max:" << config_map["data.day_max"].as<std::string>() << "\n";


	//vector<AnalysisEvent*>  analysis_evts;

	// Load selections



	for(auto selection : selections)
	{

		for(auto & itr_vec : GS->GetRuns( selection.second.get<boost::filesystem::path>("input")) )
		{
			int run_nr = stoi(itr_vec.filename().string().substr(4));
			cout << run_nr << endl;
		}
	// 	// Data based selection
	//
	// 	// Load runs
	//
	// 	// Meta data based selection
	//
	// 	// Load waveforms
	//
	// 	// Build analysis event
	//  AnalysisEvent* analysis_evt = new AnalysisEvent();
	//  Fill events in anaysis_evt

	//  analysis_evts.push_back(anaysis_evt);
	}



	// for(auto anaysis_evt:analysis_evts)
	// {
	//     anaysis_evt-> do shit like fft
	// }



	// For all targets
	// {
	//     // Create target visualization
	//     // for( auto evt: anaevetn)
	//    // {
	//    //   AnaEvent-> Get target result -> Fill target vis
	//    //  }
	//    //  Write targe vis
	// }

	return 0;
}



// // Collect commandline options
//
//
// // Select data to process
//
//
// // Load Settings and
// property_tree::ptree selections;
// property_tree::ini_parser::read_ini("/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_selections.ini", selections);
//

//
// For all targets
// {
//     // Create target visualization
//     // for( auto evt: anaevetn)
//    // {
//    //   AnaEvent-> Get target result -> Fill target vis
//    //  }
//    //  Write targe vis
// }


// for(auto option : selection.second)
// {
// 	cout<< "egal" << endl;
// }
// cout << "whatever" << endl;
// cout << selection.first << endl;
