/*
 * ntp_handler.hh
 *
 *  Created on: June 9, 2018
 *      Author: mgabriel
 */

#ifndef CLAWS_NTP_HANDLER_H_
#define CLAWS_NTP_HANDLER_H_

// c++
#include <ctime>
// ## include <>

// boost
// #include <boost/program_options.hpp>
 #include <boost/filesystem.hpp>

// ROOT includes
 #include <TFile.h>
 #include <TTree.h>


using namespace std;

class NTP_Handler
{
public:
NTP_Handler(boost::filesystem::path search_path);
virtual ~NTP_Handler();

// template<typename T>
// T GetPV(double ts, std::string pv);

template<typename T>
T TestMethode()
{
		T rtn;
		return rtn;
};

template<typename T>
T GetPV(double ts, std::string pv)
{
		//T rtn = 0;

		if(rfile_ != nullptr and ttree_ != nullptr)
		{
				try
				{
						T rtn = this->GetPVFromTree<T>(ts, pv, ttree_);
						return rtn;
						//return rtn;
				}
				catch(int e)
				{
						delete ttree_;
						ttree_ = nullptr;
						rfile_->Close();
						rfile_ = nullptr;
						//	std::cout << "An exception occurred. Exception Nr. " << e << '\n';
				}
		}

		for(auto & filepath : ntp_files)
		{
				rfile_ = new TFile(filepath.string().c_str(), "OPEN");
				// auto list = rfile_->GetListOfKeys();
				// rfile_->GetListOfKeys()->Print();

				if( !rfile_->GetListOfKeys()->Contains("tout") )
				{
						rfile_->Close();
						rfile_ = nullptr;
						continue;
				}

				ttree_ = (TTree*)rfile_->Get("tout");

				try
				{
						// vector<double>* test =  this->GetPVFromTree<vector<double>*>(ts, pv, ttree_);
						// return rtn;
						T rtn = this->GetPVFromTree<T>(ts, pv, ttree_);
						return rtn;
				}
				catch(int e)
				{
						delete ttree_;
						rfile_->Close();
						ttree_ = nullptr;
						rfile_ = nullptr;
						//	std::cout << "An exception occurred. Exception Nr. " << e << '\n';
				}
		}

		// current date/time based on current system
		time_t now = time(0);

		// convert now to string form
		char* dt = ctime(&now);

		cout<< pv << " for timestamp " << fixed << setprecision(10) << ts << " NOT FOUND @: " << dt << endl;
		//exit(1);
		// delete ttree_;
		// rfile_->Close();
		// ttree_ = nullptr;
		// rfile_ = nullptr;
		throw -3;

		T rtn;
		return rtn;
};


template<typename T>
T GetPVFromTree(double ts, std::string pv, TTree* tree)
{
		// check if tree has branch pv;

		bool found = false;

		TObjArray* array = tree->GetListOfBranches();
		for(int i = 0; i < array->GetEntries(); ++i)
		{
				if(array->At(i)->GetName() == pv)
				{
						found = true;
						break;
				}
				//	cout << array->At(i)->GetName() << '\n';
		}

		if(!found) throw -1;

		// check it ts is in tree;
		Long64_t nentries = tree->GetEntries();
		UInt_t ts_tree;
		T val_tree = 0;
		T prev_val_tree = 0;

		tree->SetBranchAddress("ts",&ts_tree);
		tree->SetBranchAddress(pv.c_str(), &val_tree);


		tree->GetEntry(0);
		double t1 = ts_tree;
		tree->GetEntry(nentries-1);
		double t2 = ts_tree;
		prev_val_tree = val_tree;

		if(t1 <= ts && ts <= t2)
		{
				//UInt_t ts_last = 0;
				for (Long64_t i=0; i<nentries; i++)
				{
						tree->GetEntry(i);

						if( t1 <= ts && ts<= ts_tree)
						{
								// ts found!

								// Since we passed the address of a local variable we need
								// to remove it.
								tree->ResetBranchAddresses();

								if((ts-t1) < (ts_tree-ts)) return prev_val_tree;
								else return val_tree;
						}
						prev_val_tree = val_tree;
						t1= ts_tree;

				}
		}
		// Since we passed the address of a local variable we need
		// to remove it.
		tree->ResetBranchAddresses();

		throw -1;

		// return T;
};

std::vector<boost::filesystem::path> ntp_files;
TFile* rfile_;
TTree* ttree_;
// TBranch* br_ts_;
// TBranch* br_ts_;
};

#endif /* CLAWS_NTP_HANDLER_H_ */
