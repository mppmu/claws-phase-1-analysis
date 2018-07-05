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
#include <boost/any.hpp>
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


		//################ Part 1: Load and parser all the settings ################
		program_options::options_description generic_options("Generic options");
		generic_options.add_options()

		        ("help", "produce help message")

		        ("config,c", program_options::value<filesystem::path>()->default_value(
						"./config/analysis_config.ini"),
		        "Config file to get generic options from.")


		;

		program_options::variables_map cmdline_vm;
		store(program_options::command_line_parser(argc, argv).options(generic_options).run(), cmdline_vm);

		program_options::options_description config_options("Data");

		config_options.add_options()
		        ("general.selection,s", program_options::value<sfilesystem::path>()->composing(),
		        "File to get the selections from.")

		        ("data.input,I",
		        program_options::value<filesystem::path>()->composing(),
		        "input path")

		        ("data.output,o",
		        program_options::value<filesystem::path>()->composing(),
		        "output path")

		        ("data.ts_min",
		        program_options::value<double>()->composing(),
		        "If selected, smallest timestamp of events used.")

		        ("data.ts_max",
		        program_options::value<double>()->composing(),
		        "If selected, largest timestamp of events used.")

		        ("data.event_min",
		        program_options::value<int>()->composing(),
		        "If selected, smallest event number of events used.")

		        ("data.event_max",
		        program_options::value<int>()->composing(),
		        "If selected, largest event number of events used.")

		        ("data.run_min",
		        program_options::value<int>()->composing(),
		        "If selected, smallest run number of events used.")

		        ("data.run_max",
		        program_options::value<int>()->composing(),
		        "If selected, largest run number of events used.")

		        ("data.day_min",
		        program_options::value<string>()->composing(),
		        "If selected, earliest day used.")

		        ("data.day_max",
		        program_options::value<string>()->composing(),
		        "If selected, latest used.")
		;


		program_options::options_description options;
		options.add(generic_options).add(config_options);

		program_options::variables_map vm;
		store(program_options::command_line_parser(argc, argv).options(options).run(), vm);

		ifstream ifs(cmdline_vm["config"].as<filesystem::path>().string());
		store(parse_config_file(ifs, options), vm);
		notify(vm);

		if (vm.count("help"))
		{
				cout << "Usage: " << argv[0] << " [options] input-files\n";
				cout << options << "\n";
				return 0;
		}

		// Now opening the event selections:

		property_tree::ptree selections;

		if(!filesystem::exists(vm["general.selection"].as<filesystem::path>().c_str()))
		{
				cout << "Can not open file for data and parameter selection: " << vm["general.selection"].as<filesystem::path>() << "\n";
				return 0;
		}
		else
		{
				std::cout << "File used for data and parameter selection: " << vm["general.selection"].as<filesystem::path>().c_str() << ".\n";
				property_tree::ini_parser::read_ini(vm["general.selection"].as<filesystem::path>().string(), selections);
		}


		//################ Part 2: Create and select events ################

		// Create the vector to hold the analysis events:
		//vector<AnalysisEvent*>  analysis_evts;

		// Load and select the events
		for(auto selection : selections)
		{
				// Do date based selection of events

				// Do parameter based selection of events

				// Build analysis events
		}



		//################ Part 3: Do analysis on events ################

		// for(auto anaysis_evt:analysis_evts)
		// {
		//     anaysis_evt-> do shit like fft
		// }




		//################ Part 4: Produce results and plots ################

		// if( plotype == "Waveform")
		// {
		//
		// }
		// else if( plotype == "Scatter")
		// {
		//
		// }

		return 0;
}



// for(const auto& entry : vm)
// {
//      cout << entry.first <<endl;
//      try
//      {
//              cout << any_cast<string>(entry.second.value()) << '\n';
//      }
//      catch (bad_any_cast &e)
//      {
//              std::cerr << e.what() << '\n';
//      }
//
//
//
// }





// Build the events == select events according to selection

// for(auto evt_selection : selection)
// {
//  if( starts_with( evt_selection.first, "Sel" ) )
//  {
//
//      for(auto & itr : GS->GetRuns( config_map["Data.input"].as<filesystem::path>() ) )
//      {
//          cout << itr << endl;
//      //	int run_nr = stoi(itr.filename().string().substr(4));
//      }
//
//
//      // cout << evt_selection.second.get<boost::filesystem::path>("input")
//      // cout << evt_selection.second.get<string>("merge_files") << endl;
//      // cout << evt_selection.first << endl;
//
//  }


//
// GS->LoadCalibrationConfig(config_map["config"].as<filesystem::path>());


//vector<AnalysisEvent*>  analysis_evts;


// for(auto selection : selections)
// {
//  cout << selection.second.get<boost::filesystem::path>("input") << endl;
//  for(auto & itr_vec : GS->GetRuns( selection.second.get<boost::filesystem::path>("input")) )
//  {
//      int run_nr = stoi(itr_vec.filename().string().substr(4));
//      cout << run_nr << endl;
//  }
//  // Data based selection
//
//  // Load runs
//
//  // Meta data based selection
//
//  // Load waveforms
//
//  // Build analysis event
//  AnalysisEvent* analysis_evt = new AnalysisEvent();
//  Fill events in anaysis_evt

//  analysis_evts.push_back(anaysis_evt);
//}
