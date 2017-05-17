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

	// TApplication *app=new TApplication("app",0,0);
//	boost::property_tree::ptree config_file;

	boost::program_options::options_description options("Generic options");
    options.add_options()
    	("help", "Displays this help message.")
        ("config,c", boost::program_options::value<boost::filesystem::path>()->default_value("./config/analysis_parameters.ini"), "Config file to get parameters from.")
		("out", boost::program_options::value<boost::filesystem::path>()->default_value("/remote/ceph/group/ilc/claws/data"), "Data directory containing results.")
        ;

	boost::program_options::options_description data_options("Data");
	data_options.add_options()
		("data.test", boost::program_options::value<std::string>()->default_value("default_value"), "test value")
		("data.input", boost::program_options::value<boost::filesystem::path>(), "Data directory containing runs meant to be analysed.")
		("data.ts_min", boost::program_options::value<double>(), "If selected, smallest timestamp of events used.")
		("data.ts_max", boost::program_options::value<double>(), "If selected, largest timestamp of events used.")
		("data.event_min", boost::program_options::value<int>(), "If selected, smallest event number of events used.")
		("data.event_max", boost::program_options::value<int>(), "If selected, largest event number of events used.")
		("data.run_min", boost::program_options::value<int>(), "If selected, smallest run number of events used.")
		("data.run_max", boost::program_options::value<int>(), "If selected, largest run number of events used.")
		("data.day_min", boost::program_options::value<std::string>(), "If selected, earliest day used.")
		("data.day_max", boost::program_options::value<std::string>(), "If selected, latest used.")
		;
	options.add(data_options);


	boost::program_options::options_description parameter_options("Parameters");

	parameter_options.add_options()
		("parameters.ler_current_min", boost::program_options::value<double>(), "Minimum ler current.")
		("parameters.ler_current_max", boost::program_options::value<double>(), "Maximum ler current.")
		("parameters.her_current_min", boost::program_options::value<double>(), "Minimum her current.")
		("parameters.her_current_max", boost::program_options::value<double>(), "Maximum her current.")
		("parameters.inj", boost::program_options::value<int>(), "Injection requirement:\n-1: doesn't matter\n0: no injection \n1: injection in one or both rings\n2: in LER only\n3: in HER only\n3: in both rings")
		("parameters.inj_rate_min", boost::program_options::value<double>(), " ")
		("parameters.inj_rate_max", boost::program_options::value<double>(), " ")
		;

	options.add(parameter_options);


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

	std::cout << config_map["data.input"].as<boost::filesystem::path>() << "\n";
	std::cout << config_map["data.day_min"].as<std::string>() << "\n";
	std::cout << config_map["data.day_max"].as<std::string>() << "\n";
	std::vector <AnalysisRun*> runs;

	/** Check which runs are supposed to go into
	 *
     */
	 for(auto & itr_vec : GS->GetRuns( config_map["data.input"].as<boost::filesystem::path>()) )
	 {
	     int run_nr = stoi(itr_vec.filename().string().substr(4));

	     if(	run_nr >= config_map["data.run_min"].as<int>()
		 	 && run_nr <= config_map["data.run_max"].as<int>() )
		 {
			 std::string str = itr_vec.parent_path().filename().string();
			 int day   = std::stoi(str.substr(str.length() - 2) );
			 int month = std::stoi(str.substr(str.length() - 5 , 2) );

			 if( 	 month > stoi( config_map["data.day_min"].as<std::string>().substr(0, 2))
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
									config_map["parameters.ler_current_max"].as<double>()	);
		 (*itr)->SetCurrentLimit(   "HER",
		 		 					config_map["parameters.her_current_min"].as<double>(),
						   			config_map["parameters.her_current_max"].as<double>()	);

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

	 for( auto & run : runs )
	 {
		 run->LoadWaveforms();
	 }
	 
	 AnalysisEvent* analysis_event = new AnalysisEvent();

	//  for(auto & event : )
	//  {
	// 	event->LoadRootFile();
	//  }

	 std::cout << "Runs: "<< runs.size() << " with Total Events: " << n_events <<  " selected for analysis!" << std::endl;








	 	//if(itr_vec.parent_path().filename().string()[]
	 	//{
			//
	 	//}




	// app->Run();
	return 0;
}
