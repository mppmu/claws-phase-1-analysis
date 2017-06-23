//
//   @file    Belle2Utils.h
//
//


#ifndef __BELLE2UTILS_H
#define __BELLE2UTILS_H

#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"

void BELLE2_LABEL(Double_t x, Double_t y, Color_t color = 1);

TGraphErrors* myTGraphErrorsDivide(TGraphErrors* g1, TGraphErrors* g2);

TGraphAsymmErrors* myTGraphErrorsDivide(TGraphAsymmErrors* g1, TGraphAsymmErrors* g2);

TGraphAsymmErrors* myMakeBand(TGraphErrors* g0, TGraphErrors* g1, TGraphErrors* g2);

void myAddtoBand(TGraphErrors* g1, TGraphAsymmErrors* g2);

TGraphErrors* TH1TOTGraph(TH1* h1);

void myText(Double_t x, Double_t y, Color_t color, const char* text);

void myBoxText(Double_t x, Double_t y, Double_t boxsize, Int_t mcolor, const char* text);

void myMarkerText(Double_t x, Double_t y, Int_t color, Int_t mstyle, const char* text, Float_t msize = 2.);

#endif // __BELLE2UTILS_H
