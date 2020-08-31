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


int plt_miptime()
{
    SetPhDStyle();

    // int width = round(vis_multi*textwidth*pt2pixel);
    // double ratio = 21./9.;

    int width = round(vis_multi*textwidth*pt2pixel*1/2);
    //double ratio = 21./9.;
    double ratio = 2./2.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    //can->SetRightMargin(0.16);
    can->SetTicky(0);
    //can->SetLogy();
    can->SetLeftMargin(0.14);
    can->SetRightMargin(0.14);
//    can->SetBottomMargin(0.16);

    int length   = 44;
    //int start   = 549335;
    int start   = 172039;
    //int start   = 0;
    double scale = 1e-9;
    double xlow = 0;
    double xup  = length*0.8;
    // double ylow = -19.99;
    // double yup  = 39.99;
    double ylow = 0;
    double yup  = 2.35;
    int ndiv = 506;

    TH1F *axis = new TH1F("axis","axis", 1000, -1, 1000);
    axis->GetXaxis()->SetRangeUser(xlow, xup);
    axis->GetYaxis()->SetRangeUser(ylow, yup*15.56);
    //axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ns]");
    axis->GetYaxis()->SetTitleOffset(1.2);
    axis->GetYaxis()->SetTitle("Photons [p.e./0.8 ns]");
    axis->GetYaxis()->SetNdivisions(ndiv);
    axis->GetXaxis()->SetNdivisions(ndiv);
//    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );

     string path = "./Calibration/Data/";
     string fname = "PhysicsEvent_401141072_calibrated.root";

     string evtnr = fname.substr(13,9);
//
     TFile* rfile = new TFile((path+fname).c_str(), "open");

     TH1F* pe = (TH1F*) rfile->Get("FWD1_pe");

    // int nbins = pe->GetNbinsX(nbins);
     // double xmin = pe->GetBinLowEdge(1)/scale;
     // double xmax = (pe->GetBinLowEdge(nbins)+pe->GetBinWidth(nbins))/scale;
     // pe->SetBins(nbins, xmin, xmax)
     TH1F *pem = new TH1F("pem","pem", length, - 0.4, length*0.8 - 0.4);

     for(int i = 1; i<=length; ++i )
     {
        double cont = pe->GetBinContent(i+start);
        double err  = pe->GetBinError(i+start);
        pem->SetBinContent(i,cont);
        pem->SetBinError(i, err);
     }


     TH1F* mip = (TH1F*) rfile->Get("FWD1_mip");

     TH1F *mipm = new TH1F("mipm","mipm", length, - 0.4, length*0.8 - 0.4);

     for(int i = 1; i<=length; ++i )
     {
        double cont = mip->GetBinContent(i+start);
        double err  = mip->GetBinError(i+start);
        mipm->SetBinContent(i,cont);
        mipm->SetBinError(i, err);
     }


//
//      TH1F* reco = (TH1F*) rfile->Get("FWD1_reco");
//      TH1F *recom = new TH1F("recom","recom", length, - 0.4, length*0.8 - 0.4);
//
//       for(int i = 1; i<=length; ++i )
//       {
//           double cont = reco->GetBinContent(i+start);
//           double err  = reco->GetBinError(i+start);
//           recom->SetBinContent(i,cont);
//           recom->SetBinError(i, err);
//       }
//
//       TH1I* pe = (TH1I*) rfile->Get("FWD1_pe");
//       TH1I *pem = new TH1I("pem","pem", length, - 0.4, length*0.8 - 0.4);
//
//        for(int i = 1; i<=length; ++i )
//        {
//            double cont = pe->GetBinContent(i+start);
//            double err  = pe->GetBinError(i+start);
//            pem->SetBinContent(i,cont);
//            pem->SetBinError(i, err);
//        }
//
//     // Float_t rightmax = 3.*pem->GetMaximum();
//     // cout << "rightmax: " << rightmax << endl;
//     // cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
//     // Float_t scale = gPad->GetUymax()/rightmax;
//     // cout << scale << endl;
// //    pem->Scale(1/scale);
//
//      // Colors and style
     double linewidth = 2;
//
//     int c1 = kBlack;
//     //int cpe = claws_gray->GetNumber();

        int cpe = tab20_blue->GetNumber();
        int cmip = claws_red->GetNumber();
        int cl1 = claws_gray->GetNumber();
        // int cl2 = tab20_orange->GetNumber();
        int cl2 = cl1;
//     int cpe = tab20_lila->GetNumber();
//     int creco = claws_red->GetNumber();
// //    int creco = claws_red->GetNumber();
//
//     double fillerr = 1.0;
//     int fillstyle = 1001;
//
//     wfm->SetLineWidth(linewidth);
     pem->SetLineColorAlpha(cpe,1);
     pem->SetFillColorAlpha(cpe,1);
     pem->SetLineWidth(1);
// //    wfm->SetFillColorAlpha(c1,0.15);
//     wfm->SetFillColorAlpha(c1,0.12);
//     wfm->SetFillStyle(1001);
//
//     recom->SetLineWidth(linewidth);
    mipm->SetLineColorAlpha(cmip,1);
    mipm->SetLineWidth(1);
    mipm->SetMarkerStyle(22);
    //mipm->SetLineStyle(9);
     mipm->SetFillColorAlpha(cmip,1);
    cout << "Bin6: " << mipm->GetBinContent(6) << endl;
    cout << "Bin6 Err: " << mipm->GetBinError(6) << endl;
//     pem->SetLineWidth(linewidth);
//     pem->SetLineColorAlpha(cpe,1.);
//     pem->SetFillColorAlpha(cpe,1);
//     pem->SetFillStyle(1001);
    int linestyle =7;
    double intwindow= 9.6;

    double y1 = 34;
    TLine *line = new TLine(2.,0,2.,y1);
    line->SetLineColor(cl1);
    line->SetLineStyle(linestyle);
    line->SetLineWidth(linewidth);

    TLine *line2 = new TLine( 2.+intwindow,0,2.+intwindow,y1);
    line2->SetLineColor(cl1);
    line2->SetLineStyle(linestyle);
    line2->SetLineWidth(linewidth);

    double y2 = 8.5;
    TLine *line3 = new TLine(14,0,14,y2);
    line3->SetLineColor(cl2);
    line3->SetLineStyle(linestyle);
    line3->SetLineWidth(linewidth);

    TLine *line4 = new TLine(14+intwindow,0,14+intwindow,y2);
    line4->SetLineColor(cl2);
    line4->SetLineStyle(linestyle);
    line4->SetLineWidth(linewidth);

    TBox* b1 = new TBox(2,0,2+intwindow,y1);
    b1->SetFillColorAlpha(cl1, 0.75);

    TArrow* ar1 = new TArrow(2,y1,2+intwindow,y1, 0.015, "<|>");
    ar1->SetLineColor(cl1);
    ar1->SetFillColorAlpha(cl1,0);
    ar1->SetLineWidth(linewidth);

    TArrow* ar2 = new TArrow(14,y2,14+intwindow,y2, 0.015, "<|>");
    ar2->SetLineColor(cl2);
    ar2->SetFillColorAlpha(cl2,0);
    ar2->SetLineWidth(linewidth);
//    ar2->SetLineStyle(linestyle);

    TLegend* leg =  new TLegend(0.425, 0.68, 0.635, 0.87);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);

    TH1F* pel = (TH1F*) pem->Clone("mipl");
