
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

int plt_phpds_hist()
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

    string path = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/PDS_Physics/Waveforms/";
    string fname = "PhysicsEvent_401141001_pd_subtracted.root";

    string evtnr = fname.substr(13,9);

    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1I* fwd1 = (TH1I*) rfile->Get("FWD1_pd");
    fwd1->SetDirectory(0);

    TF1* fit = fwd1->GetFunction("gaus");
    fit->SetBit(TF1::kNotDraw);
    double linewidth = 2;

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

    // double xlow = -64;
    // double xup  = 124;
    double xlow = -34.99;
    double xup  = 69.99;

    double ylow = 0.5;
    double yup  = 0.9e9;
    //
    TLine *line = new TLine(0,ylow,0,5e7);
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
    fwd1->SetLineWidth(linewidth);

    fwd1->SetFillStyle(0);

    fwd1->GetXaxis()->SetRangeUser(xlow, xup);
//    fwd1->GetXaxis()->SetTitleOffset(1.1);
    fwd1->GetXaxis()->SetTitle("Amplitude [mV]");
    fwd1->GetYaxis()->SetRangeUser(ylow, yup);
//    fwd1->GetYaxis()->SetTitleOffset(1.4);
    fwd1->GetYaxis()->SetTitle("Entries [1/7.87 mV]");
//    fwd1->GetXaxis()->SetNdivisions(505);

    fit->SetLineColor(cfit);
    fit->SetLineWidth(linewidth+1);

    TH1F* fwd1_2 = (TH1F*) fwd1->Clone("fwd_2");

    fwd1->SetFillColorAlpha(c1,0.6);

    TH1I *err = (TH1I*) fwd1->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(3353);
    //err->SetFillStyle(1001);
    //err->SetFillColorAlpha(cerr,1);
    err->SetFillColorAlpha(kBlack,1);
    err->SetLineWidth(0);

    double legy = 0.8;

    TLegend* leg =  new TLegend(0.53, 0.74, 0.79, 0.94);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);


    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(fwd1), "Pedestal Samples", "f");
    leg->AddEntry(err, "Stat. Unc.", "f");
    leg->AddEntry(set_line_width(fit), "Gaussian Fit:", "l");

    //leg2->AddEntry(fwd1_2, "FWD1", "l");

    // char my_char[50];
    // sprintf(my_char,"c. = %.2f mV^{-1}",fit->GetParameter(0));
    // string con(my_char);


//    leg2->AddEntry(fit, "Gaussian fit:", "l");
    cout<< gStyle->GetLegendTextSize() << endl;
//    gStyle->SetLegendTextSize(gStyle->GetLegendTextSize()*0.1);

    TLegend* leg2 = new TLegend(0.54, 0.59, 0.79, 0.73);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss;
    ss << fixed << setprecision(2) << "c. = (" << fit->GetParameter(0)/1e3 << " #pm "  << fit->GetParError(0)/1e3<<") V^{-1}";
    leg2->AddEntry((TObject*)0, ss.str().c_str(), "");

    stringstream ss2;
    ss2 << fixed << setprecision(2) << "#mu = " << fit->GetParameter(1) << " mV #pm "  << fit->GetParError(1)*1e3 << " #muV";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg2->AddEntry((TObject*)0, ss2.str().c_str(), "");

    cout << fit->GetParError(0) << endl;
    cout << fit->GetParError(1) << endl;
    cout << fit->GetParError(2) << endl;
    stringstream ss3;
    ss3 << fixed << setprecision(2) << "#sigma = " << fit->GetParameter(2) << " mV  #pm "  << fit->GetParError(2)*1e3 << " #muV";
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
    text->DrawLatex(0.21, 0.89, ("Evt. "+evtnr+":").c_str());
    text->DrawLatex(0.21, 0.82, "FWD1");
//    leg2->Draw("same");

    fwd1->Draw("same axis");

    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_hist.pdf");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_hist.png");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_hist.eps");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_hist.jpg");

    return 0;
}
