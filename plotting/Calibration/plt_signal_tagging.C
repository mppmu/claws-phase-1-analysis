
#include <TFile.h>


void plt_signal_tagging()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel);
    // double height = 900;
    // double ratio = 16./9.;
    // double ratio = 2./3.;
    double ratio = 2./1.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLeftMargin(0.08);
    //can->SetRightMargin(0.08);
    // can->SetLeftMargin(0.08);
    // can->SetBottomMargin(0.14);
    // can->SetRightMargin(0.02);

    double xlow = -3.49;
    double xup  = 5.49;
    // double ylow = -19.99;
    // double yup  = 39.99;
    double ylow = - 9.99;
    double yup  = 21.99;

    double linewidth = 2;

    int c1 = kBlack;
    int cerr = kGray;
    int creco = claws_red->GetNumber();
    double fillerr = 1.0;
    int fillstyle = 1001;

    // Hists

    int length   = 600;
    int start   = 423466;

    string path  = "./Calibration/Data/";
    string fname = "PhysicsEvent_401145038_tagged.root";
    string runnr = fname.substr(13,9);

    TFile* rfile = new TFile((path + fname).c_str(), "open");

    TH1D* fwd1 = (TH1D*) rfile->Get("FWD1");


    TH1D *fwd1m = new TH1D("fwd1m","fwd1m", length, - 0.4, length*0.8 - 0.4);

    for(int i = 1; i<=length; ++i )
    {
        double cont = fwd1->GetBinContent(i+start);
        double err  = fwd1->GetBinError(i+start);
        fwd1m->SetBinContent(i,cont);
        fwd1m->SetBinError(i, err);
    }

    fwd1m->SetLineWidth(linewidth);
    fwd1m->SetLineColorAlpha(c1,1.);

    TH1I *err = (TH1I*) fwd1m->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(fillstyle);
    err->SetFillColorAlpha(cerr,fillerr);
    err->SetLineWidth(1);

    TH1D* reco = (TH1D*) rfile->Get("FWD1_reco");

    TH1D *recom = new TH1D("fwd1m","fwd1m", length, - 0.4, length*0.8 - 0.4);

    for(int i = 1; i<=length; ++i )
    {
        double cont = reco->GetBinContent(i+start);
        double err  = reco->GetBinError(i+start);
        recom->SetBinContent(i,cont);
        recom->SetBinError(i, err);
    }

    recom->SetLineColorAlpha(creco,1);
    recom->SetLineWidth(linewidth);

    TH1D* axis = (TH1D*) fwd1m->Clone("axis");
    axis->GetYaxis()->SetRangeUser(ylow, yup);
//    axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ns]");
    axis->GetYaxis()->SetTitleOffset(0.6);
    axis->GetYaxis()->SetTitle("Amplitude [mV]");
    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );

    TLine *line = new TLine(axis->GetXaxis()->GetXmin(),0,axis->GetXaxis()->GetXmax(),0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);


    TLegend* leg =  new TLegend(0.775, 0.65, 0.92, 0.86);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->AddEntry(fwd1m, "FWD1 Untagged", "l");
    leg->AddEntry(err, "Sys. Unc.", "f");
    leg->AddEntry(recom, "FWD1 Tagged", "l");

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // Plotting
    axis->Draw("AXIS");
    line->Draw("same");
    //err->Draw("same E3");
    fwd1m->Draw("hist L same ][");
    recom->Draw("hist L same ][");

    leg->Draw();
    text->DrawLatex(0.78, 0.88, ("Run " + runnr).c_str());

    can->SaveAs("./Calibration/signal_taggin.pdf");
    can->SaveAs("./Calibration/signal_taggin.png");
    can->SaveAs("./Calibration/signal_taggin.eps");
    can->SaveAs("./Calibration/signal_taggin.jpg");

}

