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


int plt_wfdecomposition_forFrank()
{
    SetPhDStyle();

    int width = round(vis_multi*textwidth*pt2pixel/2);
    // double height = 900;
    // double ratio = 16./9.;
    // double ratio = 2./3.;
    double ratio = 1.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLeftMargin(0.13);
    // can->SetBottomMargin(0.14);
    can->SetRightMargin(0.13);
    can->SetTicky(0);
    //can->SetLogy();

    int length   = 210;
    int start   = 14945;

    // double xlow = -3.49;
    // double xup  = 5.49;
    // double ylow = -19.99;
    // double yup  = 39.99;
    double ylow = -2.99;
    double yup  = 79.9;
    int ndiv = 506;

    TH1F *axis = new TH1F("axis","axis", length, - 0.4, length*0.8 - 0.4);
    axis->GetYaxis()->SetRangeUser(ylow, yup);
//    axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ns]");
    axis->GetYaxis()->SetTitleOffset(1.2);
    axis->GetYaxis()->SetTitle("Amplitude [mV]");
    axis->GetYaxis()->SetNdivisions(ndiv);
    //axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );
//    gStyle->SetTickLength(axis->GetXaxis()->GetTickLength()/ratio,"y");
    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );

    axis->GetXaxis()->SetLabelSize(gStyle->GetLegendTextSize()*0.95);
    axis->GetYaxis()->SetLabelSize(gStyle->GetLegendTextSize()*0.95);

    string path = "./Calibration/WFDecomposition/";
    string fname = "PhysicsEvent_9001200030_wf_decomposed.root";

    string evtnr = fname.substr(13,9);

    TFile* rfile = new TFile((path+fname).c_str(), "open");


    TH1F* wf = (TH1F*) rfile->Get("FWD3");
    TH1F *wfm = new TH1F("wfm","wfm", length, - 0.4, length*0.8 - 0.4);

     for(int i = 1; i<=length; ++i )
     {
         double cont = wf->GetBinContent(i+start);
         double err  = wf->GetBinError(i+start);
         wfm->SetBinContent(i,cont);
         wfm->SetBinError(i, err);
     }

     TFile* rfile_reco = new TFile((path+"PhysicsEvent_9001200030_wf_reconstructed.root").c_str(), "open");
     TH1F* reco = (TH1F*) rfile_reco->Get("FWD3_reco");
     TH1F *recom = new TH1F("recom","recom", length, - 0.4, length*0.8 - 0.4);

      for(int i = 1; i<=length; ++i )
      {
          double cont = reco->GetBinContent(i+start);
          double err  = reco->GetBinError(i+start);
          recom->SetBinContent(i,cont);
          recom->SetBinError(i, err);
      }


      TH1I* pe = (TH1I*) rfile->Get("FWD3_pe");
      TH1I *pem = new TH1I("pem","pem", length, - 0.4, length*0.8 - 0.4);

       for(int i = 1; i<=length; ++i )
       {
           double cont = pe->GetBinContent(i+start);
           double err  = pe->GetBinError(i+start);
           pem->SetBinContent(i,cont);
           pem->SetBinError(i, err);
       }

    string fname2 = "run_900120_gainstate_extended_ph1v4.root";

    TFile* rfile2 = new TFile((path+fname2).c_str(), "open");

    TH1F* avg = (TH1F*) rfile2->Get("FWD3_avg");
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
     // Colors and style
    double linewidth = 2;

    int c1 = kBlack;
    //int cpe = claws_gray->GetNumber();
    int cpe = tab20_blue->GetNumber();
    int cavg = tab20_red->GetNumber();
    int creco = tab20_lila->GetNumber();

    double fillerr = 1.0;
    int fillstyle = 1001;

    wfm->SetLineWidth(1);
    wfm->SetLineColorAlpha(c1,1.);
//    wfm->SetFillColorAlpha(c1,0.15);
    // wfm->SetFillColorAlpha(c1,0.12);
    // wfm->SetFillStyle(1001);
    recom->SetLineWidth(linewidth);
    recom->SetLineColorAlpha(creco,1.);

    avgm->SetLineWidth(linewidth);
    avgm->SetLineColorAlpha(cavg,1.);

    pem->SetLineWidth(linewidth);
    pem->SetLineColorAlpha(cpe,1.);
    pem->SetFillColorAlpha(cpe,0);
    pem->SetFillStyle(1001);


    TLegend* leg =  new TLegend(0.44, 0.56, 0.64, 0.78);
    //TLegend* leg =  new TLegend(0.49, 0.5, 0.69, 0.72);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    leg->SetTextFont(43);
    //leg->SetTextSize(gStyle->GetLegendTextSize()*0.9);
    leg->SetTextSize(gStyle->GetLegendTextSize()*0.95);
    //leg->SetNColumns(2);

    TH1F *wfmm = (TH1F*) wfm->Clone("wfmm");
    wfmm->SetMarkerSize(0);
    //err->SetFillStyle(3353);
    wfmm->SetFillStyle(fillstyle);
    wfmm->SetFillColorAlpha(c1,1.0);
    //err->SetFillColorAlpha(cerr,1.0);

    leg->AddEntry(set_line_width(wfmm,1), "Raw Waveform", "f");

    TH1F *recomm = (TH1F*) recom->Clone("recomm");
    recomm->SetMarkerSize(0);
    //err->SetFillStyle(3353);
    recomm->SetFillStyle(fillstyle);
    recomm->SetFillColorAlpha(creco,1.0);

