//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================

//
// #include <iostream>
// #include <fstream>
// #include <iterator>
//
// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/ini_parser.hpp>
// #include <boost/program_options.hpp>
// #include <boost/filesystem.hpp>
//
// // // project includes
// // #include "run.hh"
//
//
// using namespace std;
// using namespace boost;
// namespace po = boost::program_options;

//std includes
#include <iostream>
#include <fstream>
#include <iterator>

// boost
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

// project includes
#include "globalsettings.hh"
#include "run.hh"


using namespace boost;
using namespace std;

// template <class T>
// ostream& operator<<(ostream& os, const vector<T>& v)
// {
//      copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
//      return os;
// }
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

		        ("general.generic", program_options::value<filesystem::path>()->composing(),
		        "Magic shit....")

		        ("general.selection,s", program_options::value<filesystem::path>()->composing(),
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

		        ("data.date_min",
		        program_options::value<string>()->composing(),
		        "If selected, earliest day used.")

		        ("data.date_max",
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

		// Now read the event selections from file:

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

		// Magic shit, don't ask
		GS->LoadCalibrationConfig(vm["general.generic"].as<filesystem::path>());

		//################ Part 2: Create and select events ################

		// Create the vector to hold the analysis events:
		//vector<AnalysisEvent*>  analysis_evts;

		for(auto selection : selections)
		{
				// Select runs based on dates
				filesystem::path input;
				try
				{
						input = selection.second.get<filesystem::path>("input");
				}
				catch( const property_tree::ptree_bad_path &e )
				{
						input = vm["data.input"].as<filesystem::path>();
				}

				int run_min;
				try
				{
						run_min = selection.second.get<int>("run_min");
				}
				catch( const property_tree::ptree_bad_path &e )
				{
						run_min = vm["data.run_min"].as<int>();
				}

				int run_max;
				try
				{
						run_max = selection.second.get<int>("run_max");
				}
				catch( const property_tree::ptree_bad_path &e )
				{
						run_max = vm["data.run_max"].as<int>();
				}

				int year_min;
				int month_min;
				int day_min;
				try
				{
						string date = selection.second.get<string>("date_min");
						year_min = std::stoi(date.substr(0,4));
						month_min = std::stoi(date.substr(5,2));
						day_min = std::stoi(date.substr(8,2));
				}
				catch( const property_tree::ptree_bad_path &e )
				{

						string date = vm["data.date_min"].as<string>();
						year_min = std::stoi(date.substr(0,4));
						month_min = std::stoi(date.substr(5,2));
						day_min = std::stoi(date.substr(8,2));
				}

				int year_max;
				int month_max;
				int day_max;
				try
				{
						string date = selection.second.get<string>("date_max");
						year_max = std::stoi(date.substr(0,4));
						month_max = std::stoi(date.substr(5,2));
						day_max = std::stoi(date.substr(8,2));
				}
				catch( const property_tree::ptree_bad_path &e )
				{

						string date = vm["data.date_max"].as<string>();
						year_max = std::stoi(date.substr(0,4));
						month_max = std::stoi(date.substr(5,2));
						day_max = std::stoi(date.substr(8,2));
				}

				vector<filesystem::path> run_paths;

				for(auto & run_path : GS->GetRuns(input) )
				{
						bool good_run = false;

						int run_nr = stoi(run_path.filename().string().substr(4));

						if(run_nr >= run_min && run_nr <= run_max)
						{
								std::string date = run_path.parent_path().filename().string();
								int year = std::stoi(date.substr(0,4));
								int month = std::stoi(date.substr(5,2));
								int day = std::stoi(date.substr(8,2));

								if(   year > year_min && year < year_max)
								{
										good_run = true;
								}
								else if(year == year_min && year == year_max)
								{
										if( month > month_min && month < month_max)
										{
												good_run = true;
										}
										else if(month == month_min and month == month_max)
										{
												//check day
												if(day >= day_min and day <= day_max)
												{
														good_run = true;
												}
												else if(day == day_min or day == day_max)
												{
														good_run = true;
												}
										}
										else if(month == month_min)
										{
												if(day >= day_min)
												{
														good_run = true;
												}
										}
										else if(month == month_max)
										{
												if(day <= day_max)
												{
														good_run = true;
												}
										}
								}
								else if( year == year_min )
								{
										if( month > month_min )
										{
												good_run = true;
										}
										else if( month == month_min )
										{
												if( day >= day_min )
												{
														good_run = true;
												}
										}
								}
								else if( year == year_max)
								{
										if( month < month_max )
										{
												good_run = true;
										}
										else if( month == month_max )
										{
												if( day <= day_max )
												{
														good_run = true;
												}
										}
								}

						}

						if(good_run)
						{
								run_paths.push_back(run_path);
						}
				}

				// Create the runs
				vector<CalibrationRun*> runs;
				runs.reserve(run_paths.size());

				for(auto &run_path : run_paths)
				{
						CalibrationRun* run = new CalibrationRun(run_path);
						run->SynchronizePhysicsEvents();
						// run->SynchronizeCalibrationEvents();
						run->LoadRunSettings();

						runs.push_back(run);
				}


				cout << "BEFORE META BASED SELECTION"<< endl;
				for(auto & run : runs )
				{
						cout << "Run number: " << run->GetNumber() << ", NEvents: " << run->GetNEvents()<< endl;
				}


				// Do a meta data based event selection
				auto runs_itr = runs.begin();

				while( runs_itr != runs.end() )
				{
						(*runs_itr)->SetInjectionLimit(selection.second.get<string>("injection"));

						// (*runs_itr)->SetCurrentLimit(   "LER",
						//                                 config_map["parameters.ler_current_min"].as<double>(),
						//                                 config_map["parameters.ler_current_max"].as<double>() );
						// (*runs_itr)->SetCurrentLimit(   "HER",
						//                                 config_map["parameters.her_current_min"].as<double>(),
						//                                 config_map["parameters.her_current_max"].as<double>() );

						if( (*runs_itr)->GetNEvents() == 0 )
						{
								delete (*runs_itr);
								(*runs_itr) = NULL;
								runs.erase(runs_itr);
						}
						else
						{
								runs_itr++;
						}
				}


				cout << "AFTER META BASED SELECTION"<< endl;
				for(auto & run : runs )
				{
						cout << "Run number: " << run->GetNumber() << ", NEvents: " << run->GetNEvents()<< endl;
				}



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
