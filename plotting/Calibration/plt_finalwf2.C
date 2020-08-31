TH1F* set_line_width4(TH1F* wf)
{
    TH1F* clone = (TH1F*) wf->Clone("clone");
    clone->SetLineWidth(3);
    return clone;
}

TF1* set_line_width4f(TF1* wf)
{
    TF1* clone = (TF1*) wf->Clone("clone");
    clone->SetLineWidth(4);
    return clone;
}


int plt_finalwf2()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel);
    //double ratio = 2/1.;
    double ratio = 1./0.66;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    double padsplit = 0.5;


    double rightmargin = 0.08;
    double leftmargin = 0.08;
    double topmargin = 0.02;
    double bottommargin = 0.14;
    double yoffset = 2;
    double xoffset = 2.4;

    TPad* pad1 = new TPad("pad1", "The pad 80% of the height", 0.0, 0., 1, 1);
    pad1->SetLeftMargin( leftmargin );
    pad1->SetBottomMargin( 0.09 );
    pad1->SetRightMargin(0.07);
    pad1->SetTicky(0);
    //pad1->SetLeftMargin( leftmargin );

    pad1->Draw();

    TPad* pad2 = new TPad("pad2", "The pad 20% of the height", 0.32, 0.37, 0.9, 0.92);
    //pad2->SetLeftMargin(4);
    pad2->SetBottomMargin(0.13);
    pad2->SetRightMargin(0.12);
    pad2->SetTicky(0);

    pad2->Draw();




    // --- Constants and colors etc. --------------------------------------------------------------------------------------------------------------------------

    // int length   = 240;
    // int start   = 159452;
    double dt = 0.8;
    // double xlow = -3.49;
    // double xup  = 5.49;
    // double ylow = -19.99;
    // double yup  = 39.99;
    double ylow = -20.99;
    double yup  = 169.99;
    int ndiv = 506;

    double scale = 1e-3;

    int corg = kBlack;
    int cmip = claws_red->GetNumber();
    int linewidth = 2 ;

    string path = "./Calibration/Data/";
