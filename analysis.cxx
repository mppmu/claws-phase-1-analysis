 //============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Copyright   : GNU General Public License
// Description :
//============================================================================


// std includes
#include<iostream>
#include<vector>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>





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
#include "TVectorD.h"
// my includes



using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {

	TApplication *app=new TApplication("app",0,0);

	cout << "---------------------------------------------------------" << endl;
	cout << "|          Starting CLAWS phase I ntuple analysis       |" << endl;
	cout << "---------------------------------------------------------" << endl;


//    filesystem::path p("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23/CLAWS-ON-400999-1463967125-146396871.root");
	filesystem::path p("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23");
//		TFile f("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23/CLAWS-ON-400999-1463967125-1463968071.root");






	vector <TGraph*> graph_channels;

	for (int i=0;i<8;i++){
		TGraph *tmp = new TGraph();
		tmp->SetMarkerStyle(20);
		tmp->SetMarkerSize(0.8);
		tmp->SetMarkerColor(i+1);
		graph_channels.push_back(tmp);
	}



	vector<filesystem::path> folder_content;

	copy(filesystem::directory_iterator(p), filesystem::directory_iterator(), back_inserter(folder_content));

	int i_point = 0;

	for (vector<filesystem::path>::const_iterator itr = folder_content.begin(); itr != folder_content.end(); ++itr){
		cout << "Opening file: " << (*itr) << endl;
		if(    is_regular_file(*itr)
			&& starts_with((*itr).filename().string(), "CLAWS-ON-")
			&& ends_with((*itr).filename().string(), ".root"))
		{
			string file = filesystem::path(*itr).string();
			cout << file << endl;
			TFile f(file.c_str());


			if (f.IsZombie())
			{
			cerr << "not file" << endl;
	        exit(-1);
			}

			TTree *tree_rate_online = (TTree*)f.Get("rates_online");

			double rate[8], ts;
			tree_rate_online->SetBranchAddress("ts", &ts);

			tree_rate_online->SetBranchAddress("fwd1", &rate[0]);
			tree_rate_online->SetBranchAddress("fwd2", &rate[1]);
			tree_rate_online->SetBranchAddress("fwd3", &rate[2]);
			tree_rate_online->SetBranchAddress("fwd4", &rate[3]);
			tree_rate_online->SetBranchAddress("bwd1", &rate[4]);
			tree_rate_online->SetBranchAddress("bwd2", &rate[5]);
			tree_rate_online->SetBranchAddress("bwd3", &rate[6]);
			tree_rate_online->SetBranchAddress("bwd4", &rate[7]);

			for(unsigned int i=0; i < tree_rate_online->GetEntries(); i++)
			{
				tree_rate_online->GetEntry(i);

				for (int ch=0;ch<8;ch++){
					graph_channels.at(ch)->SetPoint(i+i_point,ts,rate[ch]);
				}
			}
			i_point += tree_rate_online->GetEntries();

			f.Close();
		}

	}



    cout << "Drawing now!" << endl;
    TCanvas * c1 = new TCanvas("fwd1", "fwd1", 600, 600);

	graph_channels.at(0)->Draw("AP");
	for (int ch=1;ch<8;ch++){
		graph_channels.at(ch)->Draw("P");
	}



	app->Run();
	return 0;

}
