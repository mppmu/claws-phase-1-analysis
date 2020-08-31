


int plt_wfdecomposition()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel);
    //double ratio = 16./9.;
    double ratio = 2./1.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLeftMargin(0.07);

//    can->SetBottomMargin(0.08);
    can->SetRightMargin(0.06);

    can->SetTicky(0);
    //can->SetLogy();

    int length   = 300;
    // int start   = 159432;
    //int start   = 172023; // 37
    int start   = 172008; // 37


    string fname = "WaveformDecomposition/Waveforms/PhysicsEvent_401141051_wf_decomposed.root";
    string fname3 = "WaveformReconstruction/Waveforms/PhysicsEvent_401141051_wf_reconstructed.root";
    // double xlow = -3.49;
    // double xup  = 5.49;
    // double ylow = -19.99;
    // double yup  = 39.99;
    double ylow = -9.99;
    // double yup  = 59.9;
    int ndiv = 506;

    TH1F *axis = new TH1F("axis","axis", length, - 0.4, length*0.8 - 0.4);
    // axis->GetYaxis()->SetRangeUser(ylow, yup);
    axis->GetXaxis()->SetTitleOffset(1.);
    axis->GetXaxis()->SetTitle("Time [ns]");
    axis->GetYaxis()->SetTitleOffset(0.725);
    axis->GetYaxis()->SetTitle("Amplitude [mV]");
    axis->GetYaxis()->SetNdivisions(ndiv);
    //axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );
