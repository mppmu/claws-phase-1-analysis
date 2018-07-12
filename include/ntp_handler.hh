/*
 * ntp_handler.hh
 *
 *  Created on: June 9, 2018
 *      Author: mgabriel
 */

#ifndef CLAWS_NTP_HANDLER_H_
#define CLAWS_NTP_HANDLER_H_

// boost
// #include <boost/program_options.hpp>
 #include <boost/filesystem.hpp>

// ROOT includes
 #include <TFile.h>
 #include <TTree.h>

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
						return this->GetPVFromTree<T>(ts, pv, ttree_);
						//return rtn;
				}
				catch(int e)
				{
						delete ttree_;
						rfile_->Close();
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
						continue;
				}

				ttree_ = (TTree*)rfile_->Get("tout");

				try
				{
						// vector<double>* test =  this->GetPVFromTree<vector<double>*>(ts, pv, ttree_);
						// return rtn;
						return this->GetPVFromTree<T>(ts, pv, ttree_);

				}
				catch(int e)
				{
						delete ttree_;
						rfile_->Close();
						//	std::cout << "An exception occurred. Exception Nr. " << e << '\n';
				}
		}
};


template<typename T>
T GetPVFromTree(double ts, std::string pv, TTree* tree)
{
		// check if tree has branch pv;

		bool found = false;

		TObjArray* array = tree->GetListOfBranches();
		for(int i = 0; i < array->GetEntries(); ++i)
		{
				if(array->At(i)->GetName() == pv) found = true;
				//	cout << array->At(i)->GetName() << '\n';
		}

		if(!found) throw -1;

		// check it ts is in tree;
		Int_t nentries = (Int_t)tree->GetEntries();
		UInt_t ts_tree;
		T val_tree = 0;

		tree->SetBranchAddress("ts",&ts_tree);
		tree->SetBranchAddress(pv.c_str(), &val_tree);

		//UInt_t ts_last = 0;
		for (Int_t i=0; i<nentries; i++)
		{
				tree->GetEntry(i);
				if( fabs(ts-ts_tree) < 0.5)
				{
						// ts found!

						// Since we passed the address of a local variable we need
						// to remove it.
						tree->ResetBranchAddresses();
						return val_tree;
				}

		}

		// Since we passed the address of a local variable we need
		// to remove it.
		tree->ResetBranchAddresses();

		throw -2;

		// return T;
};

std::vector<boost::filesystem::path> ntp_files;
TFile* rfile_;
TTree* ttree_;
// TBranch* br_ts_;
// TBranch* br_ts_;
};

#endif /* CLAWS_NTP_HANDLER_H_ */
