//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================


// std includes
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <typeinfo>

// mixed
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

// root include
#include "TFile.h"
#include "TSystem.h"
#include "TKey.h"
#include "TTree.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TApplication.h"
#include "TCanvas.h"




// my includes
#include "include/Event.h"


using namespace std;
using namespace boost;
using namespace boost::filesystem;
// using namespace po = boost::program_options;
namespace pt = boost::property_tree;


int error(){
	throw "DeiMudder";
	return 0;
}

string indent(int level) {
  string s;
  for (int i=0; i<level; i++) s += "  ";
  return s;
}

void printTree (pt::ptree &pt, int level) {
  if (pt.empty()) {
    cerr << "\""<< pt.data()<< "\"";
  } else {
    if (level) cerr << endl;
    cerr << indent(level) << "{" << endl;
    for (pt::ptree::iterator pos = pt.begin(); pos != pt.end();) {
      cerr << indent(level+1) << "\"" << pos->first << "\": ";
      printTree(pos->second, level + 1);
      ++pos;
      if (pos != pt.end()) {
        cerr << ",";
      }
      cerr << endl;
    }
    cerr << indent(level) << " }";
  }
  return;
}

// string fileFromPath(path p){
// 	cout << p.filename() << endl;
// 	for(const auto &f : p){
// 		cout << p.filename() << endl;
// 	}
// 	return "";
// }


//===============================================================================
// Globals for the makeBEASTntuple operations. In poor style, some
// of these are used inside BEASTdetector member functions.
//===============================================================================

// Hold run number metadata
struct run {
  std::string name;
  unsigned int run_num;
  unsigned int sub_run;
  unsigned int tsMin;
  unsigned int tsMax;
};



int main(int argc, char* argv[]) {



	cout << "---------------------------------------------------------" << endl;
	cout << "|         Starting CLAWS phase I data calibration       |" << endl;
	cout << "---------------------------------------------------------" << endl;


//-------------------------------------------------------------
// First get the corresponding Tmin and Tmac from the beast run
//-------------------------------------------------------------

	static unsigned int lastTs=0;

	std::ifstream runfile("./runs.txt", std::ios_base::in);
	std::string line;


	double runnumber = 5001;

	double runnumbertmp, tsMin, tsMax;


	if(!runfile) {
	  std::cout << "ERROR: run number file not found. Exiting." << std::endl;
	  exit(1);
	}
	else {
	  while (std::getline(runfile,line)) {
		std::istringstream iss(line);

		iss >> runnumbertmp;

		if(runnumber == runnumbertmp) {
			iss >> tsMin >> tsMax;
		}

	  }
	}
	std::cout << "Nr: " << runnumber << std::setprecision(12)<< ", tsMin: " << tsMin << ", tsMax: " << tsMax << std::endl;

//----------------------------------------------------------------------------------------------
// Now search for the right claws runs & events corresponding to the timestamp of the beast run
//----------------------------------------------------------------------------------------------


	// path path_to_day("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-23");
	path path_to_day("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-23");
	path path_to_ntuple("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23");
//	path path_to_rate("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-26/Run-401161/ok.dat");


	string file_runstart = "";
	string file_runstop  = "";


	Run myrun(path( "/remote/ceph/group/ilc/claws/data/RAW/connecticut/16-05-23/Run-" + string(argv[1]) ) );

	myrun.WriteNTuple(path_to_ntuple);



	return 0;

}

















	// cycle through the directory
// 	for (directory_iterator itr(path_to_day); itr != end_itr; ++itr)
// 	{
// 		if (is_regular_file(itr->path())) {
// 			// If it is a file do not do anything!
//
// // 			string current_file = itr->path().string();
// // //			cout <<  itr->path()<< " is directory!" << endl;
// // 			cout <<  itr->path().filename().string() << " is file!" << endl;
// 			// fileFromPath(itr->path());
// 		}
//
// 		else if(is_directory(itr->path()) && starts_with(itr->path().filename().string(), "Run-")){
// 			// If it is a directory check if it is a Run folder and proceed.
//
// 			path datafolder = itr->path() / path("data_root");
// 			directory_iterator end_itr;
// 			// Cycle through the event files.
// 			for (directory_iterator itr(datafolder); itr != end_itr; ++itr){
//
// 			}
//
//
// 		}
// 	}


	// pt::ptree pt;
    // pt::ini_parser::read_ini(p.string(), pt);
	// printTree(pt, 0);
	//
	//
	// std::ifstream ratefile(path_to_rate.string());
	//
	// double rate_fwd1, rate_fwd2, rate_fwd3, rate_fwd4, rate_bwd1, rate_bwd2, rate_bwd3, rate_bwd4;
	//
	// if (!ratefile){
	// 	cerr << "not file" << endl;
	// 	exit(1);
	// }
	// while(!ratefile.eof())
	// {
	// 	ratefile >> rate_fwd1 >> rate_fwd2 >> rate_fwd3 >> rate_fwd4 >> rate_bwd1 >> rate_bwd2 >> rate_bwd3 >> rate_bwd4;
	//
	// }
	// cout << rate_fwd1 << endl;
	// cout << rate_fwd2 << endl;
	// cout << rate_fwd3 << endl;
	// cout << rate_fwd4 << endl;
	// cout << rate_bwd1 << endl;
	// cout << rate_bwd2 << endl;
	// cout << rate_bwd3 << endl;
	// cout << rate_bwd4 << endl;
	//





//	Event e(p);
//	e.draw();











					// 	  //asdadsasdlcasca
					  //
					//   	// Path and file name.
					//   	// TODO: commandline args
					//   	TString path = "/Users/mgabriel/workspace/claws/claws_analysis/claws_analysis/Run-300022.root";
					  //
					//   	TFile * rootfile  = new TFile(path, "OPEN");
					  //
					//       if (rootfile->IsZombie()){
					//           cout<<"File: " << path << " was not found!"<<endl;
					//           return -1;
					//       }
					//   //	rootfile->GetListOfKeys()->Print();
					  //
					//   	TDirectory* dir_data;
					//   	TDirectory* dir_scope1;
					//   	TDirectory* dir_scope2;
					  //
					//   	//TODO Make a try and catch block with exceptions, in case folders are not present in the file.
					  //
					//   	dir_data   = (TDirectory*)rootfile->GetDirectory("data", true);
					  //
					//   	dir_scope1 = (TDirectory*)rootfile->GetDirectory("Scope1", true);
					  //
					//   	dir_scope2 = (TDirectory*)rootfile->GetDirectory("Scope2", true);
					  //
					  //
					  //
					  //
					  //
					//       Data* data=new Data(dir_data);
					  //
					  //
					  //
					  //
					//   	rootfile->Close();