//    gStyle->SetTickLength(axis->GetXaxis()->GetTickLength()/ratio,"y");
    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );

    string path = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/";

    string evtnr = fname.substr(45,9);

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

     double yup  = wfm->GetMaximum()*1.1;
     axis->GetYaxis()->SetRangeUser(ylow, yup);


     TH1F* residuel = (TH1F*) rfile->Get("FWD1_reco");
     TH1F *residuelm = new TH1F("residuelm","residuelm", length, - 0.4, length*0.8 - 0.4);

      for(int i = 1; i<=length; ++i )
      {
          double cont = residuel->GetBinContent(i+start);
          double err  = residuel->GetBinError(i+start);
          residuelm->SetBinContent(i,cont);
          residuelm->SetBinError(i, err);
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

    string fname2 = "/Average1PE/run_401141_gainstate_extended_ph1v6.root";

    TFile* rfile2 = new TFile((path+fname2).c_str(), "open");

    TH1F* avg = (TH1F*) rfile2->Get("FWD1_avg");
    int shift = avg->GetMaximumBin() - wfm->GetMaximumBin();
    cout << shift << endl;
    TH1F *avgm = new TH1F("avgm","avgm", avg->GetNbinsX()-350, +100*0.8 -0.8*shift- 0.4,-250*0.8 -0.8*shift+avg->GetNbinsX()*0.8 - 0.4);


    for(int i = 1; i<=avg->GetNbinsX()-350; ++i )
    {
    double cont = avg->GetBinContent(i+100)+wfm->GetMaximum()-avg->GetMaximum();
    //double cont = avg->GetBinContent(i);
        double err  = avg->GetBinError(i+100);
        avgm->SetBinContent(i,cont);
        avgm->SetBinError(i, err);
    }
    cout <<"AVG MAX X: "<< avgm->GetBinCenter(avgm->GetMaximumBin()) << endl;
    cout <<"WF MAX X: "<< wfm->GetBinCenter(wfm->GetMaximumBin()) << endl;


    TFile* rfile3 = new TFile((path+fname3).c_str(), "open");

    TH1F* reco = (TH1F*) rfile3->Get("FWD1_reco");
    TH1F *recom = new TH1F("recom","recom", length, - 0.4, length*0.8 - 0.4);

     for(int i = 1; i<=length; ++i )
     {
         double cont = reco->GetBinContent(i+start);
         double err  = reco->GetBinError(i+start);
         recom->SetBinContent(i,cont);
         recom->SetBinError(i, err);
     }


     // Colors and style
    double linewidth = 2;

    int c1 = kBlack;
    //int cpe = claws_gray->GetNumber();
    int cpe = tab20_blue->GetNumber();
    int cavg = tab20_green->GetNumber();
    int creco = tab20_orange->GetNumber();
    int cresiduel = tab20_gray->GetNumber();

    double fillerr = 1.0;
    int fillstyle = 1001;

    wfm->SetLineWidth(linewidth);
    wfm->SetLineColorAlpha(c1,1.);
//    wfm->SetFillColorAlpha(c1,0.15);
    // wfm->SetFillColorAlpha(c1,0.12);
    // wfm->SetFillStyle(1001);
    residuelm->SetLineWidth(linewidth);
    residuelm->SetLineColorAlpha(cresiduel,1.);
    //residuelm->SetLineStyle(2);

    avgm->SetLineWidth(linewidth);
    avgm->SetLineColorAlpha(cavg,1.);

    recom->SetLineWidth(linewidth);
    recom->SetLineColorAlpha(creco,1.);

    pem->SetLineWidth(2);
    //pem->SetLineColorAlpha(cpe,1.);

    pem->SetFillColorAlpha(cpe,1);
    pem->SetFillStyle(1001);
    pem->SetLineColorAlpha(cpe,1.);

    // pem->SetFillColorAlpha(cpe,0.5);
    // pem->SetLineColorAlpha(cpe,1);
    // pem->SetFillStyle(1001);

    TLegend* leg =  new TLegend(0.675, 0.56, 0.795, 0.86);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);

    leg->AddEntry(set_line_width(wfm), "Raw Physics Waveform", "l");
    leg->AddEntry(set_line_width(residuelm), "Residual Waveform", "l");
    leg->AddEntry(set_line_width(avgm), "Avg. One p.e. Waveform", "l");

    leg->AddEntry(make_box(pem,"FULL"), "Photon Waveform", "f");

    leg->AddEntry(set_line_width(recom), "Reassembled waveform", "l");
    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLine *line = new TLine(avgm->GetBinLowEdge(1),wfm->GetMaximum()-avg->GetMaximum(),avgm->GetBinLowEdge(100),wfm->GetMaximum()-avg->GetMaximum());
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    TLine *line2 = new TLine(avgm->GetBinCenter(avgm->GetMaximumBin()), 0,avgm->GetBinCenter(avgm->GetMaximumBin()),wfm->GetMaximum());
    line2->SetLineColor(kGray);
    line2->SetLineStyle(7);


    // Get the reconstructed wf

     // Plotting
    axis->Draw("AXIS");
    // wfm->Draw("hist L same ][");
    residuelm->Draw("hist L same ][");
    wfm->Draw("hist L same ][");
    line->Draw();
    // line2->Draw();
    avgm->Draw("hist L same ][");

    recom->Draw("hist L same ][");
    can->Update();

    //Float_t rightmax = 1.168*pem->GetMaximum();
    Float_t rightmax = 1.09*pem->GetMaximum();
    cout << "rightmax: " << rightmax << endl;
    cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
    Float_t scale = gPad->GetUymax()/rightmax;
    cout << scale << endl;
    pem->Scale(scale);

    pem->Draw("hist same ][");

    int carr = cavg;
    TArrow *ar = new TArrow(19,86,19,142,0.015,"<");
    ar->SetLineColorAlpha(carr,1.);
    ar->SetFillColorAlpha(carr,0.);
    ar->SetAngle(40);
    ar->SetLineWidth(2);
    ar->Draw();

    TLatex* text2 = new TLatex();
    //text2->SetNDC();
    text2->SetTextFont(leg->GetTextFont()+10);
    text2->SetTextSize(gStyle->GetLegendTextSize());
    text2->SetTextAngle(90);
    text2->SetTextColor(carr);

    text2->DrawLatex(16, 99, "Subtract");

    leg->Draw();



    text->DrawLatex(0.67775, 0.88, ("Evt. "+evtnr +": FWD1").c_str());

    TGaxis *axis2 = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),ylow/yup*rightmax,rightmax,510,"+L");
    axis2->SetLineColor(cpe);
    axis2->SetTextColor(cpe);
    axis2->SetLabelColor(cpe);
    axis2->SetTitle("Photon Rate [p.e./0.8 ns]");
    axis2->SetTitleOffset(0.6);
    axis2->SetNdivisions(ndiv);
    axis2->SetTickLength( axis->GetXaxis()->GetTickLength()*0.5);

    double fontsize = axis->GetYaxis()->GetTitleSize();
    int font     = axis->GetYaxis()->GetTitleFont();

    axis2->SetTitleFont(font);
    axis2->SetLabelFont(font);
    axis2->SetTitleSize(fontsize);
    axis2->SetLabelSize(fontsize);
    gStyle->SetPadTickY(1);
    axis2->Draw();

    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/wfdecompostion.pdf");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/wfdecompostion.png");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/wfdecompostion.eps");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/wfdecompostion.jpg");

    return 0;
}