//    string fname = "PhysicsEvent_401141072_calibrated.root";

    // --- Big content --------------------------------------------------------------------------------------------------------------------------------------

    // org.
    string fname = "PhysicsEvent_401141041_prepared.root";

    string evtnr = fname.substr(13,9);

    TFile* rfile = new TFile((path+fname).c_str(), "open");

    TH1F* org = (TH1F*) rfile->Get("FWD1");

    int nbins = org->GetNbinsX();
    double xmin = org->GetBinLowEdge(1)/scale;
    double xmax = (org->GetBinLowEdge(nbins)+org->GetBinWidth(nbins))/scale;
    org->SetBins(nbins, xmin, xmax);


    // MIP waveform
    string fname2 = "PhysicsEvent_401141041_calibrated.root";
    TFile* rfile2 = new TFile((path+fname2).c_str(), "open");

    TH1F* mip = (TH1F*) rfile2->Get("FWD1_mip");

    nbins = mip->GetNbinsX();
    xmin = mip->GetBinLowEdge(1)/scale;
    xmax = (mip->GetBinLowEdge(nbins)+mip->GetBinWidth(nbins))/scale;

    mip->SetBins(nbins, xmin, xmax);



    org->GetYaxis()->SetRangeUser(-24.9, 499.99);
    org->GetXaxis()->SetTitle("Time [ms]");
    org->GetYaxis()->SetTitle("Amplitude [mV]");

    org->GetYaxis()->SetTitleOffset(1.);
    org->GetYaxis()->SetNdivisions(ndiv);
    org->GetYaxis()->SetTickLength( org->GetXaxis()->GetTickLength()/ratio );
    org->SetLineColorAlpha(corg,1);
    org->SetLineWidth(1);


    mip->SetLineColorAlpha(cmip,1);
    mip->SetMarkerColorAlpha(cmip,1);
    mip->SetMarkerStyle(21);
    mip->SetMarkerSize(1.3);
    mip->SetLineWidth(linewidth);


     TLegend* leg =  new TLegend(0.11, 0.775, 0.22, 0.875);
     leg->SetBorderSize(0);
     leg->SetFillColor(0);
     leg->SetFillStyle(0);

     leg->AddEntry(set_line_width4(org), "Original waveform", "l");
     leg->AddEntry(set_line_width4(mip), "MIP waveform", "l");
     // leg->AddEntry(pem, "Photon waveform", "l");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    // --- Small content --------------------------------------------------------------------------------------------------------------------------------------
    int length = 300;
    int shift = 0;

    for (size_t i = 1; i <= org->GetNbinsX(); i++)
    // for (size_t i = 2000000; i <= ph->GetNbinsX(); i++)
    {
        if( org->GetBinContent(i) > 100 )
        {
            shift = i-int(50/0.8);
            break;
        }
    }

    TH1F* org_small = new TH1F("org small", "", length,  -dt/2., dt*length - dt/2.);

    TH1F* mip_small = new TH1F("mip small ", "", length,  -dt/2., dt*length - dt/2.);

    for (size_t i = 1; i <= length; i++)
    {
        double cont = org->GetBinContent(i+shift);

        org_small->SetBinContent(i, org->GetBinContent(i+shift));
        mip_small->SetBinContent(i, mip->GetBinContent(i+shift));
        mip_small->SetBinError(i, mip->GetBinError(i+shift));

    }


    org_small->GetYaxis()->SetRangeUser(-24.9, 379.99);
    org_small->GetXaxis()->SetTitle("Time [ns]");
    org_small->GetYaxis()->SetTitle("Amplitude [mV]");

    org_small->GetXaxis()->SetTitleOffset(1.8);
    org_small->GetYaxis()->SetTitleOffset(1.6);

    org_small->GetYaxis()->SetNdivisions(ndiv);

    org_small->SetLineColorAlpha(corg,1);
    org_small->SetLineWidth(2);


    mip_small->SetLineColorAlpha(cmip,1);
    mip_small->SetMarkerColorAlpha(cmip,1);
    mip_small->SetMarkerStyle(21);
    mip_small->SetMarkerSize(1.3);
    mip_small->SetLineWidth(linewidth);


         TLegend* lsmall =  new TLegend(0.5, 0.775, 0.8, 0.875);
         lsmall->SetBorderSize(0);
         lsmall->SetFillColor(0);
         lsmall->SetFillStyle(0);

         lsmall->AddEntry(set_line_width4(org_small), "Original waveform", "l");
         lsmall->AddEntry(set_line_width4(mip_small), "MIP waveform", "l");
         lsmall->SetTextSize(0.8*gStyle->GetLegendTextSize());

    TLatex* tsmall = new TLatex();
    tsmall->SetNDC();
    tsmall->SetTextFont(leg->GetTextFont()+20);
    tsmall->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    // --- Top draw --------------------------------------------------------------------------------------------------------------------------------------------
    pad1->cd();

    org->DrawCopy("AXIS");

    org->Draw("hist L same ][");


    leg->Draw();
    text->DrawLatex(0.11, 0.89, ("Evt. "+evtnr +": FWD1").c_str());

    pad1->Update();

    cout << "Mip max: " << mip->GetMaximum() << endl;
       //Float_t rightmax = 1.168*pem->GetMaximum();
    Float_t rightmax =2*mip->GetMaximum();
    cout << "rightmax: " << rightmax << endl;
    cout << "gPad->GetUymax(): " << pad1->GetUymax() << endl;
    Float_t scale2 = pad1->GetUymax()/rightmax;
    cout << scale2 << endl;

    cout << "Mip max2: " << mip->GetMaximum() << endl;
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
    // err->Draw("same E2");-24.9, 499.99

     TGaxis *axis2 = new TGaxis(pad1->GetUxmax(),pad1->GetUymin(),pad1->GetUxmax(), pad1->GetUymax(), -24.99/499.99*rightmax,rightmax,510,"+L");
     axis2->SetLineColor(cmip);
     axis2->SetTextColor(cmip);
     axis2->SetLabelColor(cmip);
     axis2->SetTitle("Particles [MIP/0.8 ns]");
     axis2->SetTitleOffset(0.85);
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

     // --- Top draw --------------------------------------------------------------------------------------------------------------------------------------------
     pad2->cd();

     org_small->DrawCopy("AXIS");

     org_small->Draw("hist L same ][");

     lsmall->Draw();
     tsmall->DrawLatex(0.45, 0.89, ("Zoom into Evt. "+evtnr +": FWD1").c_str());


     pad2->Update();


        //Float_t rightmax = 1.168*pem->GetMaximum();
     Float_t rightmax2 =2*mip_small->GetMaximum();

     Float_t scale3 = pad2->GetUymax()/rightmax2;

     mip_small->Scale(scale3);

     mip_small->Draw("hist same ][");

     TGaxis *axis3 = new TGaxis(pad2->GetUxmax(),pad2->GetUymin(),pad2->GetUxmax(), pad2->GetUymax(), -24.99/379.99*rightmax2,rightmax2,510,"+L");
     axis3->SetLineColor(cmip);
     axis3->SetTextColor(cmip);
     axis3->SetLabelColor(cmip);
     axis3->SetTitle("Particles [MIP/0.8 ns]");
     axis3->SetTitleOffset(1.4);
     axis3->SetNdivisions(ndiv);

     double fontsize2 = org_small->GetYaxis()->GetTitleSize();
     int font2     = org_small->GetYaxis()->GetTitleFont();

     axis3->SetTitleFont(font2);
     axis3->SetLabelFont(font2);
     axis3->SetTitleSize(fontsize2);
     axis3->SetLabelSize(fontsize2);
       //gStyle->SetPadTickY(1);
     axis3->SetTickLength( org_small->GetXaxis()->GetTickLength()/ratio );
     axis3->Draw();

    can->SaveAs("./Calibration/finalwf2.pdf");
    can->SaveAs("./Calibration/finalwf2.png");
    can->SaveAs("./Calibration/finalwf2.eps");
    can->SaveAs("./Calibration/finalwf2.jpg");

    return 0;
}
