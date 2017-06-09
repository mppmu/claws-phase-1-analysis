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
#include <TLegend.h>

#include "scripts.hh"


using namespace std;


//int plot_waveform(int argc, const char *argv[])
int plot_waveform()
{
    int rebin = 100;
    double binsize = 0.8;
    double timescaling = 0.001;
    // gStyle->SetLabelFont(font);
    // gStyle->SetLabelSize(textsize);

    // -------------------------------
    // Get the histograms from file
    // -------------------------------

//    std::string fname = std::string(argv[1]) + "/" + std::string(argv[2]);
    std::string fname = "/remote/ceph/group/ilc/claws/data/Analysis/Run_401131/run_401131";

    TFile* file = new TFile( (fname+".root").c_str() , "open");

    if( file->IsZombie() )
    {
        cout << "Error openning file" << endl;
        exit(-1);
    }



    TH1F * fwd1 = (TH1F*) file->Get( "FWD1" );
    fwd1->SetBins(fwd1->GetNbinsX(),0.5*binsize, (fwd1->GetNbinsX()+0.5)*binsize*timescaling );
    // std::cout << "First: " << fwd1->Integral() << std::endl;
    fwd1->Rebin(rebin);
    // std::cout << "Second: " << fwd1->Integral() << std::endl;
    TH1F * fwd2 = (TH1F*) file->Get( "FWD2" );
    fwd2->SetBins(fwd2->GetNbinsX(),0.5*binsize, (fwd2->GetNbinsX()+0.5)*binsize*timescaling );
    fwd2->Rebin(rebin);
    TH1F * fwd3 = (TH1F*) file->Get( "FWD3" );
    fwd3->SetBins(fwd3->GetNbinsX(),0.5*binsize, (fwd3->GetNbinsX()+0.5)*binsize*timescaling );
    fwd3->Rebin(rebin);

    // ---------------------------------------
    // Get the reference histograms from file
    // ---------------------------------------

    std::string fname_ref = "/remote/ceph/group/ilc/claws/data/Analysis/Run_401145/run_401145";
    TFile* file_ref = new TFile( (fname_ref+".root").c_str() , "open");

    if( file_ref->IsZombie() )
    {
        cout << "Error openning file" << endl;
        exit(-1);
    }

    TH1F * fwd1_ref = (TH1F*) file_ref->Get( "FWD1" );
    fwd1_ref->SetBins(fwd1_ref->GetNbinsX(),0.5*binsize, (fwd1_ref->GetNbinsX()+0.5)*binsize*timescaling );
    fwd1_ref->Rebin(rebin);

    double noplot = 1;
    for(int i = 1 ; i< fwd1_ref->GetNbinsX(); i++)
    {
        if(fwd1_ref->GetBinContent(i) <= noplot) fwd1_ref->SetBinContent(i, 0);
    }


    // ---------------------------------------
    // Get the ratio histograms from file
    // ---------------------------------------

    TH1F* fwd21 = (TH1F*)fwd2->Clone("fwd21");

    for(int i = 1 ; i< fwd21->GetNbinsX(); i++)
    {
        if(fwd2->GetBinContent(i) < 0.5 && fwd1->GetBinContent(i) < 0.5) fwd21->SetBinContent(i, 0);
        else fwd21->SetBinContent(i, fwd2->GetBinContent(i)*100./fwd1->GetBinContent(i));
    }

    TH1F* fwd31 = (TH1F*)fwd3->Clone("fwd31");

    for(int i = 1 ; i< fwd31->GetNbinsX(); i++)
    {
        if(fwd3->GetBinContent(i) < 0.5 && fwd3->GetBinContent(i) < 0.5) fwd31->SetBinContent(i, 0);
        else fwd31->SetBinContent(i, fwd3->GetBinContent(i)*100./fwd1->GetBinContent(i));
    }



    // ---------------------------------------
    // Global plotting shit
    // ---------------------------------------

    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    // gStyle->SetPadRightMargin(0.03);
    // gStyle->SetPadTopMargin(0.01);
    // //
    // gStyle->SetPadBottomMargin(0.20);
    // gStyle->SetPadLeftMargin(0.075);
    // gStyle->SetPadGridX(false);
    // gStyle->SetPadGridY(0);
    //
    // gStyle->SetPadTickY(0);
    // gStyle->SetPadTickX(0);
    // gStyle->SetPadBorderMode(2);

    // Declare some constants:

    int font = 63;
    //    double textsize = 0.03;
    double textsize = 0.035;
    double labelsize = 0.035;
    double scaling = 0.78/0.28;

    // ---------------------------------------
    // make the axis
    // ---------------------------------------

    double rangex = 1000;
    TH1F* axis = (TH1F*) fwd1->Clone("axis");

    axis->GetXaxis()->SetTitle("");
    axis->GetXaxis()->CenterTitle();
    // axis->GetXaxis()->SetTitleSize(textsize);
    //  axis->GetXaxis()->SetTitleFont(font);

//    axis->GetXaxis()->SetLabelOffset(999);
    axis->GetXaxis()->SetLabelSize(0);

    axis->GetYaxis()->SetTitle("Charged particle rate [MIP/80ns]");
    axis->GetYaxis()->CenterTitle();
    axis->GetYaxis()->SetTitleOffset(1.05);

    axis->GetYaxis()->SetLabelSize(labelsize);
    //axis->GetYaxis()->SetTitleSize(textsize);
    axis->GetXaxis()->SetRangeUser(0,rangex);
    TH1F* axis2 = (TH1F*) axis->Clone("axis2");


    axis2->GetXaxis()->SetTitle("Time [#mus]");
    axis2->GetXaxis()->SetTitleSize(textsize*scaling);
    axis2->GetXaxis()->SetTitleOffset(1.0);

    axis2->GetXaxis()->SetLabelSize(labelsize*scaling);
    axis2->GetXaxis()->SetLabelOffset(0.005);

    axis2->GetYaxis()->SetTitle("Deviation [%]");
    axis2->GetYaxis()->SetTitleSize(textsize*scaling);
    axis2->GetYaxis()->SetTitleOffset(0.4);

    axis2->GetYaxis()->SetLabelSize(labelsize*scaling);

    axis2->GetYaxis()->SetRangeUser(-10,110);

    TH1F* axis3 = (TH1F*) axis->Clone("axis2");

    axis3->GetXaxis()->SetTitle("Time [#mus]");
    axis3->GetXaxis()->SetTitleSize(textsize);
    axis3->GetXaxis()->SetTitleOffset(1.0);

    axis3->GetXaxis()->SetLabelSize(labelsize);
    axis3->GetXaxis()->SetRangeUser(220,550);
    // ---------------------------------------
    // make the hists nice
    // ---------------------------------------
    int linewidth = 3;
    fwd1->SetFillColor(kRed+1);
// hist->SetFillStyle(3002);
    fwd1->SetLineColor(kRed+1);
    fwd1->SetLineWidth(linewidth);

    fwd2->SetFillColor(kViolet-6);
    fwd2->SetMarkerColor(kViolet-3);
    fwd2->SetLineColor(kViolet-6);
    fwd2->SetLineWidth(linewidth);
    fwd2->SetMarkerStyle(21);
    fwd2->SetMarkerSize(1.5);

    fwd3->SetFillColor(kAzure-5);
// hist->SetFillStyle(3002);
    fwd3->SetLineColor(kAzure-5);
    fwd3->SetLineWidth(linewidth);


    double markersize = 0.9;
    fwd1_ref->SetMarkerStyle(21);
    fwd1_ref->SetMarkerSize(markersize);

    fwd21->SetMarkerStyle(22);
    fwd21->SetMarkerSize(markersize);
    fwd21->SetMarkerColor(kViolet-6);

    fwd31->SetMarkerStyle(23);
    fwd31->SetMarkerSize(markersize);
    fwd31->SetMarkerColor(kAzure-5);
    //    fwd1->GetYaxis()->SetLabelOffset(0.14);
    // fwd1->GetXaxis()->SetTitleFont(font);
    // fwd1->GetXaxis()->SetTitleSize(textsize);
    // fwd1->GetXaxis()->SetTitleOffset(1.2);




    // fwd1->GetYaxis()->SetTitleFont(font);
    // fwd1->GetYaxis()->SetTitleSize(textsize);
    // fwd1->GetYaxis()->SetLabelFont(font);
    // fwd1->GetYaxis()->SetLabelSize(textsize);
    // fwd1->GetYaxis()->SetTitleOffset(0.7);

    // ---------------------------------------
    // Legend
    // ---------------------------------------

    TLegend * ltop = new TLegend(0.7, 0.72, 0.9, 0.87);
    // ltop->SetTextFont(72)9
    ltop->SetTextSize(textsize);
    ltop->SetBorderSize(0);
    ltop->SetFillColor(0);
    ltop->AddEntry(fwd1, "FWD1", "F");
    ltop->AddEntry(fwd2, "FWD2", "F");
    ltop->AddEntry(fwd3, "FWD3", "F");
    ltop->AddEntry(fwd1_ref, "FWD1 (refernce run)", "p");
   //    TPaveText * p = new TPaveText(0.12,0.45,0.3,0.55, "NDC");
   //    p->SetTextAlign(11);
   //    p->SetTextFont(62);
   //    p->SetFillColor(10);
   //    p->SetLineColor(10);
   //    p->SetBorderSize(1);
   //    p->SetTextSize(0.05);
   //    p->AddText("CALICE");
   //    p->AddText("work in progress");
   //    p->SetTextColor(kGray+1);
   //    p->Draw();












    TCanvas *c = new TCanvas("Energy", "Graph", 0, 0, 900, 900);
    c->cd();

    TPad * pad1 = new TPad("pad1", "The pad 80% of the height", 0.0, 0.22, 1.0, 1.0, kWhite);
    TPad * pad2 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.0, 1.0, 0.28, kWhite);
    pad1->Draw();
    pad2->Draw();
    double pad1W = pad1->GetWw() * pad1->GetAbsWNDC();
    double pad1H = pad1->GetWh() * pad1->GetAbsHNDC();
    double pad2W = pad2->GetWw() * pad2->GetAbsWNDC();
    double pad2H = pad2->GetWh() * pad2->GetAbsHNDC();
    //----------------pad1-------------------------------------
    pad1->cd();
    gPad->SetTicks();
    pad1->SetFillStyle(4000);
    pad1->SetFrameFillStyle(4000);

    pad1->SetRightMargin(0.03);
    pad1->SetTopMargin(0.01);

    axis->Draw("AXIS");
    fwd1->Draw("same");
    fwd2->Draw("same");
    fwd3->Draw("same");
    fwd1_ref->Draw("sameP");

    // -----------Pad3  -----------------------------------------------
    //ltop->Draw();
    TPad * pad3 = new TPad("pad3", "The pad 20% of the height", 0.45, 0.45, 0.95, 0.95, kWhite);
    pad3->Draw();
    pad3->cd();
    pad3->SetRightMargin(0.03);
    pad3->SetTopMargin(0.01);


    axis3->Draw("AXIS");
    fwd1->Draw("same");
    fwd2->Draw("same");
    fwd3->Draw("same");
    fwd1_ref->Draw("same");

    // -----------Pad2-----------------------------------------------
    pad2->cd();
    //pad2->SetBottomMargin(0.24);
    gPad->SetTicks();

    pad2->SetFillStyle(4000);
    pad2->SetFrameFillStyle(4000);

    pad2->SetRightMargin(0.03);
    pad2->SetTopMargin(0.01);
    pad2->SetBottomMargin(0.24);



    axis2->Draw("AXIS");
    fwd21->Draw("sameP");
    fwd31->Draw("sameP");
//    axis2->Draw("AXIS");



//    fwd2to1->Draw();

    std::vector<std::string> formats = {".jpg",".eps",".png",".pdf"};

    for(auto& format : formats)
    {
        c->SaveAs( (fname+format).c_str() );
    }

//    file->Close();

    return 0;
}
