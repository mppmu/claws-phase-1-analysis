


int plt_gain_over_time()
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

    can->SetBottomMargin(0.2);

    double markersize = 1.;
    double markeralpha = 1;
    double linealpha = 1.0;
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

    string path  = "./Calibration/Data/";

    // string day = "2016-05-25";

    string fname = "gain_over_runnr_2016-06-08.root";

    string runnr = fname.substr(16,10);

    TFile* rfile = new TFile((path + fname).c_str(), "open");

    TGraphErrors* tmp = (TGraphErrors*) rfile->Get("fwd1");

    int n = tmp->GetN();
    int nbinsx = n;
    double x1,y1,x2,y2;
    tmp->GetPoint(0,x1,y1);
    tmp->GetPoint(n-1,x2,y2);
    double scale = 1e9;

    TH2D* axis = new TH2D("axis", "axis",nbinsx, x1-0.5, x2+0.5,n, 0.5*y1*scale, 1.5*y2*scale);

    double labsize = axis->GetXaxis()->GetLabelSize();
    axis->GetXaxis()->SetLabelSize(labsize*0.65);
    double x,y;
    char s[16];
    for (int i=1; i<=nbinsx; ++i) {

        tmp->GetPoint((i-1),x,y);
        sprintf(s,"Run-%d", int(x));
        axis->GetXaxis()->SetBinLabel(i, s);

    }

    double xlow = x1-0.5;
    double xup  = x2+0.5;
    double ylow = 59;
    double yup  = 80;

    axis->GetXaxis()->SetTitle("");
    axis->GetXaxis()->LabelsOption("v");
//    axis->GetXaxis()->SetRangeUser(xlow+0.5, xup-0.5);
    axis->GetYaxis()->SetLimits(ylow, yup);
        //fwd1->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Charge [10^{-15} C ]");

    // ---  FWD1 ---
    TGraphErrors* fwd1 = new TGraphErrors();
    // int i = 0;

    for(int i = 0; i < n; ++i)
    {
         tmp->GetPoint(i, x, y);
         fwd1->SetPoint(i, x, y*scale);
         fwd1->SetPointError(i, 0., tmp->GetErrorY(i)*scale);
    }

    double f1mean = fwd1->GetMean(2);
    double f1std  = 0;
    for(int i = 0; i<n;++i)
    {
        fwd1->GetPoint(i, x, y);
        f1std += (y-f1mean)*(y-f1mean);
    }

    f1std = sqrt(f1std/n);

    fwd1->SetMarkerColorAlpha( c1, markeralpha);
    //fwd1_fit_mean->SetFillColorAlpha( claws_red->GetNumber(), 0.75);
    fwd1->SetLineColorAlpha( c1, linealpha);
    fwd1->SetLineWidth(linewidth);
    fwd1->SetMarkerStyle(21);
    fwd1->SetMarkerSize( markersize );

    TLine *f1 = new TLine(xlow,f1mean,xup,f1mean);

    // TF1* f1 = new TF1("f1","[0]", xlow,xup);
    f1->SetLineColorAlpha( c1, linealpha);
    f1->SetLineWidth(linewidth);
    f1->SetLineStyle(linestyle);
    // fwd1->Fit(f1,"RS");

    // ---  FWD2 ---
    tmp = (TGraphErrors*) rfile->Get("fwd2");
    TGraphErrors* fwd2 = new TGraphErrors();
    // int i = 0;
    for(int i = 0; i < n; ++i)
    {
         tmp->GetPoint(i, x, y);
         fwd2->SetPoint(i, x, y*scale);
         fwd2->SetPointError(i, 0., tmp->GetErrorY(i)*scale);
    }

    double f2mean = fwd2->GetMean(2);
    double f2std  = 0;
    for(int i = 0; i<n;++i)
    {
        fwd2->GetPoint(i, x, y);
        f2std += (y-f2mean)*(y-f2mean);
    }

    f2std = sqrt(f2std/n);

    fwd2->SetMarkerColorAlpha( c2, markeralpha);
    //fwd2_fit_mean->SetFillColorAlpha( claws_red->GetNumber(), 0.75);
    fwd2->SetLineColorAlpha( c2, linealpha);
    fwd2->SetLineWidth(linewidth);
    fwd2->SetMarkerStyle(22);
    fwd2->SetMarkerSize( markersize );

    TLine *f2 = new TLine(xlow,f2mean,xup,f2mean);

    //TF1* f2 = new TF1("f2","[0]", xlow,xup);
    f2->SetLineColorAlpha( c2, linealpha);
    f2->SetLineWidth(linewidth);
    f2->SetLineStyle(linestyle);
