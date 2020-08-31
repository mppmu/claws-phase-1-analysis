


int plt_gain()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel*1/2);
    // double height = 900;
    // double ratio = 16./9.;
    //double ratio = 2./3.;
    double ratio = 3./3.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    // can->SetTickx(0);
    // can->SetTicky(0);
    // can->SetFrameLineWidth(0);

    // can->SetRightMargin(0.16);
    // can->SetLeftMargin(0.1);
    // can->SetBottomMargin(0.14);
    double xlow = 30;
    double xup  = 220;
    double ylow = 0.5;
    double yup  = 1600;

    gStyle->SetHatchesSpacing(0.75);
    //gStyle->SetHatchesLineWidth();
    TH1I* axis = new TH1I("axis", "axis",10, xlow, xup );

    //axis->GetXaxis()->SetRangeUser(xlow, xup);
//    axis->GetXaxis()->SetTitleOffset(1.2);
    axis->GetXaxis()->SetTitle("Charge [10^{-15} C]");
    axis->GetYaxis()->SetRangeUser(ylow, yup);
//    axis->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Entries [1/5#times10^{-15} C]");
    axis->GetXaxis()->SetNdivisions(505);


    string path = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/GainDetermination/";
    //string fname = "CalibrationEvent_401141-cal002_pd_subtracted.root";



    string fname = "run_401141_gainstate_fitted_ph1v6.root";

    string runnr = fname.substr(4,6);
    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1I* fwd1 = (TH1I*) rfile->Get("FWD1");
    //fwd1->SetDirectory(0);

    double scale = 1e9;

    TF1* f1 = fwd1->GetFunction("FWD1_g1");
    f1->SetBit(TF1::kNotDraw);
    f1->SetParameter(1,f1->GetParameter(1)*scale );
    f1->SetParameter(2,f1->GetParameter(2)*scale );
    double xmin,xmax;
    f1->GetRange(xmin,xmax);
    f1->SetRange(xmin*scale,xmax*scale);

    TF1* f2 = fwd1->GetFunction("FWD1_g2");
    f2->SetBit(TF1::kNotDraw);
    f2->SetParameter(1,f2->GetParameter(1)*scale );
    f2->SetParameter(2,f2->GetParameter(2)*scale );
    xmin,xmax;
    f2->GetRange(xmin,xmax);
    f2->SetRange(xmin*scale,xmax*scale);


    // // f2->SetParameter(1,f2->GetParameter(1)*scale );
    // // f2->SetParameter(2,f2->GetParameter(2)*scale );
    //
    // cout<< fwd1->GetBinWidth(1) << endl;
    // cout<< fwd1->GetBinCenter(2) - fwd1->GetBinCenter(1)  << endl;
    //
    int nbinsx =fwd1->GetNbinsX();
    fwd1->SetBins(nbinsx, fwd1->GetBinLowEdge(1)*scale, (fwd1->GetBinLowEdge(nbinsx)+fwd1->GetBinWidth(nbinsx))*scale);


// double markeralpha = 0.3;
    // double linealpha = 0.3;
    double linewidth = 3;
    //
    // // claws default
    //int c1 = claws_red->GetNumber();
    int c1 = kBlack;
    int cerr = claws_red->GetNumber();
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int cfit = tab20_blue->GetNumber();
    int cfit2 = tab20_orange->GetNumber();
    // // Katha 112
    // // int c1 = TColor::GetColor("#ee0000"); //rot
    // // int c2 = TColor::GetColor("#aaccff");
    // // int c3 = TColor::GetColor("#550144");
    //
    // // // root default
    // // int c1 = 2;
    // // int c2 = 4;
    // // int c3 = 1;
    //


    fwd1->SetLineColorAlpha( c1, 1.);
    fwd1->SetLineWidth(linewidth);

    fwd1->SetLineColorAlpha( c1, 1.);

    fwd1->SetMarkerColorAlpha(c1, 1);
    fwd1->SetMarkerSize(1.0);
    fwd1->SetLineWidth(linewidth-1);

    fwd1->SetFillStyle(0);

    //fwd1->SetFillColorAlpha(cerr,0.);

    f1->SetLineColor(cfit);
    f1->SetLineWidth(linewidth);
    f1->SetNpx(1e6);

    f2->SetLineColor(cfit2);
    f2->SetLineWidth(linewidth);
    f2->SetNpx(1e6);

    TH1I *err = (TH1I*) fwd1->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(3353);
    //err->SetFillStyle(1001);
    //err->SetFillColorAlpha(cerr,1);
    err->SetFillColorAlpha(kBlack,1);
    err->SetLineWidth(0);
