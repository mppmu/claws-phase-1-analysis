// #include <boost/filesystem.hpp>
#include <TFile.h>


void plt_placeholders()
{
    // General stuff
    SetPhDStyle();


    // --- Canvas and Pads

    int width = round(vis_multi*textwidth*pt2pixel);
    // double height = 900;
    // double ratio = 16./9.;
    double ratio = 2./1.;
    TCanvas * can = new TCanvas("can", "can", width, width/ratio);

    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLeftMargin(0.08);

    // Do the axis and the range shit -------------------------------------------------------------------------------------------------------------------------

    double xlow = 0;
    double xup = 2.4;

    double ylow = 0;
    double yup = 2.4;

    TH1F* axis = new TH1F("axis","axis",100,xlow, xup);

    axis->GetYaxis()->SetRangeUser(ylow, yup);
    //axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ms]");
    // // axis->GetYaxis()->SetTitleOffset(0.7);
     axis->GetYaxis()->SetTitleOffset(0.7);
    axis->GetYaxis()->SetTitle("Particles [MIP/0.8 ns]");
    //
    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );


    TLatex* tex = new TLatex();
    tex->SetNDC();
    tex->SetTextFont(63);

    // Do the drawing shit -------------------------------------------------------------

    axis->Draw("AXIS");

    tex->DrawLatex(0.35, 0.49, "PLACEHOLDER");

    can->SaveAs("./Calibration/placeholder.pdf");
    can->SaveAs("./Calibration/placeholder.png");
    can->SaveAs("./Calibration/placeholder.eps");
    can->SaveAs("./Calibration/placeholder.jpg");


}



//
//     Standard->GetXaxis()->SetNdivisions(506);
//     Standard->GetXaxis()->SetLabelFont(43);
//     Standard->GetXaxis()->SetLabelOffset(0.015);
//     Standard->GetXaxis()->SetTitleOffset(1.2);
//     Standard->GetXaxis()->SetLabelSize(30);
//     Standard->GetXaxis()->SetTitleSize(34);
//     Standard->GetXaxis()->SetTitleFont(43);
//
//     Standard->GetYaxis()->SetNdivisions(506);
//     Standard->GetYaxis()->SetLabelFont(43);
//     Standard->GetYaxis()->SetLabelOffset(0.025);
//     Standard->GetYaxis()->SetLabelSize(30);
//     Standard->GetYaxis()->SetTitleSize(34);
//     Standard->GetYaxis()->SetTitleOffset(2.7);
//     Standard->GetYaxis()->SetTitleFont(43);
//
//     Standard->SetMinimum(-5);
//
//     Standard->GetXaxis()->SetTitle("");
//     Standard->GetYaxis()->SetTitle("E_{reco}[GeV]");
//     Standard->GetXaxis()->SetLimits(0, maxP);
// //    Standard->GetYaxis()->SetLimits(0, maxP);
//     Standard->SetMinimum(-1);
//     Standard->SetMaximum(maxP);
//     Standard->SetTitle("");
//     Standard->Draw("AP");
//     SC->Draw("P same");
//
//     TLine line1(0.0, 0.0, maxP, maxP);
//     line1.SetLineStyle(2);
//     line1.Draw();
//
//     TLegend legend(0.3, 0.73, 0.45, 0.89);
//     if (DataSets == "CERN") {
//         string header = "  CERN TB,  " + Type + " #pi^{-}";
//         legend.SetHeader(header.c_str());
//     }
//     if (DataSets == "FNAL") {
//         string header = "  FNAL TB,  " + Type + " #pi^{-}";
//         legend.SetHeader(header.c_str());
//     }
//     if (DataSets == "Combined") {
//         string header = "CERN & FNAL TB,  " + Type + " #pi^{-}";
//         legend.SetHeader(header.c_str());
//     }
//     legend.SetTextFont(42);
//     legend.SetTextSize(0.03);
//     legend.SetBorderSize(0);
//     legend.SetFillColor(0);
//     legend.AddEntry(Standard, "Standard Reco.", "p");
//     legend.AddEntry(SC, sctype, "p");
//     legend.Draw();
//
//     TPaveText p(0.57, 0.04, 0.8, 0.13, "NDC");
//     p.SetTextAlign(11);
//     p.SetTextFont(43);
//     p.SetFillColor(10);
//     p.SetLineColor(10);
//     p.SetBorderSize(1);
//     p.SetTextSize(31);
//     p.AddText("CALICE Preliminary");
//     p.SetTextColor(kGray + 1);
//     p.Draw();
//
//     TPaveText pl(0.57, 0.04, 0.8, 0.05, "NDC");
//     pl.SetTextAlign(11);
//     pl.SetTextFont(43);
//     pl.SetFillColor(10);
//     pl.SetLineColor(10);
//     pl.SetBorderSize(1);
//     pl.SetTextSize(20);
//     pl.AddText("Si-W ECAL + AHCAL + TCMT");
//     pl.SetTextColor(kGray + 1);
//     pl.Draw();
// //    -----------Pad2-----------------------------------------------

