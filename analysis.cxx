//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================

// //std includes
// #include <iostream>
// #include <fstream>
// #include <iterator>
// // boost
// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/ini_parser.hpp>
// #include <boost/program_options.hpp>
// #include <boost/filesystem.hpp>
//
// // // project includes
// // #include "run.hh"
// #include "globalsettings.hh"
//
// using namespace std;
// using namespace boost;
// namespace po = boost::program_options;

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
//namespace po = boost::program_options;
using namespace boost;

#include <iostream>
#include <fstream>
#include <iterator>
using namespace std;

template <class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
    return os;
}
//#include <boost/filesystem.hpp>
int main(int argc, char* argv[])
{

	cout << "---------------------------------------------------------" << endl;
	cout << "|            CLAWS phase I anaysis                       |" << endl;
	cout << "---------------------------------------------------------" << endl;

	// Declare the supported options.
	typedef vector<string> string_vector;

       // try
       // {
           // Declare a group of options that will be allowed
           // both on the command line and in a config file
    program_options::options_description cmdline_options("Configuration");
    cmdline_options.add_options()
	    ("config,c", program_options::value<string>()->default_value(
	   	    "/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_config.ini"),
	        "Config file to get generic options from.")

        ("selection,s", program_options::value<string>()->default_value(
            "/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_selection.ini"),
            "File to get the selections from.")
        ;

	program_options::variables_map cmdline_vm;
    store(program_options::command_line_parser(argc, argv).options(cmdline_options).run(), cmdline_vm);

//           cout << cmdline_vm["config"].as<string>() << endl;

    program_options::options_description data_options("Data");

    data_options.add_options()

    ("data.input,I",
     program_options::value<string_vector>()->composing(),
     "input path")

    ("data.output,o",
     program_options::value<string_vector>()->composing(),
     "output path")
    ;

        ("data.ts_min",
        program_options::value<double>(),
        "If selected, smallest timestamp of events used.")

        ("data.ts_max",
        program_options::value<double>(),
        "If selected, largest timestamp of events used.")

        ("data.event_min",
        program_options::value<int>(),
        "If selected, smallest event number of events used.")

        ("data.event_max",
        program_options::value<int>(),
        "If selected, largest event number of events used.")

        ("data.run_min",
        program_options::value<int>(),
        "If selected, smallest run number of events used.")

        ("data.run_max",
        program_options::value<int>(),
        "If selected, largest run number of events used.")

        ("data.day_min",
        program_options::value<std::string>(),
        "If selected, earliest day used.")

        ("data.day_max",
        program_options::value<std::string>(),
        "If selected, latest used.")

    ;


    options_description options;
    options.add(cmdline_options).add(data_options);

    program_options::variables_map vm;
    store(program_options::command_line_parser(argc, argv).options(options).run(), vm);

    ifstream ifs(cmdline_vm["config"].as<string>());
    store(parse_config_file(ifs, options), vm);
    notify(vm);


		   // ifstream ifs(vm["input"].as<boost::filesystem::path>().c_str());
		   // store(parse_config_file(ifs, config_file_options), vm);
		   //
		   //
		   // po::options_description options;
		   // options.add(cmdline_options);


           // po::options_description options;
           // options.add(generic).add(config).add(hidden);
		   //
           // po::variables_map vm;
           // store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
		   //
           // ifstream ifs("multiple_sources.cfg");
           // store(parse_config_file(ifs, config_file_options), vm);
           // notify(vm);
		   //
           // if (vm.count("help"))
           // {
           //     cout << "Usage: " << argv[0] << " [options] input-files\n";
           //     cout << visible << "\n";
           //     return 0;
           // }
		   //
           // if (vm.count("version"))
           // {
           //     cout << "Multiple sources example, version 1.0\n";
           //     return 0;
           // }
		   //
           // if (vm.count("include-path"))
           // {
           //     cout << "Include paths are: " << vm["include-path"].as<string_vector>() << endl;
           // }
		   //
           // if (vm.count("input-file"))
           // {
           //     cout << "Input files are: " << vm["input-file"].as<string_vector>() << endl;
           // }
		   //
           // cout << "Optimization level is " << opt << endl;
       // }
       // catch(boost::program_options::exception &e)
       // {
       //     cout << "Exception: " << e.what() << endl;
       //     return 1;
       // }

	// Handle all the settings
	// program_options::options_description options("Generic options");
	//
	// options.add_options()
	// 	("help", "Displays this help message.")
	//
	// 	("config,c",
	// 	program_options::value<filesystem::path>()->default_value(
	// 			"/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_config.ini"),
	// 	"Config file to get config from.")
	//
	// 	("selection,s",
	// 	program_options::value<filesystem::path>()->default_value(
	// 			"/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_selection.ini"),
	// 	"Config file to get selection from.")
	// ;

	// Define all options that are related to which files & runs are read in and
	// and where the output goes to.
	//program_options::options_description data_options("Data");

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



	// program_options::variables_map config_map;
	// program_options::store(program_options::parse_command_line(argc, argv, options), config_map);
	// program_options::notify(config_map);
	//
	// if (config_map.count("help")) {
	// 		// std::cout << generic_options << "\n";
	// 		std::cout << options << "\n";
	// 		return 1;
	// }
	//
	// property_tree::ptree selection;
	//
	// if(!filesystem::exists(config_map["selection"].as<filesystem::path>().c_str()))
	// {
	// 	cout << "Can not open file for data and parameter selection: " << config_map["selection"].as<filesystem::path>() << "\n";
	// 	return 0;
	// }
	// else
	// {
	// 	std::cout << "File used for data and parameter selection: " << config_map["selection"].as<filesystem::path>().c_str() << ".\n";
	// 	property_tree::ini_parser::read_ini(config_map["selection"].as<filesystem::path>().c_str(), selection);
	// }


	//vector<AnalysisEvent*>  analysis_evts;
	// Build the events == select events according to selection

	// for(auto evt_selection : selection)
	// {
	// 	if( starts_with( evt_selection.first, "Sel" ) )
	// 	{
	//
	// 		for(auto & itr : GS->GetRuns( config_map["Data.input"].as<filesystem::path>() ) )
	// 		{
	// 			cout << itr << endl;
	// 		//	int run_nr = stoi(itr.filename().string().substr(4));
	// 		}
	//
	//
	// 		// cout << evt_selection.second.get<boost::filesystem::path>("input")
	// 		// cout << evt_selection.second.get<string>("merge_files") << endl;
	// 		// cout << evt_selection.first << endl;
	//
	// 	}
	// 	// for( list of runfiles)
	// 	// {
	// 	//
	// 	// }
	// }

	// Handle the evts == do some operation on them that is required

	// if(analysis_evts.size() == 1)
	// {
	// 	// Save waveforms
	// }
	// else(anaysis_evts.size() > 1)
	// {
	// 	// Go through the plots requested
	// 	//{
	// 		// Big case structure depending on the plot requested
	// 		// Do the plot and save it
	// 	//}
	// }
	// else
	// {
	// 	cout << "NO ANALYSIS EVENTS IN SELECTION!" << endl;
	// }

		return 0;
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

	// Load selections

	// property_tree::ptree selections;
	//
	// if(!filesystem::exists(config_map["selections"].as<filesystem::path>().c_str()))
	// {
	// 	cout << "Can not open file for data and parameter selection: " << config_map["selections"].as<filesystem::path>() << "\n";
	// 	return 0;
	// }
	// else
	// {
	// 	std::cout << "File used for data and parameter selection: " << config_map["selections"].as<filesystem::path>().c_str() << ".\n";
	// 	property_tree::ini_parser::read_ini(config_map["selections"].as<filesystem::path>().c_str(), selections);
	// }
	//
	// GS->LoadCalibrationConfig(config_map["config"].as<filesystem::path>());


	// std::cout << "Input:  " << config_map["data.input"].as<filesystem::path>() << "\n";
	// std::cout << "Output: " << config_map["data.output"].as<filesystem::path>() << "\n";
	//
	// std::cout << "Day_min:" << config_map["data.day_min"].as<std::string>() << "\n";
	// std::cout << "Dat_max:" << config_map["data.day_max"].as<std::string>() << "\n";


	//vector<AnalysisEvent*>  analysis_evts;
	//
	// for(auto & itr_vec : GS->GetRuns( config_map["data.input"].as<filesystem::path>()) )
	// {
	// 	cout << itr_vec << endl;
	// }

	// for(auto selection : selections)
	// {
	// 	cout << selection.second.get<boost::filesystem::path>("input") << endl;
	// 	for(auto & itr_vec : GS->GetRuns( selection.second.get<boost::filesystem::path>("input")) )
	// 	{
	// 		int run_nr = stoi(itr_vec.filename().string().substr(4));
	// 		cout << run_nr << endl;
	// 	}
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
	//}

	// Hier sollte ich einen vector mit analysis events haben

	// for(auto anaysis_evt:analysis_evts)
	// {
	//     anaysis_evt-> do shit like fft
	// }



	// if( plotype == "Waveform")
	// {
	//
	// }
	// else if( plotype == "Scatter")
	// {
	//
	// }
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