//     TF1* fitfull = (TF1*) f1->Clone("fitfull");
//     fitfull->SetLineColorAlpha(cfit, 0.4);
//
// //    f1->SetLineStyle(7);
//
    TLegend* leg =  new TLegend(0.46, 0.73, 0.72, 0.94);
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    TH1F* fwd1m = (TH1F*) fwd1->Clone("fwd1m");
    fwd1m->SetLineColor(kBlack);
    fwd1m->SetLineWidth(2);
    // fwd1m->SetFillColor(c1);
    // fwd1m->SetFillStyle(1001);

    leg->AddEntry(make_box(fwd1m), "Waveform Integral", "f");

    // TH1F* fwd1m = (TH1F*) fwd1->Clone("fwd1m");
    // fwd1m->SetLineColor(kWhite);
    // fwd1m->SetFillColor(c1);
    // fwd1m->SetFillStyle(1001);

    leg->AddEntry(err, "Stat. Unc.", "f");

    TH1F* f1m = (TH1F*) f1->Clone("f1m");
    f1m->SetLineWidth(4);
    // f1m->SetLineColor(kBlack);
    // f1m->SetFillColor(cfit);
    // f1m->SetFillStyle(1001);
    leg->AddEntry(set_line_width(f1m), "1 p.e. Gaussian Fit", "l");

    TLegend* leg2 = new TLegend(0.46, 0.58, 0.72, 0.72);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss;
    ss << fixed << setprecision(2) << "c = (" << f1->GetParameter(0) << " #pm "  << f1->GetParError(0)<<") #times (10^{-15} C)^{-1}";
    leg2->AddEntry((TObject*)0, ss.str().c_str(), "");

    stringstream ss2;
    ss2 << fixed << setprecision(2) << "#mu = (" << f1->GetParameter(1) << " #pm "  << f1->GetParError(1)*scale << ") #times 10^{-15} C";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss2.str().c_str(), "");

    cout << f1->GetParError(1) << endl;
    stringstream ss3;
    ss3 << fixed << setprecision(2) << "#sigma = (" << f1->GetParameter(2) << " #pm "  << f1->GetParError(2)*scale << ") #times 10^{-15} C";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss3.str().c_str(), "");

    TLegend* leg3 = new TLegend(0.46, 0.51, 0.72, 0.58);
    leg3->SetBorderSize(0);
    leg3->SetFillColor(0);
    leg3->SetFillStyle(0);

    TH1F* f2m = (TH1F*) f2->Clone("f2m");
    f2m->SetLineWidth(4);
    // f2m->SetLineColor(kBlack);
    // f2m->SetFillColor(cfit2);
    // f2m->SetFillStyle(1001);

    leg3->AddEntry(f2m, "2 p.e. Gaussian Fit", "l");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLine *line = new TLine(f1->GetParameter(1),ylow,f1->GetParameter(1),300);
    line->SetLineColorAlpha(kBlack,1);
    line->SetLineStyle(7);


    TLine *line2 = new TLine(f2->GetParameter(1),ylow,f2->GetParameter(1),13);
    line2->SetLineColorAlpha(kBlack,1);
    line2->SetLineStyle(7);
    // char my_char[50];
    // sprintf(my_char,"const.=%.2f",fit->GetParameter(0));
    // string con(my_char);
    //
    // leg->AddEntry(fit, "Gaussian fit:", "l");
    // leg->AddEntry((TObject*)0, con.c_str(), "");
    //
    // sprintf(my_char,"#mu=%.2f",fit->GetParameter(1));
    // string mu(my_char);
    // leg->AddEntry((TObject*)0, mu.c_str(), "");
    //
    // sprintf(my_char,"#sigma=%.2f",fit->GetParameter(2));
    // string sig(my_char);
    // leg->AddEntry((TObject*)0, sig.c_str(), "");

    // --- Now start the actual drawing ---

    // --- Now start the actual drawing ---

    gPad->SetLogy();
    axis->Draw("axis");
    line->Draw();
    //line2->Draw();
    fwd1->Draw("same hist");
    err->Draw("same E2");

    //err->SetFillColorAlpha(kRed)
//    fwd1->Draw("same E2");
    //
    f1->Draw("same");
    //f2->Draw("same");
    //fitfull->Draw("same");
//    fit->Draw("same");

    leg->Draw();
    leg2->Draw();
//    leg3->Draw();
    text->DrawLatex(0.21, 0.89, ("Run " + runnr+":").c_str());
    text->DrawLatex(0.21, 0.8175, "FWD1");

    axis->Draw("same axis");

    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain.pdf");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain.png");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain.eps");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain.jpg");

    return 0;
}