//
//     if (DataSets == "CERN") {
//         RStandard->SetMarkerStyle(25);
//         RSC->SetMarkerStyle(21);
//     }
//     if (DataSets == "FNAL") {
//         RStandard->SetMarkerStyle(26);
//         RSC->SetMarkerStyle(22);
//     }
//     if (DataSets == "Combined") {
//         RStandard->SetMarkerStyle(24);
//         RSC->SetMarkerStyle(20);
//     }
//     RStandard->SetMarkerColor(kBlue);
//     RSC->SetMarkerColor(kRed);
//     RStandard->GetXaxis()->SetTitle("E_{beam}[GeV]");
//     RStandard->GetXaxis()->SetLimits(0, maxP);
//
//     RStandard->GetXaxis()->SetNdivisions(506);
//     RStandard->GetXaxis()->SetLabelFont(43);
//     RStandard->GetXaxis()->SetLabelOffset(0.015);
//     RStandard->GetXaxis()->SetTitleOffset(3.5);
//     RStandard->GetXaxis()->SetLabelSize(30);
//     RStandard->GetXaxis()->SetTitleSize(34);
//     RStandard->GetXaxis()->SetTitleFont(43);
//     RStandard->GetXaxis()->SetTickLength(RStandard->GetXaxis()->GetTickLength() * 1.);
//
//     RStandard->GetYaxis()->SetTitle("(E_{reco}-E_{beam})/E_{beam}");
//     RStandard->GetYaxis()->SetNdivisions(503);
//     RStandard->GetYaxis()->SetLabelFont(43);
//     RStandard->GetYaxis()->SetLabelOffset(0.035);
//     RStandard->GetYaxis()->SetLabelSize(30);
//     RStandard->GetYaxis()->SetTitleSize(34);
//     RStandard->GetYaxis()->SetTitleOffset(2.7);
//     RStandard->GetYaxis()->SetTitleFont(43);
//     RStandard->GetYaxis()->SetTickLength(RStandard->GetYaxis()->GetTickLength() * 1.5);
//     RStandard->SetMinimum(-0.07);
//     RStandard->SetMaximum(0.07);
// //    RStandard->GetYaxis()->SetNdivisions(3);
//     RStandard->SetTitle("");
//     RStandard->Draw("AP");
//     RSC->Draw("P same");
//
//     TLine line2(0.0, 0.0, maxP, 0.0);
//     line2.SetLineStyle(2);
//     line2.Draw();
//     c.Modified();
//     PrintPlot(c, "EnergyReco", DataSets, Type);
//     c.Close();
//     Standard->Write(("Reco_St_" + Type).c_str());
//     RStandard->Write(("Deviation_Reco_St_" + Type).c_str());
//
