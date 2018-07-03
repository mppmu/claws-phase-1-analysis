//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================

//std includes
#include <iostream>
#include <fstream>
#include <iterator>
// boost
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// project includes
#include "run.hh"
#include "globalsettings.hh"



int main(int argc, char* argv[]) {

	cout << "---------------------------------------------------------" << endl;
	cout << "|            CLAWS phase I anaysis                       |" << endl;
	cout << "---------------------------------------------------------" << endl;

		// Define all
		boost::program_options::options_description options("Generic options");
		options.add_options()
		        ("help", "Displays this help message.")

		        ("config,c",
		        boost::program_options::value<boost::filesystem::path>()->default_value(
						"/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/config/analysis_parameters.ini"),
		        "Config file to get parameters from.")
		;

		// Define all options that are related to which files & runs are read in and
		// and where the output goes to.
		boost::program_options::options_description data_options("Data");

		data_options.add_options()
		        ("data.test",
		        boost::program_options::value<std::string>()->default_value("default_value"),
		        "test value")

		        ("data.input",
		        boost::program_options::value<boost::filesystem::path>(),
		        "Data directory containing runs meant to be analysed.")

		        ("data.output",
		        boost::program_options::value<boost::filesystem::path>(),
		        "Data directory containing results.")

		        ("data.output_prefix",
		        boost::program_options::value<std::string>()->default_value(""),
		        "Data directory containing results.")

		        ("data.ts_min",
		        boost::program_options::value<double>(),
		        "If selected, smallest timestamp of events used.")

		        ("data.ts_max",
		        boost::program_options::value<double>(),
		        "If selected, largest timestamp of events used.")

		        ("data.event_min",
		        boost::program_options::value<int>(),
		        "If selected, smallest event number of events used.")

		        ("data.event_max",
		        boost::program_options::value<int>(),
		        "If selected, largest event number of events used.")

		        ("data.run_min",
		        boost::program_options::value<int>(),
		        "If selected, smallest run number of events used.")

		        ("data.run_max",
		        boost::program_options::value<int>(),
		        "If selected, largest run number of events used.")

		        ("data.day_min",
		        boost::program_options::value<std::string>(),
		        "If selected, earliest day used.")

		        ("data.day_max",
		        boost::program_options::value<std::string>(),
		        "If selected, latest used.")

		;

		options.add(data_options);


		boost::program_options::options_description parameter_options("Parameters");

		parameter_options.add_options()
		        ("parameters.ler_current_min",
		        boost::program_options::value<double>(),
		        "Minimum ler current.")

		        ("parameters.ler_current_max",
		        boost::program_options::value<double>(),
		        "Maximum ler current.")

		        ("parameters.her_current_min",
		        boost::program_options::value<double>(),
		        "Minimum her current.")

		        ("parameters.her_current_max",
		        boost::program_options::value<double>(),
		        "Maximum her current.")

		        ("parameters.inj",
		        boost::program_options::value<int>(),
		        "Injection requirement:\n-1: doesn't matter\n0: no injection \n1: injection in one or both rings\n2: in LER only\n3: in HER only\n3: in both rings")

		        ("parameters.inj_rate_min",
		        boost::program_options::value<double>(),
		        " ")

		        ("parameters.inj_rate_max",
		        boost::program_options::value<double>(),
		        " ")
		;

		options.add(parameter_options);


		boost::program_options::options_description task_options("Tasks");

		task_options.add_options()
		        ("task.peak",
		        boost::program_options::value<bool>(),
		        "Run peak algorithm?")

		        ("task.fft",
		        boost::program_options::value<bool>(),
		        "Run FFT?")
		;

		options.add(task_options);


		boost::program_options::variables_map config_map;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), config_map);
		boost::program_options::notify(config_map);

		if (config_map.count("help")) {
				// std::cout << generic_options << "\n";
				std::cout << options << "\n";
				return 1;
		}

		std::ifstream ifs(config_map["config"].as<boost::filesystem::path>().c_str());
		if (!ifs)
		{
				cout << "Can not open config file: " << config_map["config"].as<boost::filesystem::path>() << "\n";
				return 0;
		}
		else
		{
				std::cout << "Used config file: " << config_map["config"].as<boost::filesystem::path>() << ".\n";
				boost::program_options::store(parse_config_file(ifs, options), config_map);
				boost::program_options::notify(config_map);
		}

		std::cout << "Input:  " << config_map["data.input"].as<boost::filesystem::path>() << "\n";
		std::cout << "Output: " << config_map["data.output"].as<boost::filesystem::path>() << "\n";

		std::cout << "Day_min:" << config_map["data.day_min"].as<std::string>() << "\n";
		std::cout << "Dat_max:" << config_map["data.day_max"].as<std::string>() << "\n";


		std::vector <AnalysisRun*> runs;

		/** Check which runs are supposed to go into
		 *
		 */

		for(auto & itr_vec : GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>()) )
		{
				int run_nr = stoi(itr_vec.filename().string().substr(4));

				if( run_nr >= config_map["data.run_min"].as<int>()
				    && run_nr <= config_map["data.run_max"].as<int>() )
				{
						std::string str = itr_vec.parent_path().filename().string();
						int day   = std::stoi(str.substr(str.length() - 2) );
						int month = std::stoi(str.substr(str.length() - 5, 2) );

						if(   month > stoi( config_map["data.day_min"].as<std::string>().substr(0, 2))
						      &&  month < stoi( config_map["data.day_max"].as<std::string>().substr(0, 2))    )
						{
								runs.push_back(new AnalysisRun(itr_vec));
						}

						else if(    month == stoi( config_map["data.day_min"].as<std::string>().substr(0, 2))
						            && month == stoi( config_map["data.day_max"].as<std::string>().substr(0, 2))    )
						{
								if(    day >= stoi( config_map["data.day_min"].as<std::string>().substr(3, 2))
								       && day <= stoi( config_map["data.day_max"].as<std::string>().substr(3, 2))    )
								{
										runs.push_back(new AnalysisRun(itr_vec));
								}
						}

						else if( month == stoi( config_map["data.day_min"].as<std::string>().substr(0, 2)) )
						{
								if( day >= stoi( config_map["data.day_min"].as<std::string>().substr(3, 2)) )
								{
										runs.push_back(new AnalysisRun(itr_vec));
								}
						}

						else if(  month == stoi( config_map["data.day_max"].as<std::string>().substr(0, 2)) )
						{
								if( day <= stoi( config_map["data.day_max"].as<std::string>().substr(3, 2)) )
								{
										runs.push_back(new AnalysisRun(itr_vec));
								}
						}
				}
		}

		for(auto & run : runs)
		{
				run->SynchronizeFiles();
				run->LoadMetaData();
		}

		auto itr = runs.begin();

		while( itr != runs.end() )
		{
				(*itr)->SetInjectionLimit(config_map["parameters.inj"].as<int>());

				(*itr)->SetCurrentLimit(   "LER",
				                           config_map["parameters.ler_current_min"].as<double>(),
				                           config_map["parameters.ler_current_max"].as<double>() );
				(*itr)->SetCurrentLimit(   "HER",
				                           config_map["parameters.her_current_min"].as<double>(),
				                           config_map["parameters.her_current_max"].as<double>() );

				if( (*itr)->NEvents() == 0 )
				{
						delete (*itr);
						(*itr) = NULL;
						runs.erase(itr);
				}
				else
				{
						itr++;
				}
		}

		int n_events = 0;

		for( auto & run : runs )
		{
				n_events += run->NEvents();
		}

		if( !runs.empty() )
		{
				boost::property_tree::ptree out_selection;

				out_selection.put("General.config", config_map["config"].as<boost::filesystem::path>() );
				out_selection.put("Data.output",   config_map["data.output"].as<boost::filesystem::path>() );
				out_selection.put("Data.input",    config_map["data.input"].as<boost::filesystem::path>() );
				out_selection.put("Data.ts_min",  config_map["data.ts_min"].as<double>() );
				out_selection.put("Data.ts_max",  config_map["data.ts_max"].as<double>() );
				out_selection.put("Data.event_min", config_map["data.event_min"].as<int>() );
				out_selection.put("Data.event_max", config_map["data.event_max"].as<int>() );
				out_selection.put("Data.run_min",  config_map["data.run_min"].as<int>() );
				out_selection.put("Data.run_max",  config_map["data.run_max"].as<int>() );
				out_selection.put("Data.day_min",  config_map["data.day_min"].as<std::string>() );
				out_selection.put("Data.day_max",  config_map["data.day_max"].as<std::string>() );

				out_selection.put("Parameters.ler_current_min",config_map["parameters.ler_current_min"].as<double>() );
				out_selection.put("Parameters.ler_current_max",config_map["parameters.ler_current_max"].as<double>() );
				out_selection.put("Parameters.her_current_min",config_map["parameters.her_current_min"].as<double>() );
				out_selection.put("Parameters.her_current_max",config_map["parameters.her_current_max"].as<double>() );
				out_selection.put("Parameters.inj",config_map["parameters.inj"].as<int>() );
				out_selection.put("Parameters.inj_rate_min",config_map["parameters.inj_rate_min"].as<double>() );
				out_selection.put("Parameters.inj_rate_max",config_map["parameters.inj_rate_max"].as<double>() );

				out_selection.put("General.TotalRuns", runs.size() );
				out_selection.put("General.TotalEvents", n_events );

				out_selection.put("SelectedRuns.TotalRuns", runs.size() );
				out_selection.put("SelectedEvents.TotalEvents", n_events );

				for(auto & run : runs)
				{
						out_selection.put("SelectedRuns."+std::to_string( run->GetRunNr() ), true );
				}

				AnalysisEvent* analysis_event = new AnalysisEvent();
				analysis_event->CreateHistograms();

				for( auto & run : runs )
				{
						for(auto & evt : run->GetEvents())
						{
								evt->LoadRootFile();

								out_selection.put("SelectedEvents."+std::to_string( evt->GetNr() ), true );
								analysis_event->AddEvent(evt);

								evt->DeleteHistograms();

								analysis_event->GetRates() + evt->GetRates();
						}
						delete run;
						run = NULL;
				}

				out_selection.put("OnlineRate.FWD1", analysis_event->GetRates().rate_online[0] );
				out_selection.put("OnlineRate.FWD2", analysis_event->GetRates().rate_online[1] );
				out_selection.put("OnlineRate.FWD3", analysis_event->GetRates().rate_online[2] );
				out_selection.put("OnlineRate.BWD1", analysis_event->GetRates().rate_online[3] );
				out_selection.put("OnlineRate.BWD2", analysis_event->GetRates().rate_online[4] );
				out_selection.put("OnlineRate.BWD3", analysis_event->GetRates().rate_online[5] );

				out_selection.put("FastRate.FWD1", analysis_event->GetRates().rate_fast[0] );
				out_selection.put("FastRate.FWD2", analysis_event->GetRates().rate_fast[1] );
				out_selection.put("FastRate.FWD3", analysis_event->GetRates().rate_fast[2] );

				out_selection.put("DecompositionResults.FWD1", analysis_event->GetRates().decomposition[0] );
				out_selection.put("DecompositionResults.FWD2", analysis_event->GetRates().decomposition[1] );
				out_selection.put("DecompositionResults.FWD3", analysis_event->GetRates().decomposition[2] );

				out_selection.put("Rate.FWD1", analysis_event->GetRates().rate[0] );
				out_selection.put("Rate.FWD2", analysis_event->GetRates().rate[1] );
				out_selection.put("Rate.FWD3", analysis_event->GetRates().rate[2] );


				analysis_event->SetErrors();
				analysis_event->Normalize();

				//-------------------------------
				//| Let's come to the fun stuff |
				//-------------------------------

				if( config_map["task.peak"].as<bool>() )
				{
						analysis_event->RunPeak();
				}

				if( config_map["task.fft"].as<bool>() )
				{
						analysis_event->RunFFT();
				}

				/**
				 *	Saving of results:
				 *  \todo write some text regarding the saving structure
				 */

				std::string file;
				//  _name;

				std::string name = "";
				std::string folder = "";

				if( config_map["data.run_min"].as<int>() == config_map["data.run_max"].as<int>() )
				{
						/**
						 *  If run_min and run_max are the same we do not need to put both in the file and folder name.
						 */
						name = std::to_string( config_map["data.run_min"].as<int>() );
						//  file   = "run_out_" + std::to_string( config_map["data.run_min"].as<int>() );
						//  folder = "Run_" + std::to_string( config_map["data.run_min"].as<int>() );
				}
				else
				{
						name = std::to_string( config_map["data.run_min"].as<int>() ) + "_" +
						       std::to_string( config_map["data.run_max"].as<int>() );
						//  file   = "run_out_" +
						//        std::to_string( config_map["data.run_min"].as<int>() ) + "_" +
						//         std::to_string( config_map["data.run_max"].as<int>() );
						//
						//  folder = "Run_"
						//   +
						//        std::to_string( config_map["data.run_min"].as<int>() ) + "_" +
						//         std::to_string( config_map["data.run_max"].as<int>() );
				}
				if(config_map["parameters.ler_current_min"].as<double>() >= -50 || config_map["parameters.ler_current_max"].as<double>() <= 2000 )
				{
						name += "_ler" + std::to_string(config_map["parameters.ler_current_min"].as<double>()) +
						        "_"   + std::to_string(config_map["parameters.ler_current_max"].as<double>());
				}

				if(config_map["parameters.her_current_min"].as<double>() >= -50 || config_map["parameters.her_current_max"].as<double>() <= 2000 )
				{
						name += "_ler" + std::to_string(config_map["parameters.her_current_min"].as<double>()) +
						        "_"   + std::to_string(config_map["parameters.her_current_max"].as<double>());
				}

				if(config_map["parameters.inj"].as<int>() != -1)
				{
						name += "_inj" + std::to_string(config_map["parameters.inj"].as<int>());
				}

				folder = name;
				name   = "analysis_run_" + name;

				if( config_map["data.output_prefix"].as<std::string>() != "")
				{
						folder = config_map["data.output_prefix"].as<std::string>() + folder;
				}
				else
				{
						folder = "Run_" + folder;
				}

				if( !boost::filesystem::is_directory(config_map["data.output"].as<boost::filesystem::path>()/folder ) )
				{
						boost::filesystem::create_directory( config_map["data.output"].as<boost::filesystem::path>()/folder );
				}

				analysis_event->SaveEvent( config_map["data.output"].as<boost::filesystem::path>()/(folder + "/" + name +".root" ));
				boost::property_tree::write_ini( path(config_map["data.output"].as<boost::filesystem::path>()/(folder + "/" + name +".ini" )).string(), out_selection);

				std::cout << "Runs: "<< runs.size() << " with Total Events: " << n_events <<  " selected for analysis!" << std::endl;
		}
		else
		{
				std::cout << "\033[33;1mNo Runs and Events suvived the selction criterions!\033[0m running" << "\r" << std::cout;
		}

		return 0;
}
