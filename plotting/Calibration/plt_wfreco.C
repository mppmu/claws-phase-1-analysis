


int plt_wfreco()
{
    SetPhDStyle();

    // int width = round(vis_multi*textwidth*pt2pixel*1/2);
    // double ratio = 3./3.;

    int width = round(vis_multi*textwidth*pt2pixel);
    //double ratio = 21./9.;
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
    double ylow = -4.99;
    double yup  = 54.9;
    int ndiv = 506;

    TH1F *axis = new TH1F("axis","axis", length, - 0.4, length*0.8 - 0.4);
    axis->GetYaxis()->SetRangeUser(ylow, yup);
//    axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ns]");
//    axis->GetYaxis()->SetTitleOffset(0.7);
    axis->GetYaxis()->SetTitle("Amplitude [mV]");
    axis->GetYaxis()->SetNdivisions(ndiv);

    axis->GetYaxis()->SetTitleOffset(0.6);
    //axis->GetXaxis()->SetTitleOffset(1.3);

//    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );

    string path = "./Calibration/Data/";
    string fname = "PhysicsEvent_401141038_wf_reconstructed.root";

    string evtnr = fname.substr(13,9);

    TFile* rfile = new TFile((path+fname).c_str(), "open");


    TH1F* wf = (TH1F*) rfile->Get("FWD1");
    TH1F *wfm = new TH1F("wfm","wfm", length, - 0.4, length*0.8 - 0.4);

     for(int i = 1; i<=length; ++i )
     {
         double cont = wf->GetBinContent(i+start);
         double err  = wf->GetBinError(i+start);
         wfm->SetBinContent(i,cont);
         wfm->SetBinError(i, err);
     }

     TH1F* reco = (TH1F*) rfile->Get("FWD1_reco");
     TH1F *recom = new TH1F("recom","recom", length, - 0.4, length*0.8 - 0.4);

      for(int i = 1; i<=length; ++i )
      {
          double cont = reco->GetBinContent(i+start);
          double err  = reco->GetBinError(i+start);
          recom->SetBinContent(i,cont);
          recom->SetBinError(i, err);
      }

      TH1I* pe = (TH1I*) rfile->Get("FWD1_pe");
      TH1I *pem = new TH1I("pem","pem", length, - 0.4, length*0.8 - 0.4);

       for(int i = 1; i<=length; ++i )
       {
           double cont = pe->GetBinContent(i+start);
           double err  = pe->GetBinError(i+start);
           pem->SetBinContent(i,cont);
           pem->SetBinError(i, err);
       }

    // Float_t rightmax = 3.*pem->GetMaximum();
    // cout << "rightmax: " << rightmax << endl;
    // cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
    // Float_t scale = gPad->GetUymax()/rightmax;
    // cout << scale << endl;
//    pem->Scale(1/scale);

     // Colors and style
    double linewidth = 2;

    int c1 = kBlack;
    //int cpe = claws_gray->GetNumber();
    int cpe = tab20_lila->GetNumber();
    int creco = claws_red->GetNumber();
//    int creco = claws_red->GetNumber();

    double fillerr = 1.0;
    int fillstyle = 1001;

    wfm->SetLineWidth(linewidth);
    wfm->SetLineColorAlpha(c1,1.);
//    wfm->SetFillColorAlpha(c1,0.15);
    wfm->SetFillColorAlpha(c1,0.12);
    wfm->SetFillStyle(1001);

    recom->SetLineWidth(linewidth);
    recom->SetLineColorAlpha(creco,1.);

    pem->SetLineWidth(linewidth);
    pem->SetLineColorAlpha(cpe,1.);
    pem->SetFillColorAlpha(cpe,1);
    pem->SetFillStyle(1001);


    TLegend* leg =  new TLegend(0.41, 0.66, 0.61, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);

    leg->AddEntry(wfm, "Original waveform", "l");
    leg->AddEntry(recom, "Reco. waveform", "l");
    leg->AddEntry(pem, "Photon waveform", "l");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

     // Plotting
//    gStyle->SetPadTickY(-1);
    axis->Draw("AXIS");
    wfm->Draw("hist L same ][");
    recom->Draw("hist L same ][");

    can->Update();

    //Float_t rightmax = 1.168*pem->GetMaximum();
    Float_t rightmax = 4.9*pem->GetMaximum();
    cout << "rightmax: " << rightmax << endl;
    cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
    Float_t scale = gPad->GetUymax()/rightmax;
    cout << scale << endl;
    pem->Scale(scale);

    pem->Draw("hist same ][");

    leg->Draw();
    text->DrawLatex(0.42, 0.89, ("Evt. "+evtnr +": FWD1").c_str());

    TGaxis *axis2 = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),ylow/yup*rightmax,rightmax,510,"+L");
    axis2->SetLineColor(cpe);
    axis2->SetTextColor(cpe);
    axis2->SetLabelColor(cpe);
    axis2->SetTitle("Photons [p.e.]");
    axis2->SetTitleOffset(0.6);
    axis2->SetNdivisions(ndiv);

    double fontsize = axis->GetYaxis()->GetTitleSize();
    int font     = axis->GetYaxis()->GetTitleFont();

    axis2->SetTitleFont(font);
    axis2->SetLabelFont(font);
    axis2->SetTitleSize(fontsize);
    axis2->SetLabelSize(fontsize);
    //gStyle->SetPadTickY(1);
    axis2->Draw();

    can->SaveAs("./Calibration/wfreco.pdf");
    can->SaveAs("./Calibration/wfreco.png");
    can->SaveAs("./Calibration/wfreco.eps");
    can->SaveAs("./Calibration/wfreco.jpg");

    return 0;
}