//     TH1D* calwf_axis = new TH1D("calwf axis", "",length,  -dt/2., dt*length - dt/2.);
//
//     for (size_t i = 1; i <= calwf_long->GetNbinsX(); i++)
//     {
//         double cont = -1*calwf_long->GetBinContent(i) * 50./32512 ;
//         calwf_axis->SetBinContent(i, cont);
//     }
//
//     TH1D* calwf = new TH1D("calwf", "",230,  -dt/2., dt*230 - dt/2.);
//
//     TGraphErrors* calgr= new TGraphErrors();
//
//     for (size_t i = 1; i <= 230; i++)
//     {
//         double cont = -1*calwf_long->GetBinContent(i) * 50./32512 ;
//         calwf->SetBinContent(i, cont);
//         calwf->SetBinError(i, 1.);
//
//         calgr->SetPoint(i-1, (i-1)*dt, cont);
//         calgr->SetPointError(i-1, dt/2., 1.);
//     }
//
//     delete calwf_long;
//
//     calwf_axis->GetXaxis()->SetTitle("Time [ns]");
//     calwf_axis->GetYaxis()->SetTitle("Voltage [mV]");
//     calwf_axis->GetYaxis()->SetRangeUser( -1.99, 7.99);
//
//     // calwf_axis->GetYaxis()->SetTickLength(0.015);
//
//     calwf->SetLineColor(1);
//     calwf->SetFillColor(claws_red2->GetNumber());
//     calwf->SetMarkerSize(0.);
//     //calwf->GetXaxis()->SetRangeUser(-dt/2., dt*length - dt/2.);
// //    calwf->SetAxisRange(0., 100, "X");
//
//     // --- Physics waveform ---
//     fname = "/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/Event-401141041.root";
//     //fname = "/Users/mgabriel/workspace/Plotting/PhD_Thesis/Calibration/Data/Event-401141041.root";
//     TFile *rfile = new TFile(fname.c_str(), "open");
//
//     TH1I* phwf_long = (TH1I*) rfile->Get("FWD1");
//     phwf_long->SetDirectory(0);
//
//     rfile->Close("R");
//     delete rfile;
//
//     for (size_t i = 1; i <= phwf_long->GetNbinsX(); i++)
//     {
//         if( phwf_long->GetBinContent(i) < 26000 )
//         {
//             shift = i-132;
//             break;
//         }
//     }
//
//     TH1D* phwf = new TH1D("phwf", "",length,  -dt/2., dt*length - dt/2.);
//
//     for (size_t i = 1; i <= phwf->GetNbinsX(); i++)
//     {
//         double cont = -(phwf_long->GetBinContent(i+shift) -32512*0.9) * 1000./32512 ;
//         phwf->SetBinContent(i, cont);
//         phwf->SetBinError(i, 7.8125);
//     }
//
//     delete phwf_long;
//     // - 32512 * 0.9) / (256**2) * 1000)
//
//     phwf->GetXaxis()->SetTitle("Time [ns]");
//     phwf->GetYaxis()->SetTitle("Voltage [mV]");
//     phwf->GetYaxis()->SetRangeUser(-74.9, 449.999);
//
//     phwf->SetMarkerSize(0.);
//     phwf->SetLineColor(kBlack);
//     phwf->SetFillColor( claws_gray->GetNumber());
//

