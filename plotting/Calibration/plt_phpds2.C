
// TH1F* set_line_width4(TH1F* wf)
// {
//     TH1F* clone = (TH1F*) wf->Clone("clone");
//     clone->SetLineWidth(4);
//     return clone;
// }
//
// TF1* set_line_width4f(TF1* wf)
// {
//     TF1* clone = (TF1*) wf->Clone("clone");
//     clone->SetLineWidth(4);
//     return clone;
// }

int plt_phpds2()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel*2/4);
    // double height = 900;
    // double ratio = 16./9.;
    double ratio = 4./4.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    double markersize = 1.0;
    double markeralpha = 0.6;
    double linealpha = 0.6;
    double linewidth = 2;
    int linestyle = 7;


    // claws default
    // int c1 = claws_red->GetNumber();
    // int c3 = claws_gray->GetNumber();
    // int c2 = claws_black->GetNumber();

    // Katha 112
    // int c1 = TColor::GetColor("#ee0000"); //rot
    // int c2 = TColor::GetColor("#aaccff");
    // int c3 = TColor::GetColor("#550144");

    // // Tableau 20
    int c1 = tab20_blue->GetNumber();
    int c2 = tab20_orange->GetNumber();
    int c3 = tab20_green->GetNumber();
    //int c3 = TColor::GetColor(148,103,189); //lila

    // Tableau 20
    // int c1 = TColor::GetColor( 148, 103, 189);
    // int c2 = TColor::GetColor(23,190,207);
    // int c3 = TColor::GetColor( 140,86,75);

    // // root default
    // int c1 = 2;
    // int c2 = 4;
    // int c3 = 1;

    double xlow = -10;
    double xup  = 272;
    double ylow = -3.749;
    double yup  = 5.99;

    int nbins = 1000;
    TH2D* axis = new TH2D("axis", "axis",nbins, -100 , 1000 , nbins , -1000, 1000);
    axis->GetXaxis()->SetRangeUser(xlow,xup);
    axis->GetYaxis()->SetLimits(ylow,yup);


    //axis->GetXaxis()->SetTitleOffset(1.1);
    axis->GetXaxis()->SetTitle("T - T_{Run Start} [s]");
    //axis->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Pedestal [mV]");


    TLine *line = new TLine(xlow,0,xup,0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    // calibration file
    string path  = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/PDS_Physics/";

    string fname = "run_401141_pedestal_ph1v6.root";

    string runnr = fname.substr(4,6);

    TFile* rfile = new TFile((path + fname).c_str(), "open");

//    TGraphErrors* tmp = (TGraphErrors*) rfile->Get("FWD1_fit_mean");

    TGraphErrors* fwd1 = (TGraphErrors*) rfile->Get("FWD1_fit_mean");
    double tstart,dummy;
    fwd1->GetPoint(0,tstart,dummy);

    for(int i = 0; i < fwd1->GetN(); ++i)
    {
        double x,y;
        fwd1->GetPoint(i,x,y);
        fwd1->SetPoint(i,x-tstart,y);
    }

    TGraphErrors* fwd2 = (TGraphErrors*) rfile->Get("FWD2_fit_mean");

    for(int i = 0; i < fwd2->GetN(); ++i)
    {
        double x,y;
        fwd2->GetPoint(i,x,y);
        fwd2->SetPoint(i,x-tstart,y);
    }

    TGraphErrors* fwd3 = (TGraphErrors*) rfile->Get("FWD3_fit_mean");

    for(int i = 0; i < fwd3->GetN(); ++i)
    {
        double x,y;
        fwd3->GetPoint(i,x,y);
        fwd3->SetPoint(i,x-tstart,y);
    }


    fwd1->SetMarkerColorAlpha( c1, markeralpha);
    fwd1->SetMarkerColorAlpha( c1, markeralpha);
    //fwd1->SetFillColorAlpha( c1, 0.75);
    fwd1->SetLineColorAlpha( c1, linealpha);

    fwd1->SetLineWidth(linewidth);
    fwd1->SetMarkerStyle(21);
    fwd1->SetMarkerSize( markersize );


    fwd2->SetMarkerColorAlpha( c2, markeralpha);
    fwd2->SetMarkerColorAlpha( c2, markeralpha);
    //fwd2->SetFillColorAlpha( c2, 0.75);
    fwd2->SetLineColorAlpha( c2, linealpha);

    fwd2->SetLineWidth(linewidth);
    fwd2->SetMarkerStyle(22);
    fwd2->SetMarkerSize( markersize );

    fwd3->SetMarkerColorAlpha( c3, markeralpha);
    fwd3->SetMarkerColorAlpha( c3, markeralpha);
    //fwd3->SetFillColorAlpha( c3, 0.75);
    fwd3->SetLineColorAlpha( c3, linealpha);

    fwd3->SetLineWidth(linewidth);
    fwd3->SetMarkerStyle(20);
    fwd3->SetMarkerSize( markersize +0.2);

    // Get Means

    double x,y;

    double f1mean = fwd1->GetMean(2);
    double f1std  = 0;

    for(int i = 0; i<fwd1->GetN();++i)
    {
        fwd1->GetPoint(i, x, y);
        f1std += (y-f1mean)*(y-f1mean);
    }

    f1std = sqrt(f1std/fwd1->GetN());

    TLine *f1 = new TLine(xlow,f1mean,xup,f1mean);

    //TF1* f2 = new TF1("f2","[0]", xlow,xup);
    f1->SetLineColorAlpha( c1, 1.0);
    f1->SetLineWidth(linewidth);
    f1->SetLineStyle(linestyle);

    double f2mean = fwd2->GetMean(2);
    double f2std  = 0;

    for(int i = 0; i<fwd2->GetN();++i)
    {
        fwd2->GetPoint(i, x, y);
        f2std += (y-f2mean)*(y-f2mean);
    }

    f2std = sqrt(f2std/fwd2->GetN());

    TLine *f2 = new TLine(xlow,f2mean,xup,f2mean);

    //TF1* f2 = new TF1("f2","[0]", xlow,xup);
    f2->SetLineColorAlpha( c2, 1.0);
    f2->SetLineWidth(linewidth);
    f2->SetLineStyle(linestyle);

    double f3mean = fwd3->GetMean(2);
    double f3std  = 0;

    for(int i = 0; i<fwd3->GetN();++i)
    {
        fwd3->GetPoint(i, x, y);
        f3std += (y-f3mean)*(y-f3mean);
    }

    f3std = sqrt(f3std/fwd3->GetN());
    TLine *f3 = new TLine(xlow,f3mean,xup,f3mean);

    //TF1* f2 = new TF1("f2","[0]", xlow,xup);
    f3->SetLineColorAlpha( c3, 1.0);
    f3->SetLineWidth(linewidth);
    f3->SetLineStyle(linestyle);

    // legend
    TLegend* leg =  new TLegend(0.2, 0.65, 0.46, 0.86);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    // leg->SetNColumns(3);
    leg->AddEntry(fwd1, "FWD1", "lp");
    leg->AddEntry(fwd2, "FWD2", "lp");
    leg->AddEntry(fwd3, "FWD3", "lp");

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(vis_multi*textsize*pt2pixel*0.8);


    TLegend* leg2 =  new TLegend(0.39, 0.65, 0.59, 0.86);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);

    stringstream ss1;
    ss1 << std::fixed << setprecision(2) << "#bar{FWD1} = (" << f1mean << " #pm "  << f1std << ") mV";

    TLine * f1_2 = (TLine*) f1->Clone("f1_2");
    f1_2->SetLineWidth(4);
    leg2->AddEntry(f1_2, ss1.str().c_str(), "l");

    stringstream ss2;
    ss2 << fixed << setprecision(2) << "#bar{FWD2} = (" << f2mean << " #pm "  << f2std << ") mV";
    TLine * f2_2 = (TLine*) f2->Clone("f2_2");
    f2_2->SetLineWidth(4);
    leg2->AddEntry(f2_2, ss2.str().c_str(), "l");

    stringstream ss3;
    ss3 << fixed << setprecision(2) << "#bar{FWD3} = (" << f3mean << " #pm "  << f3std << ") mV";
    TLine * f3_2 = (TLine*) f3->Clone("f3_2");
    f3_2->SetLineWidth(4);
    leg2->AddEntry(f3_2, ss3.str().c_str(), "l");

    // --- Now start the actual drawing ---

    //axis
    axis->Draw("AXIS");

    fwd1->Draw("same p");
    fwd2->Draw("same p");
    fwd3->Draw("same p");

    TGraphErrors* fwd1c = (TGraphErrors*) fwd1->Clone("fwd1c");

    fwd1c->SetMarkerColorAlpha(c1, 1.);
    fwd1c->SetMarkerStyle(25);
    fwd1c->Draw("same px");

    f1->Draw("same");

    TGraphErrors* fwd2c = (TGraphErrors*) fwd2->Clone("fwd2c");

    fwd2c->SetMarkerColorAlpha(c2, 1.);
    fwd2c->SetMarkerStyle(26);
    fwd2c->Draw("same px");

    f2->Draw("same");

    TGraphErrors* fwd3c = (TGraphErrors*) fwd3->Clone("fwd3c");

    fwd3c->SetMarkerColorAlpha(c3, 1.);
    fwd3c->SetMarkerStyle(24);
    fwd3c->Draw("same px");

    f3->Draw("same");

    axis->Draw("SAME AXIS");

    // line->Draw("same");

    leg->Draw("same");
    leg2->Draw("same");

    TLegend* leg3 = (TLegend*) leg->Clone("leg3");
    leg3->Clear();

    leg3->AddEntry(fwd1c, "FWD1", "p");
    leg3->AddEntry(fwd2c, "FWD2", "p");
    leg3->AddEntry(fwd3c, "FWD3", "p");

    leg3->Draw();

    text->DrawLatex(0.21, 0.89, ("Run " + runnr).c_str());

    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_mean2.pdf");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_mean2.png");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_mean2.eps");
    can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/phpds_mean2.jpg");


    return 0;
}
