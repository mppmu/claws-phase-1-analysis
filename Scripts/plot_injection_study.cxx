// cpp includes
#include <iostream>
#include <vector>
#include <string>
// root includes
#include <TFile.h>
#include "TH1D.h"
#include "TH1F.h"
#include <TRandom3.h>
#include <TF1.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TStyle.h>

#include "scripts.hh"


using namespace std;

//int plot_waveform()
int plot_injection_study(int argc, const char *argv[])
{
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);

    gStyle->SetPadTopMargin(0.01);
    gStyle->SetPadRightMargin(0.03);
    gStyle->SetPadBottomMargin(0.10);
    gStyle->SetPadLeftMargin(0.075);
    gStyle->SetPadGridX(false);
    gStyle->SetPadGridY(0);

    gStyle->SetPadTickY(0);
    gStyle->SetPadTickX(0);
    gStyle->SetPadBorderMode(2);

    // Declare some constants:
    int rebin = 100;
    int font = 63;
    //    double textsize = 0.03;
    double textsize = 20;
    // gStyle->SetLabelFont(font);
    // gStyle->SetLabelSize(textsize);

//    std::string fname = std::string(argv[1]) + "/" + std::string(argv[2]);
std::string fname = "/remote/ceph/group/ilc/claws/data/Analysis/Run_401134/run_401134";
    TFile* file = new TFile( (fname+".root").c_str() , "open");

    if( file->IsZombie() )
    {
        cout << "Error openning file" << endl;
        exit(-1);
    }

    TH1F * fwd1 = (TH1F*) file->Get( "FWD1" );
    std::cout << "First: " << fwd1->Integral() << std::endl;
    fwd1->Rebin(rebin);
    std::cout << "Second: " << fwd1->Integral() << std::endl;
    TH1F * fwd2 = (TH1F*) file->Get( "FWD2" );
    fwd2->Rebin(rebin);
    TH1F * fwd3 = (TH1F*) file->Get( "FWD3" );
    fwd3->Rebin(rebin);


    fwd1->GetXaxis()->SetTitle("Time [#mys]");
    fwd1->GetXaxis()->CenterTitle();
    //    fwd1->GetYaxis()->SetLabelOffset(0.14);
    // fwd1->GetXaxis()->SetTitleFont(font);
    // fwd1->GetXaxis()->SetTitleSize(textsize);
    // fwd1->GetXaxis()->SetTitleOffset(1.2);

    fwd1->SetFillColor(kRed+1);
// hist->SetFillStyle(3002);
    fwd1->SetLineColor(kRed+1);

    fwd1->GetYaxis()->SetTitle("Charged particle rate [MIP/80ns]");
    fwd1->GetYaxis()->CenterTitle();
    // fwd1->GetYaxis()->SetTitleFont(font);
    // fwd1->GetYaxis()->SetTitleSize(textsize);
    // fwd1->GetYaxis()->SetLabelFont(font);
    // fwd1->GetYaxis()->SetLabelSize(textsize);
    // fwd1->GetYaxis()->SetTitleOffset(0.7);


    fwd2->SetFillColor(kViolet-6);
// hist->SetFillStyle(3002);
    fwd2->SetLineColor(kViolet-6);

    fwd3->SetFillColor(kAzure-5);
// hist->SetFillStyle(3002);
    fwd3->SetLineColor(kAzure-5);

    TH1F* fwd2to1 = (TH1F*)fwd2->Clone("fwd2to1");
    fwd2to1->Divide(fwd1);



    TCanvas * c1 = new TCanvas("c", "c", 1000, 1000);
    c1->Divide(1,2);
    //
    c1->cd(1);
    fwd1->Draw();
    fwd2->Draw("same");
    fwd3->Draw("same");


    c1->cd(2);
    fwd2to1->Draw();

    std::vector<std::string> formats = {".jpg",".eps",".png",".pdf"};

    for(auto& format : formats)
    {
        c1->SaveAs( (fname+format).c_str() );
    }

//    file->Close();

    return 0;
}
