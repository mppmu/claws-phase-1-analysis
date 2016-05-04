//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================


// std includes
#include <iostream>
#include <vector>
#include <map>

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


int error(){
	throw "DeiMudder";
	return 0;
}

int main() {


	cout << "---------------------------------------------------------" << endl;
	cout << "|            Starting CLAWS phase I data analysis       |" << endl;
	cout << "---------------------------------------------------------" << endl;

	

	// Path and file name.
	// TODO: commandline args
	TString path = "/Users/mgabriel/workspace/claws/claws_analysis/claws_analysis/Run-300022.root";

	TFile * rootfile  = new TFile(path, "OPEN");

    if (rootfile->IsZombie()){
        cout<<"File: " << path << " was not found!"<<endl;
        return -1;
    }
//	rootfile->GetListOfKeys()->Print();

	TDirectory* dir_data;
	TDirectory* dir_scope1;
	TDirectory* dir_scope2;

	//TODO Make a try and catch block with exceptions, in case folders are not present in the file.

	dir_data   = (TDirectory*)rootfile->GetDirectory("data", true);

	dir_scope1 = (TDirectory*)rootfile->GetDirectory("Scope1", true);

	dir_scope2 = (TDirectory*)rootfile->GetDirectory("Scope2", true);

    

	TTree* data = (TTree*)dir_data->Get("300022500-data");
    TTree* meta = (TTree*)dir_data->Get("300022500-meta");
    
    Event* event = new Event(meta, data);

    Data* data=new Data(dir_data);

//	long int n_entries = data->GetEntries();
//	TBranch* tree_bwd1=data->GetBranch("BWD1");
//
//	short int_bwd1;
//
//	tree_bwd1->SetAddress(&int_bwd1);
//
//	TGraph* graph_bwd1 = new TGraph();
//
//
//	for (int var = 0; var < n_entries; ++var) {
//		data->GetEntry(var);
//		graph_bwd1->SetPoint(var, var, int_bwd1);
//	}




//	vector<TBranch*> branches;
//	branches.push_back(testtree->GetBranch("BWD1"));
//	branches.at(0)->
//	vector<int> bwd1;
//	for (int i = 0; i < n_entries; ++i) {
//
//	}
//	TList* filecontent = rootfile->GetListOfKeys();
//	filecontent->Print();
//	filecontent->







	rootfile->Close();
    
//    TApplication app("app", NULL, NULL);
//    event->eventToPdf("/Users/mgabriel/workspace/claws/claws_analysis/claws_analysis/Run-300022.pdf");
//	app.Run();

	return 0;
                          }
