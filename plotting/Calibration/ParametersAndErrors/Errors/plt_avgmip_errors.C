TH1F* set_line_width(TH1F* wf, int width = 4)
{
    TH1F* clone = (TH1F*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}

TH1I* set_line_width(TH1I* wf, int width = 4)
{
    TH1I* clone = (TH1I*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}

TF1* set_line_width_f(TF1* wf, int width = 4)
{
    TF1* clone = (TF1*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}


void plt_avgmip_errors()
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

//    can->SetLogy();
    TPad* pad1 = new TPad("pad1", "The pad 80% of the height", 0.0, 0.5, 1, 1);
    // pad1->SetLeftMargin( leftmargin );
    // pad1->SetBottomMargin( 0.09 );
    // pad1->SetRightMargin(0.07);
    // pad1->SetTicky(0);
    //pad1->SetLeftMargin( leftmargin );

    pad1->Draw();

    TPad* pad2 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.0, 1, 0.5);
    //pad2->SetLeftMargin(4);
    // pad2->SetBottomMargin(0.13);
    // pad2->SetRightMargin(0.12);
    // pad2->SetTicky(0);
    //
    // pad2->Draw();

    // -----   Axis   ---------------------

     double pe2mip = 15.65;
     double sigma_pe2mip = 0.21;
     double sigma_corr = 0.10;
     double corr = 0.153978;

     double npe = (1-corr)*pe2mip;
     // double 1mip = 1;

     double mip1_sig_stat = sqrt(npe)/pe2mip;
     double mip1_sig_sys = sigma_pe2mip*npe/(pe2mip*pe2mip) + sigma_corr;
     double mip1_sig= sqrt(mip1_sig_stat*mip1_sig_stat+ mip1_sig_sys*mip1_sig_sys);
    // double binwidth = 1/pe2mip;
    // double npe = 3*15+10;
    // double nbins = npe + 1;

    double n_wf = 100;
    double nmip = 3;
    double nbins = nmip*10 +1;


    // double xlow = - binwidth*0.5;
    // double xup = nbins*binwidth - 0.5;

    // double ylow = 0.0;
    // double yup = 70;


    TH1F* axis = new TH1F("axis", "axis", nbins, -0.5, 3.5);

//    axis->GetXaxis()->SetRangeUser(xlow, xup);
    //    fwd1->GetXaxis()->SetTitleOffset(1.1);
    axis->GetXaxis()->SetTitle("Deposited Energy [MIP]");
    axis->GetXaxis()->SetNdivisions(505);
    axis->GetYaxis()->SetNdivisions(505);
//    axis->GetYaxis()->SetRangeUser(ylow, yup);
    //    fwd1->GetYaxis()->SetTitleOffset(1.4);
    axis->GetYaxis()->SetTitle("Relative Energy Uncertainty [%]");

    // -----   Colors and Style   ---------------------
    double linewidth = 2;

    int cstat = tab20_blue->GetNumber();
    int csys = tab20_orange->GetNumber();
    int cerr = tab20_green->GetNumber();
 //    // int c3 = claws_black->GetNumber();
 //    // int cfit = TColor::GetColor("#2ca02c");
 //    // int cfit = kAzure+1;
 //    int cfit = claws_red->GetNumber();
 //    // // Katha 112
 //
 //
 //
     // -----   Functions   ---------------------
     string path = "./Calibration/ParametersAndErrors/Errors/";
 //    string fname = "run_900122_systematics_ph1v4.root";
 //
 //    string evtnr = fname.substr(4,6);

    //TF1* stat_error = new TF1("stat_error","sqrt(x)/15.65",xlow,xup);

    //
    // TF1* sys_error = new TF1("sys_error","x/15.65**2 *0.21 + 0.102713",xlow,xup);

     TH1F* stat_error = new TH1F("stat_error","stat_error", nbins, -0.5, 3.5);
//     stat_error->SetLineColor(cstat);
//     stat_error->SetLineWidth(linewidth);
// //    TH1F* stat_error_rel = (TH1F*) stat_error->Clone("");
//
//     TH1F* sys_error = new TH1F("sys_error","sys_error", nbins, xlow,xup);
//     sys_error->SetLineColor(csys);
//     sys_error->SetLineWidth(linewidth);
//
//     TH1F* error = new TH1F("error","error", nbins, xlow,xup);
//     error->SetLineColor(cerr);
//     error->SetLineWidth(linewidth);


    for( int bin = 1; bin <= nbins; ++bin)
    {
        double avgenergy = (bin-1)/10.;
        double energy = avgenergy *n_wf;
        cout << energy << endl;
        // double staterr = sqrt(mip*mip1_sig_stat*mip1_sig_stat)/n_wf;
        // stat_error->SetBinContent(bin, staterr);
//         double alpha = (bin-1)/pe2mip + corr;
//         double stat = sqrt(bin-1)/pe2mip;
//         stat_error->SetBinContent(bin, stat*100/alpha);
//         double sys = (bin-1)*sigma_pe2mip/(pe2mip*pe2mip) + sigma_corr;
//         sys_error->SetBinContent(bin, sys*100/alpha);
//         double tot = sqrt(stat*stat+ sys*sys);
//         error->SetBinContent(bin, tot*100/alpha);
    }

//      // -----   Legend   ---------------------

    TLegend* leg =  new TLegend(0.58, 0.68, 0.84, 0.86);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);


    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

//     leg->AddEntry(set_line_width(stat_error,3), "Stat. Unc. #sigma_{MIP_{stat.}}", "l");
//     leg->AddEntry(set_line_width(sys_error,3), "Sys. Unc. #sigma_{MIP_{sys.}}", "l");
//     leg->AddEntry(set_line_width(error,3), "Tot. Unc. #sigma_{MIP}", "l");
//
    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // ------   Drawing   --------------------

    axis->Draw("axis");

    stat_error->Draw("same");
    // sys_error->Draw("same");
    // error->Draw("same");

//     leg->Draw();
    // leg2->Draw();

    text->DrawLatex(0.22, 0.88, "Averaged MIP Waveform Uncertainties: FWD1");

    can->SaveAs((path+"avgmip_errors.pdf").c_str());
    can->SaveAs((path+"avgmip_errors.png").c_str());
    can->SaveAs((path+"avgmip_errors.eps").c_str());
    can->SaveAs((path+"avgmip_errors.jpg").c_str());
}
