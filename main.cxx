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
#include "Event.h"


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

string fileFromPath(path p){
	cout << p.filename() << endl;
	for(const auto &f : p){
		cout << p.filename() << endl;
	}
	return "";
}


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
	cout << "|            Starting CLAWS phase I data analysis       |" << endl;
	cout << "---------------------------------------------------------" << endl;


//-------------------------------------------------------------
// First get the corresponding Tmin and Tmac from the beast run
//-------------------------------------------------------------

	static unsigned int lastTs=0;

	std::ifstream runfile("./runs.txt", std::ios_base::in);
	std::string line;

//	double runnumber = 5001;
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
	path path_to_rate("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-26/Run-401161/Rate-Run--11610");
//	path path_to_rate("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-26/Run-401161/ok.dat");


	string file_runstart = "";
	string file_runstop  = "";

	directory_iterator end_itr;
	vector<path> dircontent;
	vector<pair<path, double>> events;

	copy(directory_iterator(path_to_day), directory_iterator(), back_inserter(dircontent));
	sort(dircontent.begin(), dircontent.end());

	for (vector<path>::const_iterator itr = dircontent.begin(); itr != dircontent.end(); ++itr)
	{

		 if(is_directory(*itr) && starts_with((*itr).filename().string(), "Run-")){
			cout << "Looking at: " << (*itr).filename() << '\n';
			path datafolder = (*itr) / path("data_root");
			vector<path> dircontent;							// local copy
			copy(directory_iterator(datafolder), directory_iterator(), back_inserter(dircontent));
			sort(dircontent.begin(), dircontent.end());
			for (vector<path>::const_iterator itr = dircontent.begin(); itr != dircontent.end(); ++itr){
				if(is_regular_file(*itr) && starts_with((*itr).filename().string(), "Event-") && ends_with((*itr).filename().string(), ".ini") ){
				//	cout << " " << (*itr).filename() << '\n';

					pt::ptree pt;
					pt::ini_parser::read_ini((*itr).string(), pt);
					double unixtime = pt.get<double>("Properties.UnixTime");
					bool lerbg = pt.get<double>("SuperKEKBData.LERBg");
					bool herbg = pt.get<double>("SuperKEKBData.HERBg");
					if(unixtime >= tsMin && unixtime <= tsMax && !lerbg && !herbg){
						cout << "Adding : " << (*itr) << ", with unixtime: "<< unixtime << "and tsMin: " << tsMin << ", and time diff: " << unixtime - tsMin << endl;
						events.push_back(make_pair((*itr), unixtime));
					}
					// else{
					// 	cout << "Not Adding : " << (*itr) << ", with unixtime: "<< unixtime << "and tsMin: " << tsMin << ", and time diff: " << unixtime - tsMin << endl;
					// }
					// else if(unixtime >= tsMax && file_runstart != ""){
					//
					// }
					// cout <<  unixtime << std::setprecision(22) << endl;


				}


			}
	//	 	cout << " " << (*itr).filename() << '\n';
  	  // 			path datafolder = itr->path() / path("data_root");
  	  // 			directory_iterator end_itr;
  	  // 			// Cycle through the event files.
  	  // 			for (directory_iterator itr(datafolder); itr != end_itr; ++itr){
  	  //

  	  	}
	}
	// for(int i=0; i < events.size(); i++){
	// 	cout << events.at(i).first << ", at:" << events.at(i).second << "\n";
	// }

