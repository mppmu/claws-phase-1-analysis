


int plt_finalwf()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel);
    double ratio = 2./1.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);
    can->SetRightMargin(0.16);
    can->SetTicky(0);
    //can->SetLogy();
    can->SetLeftMargin(0.08);
    can->SetRightMargin(0.08);

    // can->SetBottomMargin(0.16);

    int length   = 240;
    int start   = 159452;

    // double xlow = -3.49;
    // double xup  = 5.49;
    // double ylow = -19.99;
    // double yup  = 39.99;
    double ylow = -20.99;
    double yup  = 169.99;
    int ndiv = 506;

    double scale = 1e-6;

    string path = "./Calibration/Data/";
//    string fname = "PhysicsEvent_401141072_calibrated.root";
    string fname = "PhysicsEvent_401141072_pd_subtracted.root";

    string evtnr = fname.substr(13,9);
//
    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1F* org = (TH1F*) rfile->Get("FWD1");

    int nbins = org->GetNbinsX();
    double xmin = org->GetBinLowEdge(1)/scale;
    double xmax = (org->GetBinLowEdge(nbins)+org->GetBinWidth(nbins))/scale;
    org->SetBins(nbins, xmin, xmax);


    //     TH1F *axis = new TH1F("axis","axis", length, - 0.4, length*0.8 - 0.4);
    org->GetYaxis()->SetRangeUser(ylow, yup);
    // //    axis->GetXaxis()->SetTitleOffset(1.3);
    org->GetXaxis()->SetTitle("Time [#mus]");
    org->GetYaxis()->SetTitle("Amplitude [mV]");
    org->GetYaxis()->SetTitleOffset(0.8);
    org->GetYaxis()->SetNdivisions(ndiv);
    org->GetYaxis()->SetTickLength( org->GetXaxis()->GetTickLength()/ratio );


    // MIP waveform
    string fname2 = "PhysicsEvent_401141072_calibrated.root";
    TFile* rfile2 = new TFile((path+fname2).c_str(), "open");

    TH1F* mip = (TH1F*) rfile2->Get("FWD1_mip");

    nbins = mip->GetNbinsX();
    xmin = mip->GetBinLowEdge(1)/scale;
    xmax = (mip->GetBinLowEdge(nbins)+mip->GetBinWidth(nbins))/scale;

    mip->SetBins(nbins, xmin, xmax);


    int corg = kBlack;
    int cmip = claws_red->GetNumber();
    int linewidth = 2 ;

    org->SetLineColorAlpha(corg,1);
    org->SetLineWidth(1);


    mip->SetLineColorAlpha(cmip,1);
    mip->SetMarkerColorAlpha(cmip,1);
    mip->SetMarkerStyle(21);
    mip->SetMarkerSize(1.3);
    mip->SetLineWidth(linewidth);

     TLegend* leg =  new TLegend(0.535, 0.86, 0.89, 0.945);
     leg->SetBorderSize(0);
     leg->SetFillColor(0);
     leg->SetFillStyle(0);
     leg->SetNColumns(2);

     leg->AddEntry(org, "Original waveform", "l");
     leg->AddEntry(mip, "MIP waveform", "l");
     // leg->AddEntry(pem, "Photon waveform", "l");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

//      // Plotting
       org->Draw("AXIS");

       org->Draw("hist same ");


      leg->Draw();
       text->DrawLatex(0.11, 0.89, ("Evt. "+evtnr +": FWD1").c_str());

       can->Update();

       //Float_t rightmax = 1.168*pem->GetMaximum();
    Float_t rightmax =2*mip->GetMaximum();
    cout << "rightmax: " << rightmax << endl;
    cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
    Float_t scale2 = gPad->GetUymax()/rightmax;
    cout << scale2 << endl;
    mip->Scale(scale2);



     TH1F *err = (TH1F*) mip->Clone("err");
     err->SetMarkerSize(0);
     err->SetFillStyle(3144);
     //err->SetFillStyle(1001);
     //err->SetFillColorAlpha(cerr,1);
     err->SetFillColorAlpha(tab20_blue->GetNumber(),1);
     err->SetLineWidth(1);
     err->SetLineColorAlpha(tab20_blue->GetNumber(),1);

    // err->Draw("same E2");
    mip->Draw("hist same ][");
    //err->Draw("same E1 X0");
    //mip->Draw("same EP");
    // err->Draw("same E2");

     TGaxis *axis2 = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),ylow/yup*rightmax,rightmax,510,"+L");
     axis2->SetLineColor(cmip);
     axis2->SetTextColor(cmip);
     axis2->SetLabelColor(cmip);
     axis2->SetTitle("Particles [MIP/0.8 ns]");
     axis2->SetTitleOffset(0.6);
     axis2->SetNdivisions(ndiv);

     double fontsize = org->GetYaxis()->GetTitleSize();
     int font     = org->GetYaxis()->GetTitleFont();

     axis2->SetTitleFont(font);
     axis2->SetLabelFont(font);
     axis2->SetTitleSize(fontsize);
     axis2->SetLabelSize(fontsize);
       //gStyle->SetPadTickY(1);
     axis2->SetTickLength( org->GetXaxis()->GetTickLength()/ratio );
     axis2->Draw();

     cout << "err entries: " << err->GetEntries() << endl;

    can->SaveAs("./Calibration/finalwf.pdf");
    can->SaveAs("./Calibration/finalwf.png");
    can->SaveAs("./Calibration/finalwf.eps");
    can->SaveAs("./Calibration/finalwf.jpg");

    return 0;
}