//    fwd2->Fit(f2,"RS");

    // ---  FWD2 ---
    tmp = (TGraphErrors*) rfile->Get("fwd3");
    TGraphErrors* fwd3 = new TGraphErrors();

    for(int i = 0; i <  n; ++i )
    {
        tmp->GetPoint(i, x, y);
        fwd3->SetPoint(i, x, y*scale);
        fwd3->SetPointError(i, 0, tmp->GetErrorY(i)*scale);
    }

    double f3mean = fwd3->GetMean(2);
    double f3std  = 0;
    for(int i = 0; i<n;++i)
    {
        fwd3->GetPoint(i, x, y);
        f3std += (y-f3mean)*(y-f3mean);
    }

    f3std = sqrt(f3std/n);

    fwd3->SetMarkerColorAlpha(c3, markeralpha);
    fwd3->SetLineColorAlpha( c3, linealpha);
    fwd3->SetLineWidth(linewidth);
    fwd3->SetMarkerStyle(20);
    fwd3->SetMarkerSize( markersize );

    TLine *f3 = new TLine(xlow,f3mean,xup,f3mean);

//    TF1* f3 = new TF1("f3","[0]", xlow,xup);
    f3->SetLineColorAlpha( c3, linealpha);
    f3->SetLineWidth(linewidth);
    f3->SetLineStyle(linestyle);
//    fwd3->Fit(f3,"RS");

    // legend
    TLegend* leg =  new TLegend(0.21, 0.65, 0.49, 0.86);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    //leg->SetNColumns(2);

    TGraphErrors* fwd1_2 = (TGraphErrors*) fwd1->Clone("fwd1_2");
    fwd1_2->SetLineWidth(linewidth+1);
    fwd1_2->SetMarkerSize(markersize+0.3);
    leg->AddEntry(fwd1_2, "FWD1", "lp");

    TGraphErrors* fwd2_2 = (TGraphErrors*) fwd2->Clone("fwd2_2");
    fwd2_2->SetLineWidth(linewidth+1);
    fwd2_2->SetMarkerSize(markersize+0.3);
    leg->AddEntry(fwd2_2, "FWD2", "lp");

    TGraphErrors* fwd3_2 = (TGraphErrors*) fwd3->Clone("fwd3_2");
    fwd3_2->SetLineWidth(linewidth+1);
    fwd3_2->SetMarkerSize(markersize+0.3);
    leg->AddEntry(fwd3_2, "FWD3", "lp");
    // cout<< gStyle->GetLegendTextSize() << endl;
    // gStyle->SetLegendTextSize(gStyle->GetLegendTextSize()*0.9);
    // cout<< gStyle->GetLegendTextSize() << endl;



    TLegend* leg2 =  new TLegend(0.4, 0.65, 0.62, 0.86);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
//    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss1;
    ss1 << std::fixed << setprecision(1) << "#bar{FWD1} = (" << f1mean << " #pm "  << f1std << ") #times10^{-15} C";

    TLine * f1_2 = (TLine*) f1->Clone("f1_2");
    f1_2->SetLineWidth(4);
    leg2->AddEntry(f1_2, ss1.str().c_str(), "l");

    stringstream ss2;
    ss2 << fixed << setprecision(1) << "#bar{FWD2} = (" << f2mean << " #pm "  << f2std << ") #times10^{-15} C";
    TLine * f2_2 = (TLine*) f2->Clone("f2_2");
    f2_2->SetLineWidth(4);
    leg2->AddEntry(f2_2, ss2.str().c_str(), "l");

    stringstream ss3;
    ss3 << fixed << setprecision(1) << "#bar{FWD3} = (" << f3mean << " #pm "  << f3std << ") #times10^{-15} C";
    TLine * f3_2 = (TLine*) f3->Clone("f3_2");
    f3_2->SetLineWidth(4);
    leg2->AddEntry(f3_2, ss3.str().c_str(), "l");

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    //
    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // --- Now start the actual drawing ---

    //axis
    axis->Draw("axis");

    fwd1->Draw("same p");

    TGraphErrors* fwd12 = (TGraphErrors*) fwd1->Clone("fwd12");
    fwd12->SetMarkerColorAlpha(c1, 1.);
    // fwd12->SetLineWidth(2);
    fwd12->SetMarkerStyle(25);

    fwd12->Draw("same px");

    f1->Draw("same");
    //fwd2
    fwd2->Draw("same p");

    TGraphErrors* fwd22 = (TGraphErrors*) fwd2->Clone("fwd22");
    fwd22->SetMarkerColorAlpha(c2, 1.);
    // fwd1_fit_mean2->SetLineWidth(2);
    fwd22->SetMarkerStyle(26);

    fwd22->Draw("same px");

    f2->Draw("same");

    //fwd3->Draw("p");
    fwd3->Draw("same p");

    TGraphErrors* fwd32 = (TGraphErrors*) fwd3->Clone("fwd32");
    fwd32->SetMarkerColorAlpha(c3, 1.);
    // fwd12->SetLineWidth(2);
    fwd32->SetMarkerStyle(24);

    fwd32->Draw("same px");

    f3->Draw("same");

    leg->Draw("same");
    leg2->Draw();
    //line->Draw("same");
    text->DrawLatex(0.21, 0.89, ("Day " + runnr).c_str());

    axis->Draw("same axis");

    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain_over_time.pdf");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain_over_time.png");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain_over_time.eps");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/gain_over_time.jpg");

    return 0;
}
