
TH1F* set_line_width4(TH1F* wf)
{
    TH1F* clone = (TH1F*) wf->Clone("clone");
    clone->SetLineWidth(4);
    return clone;
}

TF1* set_line_width4f(TF1* wf)
{
    TF1* clone = (TF1*) wf->Clone("clone");
    clone->SetLineWidth(4);
    return clone;
}

int plt_phpds()
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
    double markeralpha = 1.0;
    double linealpha = 0.6;
    double linewidth = 1;

    // claws default
    // int c1 = claws_red->GetNumber();
    // int c3 = claws_gray->GetNumber();
    // int c2 = claws_black->GetNumber();

    // Katha 112
    // int c1 = TColor::GetColor("#ee0000"); //rot
    // int c2 = TColor::GetColor("#aaccff");
    // int c3 = TColor::GetColor("#550144");

    // // Tableau 20
    int c1 = claws_black->GetNumber();
    int c2 = claws_red->GetNumber();
    int c3 = claws_gray->GetNumber();
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
    double yup  = 2.99;

    int nbins = 1000;
    TH2D* axis = new TH2D("axis", "axis",nbins, -100 , 1000 , nbins , -1000, 1000);
    axis->GetXaxis()->SetRangeUser(xlow,xup);
    axis->GetYaxis()->SetLimits(ylow,yup);


    axis->GetXaxis()->SetTitleOffset(1.1);
    axis->GetXaxis()->SetTitle("T - T_{Run Start} [s]");
    axis->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Amplitude [mV]");


    TLine *line = new TLine(xlow,0,xup,0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    // calibration file
    string path  = "./Calibration/Data/";

    string fname = "run_401141_pedestal_ph1v4_physics.root";

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


    // legend
    TLegend* leg =  new TLegend(0.49, 0.86, 0.925, 0.945);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    leg->SetNColumns(3);
    leg->AddEntry(fwd1, "FWD1", "lp");
    leg->AddEntry(fwd2, "FWD2", "lp");
    leg->AddEntry(fwd3, "FWD3", "lp");

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(vis_multi*textsize*pt2pixel*0.8);


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

    TGraphErrors* fwd2c = (TGraphErrors*) fwd2->Clone("fwd2c");

    fwd2c->SetMarkerColorAlpha(c2, 1.);
    fwd2c->SetMarkerStyle(26);
    fwd2c->Draw("same px");

    TGraphErrors* fwd3c = (TGraphErrors*) fwd3->Clone("fwd3c");

    fwd3c->SetMarkerColorAlpha(c3, 1.);
    fwd3c->SetMarkerStyle(24);
    fwd3c->Draw("same px");


    axis->Draw("SAME AXIS");

    // line->Draw("same");

    leg->Draw("same");

    TLegend* leg2 = (TLegend*) leg->Clone("leg2");
    leg2->Clear();

    leg2->AddEntry(fwd1c, "FWD1", "p");
    leg2->AddEntry(fwd2c, "FWD2", "p");
    leg2->AddEntry(fwd3c, "FWD3", "p");

    leg2->Draw();

    text->DrawLatex(0.21, 0.89, ("Run " + runnr).c_str());

    can->SaveAs("./Calibration/phpds_mean.pdf");
    can->SaveAs("./Calibration/phpds_mean.png");
    can->SaveAs("./Calibration/phpds_mean.eps");
    can->SaveAs("./Calibration/phpds_mean.jpg");


    return 0;
}
