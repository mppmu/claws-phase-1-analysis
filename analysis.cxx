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
#include <boost/algorithm/string.hpp>

// project includes
#include "globalsettings.hh"
#include "run.hh"
#include "ntp_handler.hh"

// ROOT inlcudes
#include <TGraphErrors.h>

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

		        ("profile-timing",  boost::program_options::value<bool>()->default_value(true), "Displays timing info for the individual steps.")


		;

		program_options::variables_map cmdline_vm;
		store(program_options::command_line_parser(argc, argv).options(generic_options).run(), cmdline_vm);

		program_options::options_description config_options("Data");

		config_options.add_options()

		        ("general.generic", program_options::value<filesystem::path>()->composing(),
		        "Magic shit....")

		        ("general.ntp", program_options::value<filesystem::path>()->composing(),
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

		bool profile_timing   = cmdline_vm["profile-timing"].as<bool>();

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

		// Path to the ntps....
		filesystem::path ntp_path = vm["general.ntp"].as<filesystem::path>();

		//################ Part 2: Create and select events ################

		NTP_Handler* ntp_handler = new NTP_Handler(ntp_path);
		// Create the vector to hold the analysis events:

		// string first_run = "";
		// string last_run = "";

		string selections_names = "";

		vector<AnalysisEvent*>  analysis_evts;

		for(auto selection : selections)
		{
				if(!starts_with( selection.first, "Selection" )) continue;

				string selections_name = selection.first;

				replace_all(selections_name, "Selection", "");

				selections_names += selections_name;
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

				int run_min = -1;
				try
				{
						run_min = selection.second.get<int>("run_min");
				}
				//catch( )
				catch( const property_tree::ptree_bad_path &e )
				{
						run_min = vm["data.run_min"].as<int>();
				}

				int run_max = 1000000;
				try
				{
						run_max = selection.second.get<int>("run_max");
				}
				catch( const property_tree::ptree_bad_path &e )
				{
						run_max = vm["data.run_max"].as<int>();
				}

				int year_min = -1;
				int month_min = -1;
				int day_min = -1;
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

				int year_max = 3000;
				int month_max = 13;
				int day_max = 42;
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

				// if(first_run != "" )
				// {
				//      int front_tmp = stoi(run_paths.front().filename().string().substr(4));
				//      int front_global = stoi(first_run.substr(4));
				//      if(front_tmp < front_global) first_run = run_paths.front().filename().string();
				// }
				// else
				// {
				//      first_run = run_paths.front().filename().string();
				//
				// }

				// if(last_run != "" )
				// {
				//      int back_tmp = stoi(run_paths.back().filename().string().substr(4));
				//      int back_global = stoi(last_run.substr(4));
				//      if(back_tmp < back_global) last_run = run_paths.back().filename().string();
				// }
				// else
				// {
				//      last_run = run_paths.back().filename().string();
				// }



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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
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
						//		cout << e.what() << endl;
				}

				string ler_status = "";
				try
				{
						ler_status = selection.second.get<string>("ler_status");
						cout << "ler_status: " << ler_status << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "ler_status: not specified!" << "\n";
						//		cout << e.what() << endl;
				}

				string her_status = "";
				try
				{
						her_status = selection.second.get<string>("her_status");
						cout << "her_status: " << her_status << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "her_status: not specified!" << "\n";
						//		cout << e.what() << endl;
				}

				string superkekb_status = "";
				try
				{
						superkekb_status = selection.second.get<string>("superkekb_status");
						cout << "superkekb_status: " << superkekb_status << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "superkekb_status: not specified!" << "\n";
				}

				int min_length = -1;
				try
				{
						min_length = selection.second.get<int>("min_length");
						cout << "min_length: " << min_length << "\n";
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						cout << "min_length: not specified!" << "\n";
				}



				std::cout << "\033[33;1mMeta Selection:\033[0m running" << "\r" << std::flush;

				double wall0 = claws::get_wall_time();
				double cpu0  = claws::get_cpu_time();


				// Select events
				auto runs_itr = runs.begin();

				while(runs_itr != runs.end())
				{
						if(injection != "") (*runs_itr)->SetInjectionLimit(injection, ntp_handler);
						if(ler_injection_rate != -1) (*runs_itr)->SetInjectionRate("LER", ler_injection_rate);
						if(her_injection_rate != -1) (*runs_itr)->SetInjectionRate("LER", ler_injection_rate);
						if(ler_current_min > -10000 or ler_current_max < 10000) (*runs_itr)->SetCurrentLimit("LER", ler_current_min, ler_current_max, ntp_handler);
						if(her_current_min > -10000 or her_current_max < 10000) (*runs_itr)->SetCurrentLimit("HER", her_current_min, her_current_max, ntp_handler);
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

				std::cout << "\033[32;1mMeta Selection:\033[0m done!       " << std::endl;
				double wall1 = claws::get_wall_time();
				double cpu1  = claws::get_cpu_time();

				if(profile_timing)
				{
						cout << "Wall Time = " << wall1 - wall0 << endl;
						cout << "CPU Time  = " << cpu1  - cpu0  << endl;
				}

				cout << "AFTER META BASED SELECTION"<< endl;
				for(auto & run : runs )
				{
						cout << "Run number: " << run->GetNumber() << ", NEvents: " << run->GetNEvents()<< endl;
				}

				if(runs.size() == 0)
				{
						cout << "WARNING: no events after meta based selection!" << endl;
						return -1;
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
						AnalysisEvent* analysis_evt = new AnalysisEvent("_INJ_"+injection);

						for(auto & run : runs )
						{
								for(auto & evt: run->GetEvents())
								{
										evt->LoadFiles(EVENTSTATE_CALIBRATED);


										if( evt->GetState() == EVENTSTATE_CALIBRATED)
										{
												if( evt->GetChannels().at(0)->GetHistogram("mip")->GetNbinsX() >= min_length)
												{
														if( injection == "NONE" )
														{
																if(!evt->CheckInjection())
																{
																		analysis_evt->AddEvent(evt);
																}
														}
														else
														{
																analysis_evt->AddEvent(evt);
														}
												}
										}
										evt->DeleteHistograms();
								}
						}
						analysis_evt->Normalize();
						analysis_evts.push_back(analysis_evt);
				}
				else
				{
						for(auto & run : runs )
						{
								for(auto & evt: run->GetEvents())
								{
										evt->LoadFiles(EVENTSTATE_CALIBRATED);

										if( evt->GetState() == EVENTSTATE_CALIBRATED)
										{
												if( injection == "NONE" )
												{
														if(!evt->CheckInjection())
														{
																analysis_evts.push_back(new AnalysisEvent(evt, "_INJ_"+injection));
														}
												}
												else
												{
														analysis_evts.push_back(new AnalysisEvent(evt, "_INJ_"+injection));
												}
										}
										evt->DeleteHistograms();

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

		string tasks = "";


		for(auto &target : selections)
		{
				if(!starts_with( target.first, "Target" )) continue;


				for(auto &entry : target.second)
				{
						if(!starts_with( entry.first, "task" )) continue;

						// vector<string> task_type;
						// split(plot_type, entry.second.data(), is_any_of(":*"), token_compress_on);
						// DO THE TASKS LIKE FFT
						if(entry.second.data() == "PEAK")
						{
								std::cout << "\033[33;1mPeak Algorithm:\033[0m running" << "\r" << std::flush;

								double wall0 = claws::get_wall_time();
								double cpu0  = claws::get_cpu_time();

								for(auto & anaysis_evt: analysis_evts)
								{
										anaysis_evt->RunPeak();
								}

								std::cout << "\033[32;1mPeak Algorithm:\033[0m done!       " << std::endl;
								double wall1 = claws::get_wall_time();
								double cpu1  = claws::get_cpu_time();

								if(profile_timing)
								{
										cout << "Wall Time = " << wall1 - wall0 << endl;
										cout << "CPU Time  = " << cpu1  - cpu0  << endl;
								}

								tasks += "_PEAK";

						}
						else if(entry.second.data() == "FFT")
						{
								std::cout << "\033[33;1mFFT Algorithm:\033[0m running" << "\r" << std::flush;

								double wall0 = claws::get_wall_time();
								double cpu0  = claws::get_cpu_time();

								for(auto & anaysis_evt: analysis_evts)
								{
										anaysis_evt->RunFFT();
								}

								std::cout << "\033[32;1mFFT Algorithm:\033[0m done!       " << std::endl;
								double wall1 = claws::get_wall_time();
								double cpu1  = claws::get_cpu_time();

								if(profile_timing)
								{
										cout << "Wall Time = " << wall1 - wall0 << endl;
										cout << "CPU Time  = " << cpu1  - cpu0  << endl;
								}

								tasks += "_FFT";
						}
						// else if(entry.second.data() == "HITENERGY")
						// {
						//      std::cout << "\033[33;1mHit Energy Spectrum:\033[0m running" << "\r" << std::flush;
						//
						//      double wall0 = claws::get_wall_time();
						//      double cpu0  = claws::get_cpu_time();
						//
						//      for(auto & anaysis_evt: analysis_evts)
						//      {
						//              anaysis_evt->HitEnergySpectrum();
						//      }
						//
						//      std::cout << "\033[32;1mHit Energy Spectrum:\033[0m done!       " << std::endl;
						//      double wall1 = claws::get_wall_time();
						//      double cpu1  = claws::get_cpu_time();
						//
						//      if(profile_timing)
						//      {
						//              cout << "Wall Time = " << wall1 - wall0 << endl;
						//              cout << "CPU Time  = " << cpu1  - cpu0  << endl;
						//      }
						//
						//      tasks += "_HITENERGY";
						// }
						else
						{
								cout<< "UNKNOWN TASTK:" << entry.second.data() << endl;
						}

				}

		}

		//################ Part 4: Produce results and plots ################

		filesystem::path output = vm["data.output"].as<filesystem::path>();

		// if(first_run == last_run) output /= first_run;
		// else output /= (first_run+ "-" + last_run.substr(4));

		// if( !boost::filesystem::is_directory(output) )
		// {
		//      boost::filesystem::create_directory(output);
		// }

		for(auto &target : selections)
		{
				if(!starts_with( target.first, "Target" )) continue;


				for(auto &entry : target.second)
				{
						if(!starts_with( entry.first, "plot" )) continue;

						vector<string> plot_type;
						split(plot_type, entry.second.data(), is_any_of(":*"), token_compress_on);


						if( plot_type.at(0) == "WAVEFORM")
						{
								//for( int i = 0; i < analysis_evts.size(); ++i)


								int first_run_ = analysis_evts.front()->GetRunNr("first");
								int last_run_ = analysis_evts.back()->GetRunNr("last");


								filesystem::path extended_output;

								if(first_run_ == last_run_) extended_output =  output /("Run_" + to_string(first_run_)+selections_names);
								else extended_output = output / ("Run_" + to_string(first_run_) + "-" + to_string(last_run_)+selections_names);

								if( !boost::filesystem::is_directory(extended_output) )
								{
										boost::filesystem::create_directory(extended_output);
								}

								for(auto & anaysis_evt: analysis_evts)
								{
										string prefix = entry.second.data() + tasks;
										//
										// if( !filesystem::is_directory(output/foldername) )
										// {
										//      filesystem::create_directory(output/foldername);
										// }
										anaysis_evt->SaveEvent(extended_output, prefix);
								}
								// Create the folder with the runs in the name


								string fname = extended_output.string() + "/" + "selections.ini";
								property_tree::write_ini(fname.c_str(), selections);


								// // Create the folder with type of output in the name
								// if( !filesystem::is_directory(extended_output/entry.second.data() ) )
								// {
								//      filesystem::create_directory(extended_output/entry.second.data());
								// }


								//	replace_last(fname, ".root", "_selections.ini");
								//	property_tree::write_ini(.c_str(), selections);

								// replace_last(fname, "_selections.ini", "_plot_only.ini");
								// property_tree::write_ini(fname.c_str(), target.second);
						}
						else if(plot_type.at(0) == "SCATTER")
						{

								//	NTP_Handler* ntp_handler = new NTP_Handler(ntp_path);
								vector<TGraphErrors*> graphs;

								// string graph_name = plot_type.at(1)+ ":" + plot_type.at(2)+":" + plot_type.at(3);
								//
								// TGraphErrors* graph = new TGraphErrors();
								// graph->SetName(graph_name.c_str());
								// graph->SetTitle(graph_name.c_str());
								// graph->SetMarkerStyle(4);

								if(plot_type.at(1) == "VACUUMSCRUBBING")
								{
										string graph_name = plot_type.at(1)+ ":" + plot_type.at(2)+":" + plot_type.at(3);

										replace_all(graph_name, ".", "_");

										TGraphErrors* graph = new TGraphErrors();
										graph->SetName(graph_name.c_str());
										graph->SetTitle(graph_name.c_str());
										graph->SetMarkerStyle(4);

										string observable = plot_type.at(3);
										string error = "";
										if(starts_with(observable, "Rate.")) error = "RateErr." + observable.substr(5);

										string ring = plot_type.at(2);
										string pv_current = "SKB_" + ring + "_current";
										string pv_int_current = "SKB_" + ring + "_integratedCurrent";

										graph->GetXaxis()->SetTitle(("Delivered " + ring + " current [Ah]").c_str());
										graph->GetYaxis()->SetTitle((observable+"/(beam current)^2").c_str());

										//analysis_evt->GetParameter<double>(plot_type.at(1));
										//	string graph_name = plot_type.at(1)+ " " + plot_type.at(2);

										for( auto & analysis_evt: analysis_evts)
										{
												double ts = analysis_evt->GetParameter<double>("Properties.UnixTime");

												try
												{
														auto current = (*ntp_handler->GetPV< vector<double>* >(ts, pv_current));
														auto int_current = (*ntp_handler->GetPV< vector<double>* >(ts, pv_int_current));
														auto rate = analysis_evt->GetParameter<double>(observable);
														// double x = ts;
														// double y = int_current[0];
														double x = int_current[0];
														double y = rate/pow(current[0],2);
														// Get X Value
														// Get Y Value
														// Add X and Y from events to plots
														int n = graph->GetN();
														graph->SetPoint(n, x, y);

														if(error != "")
														{
																double x_err = 0;
																double y_err = analysis_evt->GetParameter<double>(error)/pow(current[0],2);
																graph->SetPointError(n, x_err, y_err);
														}
												}
												catch(int e)
												{

												}


										}

										graphs.push_back(graph);
								}
								else if(plot_type.at(1) == "BGTOUSCHEK")
								{
										string graph_name = plot_type.at(1)+ ":" + plot_type.at(2)+":" + plot_type.at(3);

										replace_all(graph_name, ".", "_");

										TGraphErrors* graph = new TGraphErrors();
										graph->SetName(graph_name.c_str());
										graph->SetTitle(graph_name.c_str());
										graph->SetMarkerStyle(4);

										string observable = plot_type.at(3);
										string error = "";
										if(starts_with(observable, "Rate.")) error = "RateErr." + observable.substr(5);

										string ring = plot_type.at(2);

										string pv_current = "SKB_" + ring + "_current";
										string pv_int_current = "SKB_" + ring + "_integratedCurrent";

										graph->GetXaxis()->SetTitle("#frac{I}{PZ_e^2#sigma_y} [mA Pa^{-1}#mum^{-1}]");
										graph->GetYaxis()->SetTitle(("#frac{" + observable + "}{IPZ_E^2} [a.u.]").c_str());

										for( auto & analysis_evt: analysis_evts)
										{
												double ts = analysis_evt->GetParameter<double>("Properties.UnixTime");

												double current = (*ntp_handler->GetPV< vector<double>* >(ts, pv_current))[0];
												auto beamsize = (*ntp_handler->GetPV< vector<double>* >(ts, pv_current));

												auto int_current = (*ntp_handler->GetPV< vector<double>* >(ts, pv_int_current))[0];

												// double x = ts;
												// double y = int_current[0];
												double x = int_current;
												double y = analysis_evt->GetParameter<double>(observable)/pow(current,2);
												// Get X Value
												// Get Y Value
												// Add X and Y from events to plots
												int n = graph->GetN();
												graph->SetPoint(n, x, y);

												// if(!(error == ""))
												// {
												//         double x_err = 0;
												//         double y_err = analysis_evt->GetParameter<double>(error)/pow(current[0],2);
												//         graph->SetPointError(n, x_err, y_err);
												// }
										}
										graphs.push_back(graph);
								}
								else if(plot_type.at(1) == "BEAMDUST")
								{
										string graph_name = plot_type.at(1)+ ":Rare.FWD1";
										replace_all(graph_name, ".", "_");

										TGraphErrors* fwd1 = new TGraphErrors();
										fwd1->SetName(graph_name.c_str());
										fwd1->SetTitle(graph_name.c_str());
										fwd1->SetMarkerStyle(4);
										fwd1->GetXaxis()->SetTitle("Time [s]");
										fwd1->GetYaxis()->SetTitle("Particle Rate [MIP/s]");

										graph_name = plot_type.at(1)+ ":Rare.FWD2";
										replace_all(graph_name, ".", "_");

										TGraphErrors* fwd2 = new TGraphErrors();
										fwd2->SetName(graph_name.c_str());
										fwd2->SetTitle(graph_name.c_str());
										fwd2->SetMarkerStyle(4);
										fwd2->GetXaxis()->SetTitle("Time [s]");
										fwd2->GetYaxis()->SetTitle("Particle Rate [MIP/s]");

										graph_name = plot_type.at(1)+ ":Rare.FWD3";
										replace_all(graph_name, ".", "_");

										TGraphErrors* fwd3 = new TGraphErrors();
										fwd3->SetName(graph_name.c_str());
										fwd3->SetTitle(graph_name.c_str());
										fwd3->SetMarkerStyle(4);
										fwd2->GetXaxis()->SetTitle("Time [s]");
										fwd2->GetYaxis()->SetTitle("Particle Rate [MIP/s]");

										vector<TGraphErrors*> on_fwd;
										vector<TGraphErrors*> on_bwd;

										for(int i = 0; i<3; ++i)
										{
												TGraphErrors* tmp = new TGraphErrors();
												tmp->SetName(("OnlineRate FWD"+to_string(i+1)).c_str());
												tmp->SetTitle(("OnlineRate FWD"+to_string(i+1)).c_str());
												tmp->SetMarkerStyle(4);
												tmp->GetXaxis()->SetTitle("Time [s]");
												tmp->GetYaxis()->SetTitle("Particle Rate [MIP/s]");
												on_fwd.push_back(tmp);

												tmp = new TGraphErrors();
												tmp->SetName(("OnlineRate BWD"+to_string(i+1)).c_str());
												tmp->SetTitle(("OnlineRate BWD"+to_string(i+1)).c_str());
												tmp->SetMarkerStyle(4);
												tmp->GetXaxis()->SetTitle("Time [s]");
												tmp->GetYaxis()->SetTitle("Particle Rate [MIP/s]");
												on_bwd.push_back(tmp);
										}
										// TGraphErrors* ler_p_d2 = new TGraphErrors();
										// ler_p_d2->SetName("LER_Preasure_D02");
										// ler_p_d2->SetTitle("LER_Preasure_D02");
										// ler_p_d2->SetMarkerStyle(4);
										//
										// TGraphErrors* ler_p_d6 = new TGraphErrors();
										// ler_p_d6->SetName("LER_Preasure_D06");
										// ler_p_d6->SetTitle("LER_Preasure_D06");
										// ler_p_d6->SetMarkerStyle(4);
										TGraphErrors* her_pavg = new TGraphErrors();
										her_pavg->SetName("HER average pressure");
										her_pavg->SetTitle("HER average pressure");
										her_pavg->SetMarkerStyle(4);

										TGraphErrors* ler_pavg = new TGraphErrors();
										ler_pavg->SetName("LER average pressure");
										ler_pavg->SetTitle("LER average pressure");
										ler_pavg->SetMarkerStyle(4);


										TGraphErrors* ler_pavg_corr = new TGraphErrors();
										ler_pavg_corr->SetName("LER average pressure corrected");
										ler_pavg_corr->SetTitle("LER average pressure corrected");
										ler_pavg_corr->SetMarkerStyle(4);


										TGraphErrors* her_pavg_corr = new TGraphErrors();
										her_pavg_corr->SetName("HER average pressure corrected");
										her_pavg_corr->SetTitle("HER average pressure corrected");
										her_pavg_corr->SetMarkerStyle(4);
										// string observable =  plot_type.at(2);
										// string error = "";
										// if(starts_with(observable, "Rate.")) error = "RateErr." + observable.substr(5);

										vector<TGraphErrors*> her_pres;
										vector<TGraphErrors*> ler_pres;

										for(int i = 0; i<12; ++i)
										{
												TGraphErrors* tmp = new TGraphErrors();
												tmp->SetName(("HER pressures #"+to_string(i)).c_str());
												tmp->SetTitle(("HER pressures #"+to_string(i)).c_str());
												tmp->SetMarkerStyle(4);
												her_pres.push_back(tmp);

												TGraphErrors* tmp2 = new TGraphErrors();
												tmp2->SetName(("LER pressures #"+to_string(i)).c_str());
												tmp2->SetTitle(("LER pressures #"+to_string(i)).c_str());
												tmp2->SetMarkerStyle(4);
												ler_pres.push_back(tmp2);
										}


										vector<TGraphErrors*> her_pres_local;
										vector<TGraphErrors*> ler_pres_local;

										for(int i = 0; i<26; ++i)
										{
												TGraphErrors* tmp = new TGraphErrors();
												tmp->SetName(("HER pressures local #"+to_string(i)).c_str());
												tmp->SetTitle(("HER pressures local #"+to_string(i)).c_str());
												tmp->SetMarkerStyle(4);
												her_pres_local.push_back(tmp);

												TGraphErrors* tmp2 = new TGraphErrors();
												tmp2->SetName(("LER pressures local #"+to_string(i)).c_str());
												tmp2->SetTitle(("LER pressures local #"+to_string(i)).c_str());
												tmp2->SetMarkerStyle(4);
												ler_pres_local.push_back(tmp2);
										}

										for( auto & analysis_evt: analysis_evts)
										{

												double ts = analysis_evt->GetParameter<double>("Properties.UnixTime");

												try
												{

														// auto val_ler_p_d2 = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_partialPressures_D02"));
														// auto val_ler_p_d6 = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_partialPressures_D06"));

														auto SKB_LER_pressure_average = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_pressure_average"));
														auto SKB_LER_pressure_average_corrected = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_pressure_average_corrected"));

														auto SKB_HER_pressure_average = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_pressure_average"));
														auto SKB_HER_pressure_average_corrected = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_pressure_average_corrected"));

														auto SKB_LER_pressures = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_pressures_corrected"));
														auto SKB_HER_pressures = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_pressures_corrected"));

														// auto SKB_LER_pressures_corrected = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_pressures_corrected"));

														auto SKB_LER_pressures_local = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_pressures_local_corrected"));
														auto SKB_HER_pressures_local = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_pressures_local_corrected"));
														// auto SKB_LER_pressures_local_corrected = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_pressures_local_corrected"));
														//
														// double x = ts;
														// double y = analysis_evt->GetParameter<double>(observable);
														//
														// int n = graph->GetN();

														int n1 = fwd1->GetN();
														fwd1->SetPoint(n1, ts, analysis_evt->GetParameter<double>("Rate.FWD1"));
														fwd1->SetPointError(n1, 0, analysis_evt->GetParameter<double>("RateErr.FWD1"));

														int n2 = fwd2->GetN();
														fwd2->SetPoint(n2, ts, analysis_evt->GetParameter<double>("Rate.FWD2"));
														fwd2->SetPointError(n2, 0, analysis_evt->GetParameter<double>("RateErr.FWD2"));

														int n3 = fwd3->GetN();
														fwd3->SetPoint(n3, ts, analysis_evt->GetParameter<double>("Rate.FWD3"));
														fwd3->SetPointError(n3, 0, analysis_evt->GetParameter<double>("RateErr.FWD3"));

														for(int i = 0; i<3; ++i)
														{
																on_fwd.at(i)->SetPoint(on_fwd.at(i)->GetN(), ts, analysis_evt->GetParameter<double>("OnlineRate.FWD"+to_string(i+1)));
																on_bwd.at(i)->SetPoint(on_bwd.at(i)->GetN(), ts, analysis_evt->GetParameter<double>("OnlineRate.BWD"+to_string(i+1)));
														}

														her_pavg->SetPoint(her_pavg_corr->GetN(), ts, SKB_HER_pressure_average[0]);
														ler_pavg->SetPoint(ler_pavg_corr->GetN(), ts, SKB_LER_pressure_average[0]);

														her_pavg_corr->SetPoint(her_pavg_corr->GetN(), ts, SKB_HER_pressure_average_corrected[0]);
														ler_pavg_corr->SetPoint(ler_pavg_corr->GetN(), ts, SKB_LER_pressure_average_corrected[0]);

														for(int i = 0; i<12; ++i)
														{

																her_pres.at(i)->SetPoint(her_pres.at(i)->GetN(), ts, SKB_HER_pressures[i] );
																ler_pres.at(i)->SetPoint(ler_pres.at(i)->GetN(), ts, SKB_LER_pressures[i] );
														}


														for(int i = 0; i<26; ++i)
														{

																her_pres_local.at(i)->SetPoint(her_pres_local.at(i)->GetN(), ts, SKB_HER_pressures_local[i] );
																ler_pres_local.at(i)->SetPoint(ler_pres_local.at(i)->GetN(), ts, SKB_LER_pressures_local[i] );
														}
														//	her_p->SetPoint(her_p->GetN(), ts, val_her_p[0]);

														// if(!(error == ""))
														// {
														//      double x_err = 0;
														//      double y_err = analysis_evt->GetParameter<double>(error);
														//      graph->SetPointError(n, x_err, y_err);
														// }
												}
												catch(int e)
												{

												}

										}
										graphs.push_back(fwd1);
										graphs.push_back(fwd2);
										graphs.push_back(fwd3);

										graphs.insert(graphs.end(),on_fwd.begin(), on_fwd.end() );
										graphs.insert(graphs.end(),on_bwd.begin(), on_bwd.end() );

										graphs.push_back(her_pavg);
										graphs.push_back(ler_pavg);
										graphs.push_back(her_pavg_corr);
										graphs.push_back(ler_pavg_corr);

										graphs.insert(graphs.end(),ler_pres.begin(), ler_pres.end() );
										graphs.insert(graphs.end(),her_pres.begin(), her_pres.end() );

										graphs.insert(graphs.end(),ler_pres_local.begin(), ler_pres_local.end() );
										graphs.insert(graphs.end(),her_pres_local.begin(), her_pres_local.end() );
										//		graphs.push_back(ler_pavg_corr);
								}
								else if(plot_type.at(1) == "VAR")
								{
										string graph_name = plot_type.at(1)+ ":" + plot_type.at(2)+":" + plot_type.at(3);
										replace_all(graph_name, ".", "_");

										TGraphErrors* graph = new TGraphErrors();
										graph->SetName(graph_name.c_str());
										graph->SetTitle(graph_name.c_str());
										graph->SetMarkerStyle(4);

										string varname = plot_type.at(2);
										string observable =  plot_type.at(3);
										string error = "";
										if(starts_with(observable, "Rate.")) error = "RateErr." + observable.substr(5);

										graph->GetXaxis()->SetTitle(varname.c_str());
										graph->GetYaxis()->SetTitle("Particle Rate [MIP/s]");

										for( auto & analysis_evt: analysis_evts)
										{
												double ts = analysis_evt->GetParameter<double>("Properties.UnixTime");

												auto var = (*ntp_handler->GetPV< vector<double>* >(ts, varname))[0];
												// auto int_current = (*ntp_handler->GetPV< vector<double>* >(ts, pv_int_current))[0];

												// double x = ts;
												// double y = int_current[0];
												double x = var;
												double y = analysis_evt->GetParameter<double>(observable);
												// Get X Value
												// Get Y Value
												// Add X and Y from events to plots
												int n = graph->GetN();
												graph->SetPoint(n, x, y);

												if(!(error == ""))
												{
														double x_err = 0;
														double y_err = analysis_evt->GetParameter<double>(error);
														graph->SetPointError(n, x_err, y_err);
												}
										}
										graphs.push_back(graph);
								}

								int first_run_ = analysis_evts.front()->GetRunNr();
								int last_run_ = analysis_evts.back()->GetRunNr();

								filesystem::path extended_output;

								if(first_run_ == last_run_) extended_output =  output /("Run-" + to_string(first_run_));
								else extended_output = output / ("Run-" + to_string(first_run_) + "-" + to_string(last_run_));

								// Create the folder with the runs in the name
								if( !boost::filesystem::is_directory(extended_output) )
								{
										boost::filesystem::create_directory(extended_output);
								}

								// Create the folder with type of output in the name
								if( !filesystem::is_directory(extended_output/entry.second.data() ) )
								{
										filesystem::create_directory(extended_output/entry.second.data());
								}

								string fname = filesystem::path(extended_output/entry.second.data()).string() + "/";
								for(int i = 0; i<plot_type.size(); i++) fname += ":" + plot_type.at(i);

								fname +=tasks + ".root";
								TFile *rfile = new TFile(fname.c_str(), "RECREATE");

								for(auto & graph: graphs)
								{
										graph->Write();
										string graph_name = graph->GetName();
										replace_all(graph_name, ".", "_");
										selections.put(graph_name+".NEvts", analysis_evts.size());
										selections.put(graph_name+".CorrelationFactor", graph->GetCorrelationFactor());
								}


								rfile->Close();

								//	selections.put("Events.NEvts", analysis_evts.size());
								// Put the number of events and the range of events in the
								for( auto & analysis_evt: analysis_evts)
								{
										string sec = "Events." + to_string(analysis_evt->GetRunNr("nevent"));
										double ts = analysis_evt->GetParameter<double>("Properties.UnixTime");
										selections.put(sec, ts );
								}


								replace_last(fname, ".root", "_selections.ini");
								property_tree::write_ini(fname.c_str(), selections);

								replace_last(fname, "_selections.ini", "_plot_only.ini");
								property_tree::write_ini(fname.c_str(), target.second);

								for(auto & graph: graphs)
								{
										delete graph;
								}



						}
				}

		}
		delete ntp_handler;

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
