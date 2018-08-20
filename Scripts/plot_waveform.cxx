
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <TFile.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TStyle.h>
#include <TString.h>
#include <TROOT.h>
#include <TLegend.h>
#include "Rtypes.h"
#include "THStack.h"

#include "Belle2Utils.C"
#include "Belle2Style.C"
#include "Belle2Labels.C"
//
// #include "Belle2Utils.h"
// #include "Belle2Style.h"
// #include "Belle2Labels.h"
// void rootlogon()
// {
//     SetBelle2Style();
// }

using namespace std;



void plot_waveform(){

		// #ifdef __CINT__
		//     gROOT->LoadMacro("Belle2Style.C");
		//     gROOT->LoadMacro("Belle2Utils.C");
		//     gROOT->LoadMacro("Belle2Labels.C");
		// #endif
		// gROOT->LoadMacro("Belle2Style.C");
		SetBelle2Style();
		gStyle->SetOptStat(0);

		// Int_t ci = TColor::GetFreeColorIndex();
		TColor *red = new TColor(0.84, 0.15, 0.16);
		TColor *red2 = new TColor(213./255, 61./255, 53./255);
		TColor *orange = new TColor(255./255, 127./255, 14./255 );
		TColor *blue = new TColor(0.12, 0.47, 0.71);
		TColor *lblue = new TColor(0.68, 0.78, 0.91);
		TColor *green = new TColor(0.17, 0.63, 0.17);
		TColor *gray = new TColor(165./255, 172./255,175./255 );
		TColor *antra = new TColor(34./255, 40./255,46./255 );

		int rebin = 100;
		double binsize = 0.8;
		double timescaling = 0.001;
		int triggershift = 134310;
		// -------------------------------
		// Get the histograms from file
		// -------------------------------

		//    std::string fname = std::string(argv[1]) + "/" + std::string(argv[2]);
		// std::string fname = "/remote/ceph/group/ilc/claws/data/Analysis/Run_401131/run_401131";
		std::string fname = "/remote/ceph/group/ilc/claws/data/Analysis/Run_401141/run_401141";

		TFile* file = new TFile( (fname+".root").c_str(), "open");

		if( file->IsZombie() )
		{
				cout << "Error openning file" << endl;
				exit(-1);
		}



		TH1F * fwd1 = (TH1F*) file->Get( "FWD1" );
		fwd1->SetBins(fwd1->GetNbinsX(),(0.5- triggershift)*binsize*timescaling, (fwd1->GetNbinsX()+0.5 - triggershift )*binsize*timescaling );
		// std::cout << "First: " << fwd1->Integral() << std::endl;
		fwd1->Rebin(rebin);

		//     if(fwd1->GetBinContent(i) <= noplot) fwd1->SetBinContent(i, 0);
		// }
		// std::cout << "Second: " << fwd1->Integral() << std::endl;
		TH1F * fwd2 = (TH1F*) file->Get( "FWD2" );
		fwd2->SetBins(fwd2->GetNbinsX(),(0.5- triggershift)*binsize*timescaling, (fwd2->GetNbinsX()+0.5 - triggershift)*binsize*timescaling );
		fwd2->Rebin(rebin);
		// for(int i = 1 ; i< fwd2->GetNbinsX(); i++)
		// {
		//     if(fwd2->GetBinContent(i) <= noplot) fwd2->SetBinContent(i, 0);
		// }
		TH1F * fwd3 = (TH1F*) file->Get( "FWD3" );
		fwd3->SetBins(fwd3->GetNbinsX(), (0.5 - triggershift)*binsize*timescaling, (fwd3->GetNbinsX()+0.5 - triggershift)*binsize*timescaling );
		fwd3->Rebin(rebin);

		// for(int i = 1 ; i< fwd3->GetNbinsX(); i++)
		// {
		//     if(fwd3->GetBinContent(i) <= noplot) fwd3->SetBinContent(i, 0);
		// }

		// ---------------------------------------
		// make the hists nice
		// ---------------------------------------

		int linewidth = 3;
		fwd1->SetFillColor(green->GetNumber());
		//    fwd1->SetLineColorAlpha(green->GetNumber(), 0.2);
		fwd1->SetLineColor(red->GetNumber());
		fwd1->SetLineWidth(linewidth);

		fwd2->SetFillColor(red->GetNumber());
		// fwd2->SetMarkerColor(kViolet-3);
		//fwd2->SetLineColorAlpha(red->GetNumber(), 0.2);
		fwd2->SetLineColor(gray->GetNumber());
		fwd2->SetLineWidth(linewidth);
		// fwd2->SetMarkerStyle(21);
		// fwd2->SetMarkerSize(1.5);

		fwd3->SetFillColor(blue->GetNumber());
		// hist->SetFillStyle(3002);
		// fwd3->SetLineColorAlpha(blue->GetNumber(), 0.5);
		//    fwd3->SetLineColor(antra->GetNumber());
		fwd3->SetLineColor(antra->GetNumber());
		fwd3->SetLineWidth(linewidth);


		double markersize = 0.6;
		// double
		fwd1->SetMarkerStyle(20);
		fwd1->SetMarkerSize(markersize);

		fwd2->SetMarkerStyle(20);
		fwd2->SetMarkerSize(markersize);

		fwd3->SetMarkerStyle(20);
		fwd3->SetMarkerSize(markersize);



		// -----------------------------------
		// Make the canvas and plot the stack
		// -----------------------------------

		TCanvas* c1 = new TCanvas("c1","inclusive production of pions at Y(4S)",50,50,600,600);
		TPad* thePad = (TPad*)c1->cd();

		THStack *stack = new THStack("stack","");
		stack->Add(fwd1);
		stack->Add(fwd2);
		stack->Add(fwd3);

		// // stack->GetXaxis()->SetRangeUser(0,1000);
		// stack->SetMinimum(0);
		// stack->SetMaximum(250);

		stack->Draw();

		stack->GetYaxis()->SetTitle("Charged particle rate [MIP/80ns]");
		stack->GetXaxis()->SetTitle("Time [#mus]");

		stack->GetXaxis()->SetRangeUser(-50,2500);

		// For runs 401141 and 401140 [0,14]
		// For runs 401145 and 401131 [0,250]

		stack->SetMinimum(0);
		stack->SetMaximum(250);

		c1->Modified();


		double boxsize = 0.04;
		double height = 0.885;
		myBoxText(    0.465, height, boxsize, red->GetNumber(), "FWD1");
		myBoxText(    0.635, height, boxsize, gray->GetNumber(), "FWD2");
		myBoxText(    0.805, height, boxsize, antra->GetNumber(), "FWD3");


		double scalingfactor = 1/(0.94-0.4)*0.75;

		TPad * pad2 = new TPad("pad2", "The pad 20% of the height", 0.4, 0.3, 0.94, 0.84, kWhite);

		pad2->Draw();
		pad2->cd();
		pad2->SetRightMargin(0.06);
		pad2->SetTopMargin(0.01);

		THStack* stack_2 = (THStack*) stack->Clone();

		// For runs 401141  [-5,145] HER Ref
		// For runs 401131  [115,315] LER Phase
		// For runs 401140 [-5,145]   HER Phase
		// For runs 401145 [115,315]  LER Ref
		stack_2->GetXaxis()->SetRangeUser(115,315);
		stack_2->GetXaxis()->SetNdivisions(505);

		// For runs 401145 and 401131 [0,35]
		// For runs 401141 and 401140 [0,2.6]
		stack_2->SetMinimum(0);

		stack_2->SetMaximum(35);

		stack_2->Draw();

		stack_2->GetXaxis()->SetTitleSize(stack_2->GetXaxis()->GetTitleSize()*scalingfactor);
		stack_2->GetYaxis()->SetTitleSize(stack_2->GetYaxis()->GetTitleSize()*scalingfactor);

		stack_2->GetXaxis()->SetTitleOffset(stack_2->GetXaxis()->GetTitleOffset()/scalingfactor);
		stack_2->GetYaxis()->SetTitleOffset(stack_2->GetYaxis()->GetTitleOffset()*1.12/scalingfactor);

		stack_2->GetXaxis()->SetLabelSize(stack_2->GetXaxis()->GetLabelSize()*scalingfactor);
		stack_2->GetYaxis()->SetLabelSize(stack_2->GetYaxis()->GetLabelSize()*scalingfactor);


		pad2->Modified();


		// double boxsize = 0.04;
		//
		//  myBoxText(    0.44, 0.25, boxsize, green->GetNumber(), "FWD1");
		//  myBoxText(    0.63, 0.25, boxsize, red->GetNumber(), "FWD2");
		//  myBoxText(    0.82, 0.25, boxsize, blue->GetNumber(), "FWD3");

		//  myBoxText(    0.7, 0.74, boxsize, green->GetNumber(), "FWD1");
		//  myBoxText(    0.7, 0.69, boxsize, red->GetNumber(), "FWD2");
		//  myBoxText(    0.7, 0.64, boxsize, blue->GetNumber(), "FWD3");



		std::vector<std::string> formats = {".jpg",".eps",".png",".pdf"};

		for(auto& format : formats)
		{
				//    c1->SaveAs( (fname + format).c_str() );
				c1->SaveAs( ("InjectionMIPFinder_FWD_401141_new" + format).c_str() );
		}

		// -----------------------------------
		// Get the integrals:
		// -----------------------------------

		ofstream myfile;
		myfile.open("401141_integrals.txt");

		int limit = 1000;
		myfile << "Time[us]: " << limit << std::endl;
		myfile << "Percentage FWD1: " << fwd1->Integral(1,(int)(limit/0.080))/fwd1->Integral() << std::endl;
		myfile << "Percentage FWD2: " << fwd2->Integral(1,(int)(limit/0.080))/fwd2->Integral() << std::endl;
		myfile << "Percentage FWD3: " << fwd3->Integral(1,(int)(limit/0.080))/fwd3->Integral() << std::endl;

		limit = 1500;
		myfile << "Time[us]: " << limit << std::endl;
		myfile << "Percentage FWD1: " << fwd1->Integral(1,(int)(limit/0.080))/fwd1->Integral() << std::endl;
		myfile << "Percentage FWD2: " << fwd2->Integral(1,(int)(limit/0.080))/fwd2->Integral() << std::endl;
		myfile << "Percentage FWD3: " << fwd3->Integral(1,(int)(limit/0.080))/fwd3->Integral() << std::endl;

		myfile.close();

}