//----------------------------------------------------------------------------------------------
// Now get the rates
//----------------------------------------------------------------------------------------------

	vector<vector<double>> rates;

	for(int i=0; i < events.size(); i++){
		//  Some complex shit going on to convert the names/paths of the eventfiles to the corresponding rate files.
		string filename = events.at(i).first.filename().string();
		replace_first(filename, "Event-40", "");
		replace_last(filename, ".ini", "");
		// string ratefile = "Rate-Run--" + to_string( atoi(filename.substr(0,4).c_str()) ) + to_string( atoi(filename.substr(4,8).c_str())-1 );
		filename = "Rate-Run--" + to_string( atoi(filename.substr(0,4).c_str()) ) + to_string( atoi(filename.substr(4,8).c_str())-1 );
		path path_to_ratefile = events.at(i).first.parent_path().parent_path() / filename;

		std::ifstream ratefile(path_to_ratefile.string());

		double myarray[8] = {0};

		double unixtime = events.at(i).second;

		if (!ratefile){
			cerr << "not file" << endl;
			exit(1);
		}
		vector <double> tmp;
		tmp.push_back(unixtime);
//		int count =0;
		while(ratefile >> myarray[0] >> myarray[1] >> myarray[2] >> myarray[3] >> myarray[4] >> myarray[5] >> myarray[6] >> myarray[7])
		{
			for (int i = 0; i<8;i++){
				tmp.push_back(myarray[i]);
			}
			//ratefile >> myarray[0] >> myarray[1] >> myarray[2] >> myarray[3] >> myarray[4] >> myarray[5] >> myarray[6] >> myarray[7];
//			cout<< count << ", "<< myarray[0] << " , "<< myarray[1] << " , "<< myarray[2] << " , "<< myarray[3] << " , "<< myarray[4] << " , "<< myarray[5] << " , "<< myarray[6] << " , "<< myarray[7] << "\n";
//			count++;
			if(ratefile.eof()) break;
		}
		rates.push_back(tmp);
	}
	cout << events.size() << " , " << rates.size() << endl;
	for(int i=0; i < rates.size(); i++){
		for(int j=0; j < 9; j++){
			cout << rates.at(i).at(j) << ", ";
		}
		cout << endl;
	// 	cout << rates.at(i).first << ", " << rates.at(i).second[0]  << ", " << rates.at(i).second[1]  << ", " << rates.at(i).second[2]  << ", " << rates.at(i).second[3]  << ", " << rates.at(i).second[4]  << ", " << rates.at(i).second[5]  << ", " << rates.at(i).second[6]  << ", " << rates.at(i).second[7] << "\n";
	}


//----------------------------------------------------------------------------------------------
// Now put everything in a root file
//----------------------------------------------------------------------------------------------


	TFile * rootfile  = new TFile(("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23/CLAWS-5001-"+ to_string((int)tsMin)+ ".root").c_str(), "RECREATE");

	TTree *tout = new TTree("tout","tout");

	double ts, rate_fwd1, rate_fwd2, rate_fwd3, rate_fwd4, rate_bwd1, rate_bwd2, rate_bwd3, rate_bwd4;

	tout->Branch("ts", &ts,     "ts/D");
	tout->Branch("fwd1", &rate_fwd1,     "fwd1/D");
	tout->Branch("fwd2", &rate_fwd2,     "fwd2/D");
	tout->Branch("fwd3", &rate_fwd3,     "fwd3/D");
	tout->Branch("fwd4", &rate_fwd4,     "fwd4/D");
	tout->Branch("bwd1", &rate_bwd1,     "fbwd1/D");
	tout->Branch("bwd2", &rate_bwd2,     "fbwd2/D");
	tout->Branch("bwd3", &rate_bwd3,     "fbwd3/D");
	tout->Branch("bwd4", &rate_bwd4,     "fbwd4/D");

	for(int i=0; i < rates.size(); i++){
		ts = rates.at(i).at(0);
		rate_fwd1 = rates.at(i).at(1);
		rate_fwd2 = rates.at(i).at(2);
		rate_fwd3 = rates.at(i).at(3);
		rate_fwd4 = rates.at(i).at(4);
		rate_bwd1 = rates.at(i).at(5);
		rate_bwd2 = rates.at(i).at(6);
		rate_bwd3 = rates.at(i).at(7);
		rate_bwd4 = rates.at(i).at(8);
		tout->Fill();
	}
	tout->Write();
	rootfile->Write();

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

	return 0;

}










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
