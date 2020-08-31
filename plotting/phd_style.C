#include <iostream>

#include "phd_style.h"

#include "TROOT.h"

void SetPhDStyle()
{
  static TStyle* style = 0;
  std::cout << "\nSetting up MyPhDStyle!\n" << std::endl ;
  if ( style == 0 ) style = PhDStyle();
  gROOT->SetStyle("PhD");
  gROOT->ForceStyle();
}

TStyle* PhDStyle()
{
  TStyle *style = new TStyle("PhD","Style for my phd thesis");

  // --- Paper size ---
  //atlasStyle->SetPaperSize(20,26);

  // use plain black on white colors
  Int_t icol=0; // WHITE
  style->SetFrameBorderMode(icol);
  style->SetFrameFillColor(icol);
  style->SetCanvasBorderMode(icol);
  style->SetCanvasColor(icol);
  style->SetPadBorderMode(icol);
  style->SetPadColor(icol);
  style->SetStatColor(icol);

  // --- Pad margins ---
  style->SetPadTopMargin(0.03);
  style->SetPadRightMargin(0.025);
  style->SetPadBottomMargin(0.09);
  style->SetPadLeftMargin(0.12);

  // --- Have axis and ticks on top and right
  style->SetPadTickX(1);
  style->SetPadTickY(1);

  // get rid of the stats box
  style->SetOptStat(0);
  style->SetOptTitle(0);
  style->SetOptDate(0);

  // --- Fonts and label sizes ---

  //Int_t font=72; // Helvetica italics
  Int_t font=43; // Helvetica

  // 1 pt = 1.333 pixels
  Double_t tsize=vis_multi*textsize*pt2pixel;

  style->SetTextFont(font);

  style->SetLabelFont(font,"xyz");
  style->SetTitleFont(font,"xyz");

  //style->SetLegendFont(font+10);
  style->SetLegendFont(font+10);
  style->SetTextSize(tsize);

  style->SetLabelSize(tsize,"xyz");
  style->SetTitleSize(tsize,"xyz");

  style->SetLegendTextSize(tsize*0.8);
 // cout<< "LegendTextSize: " << tsize*0.8 << endl;
  style->SetTitleXOffset(1.2);
  style->SetTitleYOffset(1.6);

  // --- Standard hist and graph options ---
  style->SetMarkerStyle(20);
  style->SetMarkerSize(1.2);
  style->SetHistLineWidth(2.);
 // style->SetLineStyleString(2,"[12 12]"); // postscript dashes

  // get rid of X error bars
  //style->SetErrorX(0.001);
  // get rid of error bar caps
  style->SetEndErrorSize(0);

  return style;
}
