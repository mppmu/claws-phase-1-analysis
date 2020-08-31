


int plt_calpd_hist()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel*1/2);
    double ratio = 3./3.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);
    //can->SetLogy();

    string path = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/PDS_Calibration/Waveforms/";
    string fname = "CalibrationEvent_401141-cal002_pd_subtracted.root";

    string calevtnr = fname.substr(17,13);

    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1I* fwd1 = (TH1I*) rfile->Get("FWD1-INT_pd");
    fwd1->SetDirectory(0);

    TF1* fit = fwd1->GetFunction("gaus");
    fit->SetBit(TF1::kNotDraw);
    double linewidth = 3;
    //
    // // claws default

    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int cfit = tab20_blue->GetNumber();
    // // Katha 112
    // // int c1 = TColor::GetColor("#ee0000"); //rot
    // // int c2 = TColor::GetColor("#aaccff");
    // // int c3 = TColor::GetColor("#550144");

    // double xlow = -4.8;
    // double xup  = 4.8;

    double xlow = -2.9;
    double xup  = 3.9;

    double ylow = 0.5;
    double yup  = 300;
    //
    TLine *line = new TLine(0,ylow,0,120);
    line->SetLineColorAlpha(kBlack,1);
    line->SetLineStyle(7);
    //
    //
    //fwd1_fit_mean->SetMarkerColorAlpha( c1, markeralpha);
    //fwd1->SetFillColorAlpha( claws_red->GetNumber(), 1.);
    //fwd1->SetLineColorAlpha( kBlack, 1.);
    fwd1->SetLineColorAlpha( c1, 1.);
    fwd1->SetFillColorAlpha( c1, 1.);
    fwd1->SetMarkerColorAlpha(c1, 1);
    fwd1->SetMarkerSize(1.0);
    fwd1->SetLineWidth(linewidth-1);

    fwd1->SetFillStyle(0);

    fwd1->GetXaxis()->SetRangeUser(xlow, xup);
//    fwd1->GetXaxis()->SetTitleOffset(1.1);
    fwd1->GetXaxis()->SetTitle("Amplitude [mV]");
    fwd1->GetYaxis()->SetRangeUser(ylow, yup);
//    fwd1->GetYaxis()->SetTitleOffset(1.4);
    fwd1->GetYaxis()->SetTitle("Entries [1/0.39 mV]");
//    fwd1->GetXaxis()->SetNdivisions(505);

    fit->SetLineColor(cfit);
    fit->SetLineWidth(linewidth);

    TH1F* fwd1_2 = (TH1F*) fwd1->Clone("fwd_2");

    TH1I *err = (TH1I*) fwd1->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(3353);
    //err->SetFillStyle(1001);
    //err->SetFillColorAlpha(cerr,1);
    err->SetFillColorAlpha(kBlack,1);
    err->SetLineWidth(0);

    double legy = 0.8;

    TLegend* leg =  new TLegend(0.575, 0.74, 0.84, 0.94);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);


    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    TH1F* fwd1m = (TH1F*) fwd1->Clone("fwd1m");
    fwd1m->SetLineWidth(2);
    // fwd1m->SetLineColor(kBlack);
    // fwd1m->SetFillColor(c1);
    // fwd1m->SetFillStyle(1001);
    leg->AddEntry(make_box(fwd1m), "Pedestal Samples", "f");


    TH1F* errm = (TH1F*) err->Clone("errm");
    errm->SetLineColor(kWhite);
    leg->AddEntry(errm, "Stat. Unc.", "f");

    TH1F* fitm = (TH1F*) fit->Clone("fitm");
    fitm->SetLineWidth(4);
    // fitm->SetFillColor(cfit);
    // fitm->SetLineColor(kBlack);
    // fitm->SetFillStyle(1001);
    leg->AddEntry(set_line_width(fitm), "Gaussian fit", "l");

    //leg2->AddEntry(fwd1_2, "FWD1", "l");

    // char my_char[50];
    // sprintf(my_char,"c. = %.2f mV^{-1}",fit->GetParameter(0));
    // string con(my_char);


//    leg2->AddEntry(fit, "Gaussian fit:", "l");
    cout<< gStyle->GetLegendTextSize() << endl;
//    gStyle->SetLegendTextSize(gStyle->GetLegendTextSize()*0.1);

    TLegend* leg2 = new TLegend(0.57, 0.59, 0.86, 0.73);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss;
    ss << fixed << setprecision(2) << "c. = (" << fit->GetParameter(0) << " #pm "  << fit->GetParError(0)<<") mV^{-1}";
    leg2->AddEntry((TObject*)0, ss.str().c_str(), "");

    stringstream ss2;
    ss2 << fixed << setprecision(2) << "#mu = (" << fit->GetParameter(1) << " #pm "  << fit->GetParError(1) << ") mV";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss2.str().c_str(), "");

    //cout << f1->GetParError(1) << endl;
    stringstream ss3;
    ss3 << fixed << setprecision(2) << "#sigma = (" << fit->GetParameter(2) << " #pm "  << fit->GetParError(2) << ") mV";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss3.str().c_str(), "");

    //leg2->AddEntry((TObject*)0, con.c_str(), "");
//    leg2->AddEntry((TObject*)0, con.c_str(), "");

    // sprintf(my_char,"#mu = %.2f mV",fit->GetParameter(1));
    // string mu(my_char);
    // leg2->AddEntry((TObject*)0, mu.c_str(), "");

//    leg2->AddEntry((TObject*)0, mu.c_str(), "");

    // sprintf(my_char,"#sigma = %.2f mV",fit->GetParameter(2));
    // string sig(my_char);
    // leg2->AddEntry((TObject*)0, sig.c_str(), "");
//    leg2->AddEntry((TObject*)0, sig.c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());
    //text->SetTextSize(vis_multi*textsize*pt2pixel*0.8);

    // --- Now start the actual drawing ---
    gPad->SetLogy();

    fwd1->Draw("axis");
    line->Draw();


    err->Draw("same E2");

    fwd1->Draw("same hist");

    // fwd1_2->SetMarkerColorAlpha(c1, 1.);
    // fwd1_2->SetLineWidth(2);
    // fwd1_2->SetMarkerStyle(24);7
    //
    // fwd1_2->Draw("same p");

    fit->Draw("same");

    leg->Draw();
    leg2->Draw();
    text->DrawLatex(0.21, 0.89, ("Evt. "+calevtnr+":").c_str());
    text->DrawLatex(0.21, 0.82, "FWD1");
//    leg2->Draw("same");


    fwd1->Draw("same axis");


    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_hist.pdf");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_hist.png");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_hist.eps");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_hist.jpg");

    return 0;
}
