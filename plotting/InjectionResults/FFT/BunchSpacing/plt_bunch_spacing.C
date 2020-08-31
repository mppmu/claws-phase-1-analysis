

int wide(string channel = "FWD1")
{
    Target target = GetTarget("ALIGNED_VS_STAT_FULL_INJ_NON_125e5_HER_5_LER_5");
    //Target target = GetTarget("VC_STAT_FULL_INJ_NON_3e6_HER_5_LER_5");

    // Both rings
    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");


	TH1F* fft = (TH1F*) rfile->Get((channel+"_fft_mag_h").c_str());
	fft->SetDirectory(0);

    // double scale = 1e-9;
    // int nbinsx = fft->GetNbinsX();
    // double xlow = fft->GetBinLowEdge(1);
    // double xup = fft->GetBinLowEdge(nbinsx)+fft->GetBinWidth(nbinsx);
    // fft->SetBins(nbinsx, xlow*scale, xup*scale);


    // ---- Do the Axis ---
    fft->GetXaxis()->SetTitle("Frquency [Hz]");
    cout << "Max: " << fft->GetMaximumBin() << endl;
//    fft->GetYaxis()->SetRangeUser(0, fft->GetMaximum()*1.5 );

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "her_injections_hit_energy_spectrum_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLogx();
    can->SetLogy();

    fft->Draw("HIST");

    //can->SaveAs((target.pathout+"/"+target.fileout+"_bs_wide_"+channel+".pdf").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_bs_wide_"+channel+".png").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_bs_wide_"+channel+".eps").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_bs_wide_"+channel+".jpg").c_str());


    return 0;
}

void zoom(string channel = "FWD1")
{

    // Set some standards
    double linewidth = 2;
    int linestyle =7;

    int chist = kBlack;
    int cfit  = cher->GetNumber();
    int cline = tab20_gray->GetNumber();

    double nsig = 2;



    Target target = GetTarget("ALIGNED_VS_STAT_FULL_INJ_NON_125e5_HER_5_LER_5");
    //Target target = GetTarget("VC_STAT_FULL_INJ_NON_3e6_HER_5_LER_5");

    // Both rings
    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");


	TH1F* fft = (TH1F*) rfile->Get((channel+"_fft_mag_h").c_str());
	fft->SetDirectory(0);

    double scale = 1e-6;
    int nbinsx = fft->GetNbinsX();
    double xlow = fft->GetBinLowEdge(1);
    double xup = fft->GetBinLowEdge(nbinsx)+fft->GetBinWidth(nbinsx);
    fft->SetBins(nbinsx, xlow*scale, xup*scale);

    double maxX = 0;
    int maxbin =0;
    double max = 0;
    for(int i = 2; i<fft->GetNbinsX(); ++i)
    {
        if(fft->GetBinContent(i)>max)
        {
            max = fft->GetBinContent(i);
            maxbin = i;
            maxX = fft->GetBinCenter(i);
        }
    }
    cout << "Max: " << max << ", @: " << maxX << endl;

    // Fit

    TF1* gaus = new TF1("gaus", "gaus",0, 1e3);
    gaus->SetNpx(2000);
    gaus->SetLineColor(cfit);
    gaus->SetLineWidth(linewidth+1);
    gaus->SetParameters(max, maxX, 2.5e-4);
    fft->Fit(gaus,"NL", "", maxX - 2.5e-4, maxX+2.5e-4 );

    TFitResultPtr r = fft->Fit(gaus,"NL","",gaus->GetParameter(1)-nsig*gaus->GetParameter(2), gaus->GetParameter(1)+nsig*gaus->GetParameter(2));

    cout << channel << ": " << gaus->GetProb() << endl;
    double frmin = gaus->GetParameter(1)-nsig*gaus->GetParameter(2);
    double frmax = gaus->GetParameter(1)+nsig*gaus->GetParameter(2);

    gaus->SetRange(frmin, frmax);

    // ---- Do the Axis ---
    fft->GetXaxis()->SetNdivisions(506);
    fft->GetXaxis()->SetRangeUser(maxX*0.99998,maxX*1.00002);
    fft->GetXaxis()->SetTitle("Frquency [MHz]");
    cout << "Max: " << fft->GetMaximumBin() << endl;
//    fft->GetYaxis()->SetRangeUser(0, fft->GetMaximum()*1.5 );
    // Y-AXIS
    fft->GetYaxis()->SetRangeUser(0, 2*max);
    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "bunch_spacing_zoom_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//    can->SetLogx();
//    can->SetLogy();

    fft->Draw("HIST");
    gaus->Draw("same");
    can->SaveAs((target.pathout+"/"+target.fileout+"_bunch_spacing_zoom_"+channel+".pdf").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_bunch_spacing_zoom_"+channel+".png").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_bunch_spacing_zoom_"+channel+".eps").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_bunch_spacing_zoom_"+channel+".jpg").c_str());

}

void plt_bunch_spacing()
{
    SetPhDStyle();
//    wide();
    zoom();
}
