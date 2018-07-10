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
				if( ends_with((*itr).filename().string(), ".root" ))
				{
						itr++;
				}
				else
				{
						// delete (*itr);
						// (*itr) = NULL;
						ntp_files.erase(itr);
				}
		}


		cout << "Create NTP_Handler object" << endl;



};

NTP_Handler::~NTP_Handler()
{

};

double NTP_Handler::GetPV(double ts, string pv)
{

		try
		{
				double rtn = this->GetPVFromTree(ts, pv, ttree_);
				return rtn;
		}
		catch(int e)
		{
				delete ttree_;
				rfile_->Close();
				cout << "An exception occurred. Exception Nr. " << e << '\n';
		}

		for(auto & filepath : ntp_files)
		{
				TFile *rfile_ = new TFile(filepath.string().c_str(), "OPEN");
				auto list = rfile_->GetListOfKeys();
				rfile_->GetListOfKeys()->Print();

				if( !rfile_->GetListOfKeys()->Contains("tout") )
				{
						rfile_->Close();
						continue;
				}

				TTree *ttree_ = (TTree*)rfile_->Get("tout");

				try
				{
						auto rtn = this->GetPVFromTree(ts, pv, ttree_);
						return rtn;
				}
				catch(int e)
				{
						delete ttree_;
						rfile_->Close();
						cout << "An exception occurred. Exception Nr. " << e << '\n';
				}


				//      auto listofbranch = tout->GetListOfBranches();
				//
				//      // if( branch in ttree and ts in ts)
				//      // {
				//      //         Get the shit and break;
				//      //
				//      // }
				//      cout << "ThatEver" << endl;
				//      rfile->Close();
		}
};



double NTP_Handler::GetPVFromTree(double ts, std::string pv, TTree* tree)
{
		// check if tree has branch pv;

		bool found = false;

		TObjArray* array = tree->GetListOfBranches();
		for(int i = 0; i < array->GetEntries(); ++i)
		{
				if(array->At(i)->GetName() == pv) found = true;

				cout << array->At(i)->GetName() << '\n';
		}
		//       for branch in :
		// if( branch.GetName() == "TheBranchIWant" ):
		//   DoSomeAnalysis()
		//
		if(!found) throw -1;


		// vector<double> timestamps
		// timestamps.reserve(nentries);
		//
		// vector<double> values;
		// values.reserve(nentries);

		// check it ts is in tree;
		Int_t nentries = (Int_t)tree->GetEntries();
		double ts_tree, val_tree;

		tree->SetBranchAddress("ts",&ts_tree);
		tree->SetBranchAddress(pv.c_str(), &val_tree);

		for (Int_t i=0; i<nentries; i++)
		{
				tree->GetEntry(i);
				if(ts_tree > ts) return val_tree;
		}

		throw -2;

		return -1;
}
