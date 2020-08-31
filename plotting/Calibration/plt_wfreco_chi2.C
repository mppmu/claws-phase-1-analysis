


int plt_wfreco_chi2()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel*1/2);
    double ratio = 3./3.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);
    can->SetLogy();
    can->SetLogx();
    //can->SetLogy();

    // AXIS
    double xlow = 0.18;
    double xup  = 1;
    double ylow = 0.6;
    double yup  = 160;

    TH1F *axis = new TH1F("axis","axis", 1, xlow, xup );
    //axis->GetYaxis()->SetRangeUser(ylow, yup);
    axis->GetXaxis()->SetTitle("#chi^{2}/ndf");
    axis->GetYaxis()->SetTitle("Entries [1/10^{-3}]");
//    axis->GetXaxis()->SetNdivisions(505);

    //axis->GetXaxis()->SetRangeUser(xlow, xup);
    axis->GetYaxis()->SetRangeUser(ylow, yup);

    // Get the hists

    double markersize = 1.0;
    double markeralpha = 0.6;
    double linealpha = 0.6;
    double linewidth = 3;
    double linewidth2 = 0;
    int linestyle = 7;
    double rebin = 1;
    double fillalpha = 0.6;

    // // Tableau 20
    int c1 = claws_black->GetNumber();
    int c2 = claws_red->GetNumber();
    int c3 = claws_gray->GetNumber();



    // Get the hists

    string path = "./Calibration/Data/";
    string fname = "chi2_ndf_2016-06-22.root";

    string daynr = fname.substr(9,10);

    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1F* fwd1 = (TH1F*) rfile->Get("FWD1");
    fwd1->SetLineColor(c1);
    fwd1->SetLineWidth(linewidth);
    fwd1->SetFillColorAlpha(c1, fillalpha);
    fwd1->Rebin(rebin);

    TH1F* fwd2 = (TH1F*) rfile->Get("FWD2");
    fwd2->SetLineColor(c2);
    fwd2->SetLineWidth(linewidth);
    fwd2->SetFillColorAlpha(c2, fillalpha);
    fwd2->Rebin(rebin);

    TH1F* fwd3 = (TH1F*) rfile->Get("FWD3");
    fwd3->SetLineColor(c3);
    fwd3->SetLineWidth(linewidth);
    fwd3->SetFillColorAlpha(c3, fillalpha);
    fwd3->Rebin(rebin);

    // Leg and text
    TLegend* leg =  new TLegend(0.63, 0.64, 0.89, 0.83);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    // leg->SetNColumns(3);
    TH1F* fwd1l = (TH1F*) fwd1->Clone("FWD1l");
    fwd1l->SetLineColor(kBlack);
    fwd1l->SetLineWidth(linewidth2);
    fwd1l->SetFillColorAlpha(c1, 1);

    leg->AddEntry(fwd1l, "FWD1", "f");

    TH1F* fwd2l = (TH1F*) fwd2->Clone("FWD2l");
    fwd2l->SetLineColor(kBlack);
    fwd2l->SetLineWidth(linewidth2);
    fwd2l->SetFillColorAlpha(c2, 1);

    leg->AddEntry(fwd2l, "FWD2", "f");

    TH1F* fwd3l = (TH1F*) fwd3->Clone("FWD3l");
    fwd3l->SetLineColor(kBlack);
    fwd3l->SetLineWidth(linewidth2);
    fwd3l->SetFillColorAlpha(c3, 1);
    leg->AddEntry(fwd3l, "FWD3", "f");

    int legfont     = leg->GetTextFont();

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // --- Now start the actual drawing ---
    //gPad->SetLogy();

    axis->Draw("AXIS");

    fwd1->Draw("SAME HIST ][");
    fwd2->Draw("SAME HIST ][");
    fwd3->Draw("SAME HIST ][");

    leg->Draw();
    text->DrawLatex(0.64, 0.86, ("Day " + daynr).c_str());

    axis->Draw("AXIS SAME");

    can->SaveAs("./Calibration/wfreco_hist.pdf");
    can->SaveAs("./Calibration/wfreco_hist.png");
    can->SaveAs("./Calibration/wfreco_hist.eps");
    can->SaveAs("./Calibration/wfreco_hist.jpg");

    return 0;
}