//
// //    TLine *llow = new TLine(-dt/2.,0,dt*length - dt/2.,0);
// //    llow->SetLineColor(kRed);
//     // --- Canvas and Pads
//
//
//     int width = round(vis_multi*textwidth*pt2pixel);
//     // double height = 900;
//     // double ratio = 16./9.;
//     double ratio = 1./1.;
//     TCanvas * can = new TCanvas("can", "can", width, width/ratio);
//     can->SetFillColor(kWhite);
//     can->SetBorderMode(0);
//     can->SetBorderSize(2);
//     can->SetFrameBorderMode(0);
//
//     double padsplit = 0.5;
//     TPad* pad1 = new TPad("pad1", "The pad 80% of the height", 0.0, padsplit, 1, 1);
//     TPad* pad2 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.0, 1, padsplit);
//
//     pad1->Draw();
//     pad2->Draw();
//
//     double rightmargin = 0.01;
//     double leftmargin = 0.08;
//     double topmargin = 0.03;
//     double bottommargin = 0.08;
//
//     // --- pad1 ---
//     pad1->SetFrameLineWidth(0);
//     pad1->SetTickx(0);
//     pad1->SetTicky(0);
//     // gPad->SetTicks();
//     // pad1->cd();
//     // pad1->SetFillColor(kWhite);
//     // pad1->SetBorderMode(0);
//     // pad1->SetBorderSize(2);
// //    pad1->SetTickx(1);
//     // pad1->SetTicky(1);
//     // pad1->SetLeftMargin( leftmargin );
//     // pad1->SetRightMargin( rightmargin );
//     // pad1->SetTopMargin(topmargin );
//     // pad1->SetBottomMargin(bottommargin);
//     // pad1->SetFrameLineWidth(2);
//     // pad1->SetFrameBorderMode(0);
//     // pad1->SetFrameLineWidth(2);
//     // pad1->SetFrameBorderMode(0);
//
//     // --- pad2 ---
//     pad2->SetFrameLineWidth(0);
//     pad2->SetTickx(0);
//     pad2->SetTicky(0);
//     // pad2->cd();
//     // pad2->SetFillColor(kWhite);
//     // pad2->SetBorderMode(0);
//     // pad2->SetBorderSize(2);
//
//     // pad2->SetFrameBorderMode(0);
//     // pad2->SetFrameLineWidth(2);
//     // pad2->SetFrameBorderMode(0);
//
//     // TColor claws_black(1200, 0./255., 0./255.,0./255.,"CLAWS black");
//     //TColor claws_red(1201, 255./255., 0./255.,0./255.,"CLAWS red");
//     //TColor claws_gray(1202, 128./255., 128./255.,128./255.,"CLAWS gray");
//     // --- Legend ---

//
//     TLegend* llow =  new TLegend(0.72, 0.86, 0.9, 0.9);
//     //lup->SetTextFont(42);
//     // lup.SetTextSize(0.03);
//     llow->SetBorderSize(0);
//     llow->SetFillColor(0);
//     llow->AddEntry(phwf, "Physics waveform", "lf");
//
//
//
//     // --- Draw first pad ---
//     pad1->cd();
//
//     calwf_axis->GetYaxis()->SetTickLength( calwf_axis->GetYaxis()->GetTickLength()*(1-padsplit) );
//     pad1->SetTickx(0);
//
//     calwf_axis->Draw("AXIS");
//
//     //pad1->Update();
//     //calwf_axis->Draw("AXIS");
//
//     line->Draw();
//     lup->Draw("same");
//
//     //calwf->SetMarkerSize(0.5);
//     //calwf->DrawCopy("hist same pl");
//     //calgr->SetFillColorAlpha(kRed, 0.5);
//     //calgr->Draw("ap same");
//
//
//     calwf->DrawCopy("E2 same");
//
//     // calwf->DrawCopy("HIST ][ same");
//     TH1F* calwf2 = (TH1F*) calwf->Clone("cal2");
//     calwf2->SetFillColorAlpha( claws_red->GetNumber(), 0.);
//     calwf2->DrawCopy("hist same pl");
//
//
//     // --- Second pad ---
//     pad2->cd();
//
//     phwf->GetYaxis()->SetTickLength( phwf->GetYaxis()->GetTickLength()*padsplit);
//
//     phwf->DrawCopy("AXIS");
//
//     line->Draw("same");
//     llow->Draw("same");
//
//     phwf->DrawCopy("E2 same pl");
//
//     TH1F* phwf2 = (TH1F*) phwf->Clone("ph2");
//     phwf2->SetFillColorAlpha(claws_gray->GetNumber(), 0.);
//     phwf2->DrawCopy("hist same pl");




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
