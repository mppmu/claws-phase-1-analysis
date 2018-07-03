//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================

//std includes
// #include <iostream>
// #include <fstream>
// #include <iterator>
// // boost
// #include <boost/program_options.hpp>
// #include <boost/filesystem.hpp>
//
// // project includes
// #include "run.hh"
// #include "globalsettings.hh"

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {

	cout << "---------------------------------------------------------" << endl;
	cout << "|            CLAWS phase I anaysis                       |" << endl;
	cout << "---------------------------------------------------------" << endl;

	// Load Settings and
	vector<program_options::options_description> options;


	for(auto option : options)
	{
		// Data based selection

		// Load runs

		// Meta data based selection

		// Load waveforms

		// Build analysis event

		// Do stuff with analysis event

		// push_back analysis event
	}

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
