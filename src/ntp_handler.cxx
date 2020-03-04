/*
 * ntp_handler.cxx
 *
 *  Created on: June 9, 2018
 *      Author: mgabriel
 */

//std includes
 #include <iostream>
// #include <fstream>
// #include <iterator>

// boost
// #include <boost/program_options.hpp>
 #include <boost/filesystem.hpp>
// #include <boost/algorithm/string/replace.hpp>
  #include <boost/algorithm/string/predicate.hpp>
// #include <boost/any.hpp>
// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/ini_parser.hpp>
// #include <boost/property_tree/exceptions.hpp>
// // #include <boost/algorithm/string.hpp>

// // ROOT includes
// #include <TFile.h>
// #include <TTree.h>

// Project includes
#include <ntp_handler.hh>

using namespace std;
using namespace boost;

//----------------------------------------------------------------------------------------------
// Definition of the NTP_Handler class.
// TODO proper description
//----------------------------------------------------------------------------------------------

NTP_Handler::NTP_Handler(filesystem::path search_path) : rfile_(nullptr), ttree_(nullptr)
{
		// vector<filesystem::path> ntp_files;

		copy( filesystem::recursive_directory_iterator(search_path), filesystem::recursive_directory_iterator(), back_inserter(ntp_files)   );

		auto itr = ntp_files.begin();

		while( itr != ntp_files.end() )
		{
				if( ends_with((*itr).filename().string(), ".root" ) && ((*itr).filename().string().find("BEAST_2016-05-") != std::string::npos || (*itr).filename().string().find("BEAST_2016-06-") != std::string::npos) )
				// if( ends_with((*itr).filename().string(), ".root" ) && ((*itr).filename().string().find("2019_11_4_LER") != std::string::npos || (*itr).filename().string().find("2019_11_4_LER") != std::string::npos) )
				{
						itr++;
				}
				// if( ends_with((*itr).filename().string(), ".root" ) && (*itr).filename().string().find("BEAST_2016-05-28") != std::string::npos )
				// {
				//      itr++;
				// }
				else
				{
						// delete (*itr);
						// (*itr) = NULL;
						ntp_files.erase(itr);
				}
		}

		sort(ntp_files.begin(), ntp_files.end());
		cout << "Create NTP_Handler object" << endl;
};

NTP_Handler::~NTP_Handler()
{

};

// template<typename T>
// T NTP_Handler::TestMethode()
