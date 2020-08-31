


int plt_calpd2()
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

    //TPad* pad = new TPad("pad", "The pad", 0, 0, 1, 1);

    // can->SetTickx(0);
    // can->SetTicky(0);
    // can->SetFrameLineWidth(0);

    // can->SetRightMargin(0.03);
    // //can->SetLeftMargin(0.14);
    // can->SetBottomMargin(0.15);
    //gStyle->SetErrorX(1);



    double markersize = 1.0;
    double markeralpha = 0.6;
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
    int c1 = TColor::GetColor(31, 119, 180); // blau
    int c2 = TColor::GetColor(255,127,14); // orange
    int c3 = TColor::GetColor(44,160,44); //
    //int c3 = TColor::GetColor(148,103,189); //lila

    // Tableau 20
    // int c1 = TColor::GetColor( 148, 103, 189);
    // int c2 = TColor::GetColor(23,190,207);
    // int c3 = TColor::GetColor( 140,86,75);

    // // root default
    // int c1 = 2;
    // int c2 = 4;
    // int c3 = 1;
    double n = 500;
    double xlow = -5;
    double xup  = n+5;
    // double ylow = -1.19;
    // double yup  = 1.99;
    double ylow = -0.79;
    double yup  = 1.29;

    TLine *line = new TLine(xlow,0,xup,0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    // calibration file
    string path  = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/PDS_Calibration/";

    string fname = "run_401141_pedestal_ph1v6.root";


    string runnr = fname.substr(4,6);

    TFile* rfile = new TFile((path + fname).c_str(), "open");

    TGraphErrors* tmp1 = (TGraphErrors*) rfile->Get("FWD1_fit_mean");
    TGraphErrors* tmp2 = (TGraphErrors*) rfile->Get("FWD2_fit_mean");
    TGraphErrors* tmp3 = (TGraphErrors*) rfile->Get("FWD3_fit_mean");


    TGraphErrors* fwd1_fit_mean = new TGraphErrors();
    TGraphErrors* fwd2_fit_mean = new TGraphErrors();
    TGraphErrors* fwd3_fit_mean = new TGraphErrors();

    double x1,y1,x2,y2,x3,y3;

    int n_points = 0;

    for(int i = 0; i <  1000; ++i )
    {
        tmp1->GetPoint(i, x1, y1);
        tmp2->GetPoint(i, x2, y2);
        tmp3->GetPoint(i, x3, y3);

        if( fabs(y1) < 1e-15 or fabs(y2) < 1e-15 or fabs(y3) < 1e-15)
        {
            cout << y2 << endl;
            continue;
        }

        fwd1_fit_mean->SetPoint(n_points, n_points, y1);
        fwd1_fit_mean->SetPointError(n_points, 0., tmp1->GetErrorY(i));

        fwd2_fit_mean->SetPoint(n_points, n_points, y2);
        fwd2_fit_mean->SetPointError(n_points, 0., tmp2->GetErrorY(i));

        fwd3_fit_mean->SetPoint(n_points, n_points, y3);
        fwd3_fit_mean->SetPointError(n_points, 0., tmp3->GetErrorY(i));

        n_points++;
        if(n_points > n) break;
    }

    fwd1_fit_mean->SetMarkerColorAlpha( c1, markeralpha-0.3);
    //fwd1_fit_mean->SetFillColorAlpha( claws_red->GetNumber(), 0.75);
    fwd1_fit_mean->SetLineColorAlpha( c1, linealpha-0.3);
    fwd1_fit_mean->SetLineWidth(linewidth);
    fwd1_fit_mean->SetMarkerStyle(21);
    fwd1_fit_mean->SetMarkerSize( markersize );

    fwd1_fit_mean->GetXaxis()->SetLimits(xlow, xup);
    //fwd1_fit_mean->GetXaxis()->SetTitleOffset(1.1);
    fwd1_fit_mean->GetXaxis()->SetTitle("Event Number");
    fwd1_fit_mean->GetYaxis()->SetRangeUser(ylow, yup);
    //fwd1_fit_mean->GetYaxis()->SetTitleOffset(1.4);
    fwd1_fit_mean->GetYaxis()->SetTitle("Pedestal [mV]");

    // tmp = (TGraphErrors*) rfile->Get("FWD2_fit_mean");
    // TGraphErrors* fwd2_fit_mean = new TGraphErrors();
    //
    // for(int i = 0; i <  n; ++i )
    // {
    //     tmp->GetPoint(i, x, y);
    //     fwd2_fit_mean->SetPoint(i, i, y);
    //     fwd2_fit_mean->SetPointError(i, 0, tmp->GetErrorY(i));
    // }

    fwd2_fit_mean->SetMarkerColorAlpha(c2, markeralpha);
    fwd2_fit_mean->SetLineColorAlpha( c2, linealpha);

    fwd2_fit_mean->SetLineWidth(linewidth);
    fwd2_fit_mean->SetMarkerStyle(22);
    fwd2_fit_mean->SetMarkerSize( markersize );

    // tmp = (TGraphErrors*) rfile->Get("FWD3_fit_mean");
    // TGraphErrors* fwd3_fit_mean = new TGraphErrors();
    //
    // for(int i = 0; i <  n; ++i )
    // {
    //     tmp->GetPoint(i, x, y);
    //     fwd3_fit_mean->SetPoint(i, i, y);
    //     fwd3_fit_mean->SetPointError(i, 0, tmp->GetErrorY(i));
    // }

    fwd3_fit_mean->SetMarkerColorAlpha(c3, markeralpha);
    fwd3_fit_mean->SetLineColorAlpha( c3, linealpha);
    fwd3_fit_mean->SetLineWidth(linewidth);
    fwd3_fit_mean->SetMarkerStyle(20);
    fwd3_fit_mean->SetMarkerSize( markersize );

    // legend

    TLegend* leg =  new TLegend(0.45, 0.86, 0.925, 0.945);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    leg->SetNColumns(3);


    TGraphErrors* fwd1_fit_mean_m = (TGraphErrors*) fwd1_fit_mean->Clone("fwd1_fit_mean");
    fwd1_fit_mean_m->SetLineWidth(linewidth+1);
    fwd1_fit_mean_m->SetMarkerSize(markersize+0.3);

    leg->AddEntry(fwd1_fit_mean_m, "FWD1", "lp");

    TGraphErrors* fwd2_fit_mean_m = (TGraphErrors*) fwd2_fit_mean->Clone("fwd2_fit_mean");
    fwd2_fit_mean_m->SetLineWidth(linewidth+1);
    fwd2_fit_mean_m->SetMarkerSize(markersize+0.3);
    leg->AddEntry(fwd2_fit_mean_m, "FWD2", "lp");

    TGraphErrors* fwd3_fit_mean_m = (TGraphErrors*) fwd3_fit_mean->Clone("fwd3_fit_mean");
    fwd3_fit_mean_m->SetLineWidth(linewidth+1);
    fwd3_fit_mean_m->SetMarkerSize(markersize+0.3);
    leg->AddEntry(fwd3_fit_mean_m, "FWD3", "lp");




    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();


    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(vis_multi*textsize*pt2pixel*0.8);


    // --- Now start the actual drawing ---

    //axis

//    fwd1_fit_mean->Draw("axis");


    //fwd1_fit_mean->Draw("a");

    fwd1_fit_mean->Draw("ap");

    line->Draw("same");


    //fwd1
    fwd1_fit_mean->Draw("same p");

    TGraphErrors* fwd1_fit_mean2 = (TGraphErrors*) fwd1_fit_mean->Clone("fwd1_fit_mean2");
    fwd1_fit_mean2->SetMarkerColorAlpha(c1, 1.);
    // fwd1_fit_mean2->SetLineWidth(2);
    fwd1_fit_mean2->SetMarkerStyle(25);
    //fwd1_fit_mean2->SetMarkerColorAlpha(kBlack,1);
    fwd1_fit_mean2->Draw("same px");
    //fwd3
    fwd3_fit_mean->Draw("same p");

    TGraphErrors* fwd3_fit_mean2 = (TGraphErrors*) fwd3_fit_mean->Clone("fwd3_fit_mean2");
    fwd3_fit_mean2->SetMarkerColorAlpha(c3, 1.);
    // fwd1_fit_mean2->SetLineWidth(2);
    fwd3_fit_mean2->SetMarkerStyle(24);
    //fwd3_fit_mean2->SetMarkerColorAlpha(kBlack,1);
    fwd3_fit_mean2->Draw("same p");



    //fwd2
    fwd2_fit_mean->Draw("same p");

    TGraphErrors* fwd2_fit_mean2 = (TGraphErrors*) fwd2_fit_mean->Clone("fwd2_fit_mean2");
    fwd2_fit_mean2->SetMarkerColorAlpha(c2, 1.);
    // fwd1_fit_mean2->SetLineWidth(2);
    fwd2_fit_mean2->SetMarkerStyle(26);
    //fwd2_fit_mean2->SetMarkerColorAlpha(kBlack,1);
    fwd2_fit_mean2->Draw("same px");

    leg->Draw("same");

    TLegend* leg2 = (TLegend*) leg->Clone("leg2");
    leg2->Clear();

    TGraphErrors* fwd1_fit_mean2_m = (TGraphErrors*) fwd1_fit_mean2->Clone("fwd1_fit_mean2");
    fwd1_fit_mean2_m->SetLineWidth(linewidth+1);
    fwd1_fit_mean2_m->SetMarkerSize(markersize+0.3);
    //fwd1_fit_mean2_m->SetMarkerColorAlpha(kBlack,1);
    leg2->AddEntry(fwd1_fit_mean2_m, "FWD1", "p");

    TGraphErrors* fwd2_fit_mean2_m = (TGraphErrors*) fwd2_fit_mean2->Clone("fwd2_fit_mean2");
    fwd2_fit_mean2_m->SetLineWidth(linewidth+1);
    fwd2_fit_mean2_m->SetMarkerSize(markersize+0.3);
    //fwd2_fit_mean2_m->SetMarkerColorAlpha(kBlack,1);
    leg2->AddEntry(fwd2_fit_mean2_m, "FWD2", "p");

    TGraphErrors* fwd3_fit_mean2_m = (TGraphErrors*) fwd3_fit_mean2->Clone("fwd3_fit_mean2");
    fwd3_fit_mean2_m->SetLineWidth(linewidth+1);
    fwd3_fit_mean2_m->SetMarkerSize(markersize+0.3);
    //fwd3_fit_mean2_m->SetMarkerColorAlpha(kBlack,1);
    leg2->AddEntry(fwd3_fit_mean2_m, "FWD3", "p");

    leg2->Draw();

    text->DrawLatex(0.21, 0.89, ("Run " + runnr).c_str());

//    fwd1_fit_mean->Draw("same axis");

can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_mean.pdf");
can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_mean.png");
can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_mean.eps");
can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/pds_calibration_mean.jpg");


    return 0;
}