//    leg->AddEntry(set_line_width(recomm,1), "Reco. Waveform", "f");

    TH1F *avgmm = (TH1F*) avgm->Clone("avgm");
    avgmm->SetMarkerSize(0);
    avgmm->SetLineWidth(linewidth);
    //err->SetFillStyle(3353);
    avgmm->SetFillStyle(fillstyle);
    avgmm->SetFillColorAlpha(cavg,1.0);

    leg->SetHeader("Waveform Decomposition");

    leg->AddEntry(set_line_width(avgmm,1), "1 p.e. Waveform", "f");
    leg->AddEntry(set_line_width(pem,1), "Photon Distribution", "f");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont());
    text->SetTextSize(gStyle->GetLegendTextSize()*0.95);
    text->SetTextAngle(90);
    text->SetTextColor(cavg);

    TLatex* text2 = new TLatex();
    text2->SetNDC();
    text2->SetTextFont(leg->GetTextFont()+20);
    text2->SetTextSize(gStyle->GetLegendTextSize());

    TLine *line = new TLine(avgm->GetBinLowEdge(1),wfm->GetMaximum()-avg->GetMaximum(),avgm->GetBinLowEdge(100),wfm->GetMaximum()-avg->GetMaximum());
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    TLine *line2 = new TLine(avgm->GetBinCenter(avgm->GetMaximumBin()), 0,avgm->GetBinCenter(avgm->GetMaximumBin()),wfm->GetMaximum());
    line2->SetLineColor(kGray);
    line2->SetLineStyle(7);

    TArrow* ar1 = new TArrow(35,25,35,65, 0.015, "<|");
    ar1->SetLineColor(cavg);
    ar1->SetFillColorAlpha(cavg,0);
    ar1->SetLineWidth(linewidth);


     // Plotting
    axis->Draw("AXIS");
//    recom->Draw("hist L same ][");
    wfm->Draw("hist L same ][");

    line->Draw();
//    line2->Draw();
    avgm->Draw("hist L same ][");
    can->Update();

    //Float_t rightmax = 1.168*pem->GetMaximum();
    Float_t rightmax = 3.5*pem->GetMaximum();
    cout << "rightmax: " << rightmax << endl;
    cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
    Float_t scale = gPad->GetUymax()/rightmax;
    cout << scale << endl;
    pem->Scale(scale);

    pem->Draw("hist same ][");
    ar1->Draw();
    leg->Draw();
    text->DrawLatex(0.27, 0.57, "Subtract");

//    text2->DrawLatex(0.23, 0.89, "#splitline{CLAWS Waveform}{Decomposition}");
        text2->DrawLatex(0.445, 0.79, "CLAWS");

    TGaxis *axis2 = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),ylow/yup*rightmax,rightmax,510,"+L");
    axis2->SetLineColor(cpe);
    axis2->SetTextColor(cpe);
    axis2->SetLabelColor(cpe);
    axis2->SetTitle("Photons [p.e./0.8 ns]");
    axis2->SetTitleOffset(1.2);
    axis2->SetNdivisions(ndiv);
    axis2->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );



    double fontsize = axis->GetYaxis()->GetTitleSize();
    int font     = axis->GetYaxis()->GetTitleFont();

    axis2->SetTitleFont(font);
    axis2->SetLabelFont(font);
    axis2->SetTitleSize(fontsize);
    axis2->SetLabelSize(fontsize);
    axis2->SetLabelSize(gStyle->GetLegendTextSize()*0.95);




    //gStyle->SetPadTickY(1);
    axis2->Draw();

    can->SaveAs((path+"/wfdecompostion.pdf").c_str());
    can->SaveAs((path+"/wfdecompostion.png").c_str());
    can->SaveAs((path+"/wfdecompostion.eps").c_str());
    can->SaveAs((path+"/wfdecompostion.jpg").c_str());

    return 0;
}
