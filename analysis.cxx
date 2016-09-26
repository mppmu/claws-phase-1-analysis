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
// project includes
#include "GlobalSettings.hh"


using namespace std;
using namespace boost;
//using namespace claws;

int main(int argc, char* argv[]) {

	TApplication *app=new TApplication("app",0,0);

	cout << "---------------------------------------------------------" << endl;
	cout << "|          Starting CLAWS phase I ntuple analysis       |" << endl;
	cout << "---------------------------------------------------------" << endl;




//    filesystem::path p("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23/CLAWS-ON-400999-1463967125-146396871.root");
//	filesystem::path p("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23");
//		TFile f("/remote/ceph/group/ilc/claws/data/NTP/CLAWS/16-05-23/CLAWS-ON-400999-1463967125-1463968071.root");





	vector <TGraph*> graph_channels;

	for (int i=0;i<8;i++){
		TGraph *tmp = new TGraph();
		// tmp->GetYaxis()->SetRangeUser(0, 400000);
		// tmp->GetXaxis()->SetRangeUser(1463960000, 1464010000);
		tmp->SetMarkerStyle(20);
		tmp->SetMarkerSize(0.8);
		tmp->SetMarkerColor(i+1);
		graph_channels.push_back(tmp);
	};

	filesystem::path p(GS->ResetHook()->SetData()->SetNtp()->SetDetector(claws::CLW)->SetDate(atoi(argv[1]), 5, 16)->GetHook());

	vector<filesystem::path> folder_content;

	copy(filesystem::directory_iterator(p), filesystem::directory_iterator(), back_inserter(folder_content));

	int i_point = 0;

	for (vector<filesystem::path>::const_iterator itr = folder_content.begin(); itr != folder_content.end(); ++itr){
		cout << "Opening file: " << (*itr) << endl;
		if(    is_regular_file(*itr)
			&& starts_with((*itr).filename().string(), "CLAWSv0.1-")
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



			double rate[8], ts;

			TTree *tree_rate_online = (TTree*)f.Get("on");

			tree_rate_online->SetBranchAddress("ts", &ts);

			tree_rate_online->SetBranchAddress("fwd1", &rate[0]);
			tree_rate_online->SetBranchAddress("fwd2", &rate[1]);
			tree_rate_online->SetBranchAddress("fwd3", &rate[2]);
			tree_rate_online->SetBranchAddress("fwd4", &rate[3]);
			tree_rate_online->SetBranchAddress("bwd1", &rate[4]);
			tree_rate_online->SetBranchAddress("bwd2", &rate[5]);
			tree_rate_online->SetBranchAddress("bwd3", &rate[6]);
			tree_rate_online->SetBranchAddress("bwd4", &rate[7]);

			// TTree *tree_rate_online_inj = (TTree*)f.Get("on_inj");
			//
			// tree_rate_online_inj->SetBranchAddress("ts", &ts);
			//
			// tree_rate_online_inj->SetBranchAddress("fwd1", &rate[0]);
			// tree_rate_online_inj->SetBranchAddress("fwd2", &rate[1]);
			// tree_rate_online_inj->SetBranchAddress("fwd3", &rate[2]);
			// tree_rate_online_inj->SetBranchAddress("fwd4", &rate[3]);
			// tree_rate_online_inj->SetBranchAddress("bwd1", &rate[4]);
			// tree_rate_online_inj->SetBranchAddress("bwd2", &rate[5]);
			// tree_rate_online_inj->SetBranchAddress("bwd3", &rate[6]);
			// tree_rate_online_inj->SetBranchAddress("bwd4", &rate[7]);

			int point = 0;
			for(unsigned int i =0; i < tree_rate_online->GetEntries(); i++)
			{
				tree_rate_online->GetEntry(i);
			//	if (ts > 0 && ts < 1864000000)
			//	{
			///		std::cout << ts << std::setprecision(12) << std::endl;
					for (int ch=0;ch<8;ch++){
						graph_channels.at(ch)->SetPoint(point+i_point,ts,rate[ch]);
					}
					point++;
			//	}
			}
			i_point += tree_rate_online->GetEntries();
				std::cout << point << std::endl;
			// for(unsigned int i =0; i < tree_rate_online_inj->GetEntries(); i++)
			// {
			// 	tree_rate_online_inj->GetEntry(i);
			//
			// 	for (int ch=0;ch<8;ch++){
			// 		graph_channels.at(ch)->SetPoint(i+i_point,ts,rate[ch]);
			// 	}
			// }
			//i_point += tree_rate_online_inj->GetEntries();

			f.Close();
		}

	}



    cout << "Drawing now!" << endl;
    TCanvas * c1 = new TCanvas("fwd1", "fwd1", 1200, 600);

	// graph_channels.at(0)->GetXaxis()->SetRangeUser(1464000289.68, 1464002895.31);
	graph_channels.at(0)->GetYaxis()->SetRangeUser(0, 400000);

	graph_channels.at(0)->Draw("AP");
	for (int ch=1;ch<8;ch++){
		graph_channels.at(ch)->Draw("P");
	}
	c1->SaveAs("OnInj.pdf");
	c1->SaveAs("OnInj.png");
	c1->SaveAs("OnInj.eps");


	app->Run();
	return 0;

}
