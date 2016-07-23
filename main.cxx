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

// mixed
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem/fstream.hpp>

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

int main(int argc, char* argv[]) {



	cout << "---------------------------------------------------------" << endl;
	cout << "|            Starting CLAWS phase I data analysis       |" << endl;
	cout << "---------------------------------------------------------" << endl;

	path p("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-26/Run-401161/data_root/Event-401161001.ini");
	path path_to_rate("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-26/Run-401161/Rate-Run--11610");
//	path path_to_rate("/remote/ceph/group/ilc/claws/data/claws_phaseI/connecticut/16-05-26/Run-401161/ok.dat");

	pt::ptree pt;
    pt::ini_parser::read_ini(p.string(), pt);
	printTree(pt, 0);


	std::ifstream ratefile(path_to_rate.string());

	double rate_fwd1, rate_fwd2, rate_fwd3, rate_fwd4, rate_bwd1, rate_bwd2, rate_bwd3, rate_bwd4;

	if (!ratefile){
		cerr << "not file" << endl;
		exit(1);
	}
	while(!ratefile.eof())
	{
		ratefile >> rate_fwd1 >> rate_fwd2 >> rate_fwd3 >> rate_fwd4 >> rate_bwd1 >> rate_bwd2 >> rate_bwd3 >> rate_bwd4;

	}
	cout << rate_fwd1 << endl;
	cout << rate_fwd2 << endl;
	cout << rate_fwd3 << endl;
	cout << rate_fwd4 << endl;
	cout << rate_bwd1 << endl;
	cout << rate_bwd2 << endl;
	cout << rate_bwd3 << endl;
	cout << rate_bwd4 << endl;


	// directory_iterator end_itr;
	//
	// for (directory_iterator itr(p); itr != end_itr; ++itr)
	// {
	// 	if (is_regular_file(itr->path()))
	// 	{
	// 		string current_file = itr->path().string();
	// 		cout << current_file << endl;
	// 	}
	// }

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