//    pel->SetLineWidth(2);

    leg->AddEntry(pel, "Photon waveform", "f");

    TH1F* mipl = (TH1F*) mipm->Clone("mipl");
//    mipl->SetLineWidth(2);
    leg->AddEntry(mipl, "MIP waveform", "f");

//    leg->AddEntry(pem, "Photon waveform", "l");


    TLatex* textint1 = new TLatex();
    textint1->SetNDC();
    textint1->SetTextFont(leg->GetTextFont());
    textint1->SetTextSize(gStyle->GetLegendTextSize()*0.8);
    textint1->SetTextColorAlpha(cl1,1.0);

     //TLegend* leg =  new TLegend(0.41, 0.66, 0.61, 0.88);
//     leg->SetBorderSize(0);
//     leg->SetFillColor(0);
//     leg->SetFillStyle(0);
//
//     leg->AddEntry(wfm, "Original waveform", "l");
//     leg->AddEntry(recom, "Reco. waveform", "l");
//     leg->AddEntry(pem, "Photon waveform", "l");
//
    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());
//
//      // Plotting
// //    gStyle->SetPadTickY(-1);
    axis->Draw("AXIS");

      // b1->Draw();



      line->Draw();
      line2->Draw();
      ar1->Draw();
      line3->Draw();
      line4->Draw();
      ar2->Draw();


      can->Update();
// 16.5
    //  Float_t rightmax =2.2*mipm->GetMaximum();
      Float_t rightmax = yup;
      cout << "rightmax: " << rightmax << endl;
      cout << "gPad->GetUymax(): " << gPad->GetUymax() << endl;
      Float_t scale2 = gPad->GetUymax()/rightmax;
      cout << scale2 << endl;
      mipm->Scale(scale2);

      mipm->Draw("same hist ][");

      TH1I *err = (TH1I*) mipm->Clone("err");
      err->SetMarkerSize(0);
      err->SetFillStyle(3253);
      //err->SetFillStyle(1001);
      //err->SetFillColorAlpha(cerr,1);
      err->SetFillColorAlpha(kBlack,1);
      err->SetLineWidth(0);
    //  err->Draw("same E2 ][");

//      can->Update();
      TGaxis *axis2 = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(), gPad->GetUymax(),0,rightmax,510,"+L");
      axis2->SetLineColor(cmip);
      axis2->SetTextColor(cmip);
      axis2->SetLabelColor(cmip);
      axis2->SetTitle("Particles [MIP/0.8 ns]");
      axis2->SetTitleOffset(1.4);
      axis2->SetNdivisions(ndiv);

      double fontsize = axis->GetYaxis()->GetTitleSize();
      int font     = axis->GetYaxis()->GetTitleFont();

       axis2->SetTitleFont(font);
      axis2->SetLabelFont(font);
      axis2->SetTitleSize(fontsize);
      axis2->SetLabelSize(fontsize);
             //gStyle->SetPadTickY(1);
      axis2->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );
      axis2->Draw();

           text->DrawLatex(0.43, 0.89, ("Evt. "+evtnr +": FWD1").c_str());


    //       leg->AddEntry(err, "Stat. Error", "f");
           leg->AddEntry(ar1, "Int. Window: 9.6 ns", "l");
           //leg->AddEntry((TObject*)0, "9.6 ns", "");

           TLegend* leg2 =  new TLegend(0.38, 0.56, 0.6, 0.73);
           leg2->SetBorderSize(0);
           leg2->SetFillColor(0);
           leg2->SetFillStyle(0);

         //  leg->AddEntry((TObject*)0, "Conv. p.e. to MIP:", "");
           leg2->AddEntry((TObject*)0, "#beta_{pe#rightarrowMIP} = 15.6 p.e./MIP", "");

           leg->Draw();
           leg2->Draw();
           pem->Draw("same hist ][]");


    can->SaveAs("./Calibration/miptime.pdf");
    can->SaveAs("./Calibration/miptime.png");
    can->SaveAs("./Calibration/miptime.eps");
    can->SaveAs("./Calibration/miptime.jpg");

    return 0;
}
