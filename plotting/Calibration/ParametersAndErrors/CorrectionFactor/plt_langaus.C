// TH1F* set_line_width(TH1F* wf, int width = 4)
// {
//     TH1F* clone = (TH1F*) wf->Clone("clone");
//     clone->SetLineWidth(width);
//     return clone;
// }
//
// TH1I* set_line_width(TH1I* wf, int width = 4)
// {
//     TH1I* clone = (TH1I*) wf->Clone("clone");
//     clone->SetLineWidth(width);
//     return clone;
// }
//
// TF1* set_line_width_f(TF1* wf, int width = 4)
// {
//     TF1* clone = (TF1*) wf->Clone("clone");
//     clone->SetLineWidth(width);
//     return clone;
// }


void plt_langaus()
{
    SetPhDStyle();

int width = round(vis_multi*textwidth*pt2pixel*1/2);
    double ratio = 3./3.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//    can->SetLogy();

    // -----   Axis   ---------------------

    double xlow = -0.0322;
    double xup = 4.9679;
    double ylow = 0;
    double yup = 700;

    TH1F* axis = new TH1F("axis", "axis", 100, xlow, xup);


    axis->GetXaxis()->SetRangeUser(xlow, xup);
    //    fwd1->GetXaxis()->SetTitleOffset(1.1);
    axis->GetXaxis()->SetTitle("Particle Energy [MIP]");
//    axis->GetXaxis()->SetNdivisions(505);
    axis->GetYaxis()->SetRangeUser(ylow, yup);
    //    fwd1->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Entries [1/0.064 MIP]");

    // -----   Colors and Style   ---------------------
    double linewidth = 2;

    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int cfit = tab20_red->GetNumber();
    // // Katha 112



    // -----   Hist   ---------------------
     string path = "/remote/ceph/group/ilc/claws/phase1/Myon/Converted/Run-900140/Results/SystematicsStudy/";
     string fname = "run_900140_systematics_ph1v6_nominal.root";

    // string path = "./Calibration/ParametersAndErrors/CorrectionFactor/";
    // string fname = "run_900122_systematics_ph1v4.root";

    string evtnr = fname.substr(4,6);

    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1F* fwd1 = (TH1F*) rfile->Get("FWD4_mip_per_event");
    fwd1->SetDirectory(0);

    //double scale = 1e-9;
    //tres->SetBins(tres->GetNbinsX(), tres->GetBinLowEdge(1)/scale, (tres->GetBinLowEdge(tres->GetNbinsX()) + tres->GetBinWidth(tres->GetNbinsX()))/scale );

    fwd1->SetLineColorAlpha( c1, 1.);
    fwd1->SetFillColorAlpha( c1, 1.);
    fwd1->SetMarkerColorAlpha(c1, 1);
    fwd1->SetMarkerSize(1.0);
    fwd1->SetLineWidth(2);

    fwd1->SetFillStyle(0);

    // -----   Error   ---------------------

    TH1F *err = (TH1F*) fwd1->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(3353);
    //err->SetFillStyle(1001);
    //err->SetFillColorAlpha(cerr,1);
    err->SetFillColorAlpha(kBlack,1);
    err->SetLineWidth(0);

    // -----   Fit   ---------------------
    TF1* fit = fwd1->GetFunction("FWD4_mip_per_eventlangaus");
    //fit->SetBit(TF1::kNotDraw);

    // double con = fit->GetParameter(0);
    // double mean = fit->GetParameter(1);
    // double sigma = fit->GetPa    leg->AddEntry(err, "Stat. Unc.", "f");rameter(2);
    //  fit->SetParameters(con, mean/scale, sigma/scale);
    //
    // double min, max;
    // fit->GetRange(min, max);
    // fit->SetRange(min/scale, max/scale);
    //  TF1* drawfit =
     fit->SetLineColor(cfit);
    fit->SetLineWidth(3);
    fit->SetNpx(5000);

    // -----   Legend   ---------------------
    cout << fit->GetMaximumX() << endl;
    cout << 1-fit->GetMaximumX() << endl;

    TLine *line = new TLine(fit->GetMaximumX(),0,fit->GetMaximumX(),600);
    line->SetLineColorAlpha(kBlack,1);
    line->SetLineStyle(7);


    // -----   Legend   ---------------------

    TLegend* leg =  new TLegend(0.49, 0.59, 0.76, 0.86);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);


    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(fwd1), "Reconstructed Energy", "f");
    leg->AddEntry(err, "Stat. Unc.", "f");
    leg->AddEntry(set_line_width(fit), "(Laundau*Gaus) Fit:", "l");

    stringstream ss;
    ss << fixed << setprecision(2) << "MPV = " << fit->GetMaximumX() << " MIP";
    //     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg->AddEntry((TObject*)0, ss.str().c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // ------   Drawing   --------------------

    axis->Draw("axis");
    line->Draw("same");
    fwd1->Draw("same hist");
    err->Draw("same E2");
//err->Draw("same E2");
    fit->Draw("same");
    leg->Draw();

    text->DrawLatex(0.34, 0.89, ("Run "+evtnr+": Sensor 10074 (FWD3)").c_str());

    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/langaus_fwd3_1000mV.pdf");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/langaus_fwd3_1000mV.png");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/langaus_fwd3_1000mV.eps");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/langaus_fwd3_1000mV.jpg");
}
