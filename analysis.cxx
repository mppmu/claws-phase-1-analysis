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
		vector<AnalysisEvent*>  analysis_evts;

		for(auto selection : selections)
		{
				if(!starts_with( selection.first, "Selection" )) continue;

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

				// Check selection SetParameters
				cout << "SELECTION: " << selection.first << "\nPARAMETERS REQUIRED: \n";
				string injection = "";
				try
				{
						injection = selection.second.get<string>("injection");
						cout << "Injection: " << injection << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "Injection: not specified!" << "\n";
				}

				double ler_injection_rate = -1;
				try
				{
						ler_injection_rate = selection.second.get<double>("ler_injection_rate");
						cout << "Ler injection rate: " << ler_injection_rate << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "Ler injection rate: not specified!" << "\n";
				}

				double her_injection_rate = -1;
				try
				{
						her_injection_rate = selection.second.get<double>("her_injection_rate");
						cout << "Her injection rate: " << her_injection_rate << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "Her injection rate: not specified!" << "\n";
				}

				double ler_current_min = -10000;
				try
				{
						ler_current_min = selection.second.get<double>("ler_current_min");
						cout << "her current min: " << ler_current_min << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "ler current min: not specified!" << "\n";
				}

				double ler_current_max = 10000;
				try
				{
						ler_current_max = selection.second.get<double>("ler_current_max");
						cout << "Ler current max: " << ler_current_max << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "Ler current max: not specified!" << "\n";
				}

				double her_current_min = -10000;
				try
				{
						her_current_min = selection.second.get<double>("her_current_min");
						cout << "her current min: " << her_current_min << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "her current min: not specified!" << "\n";
				}

				double her_current_max = 10000;
				try
				{
						her_current_max = selection.second.get<double>("her_current_max");
						cout << "her current max: " << her_current_max << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "her current max: not specified!" << "\n";
				}

				double ts_min = -1;
				try
				{
						ts_min = selection.second.get<double>("ts_min");
						cout << "ts min: " << ts_min << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "ts min: not specified!" << "\n";
				}

				double ts_max = 1e10;
				try
				{
						ts_max = selection.second.get<double>("ts_max");
						cout << "ts max: " << ts_max << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "ts max: not specified!" << "\n";
				}

				string ler_status = "";
				try
				{
						ler_status = selection.second.get<double>("ler_status");
						cout << "ler_status: " << ler_status << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "ler_status: not specified!" << "\n";
				}

				string her_status = "";
				try
				{
						her_status = selection.second.get<double>("her_status");
						cout << "her_status: " << her_status << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "her_status: not specified!" << "\n";
				}

				string superkekb_status = "";
				try
				{
						superkekb_status = selection.second.get<double>("superkekb_status");
						cout << "superkekb_status: " << superkekb_status << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "superkekb_status: not specified!" << "\n";
				}


				// Select events
				auto runs_itr = runs.begin();

				while(runs_itr != runs.end())
				{
						if(injection != "") (*runs_itr)->SetInjectionLimit(injection);
						if(ler_injection_rate != -1) (*runs_itr)->SetInjectionRate("LER", ler_injection_rate);
						if(her_injection_rate != -1) (*runs_itr)->SetInjectionRate("LER", ler_injection_rate);
						if(ler_current_min > -10000 or ler_current_max < 10000) (*runs_itr)->SetCurrentLimit("LER", ler_current_min, ler_current_max);
						if(her_current_min > -10000 or her_current_max < 10000) (*runs_itr)->SetCurrentLimit("HER", her_current_min, her_current_max);
						if(ts_min > -1 or ts_max < 1e10) (*runs_itr)->SetTSLimit(ts_min, ts_max);
						if(superkekb_status != "") (*runs_itr)->SetStatus("SUPERKEKB", superkekb_status);
						if(ler_status != "") (*runs_itr)->SetStatus("LER", ler_status);
						if(her_status != "") (*runs_itr)->SetStatus("HER", her_status);

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

				// Build analysis events
				cout << "BUILDING EVENTS:"<< endl;
				bool merge_events = false;

				try
				{
						merge_events = selection.second.get<bool>("merge_events");
						cout << "merge_events: " << merge_events << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "merge_events: not specified!" << "\n";
				}

				if(merge_events)
				{
						AnalysisEvent* analysis_evt = new AnalysisEvent();

						for(auto & run : runs )
						{
								for(auto & evt: run->GetEvents())
								{
										evt->LoadFiles(EVENTSTATE_CALIBRATED);

										if( evt->GetState() == EVENTSTATE_CALIBRATED)
										{
												analysis_evt->AddEvent(evt);
										}
										evt->DeleteHistograms();
								}
						}

						analysis_evts.push_back(analysis_evt);
				}
				else
				{
						for(auto & run : runs )
						{
								for(auto & evt: run->GetEvents())
								{
										analysis_evts.push_back(new AnalysisEvent(evt));
								}
						}
				}

				for(auto & run : runs)
				{
						delete run;
						run = nullptr;
				}




		}



		//################ Part 3: Do analysis on events and get plots out ################

		for(auto &target : selections)
		{
				if(!starts_with( selection.first, "Target" )) continue;


				for(auto &entry : target)
				{
						if(!starts_with( entry.first, "task" )) continue;

				}

				for(auto &entry : target)
				{
						if(!starts_with( entry.first, "plot" )) continue;

						typedef vector< string > split_vector_type;

						// split_vector_type SplitVec; // #2: Search for tokens
						// split( SplitVec, str1, is_any_of("-*"), token_compress_on ); // SplitVec == { "hello abc","ABC","aBc goodbye" }


						vector<string> plot_type;
						split(plot_type, entry.second, is_any_of(".*"), token_compress_on);
						//      = entry.second;

						if( plot_type.at(0) = "WAVEFORM")
						{
								for( auto & analysis_evt: analysis_evts)
								{
										// Create and save plot
								}
						}
						else if(plot_type.at(0) = "SCATTER")
						{
								// Declare plots
								for( auto & analysis_evt: analysis_evts)
								{
										// Get X Value
										// Get Y Value
										// Add X and Y from events to plots
								}
								// Save plots
						}
				}

		}



		//################ Part 4: Produce results and plots ################



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
