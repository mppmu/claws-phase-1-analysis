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


void plt_tres()
{
    SetPhDStyle();
int width = round(vis_multi*textwidth*pt2pixel*1/2);
    //int width = round(vis_multi*.25**pt2pixel*1/2);
    double ratio = 3./3.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLogy();

    // -----   Axis   ---------------------

    double xlow = -32.9;
    double xup = 17.9;
    double ylow = 0.5;
    double yup = 20000;

    TH1F* axis = new TH1F("axis", "axis", 100, xlow, xup);


    axis->GetXaxis()->SetRangeUser(xlow, xup);
    //    fwd1->GetXaxis()->SetTitleOffset(1.1);
    axis->GetXaxis()->SetTitle("#Deltat [ns]");
    axis->GetXaxis()->SetNdivisions(505);
    axis->GetYaxis()->SetRangeUser(ylow, yup);
    //    fwd1->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Entries [1/0.8 ns]");


    // -----   Axis   ---------------------
    TLine *line = new TLine(0,ylow,0,5e7);
    line->SetLineColorAlpha(kBlack,1);
    line->SetLineStyle(7);

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

    string evtnr = fname.substr(4,6);

    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1F* tres = (TH1F*) rfile->Get("time_resolution");
    tres->SetDirectory(0);

    double scale = 1e-9;
    tres->SetBins(tres->GetNbinsX(), tres->GetBinLowEdge(1)/scale, (tres->GetBinLowEdge(tres->GetNbinsX()) + tres->GetBinWidth(tres->GetNbinsX()))/scale );

    tres->SetLineColorAlpha( c1, 1.);
    tres->SetFillColorAlpha( c1, 1.);
    tres->SetMarkerColorAlpha(c1, 1);
    tres->SetMarkerSize(1.0);
    tres->SetLineWidth(2);

    tres->SetFillStyle(0);


    // -----   Fit   ---------------------
    TF1* fit = tres->GetFunction("gaus");
    //fit->SetBit(TF1::kNotDraw);

   double con = fit->GetParameter(0);
   double mean = fit->GetParameter(1);
   double sigma = fit->GetParameter(2);
     fit->SetParameters(con, mean/scale, sigma/scale);

   double min, max;
   fit->GetRange(min, max);
   // fit->SetRange(min/scale, max/scale);
   fit->SetRange(-3.6, 3.6);
 //  TF1* drawfit =
    fit->SetLineColor(cfit);
    fit->SetLineWidth(3);


    // -----   Error   ---------------------

    TH1F *err = (TH1F*) tres->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(3353);
    //err->SetFillStyle(1001);
    //err->SetFillColorAlpha(cerr,1);
    err->SetFillColorAlpha(kBlack,1);
    err->SetLineWidth(0);

    // -----   Legend   ---------------------

    double legy = 0.8;

    TLegend* leg =  new TLegend(0.2, 0.74, 0.45, 0.94);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);


    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(tres), "t_{Sensor 10057} - t_{Sensor 10053}", "f");
    leg->AddEntry(err, "Stat. Unc.", "f");
    leg->AddEntry(set_line_width(fit), "Gaussian Fit:", "l");

    TLegend* leg2 = new TLegend(0.2, 0.59, 0.45, 0.73);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss;
    ss << fixed << setprecision(2) << "c. = (" << fit->GetParameter(0)/1e3 << " #pm "  << fit->GetParError(0)/1e3<<")/0.8 ns ";
    leg2->AddEntry((TObject*)0, ss.str().c_str(), "");

    stringstream ss2;
    ss2 << fixed << setprecision(2) << "#mu = (" << fit->GetParameter(1) << " #pm "  << fit->GetParError(1)/scale << ") ns";
    //     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss2.str().c_str(), "");

    cout << fit->GetParError(0) << endl;
    cout << fit->GetParError(1) << endl;
    cout << fit->GetParError(2) << endl;
    stringstream ss3;
    ss3 << fixed << setprecision(2) << "#sigma = (" << fit->GetParameter(2) << " #pm "  << fit->GetParError(2)/scale << ") ns";
    //     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss3.str().c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // ------   Drawing   --------------------

    axis->Draw("axis");

    tres->Draw("same hist");
    err->Draw("same E2");

    fit->Draw("same");

    leg->Draw();
    leg2->Draw();
    text->DrawLatex(0.72, 0.89, ("Run "+evtnr).c_str());
//    text->DrawLatex(0.21, 0.82, "FWD1");

    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/tres_1000mV.pdf");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/tres_1000mV.png");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/tres_1000mV.eps");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/Validation/tres_1000mV.jpg");
}
