#include <iostream>
#include <cmath>

#include "Rtypes.h"

#include "Belle2Utils.h"
#include "Belle2Style.h"
#include "Belle2Labels.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TGraphErrors.h"

using namespace std;

const int nsys=3;

void Belle2Example() 
{ 

#ifdef __CINT__
  gROOT->LoadMacro("Belle2Utils.C");
  gROOT->LoadMacro("Belle2Labels.C");
#endif

  SetBelle2Style();

  Int_t icol1=5;
  Int_t icol2=7;

  TCanvas* c1 = new TCanvas("c1","inclusive production of pions at Y(4S)",50,50,600,600);
  TPad* thePad = (TPad*)c1->cd();

  Double_t ymin=1.e-3;  Double_t ymax=14;
  Double_t xmin=0.00;  Double_t xmax=2.0;
  TH1F *h1 = thePad->DrawFrame(xmin,ymin,xmax,ymax);
  h1->SetYTitle("dBF/dM_{X} [BF/0.1 GeV]");
  h1->SetXTitle("M_{X}  [GeV]");
  h1->GetYaxis()->SetTitleOffset(1.4);
  h1->GetXaxis()->SetTitleOffset(1.4);
  //h1->GetXaxis()->SetNdivisions(5);
  h1->Draw();

  TGraphErrors *data;

  //Make up some MC and systematics variations
  TGraphErrors *g1[nsys];
  const Int_t nbins=20;
  const Int_t nmodels=3;
  for (Int_t isys=0; isys<nsys; isys++) { // loop over systematics
    Double_t x[nbins],y[nbins],ex[nbins],ey[nbins];
    Double_t sysvar=0.;
    if(isys==1)sysvar=-1.;
    if(isys==2)sysvar=+1.;
    for(Int_t i=0; i< nbins; i++){
      x[i]=i*0.1+0.05;
      y[i]=10*sin(x[i]/2+0.05*(Double_t)isys*sysvar);
      ex[i]=0.05;
      ey[i]=0.01;
    }
    g1[isys]=new TGraphErrors(nbins,x,y,ex,ey);
    g1[isys]->SetMarkerStyle(0);
    g1[isys]->SetLineColor(1);
  }

  //Example to add a second set of errors, e.g. from modelling errors
  //Again making up a few distributions that are variations on the nominal MC shape
  TGraphErrors *g2[nmodels];
  const Int_t nbins=20;
  for (Int_t imodels=0; imodels<nmodels; imodels++) { // loop over systematics
    Double_t x[nbins],y[nbins],ex[nbins],ey[nbins];
    Double_t sysvar=0.;
    if(imodels==1)sysvar=-1.;
    if(imodels==2)sysvar=+1.;
    for(Int_t i=0; i< nbins; i++){
      x[i]=i*0.1+0.05;
      y[i]=10*sin(x[i]/2+0.05*(Double_t)imodels*sysvar);
      ex[i]=0.05;
      ey[i]=0.01;
    }
    g2[imodels]=new TGraphErrors(nbins,x,y,ex,ey);
    g2[imodels]->SetMarkerStyle(0);
    g2[imodels]->SetLineColor(1);
  }



  char daname[100];
  sprintf(daname,"data"); 
  data=(TGraphErrors*)g1[0]->Clone(daname); 
  data->SetMarkerStyle(20);
  data->SetMarkerColor(1);
  
  // // just invent some data
  for (Int_t i=0; i< data->GetN(); i++) {
    Double_t x1,y1,e,dx1=0.;
    data->GetPoint(i,x1,y1);
    Double_t r1 = 0.4*(gRandom->Rndm(1)+2);
    Double_t r2 = 0.4*(gRandom->Rndm(1)+2);
    Double_t ydata;
    if (i==0) ydata=r1*y1+r1*r2*r2*x1/50000.;
    else      ydata=r1*y1;
    e=sqrt((Double_t)ydata*1000)/200;
    data->SetPoint(i,x1,ydata);
    data->SetPointError(i,dx1,e);
  }
  TGraphAsymmErrors* errorband1;
  TGraphAsymmErrors* errorband2;
  
  errorband1=  myMakeBand(g1[0],g1[1],g1[2]);
  errorband2=(TGraphAsymmErrors* ) errorband1->Clone("scalepdf");
  //Add model errors to the plot
  for (Int_t imodel=0; imodel<nmodels; imodel++) {
    myAddtoBand(g2[imodel],errorband2); 
  }

  errorband2->SetFillColor(icol1);
  errorband2->Draw("zE2");
  errorband1->SetFillColor(icol1);
  errorband1->Draw("zE2");
  g1[0]->SetLineWidth(3);
  g1[0]->SetLineStyle(1);
  g1[0]->Draw("z");
  data->Draw("P");
  
  myText(       0.3,  0.85, 1, "Y(3S)");
  myText(       0.57, 0.85, 1, "#int L = 30 fb^{-1}");
  myMarkerText( 0.55, 0.75, 1, 20, "Data 2015",1.3);
  myBoxText(    0.55, 0.67, 0.05, icol1, "MC sys error");
  
  BELLE2Label(0.5,0.2,"Example");
  
  c1->Print("Belle2Example.eps");
  c1->Print("Belle2Example.png");
  c1->Print("Belle2Example.pdf");
  
}


#ifndef __CINT__

int main()  { 
  
  Belle2Example();

  return 0;
}

#endif
