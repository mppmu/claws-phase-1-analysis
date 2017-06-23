#include <iostream>
#include <string>
#include <fstream>

#include <TCanvas.h>
#include <TH1D.h>
#include <TStyle.h>
#include <TString.h>
#include <TROOT.h>
#include <TLegend.h>

#include "Belle2Style.h"
using namespace std;

//void mip_waveform(const char* inFile){
void mip_waveform(){

    gROOT->LoadMacro("Belle2Style.C");
    SetBelle2Style();
    gStyle->SetOptStat(0);


//	INPUT <--------------------------------
//
	string RPath = "";
	string RName = RPath + "Hist_MIP-response_10074_";
	string WfName = RName + "Waveform-no-0";
    string PDFName = WfName + ".pdf";
    string CName = WfName + ".C";

	ifstream in("./MIP-data_10074-wf-0.dat");
	double inVal;
//	int noBins = 300;
	int noBins = 300;
	Double_t stepsize = 0.8E-9;
	int counter = 0;

//	TGraph* gr = new TGraph();
//	myplotStyle(gr);

	TH1F* hist = new TH1F("hist", "", noBins, 0, noBins*0.8);	

	hist->SetXTitle("Time [ns]");
	hist->SetYTitle("Amplitude [mV]");
	hist->SetLineColor(1);
	hist->SetNdivisions(510);
//	hist->GetYaxis()->SetRangeUser(-6,1.5);
//	c1->SetLogy();
//
//
//	{
//		double dummy1;
//		double dummy2;
//		double time;
//		double ChA;
//		double ChB;
//		double ChC;
//		double ChD;
//		do{
//			if(counter==0){
//				in >> dummy1;
//				in >> dummy2;
//			}
//			counter++;
//			in >> time >> ChA >> ChB >> ChC >> ChD;
//			Double_t filltime = (((Double_t)counter - 0.5) * stepsize*1E9); // xAxis in ns
////			gr->SetPoint(counter, time, ChB);
////			gr->
//			hist->Fill(filltime, ChB);
//			hist->SetBinError(counter-1, 0);
//		} while(!in.eof());
//	}
	{
		do{
			counter++;
			in >> inVal;
			Double_t filltime = (((Double_t)counter - 0.5) * stepsize*1E9); // xAxis in ns
			hist->Fill(filltime,-inVal);
			hist->SetBinError(counter-1,0);
		}while(!in.eof());
	}
	cout << counter << endl;

//	Draw Sensitivities in high res.
//	    	
	TCanvas *c1 = new TCanvas("PlotData", WfName.c_str(), 800, 600);

	TLegend *leg = new TLegend(.2,.82,.43,.87);

	leg->AddEntry(hist, "MIP Waveform", "L");
//	leg->AddEntry(gr, "1pe Waveform", "L");

	hist->Draw("hist");
//	gr->Draw("APL");
	leg->Draw("same");
	c1->SaveAs(CName.c_str());
	c1->SaveAs(PDFName.c_str());
}
