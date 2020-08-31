


int her_synchrotron(string channel = "FWD1")
{
    SetPhDStyle();

    Target target = GetTarget("HER-ALL");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* her = (TH1F*) rfile->Get((channel+"_peak").c_str());
    her->SetDirectory(0);

    // if(target.n != -1)
    // {
    //     double scale = target.n*0.8e-9;
    //     her->Scale(1./scale);
    // }

    double dt = 0.8e-3; // in mus
    double scaling = 1e-3;
    double xlow, xup;

    xlow = -dt/2.;
    xup = target.n*dt - dt/2.;

    her->SetBins(target.n, her->GetBinLowEdge(1)*scaling, (her->GetBinLowEdge(target.n) + her->GetBinWidth(target.n))*scaling );

    double t_rev =  10061.4*1e-3;

    TGraph* peaks_odd = new TGraph();
    TGraph* peaks_even = new TGraph();

    vector <TF1*> gaussians_even;
    vector <TF1*> gaussians_odd;

    double thres = her->GetMaximum()*0.0125;
    for(int bin = 1200; bin < her->GetNbinsX(); ++bin)
    {
        if(her->GetBinContent(bin) >thres)
         {
            if(fmod(bin*dt,t_rev) < t_rev*0.1 || fmod(bin*dt,t_rev)> t_rev*0.9)
            {
                double max_bin = bin;
                double max = 0;
                for(int i = bin-20 ;i<bin+60; i++ )
                {
                    if(her->GetBinContent(i)>max)
                    {
                        max_bin = i;
                        max = her->GetBinContent(i);
                    }
                }

                TF1* gaus = new TF1("gaus", "gaus", max_bin*dt-0.003, max_bin*dt+0.003 );
                gaus->SetParameters(1,her->GetBinCenter(max_bin), 0.01);
                her->Fit(gaus,"QRS");
                gaus->SetLineColor(kMagenta);
                double mean = gaus->GetParameter(1);

                if(int(round(mean/t_rev))%2) peaks_odd->SetPoint(peaks_odd->GetN(), mean, gaus->GetMaximum());
                else peaks_even->SetPoint(peaks_even->GetN(), mean, gaus->GetMaximum());

                // peaks_even->SetPoint(peaks_even->GetN(), mean, gaus->GetMaximum());
                // cout << "X: " <<mean<< ", Y: " <<gaus->GetMaximum()<< ", at: " << bin*dt << endl;
                // delete gaus;

                // cout << "bin*dt: " << bin*dt << " fmod(bin*dt,t_rev): " << fmod(bin*dt,t_rev)<< ", mean: " << mean<< endl;
                bin += 500;
                // gaussians.push_back(gaus);

                //delete gaus;
            }
        }
    }


    // ----- Aufbitchen -----

    int codd = tab20_red->GetNumber();
    int ceven = tab20_blue->GetNumber();

    double markersize = 1.2;
    double glinewidth = 3;

    peaks_odd->SetLineColor(codd);
    peaks_odd->SetLineWidth(glinewidth);
    peaks_odd->SetMarkerColor(codd);
    peaks_odd->SetMarkerSize(markersize);
    peaks_odd->SetMarkerStyle(23);

    peaks_even->SetLineColor(ceven);
    peaks_even->SetLineWidth(glinewidth);
    peaks_even->SetMarkerColor(ceven);
    peaks_even->SetMarkerSize(markersize);
    peaks_even->SetMarkerStyle(22);

    TMarker* single_marker = new TMarker(0, her->GetMaximum()*0.8, 22);
    single_marker->SetMarkerSize(2.4);
    single_marker->SetMarkerColor(kBlack);

    TF1* gaus = new TF1("gaus", "gaus", 680, 820 );
    gaus->SetLineColor(ceven);
    gaus->SetLineWidth(3);
    peaks_even->Fit(gaus,"QRS");

    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(325,435);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1090,1210);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1500,1600);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1880,1980);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    // ----- Legend -----

    // TLegend* leg =  new TLegend(0.63, 0.66, 0.82, 0.82);
    TLegend* leg =  new TLegend(0.5, 0.68, 0.7, 0.84);
    	    //leg->SetHeader("");
    	//    leg->SetNColumns(2);
    	leg->SetBorderSize(0);
    	leg->SetFillColor(0);

    	// leg->AddEntry(make_box)

    	int legfontsize = leg->GetTextSize();
    	int legfont     = leg->GetTextFont();

     leg->AddEntry(make_box(her, "EMPTY"), target.data.c_str(), "f");
     leg->AddEntry(set_line_width(peaks_even), "Even Multiples of T_{rev}", "p");
     leg->AddEntry(set_line_width(peaks_odd), "Odd Multiples of T_{rev}", "p");

    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* etext = new TLatex();
//    etext->SetNDC();
    etext->SetTextFont(43);
    etext->SetTextSize(gStyle->GetLegendTextSize());
    etext->SetTextColor(ceven);
  // -----   Axis   ---------------------
    TH1F* axis = new TH1F("axis", "axis", 1000000, xlow, xup);
    axis->SetDirectory(0);

    axis->GetXaxis()->SetTitle("Peak to Peak Distance [#mus]");
//    axis->GetYaxis()->SetTitleOffset(1.7);
    axis->GetYaxis()->SetTitle("Weighted Entries [MIP^{2}/(0.8 ns)^{3}]");

    // axis->GetXaxis()->SetRangeUser(-0.015, 4.05);
    axis->GetYaxis()->SetRangeUser(0., her->GetMaximum()*0.8);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "her_injections_peak_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//	can->SetLogx();
 // can->SetLogy();
//
//   // ---- DRAWING ----
    can->cd();
    axis->Draw("AXIS");
    her->Draw("SAME HIST");

//    her->Draw("SAME FUNC");
// /   gaus->Draw("same");
     peaks_even->Draw("SAME P");
     peaks_odd->Draw("SAME P");
     single_marker->Draw("same");

    // gaus->Draw("same");

     ttext->DrawLatex(0.50, 0.88, ("PEAK Analysis: "+channel).c_str());
     leg->Draw();

     // for(auto& gaus:gaussians_even)
     // {
     //     gaus->Draw("same");
     //
     //     stringstream ss;
     //     ss << fixed << setprecision(0) << "(" << gaus->GetParameter(1)<<"#pm"<< gaus->GetParError(2) << ") #mus";
     //     //
     //     // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
     //
     //    // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
     //     etext->DrawLatex(gaus->GetParameter(1)-250,0.15+gaus->Eval(gaus->GetParameter(1)), ss.str().c_str() );
     // }
     cout << gaussians_even.size() << endl;

     gaussians_even.at(0)->Draw("same");

     // gaussians_even.at(1)->Draw("same");
     // gaussians_even.at(2)->Draw("same");
     // gaussians_even.at(3)->Draw("same");
    // gaussians_even.at(4)->Draw("same");
     stringstream ss;
     ss << fixed << setprecision(0) << "(" << gaussians_even.at(0)->GetParameter(1)<<"#pm"<< gaussians_even.at(0)->GetParError(2) << ") #mus";
     //
     // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");

    // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
     etext->DrawLatex(gaussians_even.at(0)->GetParameter(1)-225,0.125+gaussians_even.at(0)->Eval(gaussians_even.at(0)->GetParameter(1)), ss.str().c_str() );

     TMarker* m_0 = new TMarker(gaussians_even.at(0)->GetParameter(1), gaussians_even.at(0)->Eval(gaussians_even.at(0)->GetParameter(1))+0.065, 23);
     m_0->SetMarkerSize(2);
     m_0->SetMarkerColor(ceven);

     m_0->Draw();


     // 1st peak
     gaussians_even.at(1)->Draw("same");
     stringstream ss2;
     ss2 << fixed << setprecision(0) << "(" << gaussians_even.at(1)->GetParameter(1)<<"#pm"<< gaussians_even.at(1)->GetParError(2) << ") #mus";

     etext->DrawLatex(gaussians_even.at(1)->GetParameter(1)-225,0.125+gaussians_even.at(1)->Eval(gaussians_even.at(1)->GetParameter(1)), ss2.str().c_str() );

     TMarker* m_1 = new TMarker(gaussians_even.at(1)->GetParameter(1), gaussians_even.at(1)->Eval(gaussians_even.at(1)->GetParameter(1))+0.065, 23);
     m_1->SetMarkerSize(2);
     m_1->SetMarkerColor(ceven);

     m_1->Draw();

      // 2nd peak
     gaussians_even.at(2)->Draw("same");
     stringstream ss3;
     ss3 << fixed << setprecision(0) << "(" << gaussians_even.at(2)->GetParameter(1)<<"#pm"<< gaussians_even.at(2)->GetParError(2) << ") #mus";

     etext->DrawLatex(gaussians_even.at(2)->GetParameter(1)-225,0.125+gaussians_even.at(2)->Eval(gaussians_even.at(2)->GetParameter(1)), ss3.str().c_str() );

     TMarker* m_2 = new TMarker(gaussians_even.at(2)->GetParameter(1), gaussians_even.at(2)->Eval(gaussians_even.at(2)->GetParameter(1))+0.065, 23);
     m_2->SetMarkerSize(2);
     m_2->SetMarkerColor(ceven);

     m_2->Draw();

     // 3nd peak
    gaussians_even.at(3)->Draw("same");
    stringstream ss4;
    ss4 << fixed << setprecision(0) << "(" << gaussians_even.at(3)->GetParameter(1)<<"#pm"<< gaussians_even.at(3)->GetParError(2) << ") #mus";

    etext->DrawLatex(gaussians_even.at(3)->GetParameter(1)-225,0.125+gaussians_even.at(3)->Eval(gaussians_even.at(3)->GetParameter(1)), ss4.str().c_str() );

    TMarker* m_3 = new TMarker(gaussians_even.at(3)->GetParameter(1), gaussians_even.at(3)->Eval(gaussians_even.at(3)->GetParameter(1))+0.065, 23);
    m_3->SetMarkerSize(2);
    m_3->SetMarkerColor(ceven);

    m_3->Draw();

    // 3nd peak
   gaussians_even.at(4)->Draw("same");
   stringstream ss5;
   ss5 << fixed << setprecision(0) << "(" << gaussians_even.at(4)->GetParameter(1)<<"#pm"<< gaussians_even.at(4)->GetParError(2) << ") #mus";

   etext->DrawLatex(gaussians_even.at(4)->GetParameter(1)-225,0.08+gaussians_even.at(4)->Eval(gaussians_even.at(4)->GetParameter(1)), ss5.str().c_str() );

   TMarker* m_4 = new TMarker(gaussians_even.at(4)->GetParameter(1), gaussians_even.at(4)->Eval(gaussians_even.at(4)->GetParameter(1))+0.03, 23);
   m_4->SetMarkerSize(2);
   m_4->SetMarkerColor(ceven);

   m_4->Draw();


 // TCanvas * can2 = new TCanvas("fft","fft", width, width/ratio);
 // can2->SetFillColor(kWhite);
 // can2->SetBorderMode(0);
 // can2->SetBorderSize(2);
 // can2->SetFrameBorderMode(0);
 //
 // TH1* fft = 0;
 //
 // fft = her->FFT(fft, "MAG");
 // can2->cd();
 //
 // fft->Draw();


 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".pdf").c_str());
 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".png").c_str());
 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".eps").c_str());
 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".jpg").c_str());


  return 0;
}

int ler_synchrotron(string channel = "FWD1")
{
    SetPhDStyle();

    Target target = GetTarget("LER-ALL");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* her = (TH1F*) rfile->Get((channel+"_peak").c_str());
    her->SetDirectory(0);

    // if(target.n != -1)
    // {
    //     double scale = target.n*0.8e-9;
    //     her->Scale(1./scale);
    // }

    double dt = 0.8e-3; // in mus
    double scaling = 1e-3;
    double xlow, xup;

    xlow = -dt/2.;
    xup = target.n*dt - dt/2.;

    her->SetBins(target.n, her->GetBinLowEdge(1)*scaling, (her->GetBinLowEdge(target.n) + her->GetBinWidth(target.n))*scaling );

    double t_rev =  10061.4*1e-3;

    TGraph* peaks_odd = new TGraph();
    TGraph* peaks_even = new TGraph();

    vector <TF1*> gaussians_even;
    vector <TF1*> gaussians_odd;

    // double thres = her->GetMaximum()*0.0125;
    double thres = 1e-2;
    for(int bin = 1200; bin < her->GetNbinsX(); ++bin)
    {
        if(her->GetBinContent(bin) >thres)
         {
            if(fmod(bin*dt,t_rev) < t_rev*0.1 || fmod(bin*dt,t_rev)> t_rev*0.9)
            {
                double max_bin = bin;
                double max = 0;
                for(int i = bin-20 ;i<bin+60; i++ )
                {
                    if(her->GetBinContent(i)>max)
                    {
                        max_bin = i;
                        max = her->GetBinContent(i);
                    }
                }

                TF1* gaus = new TF1("gaus", "gaus", max_bin*dt-0.003, max_bin*dt+0.003 );
                gaus->SetParameters(1,her->GetBinCenter(max_bin), 0.01);
                her->Fit(gaus,"QRS");
                gaus->SetLineColor(kMagenta);
                double mean = gaus->GetParameter(1);

                if(int(round(mean/t_rev))%2) peaks_odd->SetPoint(peaks_odd->GetN(), mean, gaus->GetMaximum());
                else peaks_even->SetPoint(peaks_even->GetN(), mean, gaus->GetMaximum());

                // peaks_even->SetPoint(peaks_even->GetN(), mean, gaus->GetMaximum());
                // cout << "X: " <<mean<< ", Y: " <<gaus->GetMaximum()<< ", at: " << bin*dt << endl;
                // delete gaus;

                // cout << "bin*dt: " << bin*dt << " fmod(bin*dt,t_rev): " << fmod(bin*dt,t_rev)<< ", mean: " << mean<< endl;
                bin += 500;
                // gaussians.push_back(gaus);

                //delete gaus;
            }
        }
    }


    // ----- Aufbitchen -----

    int codd = tab20_red->GetNumber();
    int ceven = tab20_blue->GetNumber();

    double markersize = 1.2;
    double glinewidth = 3;

    peaks_odd->SetLineColor(codd);
    peaks_odd->SetLineWidth(glinewidth);
    peaks_odd->SetMarkerColor(codd);
    peaks_odd->SetMarkerSize(markersize);
    peaks_odd->SetMarkerStyle(23);

    peaks_even->SetLineColor(ceven);
    peaks_even->SetLineWidth(glinewidth);
    peaks_even->SetMarkerColor(ceven);
    peaks_even->SetMarkerSize(markersize);
    peaks_even->SetMarkerStyle(22);

    TMarker* single_marker = new TMarker(0, her->GetMaximum()*0.8, 22);
    single_marker->SetMarkerSize(1.6);
    single_marker->SetMarkerColor(kBlack);

    TF1* gaus = new TF1("gaus", "gaus", 200, 340 );
    gaus->SetLineColor(codd);
    gaus->SetLineWidth(3);
    peaks_odd->Fit(gaus,"QRS");

    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(385 ,550);
    peaks_odd->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(740 ,900);
    peaks_odd->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1300,1420);
    peaks_odd->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1840,1940);
    gaus->SetParameter(1, 1895);
    peaks_odd->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());
    //
    // gaus->SetRange(1880,1980);
    // peaks_even->Fit(gaus,"QRS");
    // gaussians_even.push_back((TF1*)gaus->Clone());

    // ----- Legend -----

    // TLegend* leg =  new TLegend(0.63, 0.66, 0.82, 0.82);
    TLegend* leg =  new TLegend(0.5, 0.68, 0.7, 0.84);
    	    //leg->SetHeader("");
    	//    leg->SetNColumns(2);
    	leg->SetBorderSize(0);
    	leg->SetFillColor(0);

    	// leg->AddEntry(make_box)

    	int legfontsize = leg->GetTextSize();
    	int legfont     = leg->GetTextFont();

     leg->AddEntry(make_box(her, "EMPTY"), target.data.c_str(), "f");
     leg->AddEntry(set_line_width(peaks_even), "Even Multiples of T_{rev}", "p");
     leg->AddEntry(set_line_width(peaks_odd), "Odd Multiples of T_{rev}", "p");

    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* etext = new TLatex();
//    etext->SetNDC();
    etext->SetTextFont(43);
    etext->SetTextSize(gStyle->GetLegendTextSize());
    etext->SetTextColor(codd);
  // -----   Axis   ---------------------
    TH1F* axis = new TH1F("axis", "axis", 1000000, xlow, xup);
    axis->SetDirectory(0);

    axis->GetXaxis()->SetTitle("Peak to Peak Distance [#mus]");
//    axis->GetYaxis()->SetTitleOffset(1.7);
    axis->GetYaxis()->SetTitle("Weighted Entries [MIP^{2}/(0.8 ns)^{3}]");

    // axis->GetXaxis()->SetRangeUser(-0.015, 4.05);
    //axis->GetYaxis()->SetRangeUser(0., her->GetMaximum()*0.8);
    axis->GetYaxis()->SetRangeUser(1e-6, 1e4);
    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "ler_injections_peak_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//	can->SetLogx();
    can->SetLogy();
//
//   // ---- DRAWING ----
    can->cd();
    axis->Draw("AXIS");
    her->Draw("SAME HIST");

//    her->Draw("SAME FUNC");
// /   gaus->Draw("same");
     peaks_even->Draw("SAME P");
     peaks_odd->Draw("SAME P");
    // single_marker->Draw("same");

    // gaus->Draw("same");

     ttext->DrawLatex(0.50, 0.88, ("PEAK Analysis: "+channel).c_str());
     leg->Draw();

    //  for(auto& gaus:gaussians_even)
    //  {
    //      gaus->Draw("same");
    //
    //      stringstream ss;
    //      ss << fixed << setprecision(0) << gaus->GetParameter(1) << " #mus";
    //      //
    //      // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
    //
    //     // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
    //      etext->DrawLatex(10+gaus->GetParameter(1),0.025+4.0*gaus->Eval(gaus->GetParameter(1)), ss.str().c_str() );
    // }

    cout << gaussians_even.size() << endl;

    gaussians_even.at(0)->Draw("same");

    // gaussians_even.at(1)->Draw("same");
    // gaussians_even.at(2)->Draw("same");
    // gaussians_even.at(3)->Draw("same");
   // gaussians_even.at(4)->Draw("same");
    stringstream ss;
    ss << fixed << setprecision(0) << "(" << gaussians_even.at(0)->GetParameter(1)<<"#pm"<< gaussians_even.at(0)->GetParError(1) << ") #mus";
    //
    // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");

   // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
    etext->DrawLatex(gaussians_even.at(0)->GetParameter(1)-100,gaussians_even.at(0)->Eval(gaussians_even.at(0)->GetParameter(1))+9, ss.str().c_str() );

    TMarker* m_0 = new TMarker(gaussians_even.at(0)->GetParameter(1), gaussians_even.at(0)->Eval(gaussians_even.at(0)->GetParameter(1))+4, 23);
    m_0->SetMarkerSize(2);
    m_0->SetMarkerColor(codd);

    m_0->Draw();

    // 1st peak
    gaussians_even.at(1)->Draw("same");
    stringstream ss2;
    ss2 << fixed << setprecision(0) << "(" << gaussians_even.at(1)->GetParameter(1)<<"#pm"<< gaussians_even.at(1)->GetParError(1) << ") #mus";

    etext->DrawLatex(gaussians_even.at(1)->GetParameter(1)-100,gaussians_even.at(1)->Eval(gaussians_even.at(1)->GetParameter(1))+1.5, ss2.str().c_str() );

    TMarker* m_1 = new TMarker(gaussians_even.at(1)->GetParameter(1), gaussians_even.at(1)->Eval(gaussians_even.at(1)->GetParameter(1))+0.75, 23);
    m_1->SetMarkerSize(2);
    m_1->SetMarkerColor(codd);

    m_1->Draw();

    // 2nd peak
   gaussians_even.at(2)->Draw("same");
   stringstream ss3;
   ss3 << fixed << setprecision(0) << "(" << gaussians_even.at(2)->GetParameter(1)<<"#pm"<< gaussians_even.at(2)->GetParError(1) << ") #mus";

   etext->DrawLatex(gaussians_even.at(2)->GetParameter(1)-150,gaussians_even.at(2)->Eval(gaussians_even.at(2)->GetParameter(1))+0.3, ss3.str().c_str() );

   TMarker* m_2 = new TMarker(gaussians_even.at(2)->GetParameter(1), gaussians_even.at(2)->Eval(gaussians_even.at(2)->GetParameter(1))+0.125, 23);
   m_2->SetMarkerSize(2);
   m_2->SetMarkerColor(codd);

   m_2->Draw();

   // 3nd peak
    gaussians_even.at(3)->Draw("same");
    stringstream ss4;
    ss4 << fixed << setprecision(0) << "(" << gaussians_even.at(3)->GetParameter(1)<<"#pm"<< gaussians_even.at(3)->GetParError(1) << ") #mus";

    etext->DrawLatex(gaussians_even.at(3)->GetParameter(1)-150, gaussians_even.at(3)->Eval(gaussians_even.at(3)->GetParameter(1))+0.225, ss4.str().c_str() );

    TMarker* m_3 = new TMarker(gaussians_even.at(3)->GetParameter(1), gaussians_even.at(3)->Eval(gaussians_even.at(3)->GetParameter(1))+0.1, 23);
    m_3->SetMarkerSize(2);
    m_3->SetMarkerColor(codd);

    m_3->Draw();

    // 3nd peak
    gaussians_even.at(4)->Draw("same");
    stringstream ss5;
    ss5 << fixed << setprecision(0) << "(" << gaussians_even.at(4)->GetParameter(1)<<"#pm"<< gaussians_even.at(4)->GetParError(1) << ") #mus";

    etext->DrawLatex(gaussians_even.at(4)->GetParameter(1)-150,gaussians_even.at(4)->Eval(gaussians_even.at(4)->GetParameter(1))+0.075, ss5.str().c_str() );

    TMarker* m_4 = new TMarker(gaussians_even.at(4)->GetParameter(1), gaussians_even.at(4)->Eval(gaussians_even.at(4)->GetParameter(1))+0.03, 23);
    m_4->SetMarkerSize(2);
    m_4->SetMarkerColor(codd);

    m_4->Draw();

 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".pdf").c_str());
 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".png").c_str());
 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".eps").c_str());
 can->SaveAs((target.pathout+"/Synchrotron/"+target.fileout+"_her_synchrotron_peak_"+channel+".jpg").c_str());


  return 0;
}

void ler_all( string channel = "FWD1")
{

    SetPhDStyle();

    Target target = GetTarget("LER-ALL");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* ler = (TH1F*) rfile->Get((channel+"_peak").c_str());
    ler->SetDirectory(0);

    // if(target.n != -1)
    // {
    //     double scale = target.n*0.8e-9;
    //     her->Scale(1./scale);
    // }

    // double dt = 0.8e-3; // in mus
    // double scaling = 1e-3;
    // double xlow, xup;
    //
    // xlow = -dt/2.;
    // xup = target.n*dt - dt/2.;
    //
    // ler->SetBins(target.n, ler->GetBinLowEdge(1)*scaling, (ler->GetBinLowEdge(target.n) + ler->GetBinWidth(target.n))*scaling );

    // Make the axis nice

    ler->GetXaxis()->SetTitleOffset(1.3);
    ler->GetYaxis()->SetTitle("Weighted Entries [MIP^{2}/(0.8 ns)^{3}]");
    ler->GetYaxis()->SetTitleOffset(0.65);

    if(channel == "FWD1")
    {
        ler->GetYaxis()->SetRangeUser(0, 90);
    }
    else if(channel == "FWD2")
    {
        ler->GetYaxis()->SetRangeUser(0, 30);
    }
    else if(channel == "FWD3")
    {
        ler->GetYaxis()->SetRangeUser(0, 13.5);
    }

    // Make the legend


    TLegend* leg =  new TLegend(0.75, 0.795, 0.84, 0.845);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    // leg->AddEntry(make_box)
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(ler, "EMPTY"), target.data.c_str(), "f");

    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

    // Make the rest look nice and set notes
    double linewidth = 2;

    int cdb = tab20_red->GetNumber();
    int ctrev = tab20_green->GetNumber();
    int conoff = tab20_orange->GetNumber();
    int cbeta = tab20_blue->GetNumber();
    int csyn = tab20_lila->GetNumber();

    double makerksize = 2.0;
    double nsig = 2.5;

    ler->SetLineWidth(linewidth);

    TF1* g1 = new TF1("gaus", "gaus(0)", 0, 1e6);
    g1->SetParameters(30,1e2,2.5);
    g1->SetNpx(2000);
    g1->SetLineColor(cdb);
    g1->SetLineWidth(linewidth+1);

    ler->Fit(g1,"NLQ", "", 90, 110 );
    ler->Fit(g1,"NLQ", "", g1->GetParameter(1) - nsig*g1->GetParameter(2), g1->GetParameter(1) + nsig*g1->GetParameter(2) );
    stringstream ss;
   //     ss << fixed << setprecision(0) << gaus->GetParameter(1) << " #mus";
    cout << "DB-Injection: " << fixed << setprecision(3) << g1->GetParameter(1) << " +- " << g1->GetParError(1) << endl;
    g1->SetRange(g1->GetParameter(1) - nsig*g1->GetParameter(2),  g1->GetParameter(1) + nsig*g1->GetParameter(2) );

    TMarker* mdb;

        if(channel == "FWD1")
        {
            mdb = new TMarker(g1->GetParameter(1), g1->GetMaximum()+3, 23);
        }
        else if(channel == "FWD2")
        {
            mdb = new TMarker(g1->GetParameter(1), g1->GetMaximum()+1, 23);
        }
        else if(channel == "FWD3")
        {
            mdb = new TMarker(g1->GetParameter(1)+1, g1->GetMaximum()+0.5, 23);
        }

//    TMarker* mdb = new TMarker(g1->GetParameter(1), g1->GetMaximum()+3, 23);
    mdb->SetMarkerSize(makerksize);
    mdb->SetMarkerColor(cdb);

    TLatex* tdb = new TLatex();
// /    deltatext->SetNDC();
    tdb->SetTextFont(leg->GetTextFont());
    tdb->SetTextSize(gStyle->GetLegendTextSize());
    tdb->SetTextColor(cdb);


    TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
    gtrev->SetParameters(14,10060,4);
    gtrev->SetNpx(2000);
    gtrev->SetLineColor(ctrev);
    gtrev->SetLineWidth(linewidth+1);

    ler->Fit(gtrev,"NLQ", "", 10050, 10070 );
    ler->Fit(gtrev,"NLQ", "", gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2), gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
    cout << "Trev: " << fixed << setprecision(3) << gtrev->GetParameter(1) << " +- " << gtrev->GetParError(1) << endl;
    gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );


TMarker* mtrev;
    if(channel == "FWD1")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+3, 23);
    }
    else if(channel == "FWD2")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+1, 23);
    }
    else if(channel == "FWD3")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+0.5, 23);
    }

//    TMarker* mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+3, 23);
    mtrev->SetMarkerSize(makerksize);
    mtrev->SetMarkerColor(ctrev);

    TLatex* ttrev = new TLatex();
// /    deltatext->SetNDC();
    ttrev->SetTextFont(leg->GetTextFont());
    ttrev->SetTextSize(gStyle->GetLegendTextSize());
    ttrev->SetTextColor(ctrev);

    TF1* gonoff = new TF1("gaus", "gaus(0)", 0, 1e6);
    gonoff->SetParameters(60,20125,4);
    gonoff->SetNpx(2000);
    gonoff->SetLineColor(conoff);
    gonoff->SetLineWidth(linewidth+1);

    ler->Fit(gonoff,"NLQ", "", 20110, 20130 );
    ler->Fit(gonoff,"NLQ", "", gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2), gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );
    cout << "On/off: " << fixed << setprecision(3) << gonoff->GetParameter(1) << " +- " << gonoff->GetParError(1) << endl;
    gonoff->SetRange(gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2),  gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );



    TMarker* monoff;
        if(channel == "FWD1")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+3, 23);
        }
        else if(channel == "FWD2")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+1, 23);
        }
        else if(channel == "FWD3")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+0.5, 23);
        }

//    TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+3, 23);
    monoff->SetMarkerSize(makerksize);
    monoff->SetMarkerColor(conoff);

    TLatex* tonoff = new TLatex();
// /    deltatext->SetNDC();
    tonoff->SetTextFont(leg->GetTextFont());
    tonoff->SetTextSize(gStyle->GetLegendTextSize());
    tonoff->SetTextColor(conoff);

    TF1* gbeta = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta->SetParameters(40,90552,2);
    gbeta->SetNpx(2000);
    gbeta->SetLineColor(cbeta);
    gbeta->SetLineWidth(linewidth+1);

    ler->Fit(gbeta,"NLQ", "", 90545, 90560 );
    ler->Fit(gbeta,"NLQ", "", gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2), gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );
    gbeta->SetRange(gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2),  gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );

    TMarker* mbeta;
        if(channel == "FWD1")
        {
            mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+3, 23);
        }
        else if(channel == "FWD2")
        {
            mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+2, 23);
        }
        else if(channel == "FWD3")
        {
            mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+1, 23);
        }

    //TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+3, 23);
    mbeta->SetMarkerSize(makerksize);
    mbeta->SetMarkerColor(cbeta);

    TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta2->SetParameters(5,100615,2);
    gbeta2->SetNpx(2000);
    gbeta2->SetLineColor(cbeta);
    gbeta2->SetLineWidth(linewidth+1);

    ler->Fit(gbeta2,"NLQ", "", 100610, 100620 );
    ler->Fit(gbeta2,"NLQ", "", gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
    gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );


    TMarker* mbeta2;
        if(channel == "FWD1")
        {
            mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+3, 23);
        }
        else if(channel == "FWD2")
        {
            mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+10.5, 23);
        }
        else if(channel == "FWD3")
        {
            mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+4.5, 23);
        }

//    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+3, 23);
    mbeta2->SetMarkerSize(makerksize);
    mbeta2->SetMarkerColor(cbeta);

    TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta3->SetParameters(25,110675,2);
    gbeta3->SetNpx(2000);
    gbeta3->SetLineColor(cbeta);
    gbeta3->SetLineWidth(linewidth+1);

    ler->Fit(gbeta3,"NLQ", "", 110670, 110680 );
    ler->Fit(gbeta3,"NLQ", "", gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );

    TMarker* mbeta3;
        if(channel == "FWD1")
        {
            mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+3, 23);
        }
        else if(channel == "FWD2")
        {
            mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+2, 23);
        }
        else if(channel == "FWD3")
        {
            mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+1, 23);
        }

    //TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+3, 23);
    mbeta3->SetMarkerSize(makerksize);
    mbeta3->SetMarkerColor(cbeta);

    TLatex* tbeta = new TLatex();
// /    deltatext->SetNDC();
    tbeta->SetTextFont(leg->GetTextFont());
    tbeta->SetTextSize(gStyle->GetLegendTextSize());
    tbeta->SetTextColor(cbeta);


    TMarker* msyn;
        if(channel == "FWD1")
        {
            msyn = new TMarker(8e5, 7, 23);
        }
        else if(channel == "FWD2")
        {
            msyn = new TMarker(8e5, 2, 23);
        }
        else if(channel == "FWD3")
        {
            msyn = new TMarker(8e5, 1, 23);
        }

//    TMarker* msyn = new TMarker(8e5, 7, 23);
    msyn->SetMarkerSize(makerksize);
    msyn->SetMarkerColor(csyn);

    TLatex* tsyn = new TLatex();
// /    deltatext->SetNDC();
    tsyn->SetTextFont(leg->GetTextFont());
    tsyn->SetTextSize(gStyle->GetLegendTextSize());
    tsyn->SetTextColor(csyn);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel);
    double ratio = 2./1.;

    string cantitle = "_peak_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetRightMargin(0.01);
    can->SetLeftMargin(0.08);
    can->SetTopMargin(0.02);


	can->SetLogx();
//    can->SetLogy();
//
//   // ---- DRAWING ----
    can->cd();
    ler->Draw("hist");

    ttext->DrawLatex(0.75, 0.88, ("PEAK Analysis: "+channel).c_str());
    leg->Draw();

    mdb->Draw();
    g1->Draw("same");

    stringstream ss1;
    ss1 << fixed << setprecision(1) << "#splitline{Double Bunch Injection:}{(" << g1->GetParameter(1)<<" #pm "<< g1->GetParError(2) << ") ns}";

    // tdb->DrawLatex(22, g1->GetMaximum()+8.5, ss1.str().c_str());

    //tdb->DrawLatex(22, g1->GetMaximum()+6.5, "Double Bunch Injection");

    mtrev->Draw();
    gtrev->Draw("same");
    stringstream ss2;
    ss2 << fixed << setprecision(1) << "T_{rev}: (10 061.2 #pm "<< gtrev->GetParError(2) << ") ns";
    //ss2 << fixed << setprecision(1) << "T_{rev}: (" << gtrev->GetParameter(1)<<" #pm "<< gtrev->GetParError(2) << ") ns";
    // ttrev->DrawLatex(600, gtrev->GetMaximum()+6, ss2.str().c_str());

    monoff->Draw();
    gonoff->Draw("same");
    // tonoff->DrawLatex(9000, gonoff->GetMaximum()+9, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");

    mbeta->Draw();
    gbeta->Draw("same");
    mbeta2->Draw();
    gbeta2->Draw("same");
    mbeta3->Draw();
    gbeta3->Draw("same");
    // tbeta->DrawLatex(70000, gbeta->GetMaximum()+9, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
    // tbeta->DrawLatex(48000, gbeta->GetMaximum()+10, "#splitline{#approx9#upointT_{rev}}{#approx10#upointT_{rev}}");
    // tbeta->DrawLatex(48000, gbeta->GetMaximum()+8, "#approx11#upointT_{rev}");


    msyn->Draw();
    //tsyn->DrawLatex(2.5e5, 3+10, "#splitline{Synchrotron}{Oscillations}");



            if(channel == "FWD1")
            {
                    tdb->DrawLatex(22, g1->GetMaximum()+8.5, ss1.str().c_str());
                ttrev->DrawLatex(600, gtrev->GetMaximum()+6, ss2.str().c_str());

                tonoff->DrawLatex(9000, gonoff->GetMaximum()+9, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");
                tbeta->DrawLatex(70000, gbeta->GetMaximum()+9, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
                tsyn->DrawLatex(2.5e5, 3+10, "#splitline{Synchrotron}{Oscillations}");

            }
            else if(channel == "FWD2")
            {
                tdb->DrawLatex(22, g1->GetMaximum()+3.5, ss1.str().c_str());
                ttrev->DrawLatex(600, gtrev->GetMaximum()+2.5, ss2.str().c_str());

                tonoff->DrawLatex(9000, gonoff->GetMaximum()+3, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");
                tbeta->DrawLatex(70000, gbeta->GetMaximum()+3, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
                tsyn->DrawLatex(2.5e5, 4, "#splitline{Synchrotron}{Oscillations}");

            }
            else if(channel == "FWD3")
            {
                    tdb->DrawLatex(22, g1->GetMaximum()+1.5, ss1.str().c_str());
                ttrev->DrawLatex(600, gtrev->GetMaximum()+1, ss2.str().c_str());

                tonoff->DrawLatex(9000, gonoff->GetMaximum()+1.5, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");
                tbeta->DrawLatex(70000, gbeta->GetMaximum()+1.5, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
                tsyn->DrawLatex(2.5e5, 2, "#splitline{Synchrotron}{Oscillations}");

            }


//     axis->Draw("AXIS");
//     her->Draw("SAME HIST");
//
// //    her->Draw("SAME FUNC");
// // /   gaus->Draw("same");
//      peaks_even->Draw("SAME P");
//      peaks_odd->Draw("SAME P");
//      single_marker->Draw("same");
//
//     // gaus->Draw("same");
//
//      ttext->DrawLatex(0.50, 0.88, ("PEAK Analysis: "+channel).c_str());
//      leg->Draw();
//
//      // for(auto& gaus:gaussians_even)
     // {
     //     gaus->Draw("same");
     //
     //     stringstream ss;
     //     ss << fixed << setprecision(0) << gaus->GetParameter(1) << " #mus";
     //     //
     //     // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
     //
     //    // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
     //     etext->DrawLatex(1.*gaus->GetParameter(1),0.05+gaus->Eval(gaus->GetParameter(1)), ss.str().c_str() );
     // }

 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".pdf").c_str());
 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".png").c_str());
 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".eps").c_str());
 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".jpg").c_str());

}

void her_ref_zoom( string channel = "FWD1")
{

    SetPhDStyle();

    Target target = GetTarget("HER-REF");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* her = (TH1F*) rfile->Get((channel+"_peak").c_str());
    her->SetDirectory(0);

    // if(target.n != -1)
    // {
    //     double scale = target.n*0.8e-9;
    //     her->Scale(1./scale);
    // }

    double dt = 0.8e-3; // in mus
    double scaling = 1e-3;
    double xlow, xup;

    xlow = -dt/2.;
    xup = target.n*dt - dt/2.;

    her->SetBins(target.n, her->GetBinLowEdge(1)*scaling, (her->GetBinLowEdge(target.n) + her->GetBinWidth(target.n))*scaling );

    her->GetXaxis()->SetTitleOffset(1.2);
    her->GetXaxis()->SetRangeUser(8, 160);
    her->GetXaxis()->SetTitle("Peak to Peak Distance [#mus]");
    her->GetYaxis()->SetTitle("Weighted Entries [MIP^{2}/(0.8 ns)^{3}]");
//    her->GetYaxis()->SetTitleOffset(0.65);


    if(channel == "FWD1")
    {
        her->GetYaxis()->SetRangeUser(0, 1.05);
    }
    else if(channel == "FWD2")
    {
        her->GetYaxis()->SetRangeUser(0, 0.6);
    }
    else if(channel == "FWD3")
    {
    her->GetYaxis()->SetRangeUser(0, 0.6);
    }

    // her->GetYaxis()->SetRangeUser(0, 1.05);
    // Make the legend

    TLegend* leg =  new TLegend(0.525, 0.795, 0.72, 0.845);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    // leg->AddEntry(make_box)
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(her, "EMPTY"), target.data.c_str(), "f");

    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

    // Make the rest look nice and set notes
    double linewidth = 2;

    int cdb = tab20_red->GetNumber();
    int ctrev = tab20_green->GetNumber();
    int conoff = tab20_orange->GetNumber();
    int cbeta = tab20_blue->GetNumber();
    int csyn = tab20_orange->GetNumber();

    double makerksize = 2;
    double nsig = 2.5;

    her->SetLineWidth(linewidth);
    // TF1* g1 = new TF1("gaus", "gaus(0)", 0, 1e6);
    // g1->SetParameters(30,1e2,2.5);
    // g1->SetNpx(2000);
    // g1->SetLineColor(cdb);
    // g1->SetLineWidth(linewidth+1);
    //
    // her->Fit(g1,"NL", "", 90, 110 );
    // her->Fit(g1,"NL", "", g1->GetParameter(1) - nsig*g1->GetParameter(2), g1->GetParameter(1) + nsig*g1->GetParameter(2) );
    // g1->SetRange(g1->GetParameter(1) - nsig*g1->GetParameter(2),  g1->GetParameter(1) + nsig*g1->GetParameter(2) );
    //
    //     TMarker* mdb = new TMarker(g1->GetParameter(1), g1->GetMaximum()+2, 23);
    //     mdb->SetMarkerSize(makerksize);
    //     mdb->SetMarkerColor(cdb);
    //
    //     TLatex* tdb = new TLatex();
    // // /    deltatext->SetNDC();
    //     tdb->SetTextFont(leg->GetTextFont());
    //     tdb->SetTextSize(gStyle->GetLegendTextSize()*0.8);
    //     tdb->SetTextColor(cdb);


        TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
        gtrev->SetParameters(14,10060,4);
        gtrev->SetNpx(2000);
        gtrev->SetLineColor(ctrev);
        gtrev->SetLineWidth(linewidth+1);

        her->Fit(gtrev,"NL", "", 10.050, 10.070 );
        her->Fit(gtrev,"NL", "", gtrev->GetParameter(1) - 2*gtrev->GetParameter(2), gtrev->GetParameter(1) + 2*gtrev->GetParameter(2) );
        gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );


        // TMarker* mtrev ;
        // if(channel == "FWD1")
        // {
        //     mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 0.05, 23);
        // }
        // else if(channel == "FWD2")
        // {
        //     mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 0.05, 23);
        // }
        // else if(channel == "FWD3")
        // {
        //     mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 0.05, 23);
        // }

         TMarker* mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 0.05, 23);
        mtrev->SetMarkerSize(makerksize);
        mtrev->SetMarkerColor(ctrev);

        TLatex* ttrev = new TLatex();
    // /    deltatext->SetNDC();
        ttrev->SetTextFont(leg->GetTextFont());
        ttrev->SetTextSize(gStyle->GetLegendTextSize());
        ttrev->SetTextColor(ctrev);

        TF1* gonoff = new TF1("gaus", "gaus(0)", 0, 1e6);
        gonoff->SetParameters(60,20.125,4);
        gonoff->SetNpx(2000);
        gonoff->SetLineColor(conoff);
        gonoff->SetLineWidth(linewidth+1);

        her->Fit(gonoff,"NL", "", 20.110, 20.130 );
        her->Fit(gonoff,"NL", "", gonoff->GetParameter(1) - 1.5*gonoff->GetParameter(2), gonoff->GetParameter(1) + 1.5*gonoff->GetParameter(2) );
        gonoff->SetRange(gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2),  gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );


        TMarker* monoff ;
        if(channel == "FWD1")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 0.1, 23);
        }
        else if(channel == "FWD2")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 0.05, 23);
        }
        else if(channel == "FWD3")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 0.05, 23);
        }

        // TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 0.1, 23);
        monoff->SetMarkerSize(makerksize);
        monoff->SetMarkerColor(conoff);

        TLatex* tonoff = new TLatex();
    // /    deltatext->SetNDC();
        tonoff->SetTextFont(leg->GetTextFont());
        tonoff->SetTextSize(gStyle->GetLegendTextSize());
        tonoff->SetTextColor(conoff);

        TF1* gbeta = new TF1("gaus", "gaus(0)", 0, 1e6);
        gbeta->SetParameters(40,90552,2);
        gbeta->SetNpx(2000);
        gbeta->SetLineColor(cbeta);
        gbeta->SetLineWidth(linewidth+1);

        her->Fit(gbeta,"NL", "", 90.545, 90.560 );
        her->Fit(gbeta,"NL", "", gbeta->GetParameter(1) - 2*gbeta->GetParameter(2), gbeta->GetParameter(1) + 2*gbeta->GetParameter(2) );
        gbeta->SetRange(gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2),  gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );

        TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+0.04, 23);
        mbeta->SetMarkerSize(makerksize);
        mbeta->SetMarkerColor(cbeta);

        TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
        gbeta2->SetParameters(5,100615,2);
        gbeta2->SetNpx(2000);
        gbeta2->SetLineColor(cbeta);
        gbeta2->SetLineWidth(linewidth+1);

        her->Fit(gbeta2,"NL", "", 100.610, 100.620 );
        her->Fit(gbeta2,"NL", "", gbeta2->GetParameter(1) - 2*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + 2*gbeta2->GetParameter(2) );
        gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );

    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+0.04, 23);
    mbeta2->SetMarkerSize(makerksize);
    mbeta2->SetMarkerColor(cbeta);

    TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta3->SetParameters(25,110675,2);
    gbeta3->SetNpx(2000);
    gbeta3->SetLineColor(cbeta);
    gbeta3->SetLineWidth(linewidth+1);

    her->Fit(gbeta3,"NL", "", 110.670, 110.680 );
    her->Fit(gbeta3,"NL", "", gbeta3->GetParameter(1) - 2*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + 2*gbeta3->GetParameter(2) );
    gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );

    TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+0.04, 23);
    mbeta3->SetMarkerSize(makerksize);
    mbeta3->SetMarkerColor(cbeta);

    TLatex* tbeta = new TLatex();
    // /    deltatext->SetNDC();
    tbeta->SetTextFont(leg->GetTextFont());
    tbeta->SetTextSize(gStyle->GetLegendTextSize());
    tbeta->SetTextColor(cbeta);


    TMarker* msyn = new TMarker(8e5, 3, 23);
    msyn->SetMarkerSize(makerksize);
    msyn->SetMarkerColor(csyn);

    TLatex* tsyn = new TLatex();
    // /    deltatext->SetNDC();
    tsyn->SetTextFont(leg->GetTextFont());
    tsyn->SetTextSize(gStyle->GetLegendTextSize());
    tsyn->SetTextColor(csyn);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2);
    double ratio = 1./1.;

    string cantitle = "_peak_her_ref_zoom" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    // can->SetRightMargin(0.01);
    // can->SetLeftMargin(0.08);
    // can->SetTopMargin(0.02);


    can->SetLogx();
    //    can->SetLogy();
    //
    //   // ---- DRAWING ----
    can->cd();
    her->Draw("hist");

    ttext->DrawLatex(0.525, 0.88, ("PEAK Analysis: "+channel).c_str());
    leg->Draw();

    // mdb->Draw();
    // g1->Draw("same");
    // tdb->DrawLatex(50, g1->GetMaximum()+7, "#splitline{Double Bunch}{Injection}");

    mtrev->Draw();
    gtrev->Draw("same");

    // ttrev->DrawLatex(9.25, gtrev->GetMaximum()+0.085, "T_{rev}");

    monoff->Draw();
    gonoff->Draw("same");
    // tonoff->DrawLatex(11.7500, gonoff->GetMaximum()+0.14, "On/off-pattern: #approx2#upointT_{rev}");

    mbeta->Draw();
    gbeta->Draw("same");
    mbeta2->Draw();
    gbeta2->Draw("same");
    mbeta3->Draw();
    gbeta3->Draw("same");


    if(channel == "FWD1")
    {
        ttrev->DrawLatex(9.25, gtrev->GetMaximum()+0.085, "T_{rev}");
        tonoff->DrawLatex(11.7500, gonoff->GetMaximum()+0.14, "On/off-pattern: #approx2#upointT_{rev}");
        tbeta->DrawLatex(65.000, 0.425, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");
    }
    else if(channel == "FWD2")
    {
        ttrev->DrawLatex(9.25, gtrev->GetMaximum()+0.085, "T_{rev}");
        tonoff->DrawLatex(11.7500, gonoff->GetMaximum()+0.08, "On/off-pattern: #approx2#upointT_{rev}");
        tbeta->DrawLatex(65.000, 0.275, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");
    }
    else if(channel == "FWD3")
    {
        ttrev->DrawLatex(9.25, gtrev->GetMaximum()+0.085, "T_{rev}");
        tonoff->DrawLatex(11.7500, gonoff->GetMaximum()+0.08, "On/off-pattern: #approx2#upointT_{rev}");
        tbeta->DrawLatex(65.000, 0.275, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");
    }

    // tbeta->DrawLatex(65.000, 0.425, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");


    msyn->Draw();
    tsyn->DrawLatex(3.5e5, 3+5, "#splitline{Synchrotron}{Oscillations}");

     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_her_zoom_"+channel+".pdf").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_her_zoom_"+channel+".png").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_her_zoom_"+channel+".eps").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_her_zoom_"+channel+".jpg").c_str());

}

void ler_ref_zoom( string channel = "FWD1")
{

    SetPhDStyle();

    Target target = GetTarget("LER-REF");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* ler = (TH1F*) rfile->Get((channel+"_peak").c_str());
    ler->SetDirectory(0);

    // if(target.n != -1)
    // {
    //     double scale = target.n*0.8e-9;
    //     ler->Scale(1./scale);
    // }

    double dt = 0.8e-3; // in mus
    double scaling = 1e-3;
    double xlow, xup;

    xlow = -dt/2.;
    xup = target.n*dt - dt/2.;

    ler->SetBins(target.n, ler->GetBinLowEdge(1)*scaling, (ler->GetBinLowEdge(target.n) + ler->GetBinWidth(target.n))*scaling );

    ler->GetXaxis()->SetTitleOffset(1.2);
    ler->GetXaxis()->SetRangeUser(8, 160);
    ler->GetXaxis()->SetTitle("Peak to Peak Distance [#mus]");
    ler->GetYaxis()->SetTitle("Weighted Entries [MIP^{2}/(0.8 ns)^{3}]");


    if(channel == "FWD1")
    {
    ler->GetYaxis()->SetRangeUser(0, 360);

    }
    else if(channel == "FWD2")
    {
    ler->GetYaxis()->SetRangeUser(0, 90);
    }
    else if(channel == "FWD3")
    {
    ler->GetYaxis()->SetRangeUser(0, 45);
    }

//    ler->GetYaxis()->SetTitleOffset(0.65);
    // ler->GetYaxis()->SetRangeUser(0, 360);
    // Make the legend

    TLegend* leg =  new TLegend(0.525, 0.795, 0.72, 0.845);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    // leg->AddEntry(make_box)
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(ler, "EMPTY"), target.data.c_str(), "f");

    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

    // Make the rest look nice and set notes
    double linewidth = 2;

    int cdb = tab20_red->GetNumber();
    int ctrev = tab20_green->GetNumber();
    int conoff = tab20_orange->GetNumber();
    int cbeta = tab20_blue->GetNumber();
    int csyn = tab20_orange->GetNumber();

    double makerksize = 2;
    double nsig = 2;

    ler->SetLineWidth(linewidth);

        TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
        gtrev->SetParameters(14,10060,4);
        gtrev->SetNpx(2000);
        gtrev->SetLineColor(ctrev);
        gtrev->SetLineWidth(linewidth+1);

        ler->Fit(gtrev,"NL", "", 10.050, 10.070 );
        ler->Fit(gtrev,"NL", "", gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2), gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
        gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );

        TMarker* mtrev ;
        if(channel == "FWD1")
        {
            mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 9, 23);
        }
        else if(channel == "FWD2")
        {
            mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 3, 23);
        }
        else if(channel == "FWD3")
        {
            mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 2, 23);
        }

        //TMarker* mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 9, 23);
        mtrev->SetMarkerSize(makerksize);
        mtrev->SetMarkerColor(ctrev);

        TLatex* ttrev = new TLatex();
    // /    deltatext->SetNDC();
        ttrev->SetTextFont(leg->GetTextFont());
        ttrev->SetTextSize(gStyle->GetLegendTextSize());
        ttrev->SetTextColor(ctrev);

        TF1* gonoff = new TF1("gaus", "gaus(0)", 0, 1e6);
        gonoff->SetParameters(60,20.125,4);
        gonoff->SetNpx(2000);
        gonoff->SetLineColor(conoff);
        gonoff->SetLineWidth(linewidth+1);

        ler->Fit(gonoff,"NL", "", 20.110, 20.130 );
        ler->Fit(gonoff,"NL", "", gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2), gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );
        gonoff->SetRange(gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2),  gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );



                TMarker* monoff ;
                if(channel == "FWD1")
                {
                    monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 14, 23);
                }
                else if(channel == "FWD2")
                {
                    monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 3, 23);
                }
                else if(channel == "FWD3")
                {
                    monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 2, 23);
                }



        // TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 14, 23);
        monoff->SetMarkerSize(makerksize);
        monoff->SetMarkerColor(conoff);

        TLatex* tonoff = new TLatex();
    // /    deltatext->SetNDC();
        tonoff->SetTextFont(leg->GetTextFont());
        tonoff->SetTextSize(gStyle->GetLegendTextSize());
        tonoff->SetTextColor(conoff);

        TF1* gbeta = new TF1("gaus", "gaus(0)", 0, 1e6);
        gbeta->SetParameters(40,90552,2);
        gbeta->SetNpx(2000);
        gbeta->SetLineColor(cbeta);
        gbeta->SetLineWidth(linewidth+1);

        ler->Fit(gbeta,"NL", "", 90.545, 90.560 );
        ler->Fit(gbeta,"NL", "", gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2), gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );
        gbeta->SetRange(gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2),  gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );

        TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+7, 23);
        mbeta->SetMarkerSize(makerksize);
        mbeta->SetMarkerColor(cbeta);

        TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
        gbeta2->SetParameters(5,100615,2);
        gbeta2->SetNpx(2000);
        gbeta2->SetLineColor(cbeta);
        gbeta2->SetLineWidth(linewidth+1);

        ler->Fit(gbeta2,"NL", "", 100.610, 100.620 );
        ler->Fit(gbeta2,"NL", "", gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
        gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );

    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+9, 23);
    mbeta2->SetMarkerSize(makerksize);
    mbeta2->SetMarkerColor(cbeta);

    TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta3->SetParameters(25,110675,2);
    gbeta3->SetNpx(2000);
    gbeta3->SetLineColor(cbeta);
    gbeta3->SetLineWidth(linewidth+1);

    ler->Fit(gbeta3,"NL", "", 110.670, 110.680 );
    ler->Fit(gbeta3,"NL", "", gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );

    TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+ 9, 23);
    mbeta3->SetMarkerSize(makerksize);
    mbeta3->SetMarkerColor(cbeta);

    TLatex* tbeta = new TLatex();
    // /    deltatext->SetNDC();
    tbeta->SetTextFont(leg->GetTextFont());
    tbeta->SetTextSize(gStyle->GetLegendTextSize());
    tbeta->SetTextColor(cbeta);


    TMarker* msyn = new TMarker(8e5, 3, 23);
    msyn->SetMarkerSize(makerksize);
    msyn->SetMarkerColor(csyn);

    TLatex* tsyn = new TLatex();
    // /    deltatext->SetNDC();
    tsyn->SetTextFont(leg->GetTextFont());
    tsyn->SetTextSize(gStyle->GetLegendTextSize());
    tsyn->SetTextColor(csyn);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2);
    double ratio = 1./1.;

    string cantitle = "_peak_ler_ref_zoom" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    // can->SetRightMargin(0.01);
    // can->SetLeftMargin(0.08);
    // can->SetTopMargin(0.02);


    can->SetLogx();
    //    can->SetLogy();
    //
    //   // ---- DRAWING ----
    can->cd();
    ler->Draw("hist");

    ttext->DrawLatex(0.525, 0.88, ("PEAK Analysis: "+channel).c_str());
    leg->Draw();

    // mdb->Draw();
    // g1->Draw("same");
    // tdb->DrawLatex(50, g1->GetMaximum()+7, "#splitline{Double Bunch}{Injection}");

    mtrev->Draw();
    gtrev->Draw("same");
    // ttrev->DrawLatex(9.25, gtrev->GetMaximum()+20, "T_{rev}");

    monoff->Draw();
    gonoff->Draw("same");
    // tonoff->DrawLatex(11.75, gonoff->GetMaximum()+28, "On/off-pattern: #approx2#upointT_{rev}");

    mbeta->Draw();
    gbeta->Draw("same");
    mbeta2->Draw();
    gbeta2->Draw("same");
    mbeta3->Draw();
    gbeta3->Draw("same");

    if(channel == "FWD1")
    {
        ttrev->DrawLatex(9.5, gtrev->GetMaximum()+20, "T_{rev}");
        tonoff->DrawLatex(11.75, gonoff->GetMaximum()+28, "On/off-pattern: #approx2#upointT_{rev}");
 tbeta->DrawLatex(65.000, 245, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");

    }
    else if(channel == "FWD2")
    {
        ttrev->DrawLatex(9.25, gtrev->GetMaximum()+7, "T_{rev}");
        tonoff->DrawLatex(13, gonoff->GetMaximum()+7, "On/off-pattern: #approx2#upointT_{rev}");
 tbeta->DrawLatex(65.000, 58, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");
    }
    else if(channel == "FWD3")
    {
        ttrev->DrawLatex(9.25, gtrev->GetMaximum()+5, "T_{rev}");
        tonoff->DrawLatex(13, gonoff->GetMaximum()+5, "On/off-pattern: #approx2#upointT_{rev}");
        tbeta->DrawLatex(65.000, 29, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");
    }


    // tbeta->DrawLatex(65.000, 235, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{#approx  9#upointT_{rev}}{#splitline{#approx10#upointT_{rev}}{#approx11#upointT_{rev}}}}");


    msyn->Draw();
    tsyn->DrawLatex(3.5e5, 3+5, "#splitline{Synchrotron}{Oscillations}");

     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".pdf").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".png").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".eps").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".jpg").c_str());

}

void her_all( string channel = "FWD1")
{

    SetPhDStyle();

    Target target = GetTarget("HER-ALL");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* her = (TH1F*) rfile->Get((channel+"_peak").c_str());
    her->SetDirectory(0);

    // Make the axis nice

    her->GetXaxis()->SetTitleOffset(1.5);
    her->GetYaxis()->SetTitle("Weighted Entries [MIP^{2}/(0.8 ns)^{3}]");
    her->GetYaxis()->SetTitleOffset(0.9);

    // if(channel == "FWD1")
    // {
        her->GetXaxis()->SetRangeUser(5000, 2400000);
        her->GetYaxis()->SetRangeUser(0, 3.5);
    // }
    // else if(channel == "FWD2")
    // {
    //     her->GetYaxis()->SetRangeUser(0, 30);
    // }
    // else if(channel == "FWD3")
    // {
    //     her->GetYaxis()->SetRangeUser(0, 13.5);
    // }

    // Make the legend

    TLegend* leg =  new TLegend(0.735, 0.68, 0.845, 0.845);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    // leg->AddEntry(make_box)
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(her, "EMPTY"), target.data.c_str(), "f");


    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

    // Make the rest look nice and set notes
    double linewidth = 1;

    int cdb = tab20_red->GetNumber();
    int ctrev = tab20_green->GetNumber();
    int conoff = tab20_orange->GetNumber();
    int cbeta = tab20_lila->GetNumber();
    int csyn = tab20_blue->GetNumber();

    double makerksize = 2.0;
    double nsig = 2.5;

    her->SetLineWidth(linewidth);

    // TF1* g1 = new TF1("gaus", "gaus(0)", 0, 1e6);
    // g1->SetParameters(30,1e2,2.5);
    // g1->SetNpx(2000);
    // g1->SetLineColor(cdb);
    // g1->SetLineWidth(linewidth+1);


    TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
    gtrev->SetParameters(14,10060,4);
    gtrev->SetNpx(20000);
    gtrev->SetLineColor(ctrev);
    gtrev->SetLineWidth(linewidth+1);

    her->Fit(gtrev,"NLQ", "", 10054, 10068 );
    //her->Fit(gtrev,"NLQ", "", gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2), gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
    cout << "Trev: " << fixed << setprecision(3) << gtrev->GetParameter(1) << " +- " << gtrev->GetParError(1) << endl;
    gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
    gtrev->SetRange(10054,  10068);

    TMarker* mtrev;
    if(channel == "FWD1")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+0.25, 23);
    }
    else if(channel == "FWD2")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+1, 23);
    }
    else if(channel == "FWD3")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+0.5, 23);
    }

//    TMarker* mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()+3, 23);
    mtrev->SetMarkerSize(makerksize);
    mtrev->SetMarkerColor(ctrev);

    TLatex* ttrev = new TLatex();
// /    deltatext->SetNDC();
    ttrev->SetTextFont(leg->GetTextFont());
    ttrev->SetTextSize(gStyle->GetLegendTextSize());
    ttrev->SetTextColor(ctrev);

    TF1* gonoff = new TF1("gaus", "gaus(0)", 0, 1e7);
    gonoff->SetParameters(2.05,20122,3.8);
    gonoff->SetNpx(20000);
    gonoff->SetLineColor(conoff);
    gonoff->SetLineWidth(linewidth+1);

    her->Fit(gonoff,"NLQ", "", 20119, 20127 );
    //her->Fit(gonoff,"NLQ", "", gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2), gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );
    cout << "On/off: " << fixed << setprecision(3) << gonoff->GetParameter(1) << " +- " << gonoff->GetParError(1) << endl;
    gonoff->SetRange(gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2),  gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );



    TMarker* monoff;
        if(channel == "FWD1")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+0.35, 23);
        }
        else if(channel == "FWD2")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+1, 23);
        }
        else if(channel == "FWD3")
        {
            monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+0.5, 23);
        }

//    TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+3, 23);
    monoff->SetMarkerSize(makerksize);
    monoff->SetMarkerColor(conoff);

    TLatex* tonoff = new TLatex();
// /    deltatext->SetNDC();
    tonoff->SetTextFont(leg->GetTextFont());
    tonoff->SetTextSize(gStyle->GetLegendTextSize());
    tonoff->SetTextColor(conoff);

    TF1* gbeta = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta->SetParameters(40,90552,2);
    gbeta->SetNpx(2000);
    gbeta->SetLineColor(cbeta);
    gbeta->SetLineWidth(linewidth+1);

    her->Fit(gbeta,"NLQ", "", 90545, 90560 );
    her->Fit(gbeta,"NLQ", "", gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2), gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );
    gbeta->SetRange(gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2),  gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );

    TMarker* mbeta;
        if(channel == "FWD1")
        {
            mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+0.3, 23);
        }
        else if(channel == "FWD2")
        {
            mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+2, 23);
        }
        else if(channel == "FWD3")
        {
            mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+1, 23);
        }

    //TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+3, 23);
    mbeta->SetMarkerSize(makerksize);
    mbeta->SetMarkerColor(cbeta);

    TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta2->SetParameters(5,100615,2);
    gbeta2->SetNpx(2000);
    gbeta2->SetLineColor(cbeta);
    gbeta2->SetLineWidth(linewidth+1);

    her->Fit(gbeta2,"NLQ", "", 100610, 100620 );
    her->Fit(gbeta2,"NLQ", "", gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
    gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );


    TMarker* mbeta2;
        if(channel == "FWD1")
        {
            mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+0.3, 23);
        }
        else if(channel == "FWD2")
        {
            mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+10.5, 23);
        }
        else if(channel == "FWD3")
        {
            mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+4.5, 23);
        }

//    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+3, 23);
    mbeta2->SetMarkerSize(makerksize);
    mbeta2->SetMarkerColor(cbeta);

    TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta3->SetParameters(25,110675,2);
    gbeta3->SetNpx(2000);
    gbeta3->SetLineColor(cbeta);
    gbeta3->SetLineWidth(linewidth+1);

    her->Fit(gbeta3,"NLQ", "", 110670, 110680 );
    her->Fit(gbeta3,"NLQ", "", gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );

    TMarker* mbeta3;
        if(channel == "FWD1")
        {
            mbeta3 = new TMarker(gbeta3->GetParameter(1), 0.75, 23);
            mbeta2->SetY(1.15);
            mbeta->SetY(1.55);
        }
        else if(channel == "FWD2")
        {
            mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+2, 23);
        }
        else if(channel == "FWD3")
        {
            mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+1, 23);
        }

    //TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+3, 23);
    mbeta3->SetMarkerSize(makerksize);
    mbeta3->SetMarkerColor(cbeta);

    TLatex* tbeta = new TLatex();
// /    deltatext->SetNDC();
    tbeta->SetTextFont(leg->GetTextFont());
    tbeta->SetTextSize(gStyle->GetLegendTextSize());
    tbeta->SetTextColor(cbeta);

    // Synchrotron
    // double dt = 0.8e-3; // in mus
    double dt = 0.8; // in ns
    double scaling = 1e-3;
    double xlow, xup;

    double t_rev =  10061.4;
    // double t_rev =  10061.4*1e-3;

    TGraph* peaks_odd = new TGraph();
    TGraph* peaks_even = new TGraph();

    vector <TF1*> gaussians_even;
    vector <TF1*> gaussians_odd;

    double thres = her->GetMaximum()*0.0125;
    for(int bin = 250000; bin < her->GetNbinsX(); ++bin)
    {
        if(her->GetBinContent(bin) >thres)
         {
            if(fmod(bin*dt,t_rev) < t_rev*0.1 || fmod(bin*dt,t_rev)> t_rev*0.9)
            {
                double max_bin = bin;
                double max = 0;
                for(int i = bin-20 ;i<bin+60; i++ )
                {
                    if(her->GetBinContent(i)>max)
                    {
                        max_bin = i;
                        max = her->GetBinContent(i);
                    }
                }

                TF1* gaus = new TF1("gaus", "gaus", max_bin*dt-0.003, max_bin*dt+0.003 );
                gaus->SetParameters(1,her->GetBinCenter(max_bin), 0.01);
                her->Fit(gaus,"QR");
    //             gaus->SetLineColor(kMagenta);
                double mean = gaus->GetParameter(1);
    //
                if(int(round(mean/t_rev))%2) peaks_odd->SetPoint(peaks_odd->GetN(), mean, gaus->GetMaximum());
                else peaks_even->SetPoint(peaks_even->GetN(), mean, gaus->GetMaximum());
    //
    //             // peaks_even->SetPoint(peaks_even->GetN(), mean, gaus->GetMaximum());
    //             // cout << "X: " <<mean<< ", Y: " <<gaus->GetMaximum()<< ", at: " << bin*dt << endl;
    //             // delete gaus;
    //
    //             // cout << "bin*dt: " << bin*dt << " fmod(bin*dt,t_rev): " << fmod(bin*dt,t_rev)<< ", mean: " << mean<< endl;
                bin += 500;
    //             // gaussians.push_back(gaus);
    //
                delete gaus;
            }
        }
    }


    // ----- Aufbitchen -----

    int codd = tab20_red->GetNumber();
    int ceven = tab20_blue->GetNumber();

    double markersize = 0.8;
    double glinewidth = 3;

    peaks_odd->SetLineColor(codd);
    peaks_odd->SetLineWidth(glinewidth);
    peaks_odd->SetMarkerColor(codd);
    peaks_odd->SetMarkerSize(markersize);
    peaks_odd->SetMarkerStyle(23);

    peaks_even->SetLineColor(ceven);
    peaks_even->SetLineWidth(glinewidth);
    peaks_even->SetMarkerColor(ceven);
    peaks_even->SetMarkerSize(markersize);
    peaks_even->SetMarkerStyle(22);
    //
    // TMarker* single_marker = new TMarker(0, her->GetMaximum()*0.8, 22);
    // single_marker->SetMarkerSize(2.4);
    // single_marker->SetMarkerColor(kBlack);
    //
    TF1* gaus = new TF1("gaus", "gaus", 680000, 820000 );
    gaus->SetLineColor(ceven);
    gaus->SetLineWidth(3);
    peaks_even->Fit(gaus,"RS");

    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(325000,435000);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1050000,1250000);
    gbeta3->SetParameters(0.35,1150000,70162);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    gaus->SetRange(1460000,1600000);
    peaks_even->Fit(gaus,"QRS");
    gaussians_even.push_back((TF1*)gaus->Clone());

    // gaus->SetRange(1880000,1980000);
    // peaks_even->Fit(gaus,"QRS");
    // gaussians_even.push_back((TF1*)gaus->Clone());

    //

    // single_marker->Draw("same");


    TLatex* tsyn = new TLatex();
// /    deltatext->SetNDC();
    tsyn->SetTextFont(leg->GetTextFont());
    tsyn->SetTextSize(gStyle->GetLegendTextSize());
    tsyn->SetTextColor(csyn);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel);
    double ratio = 2./1.;

    string cantitle = "_peak_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetRightMargin(0.01);
    can->SetLeftMargin(0.06);
    can->SetBottomMargin(0.1);
    can->SetTopMargin(0.025);


	can->SetLogx();
//    can->SetLogy();
//
//   // ---- DRAWING ----
    can->cd();
    her->Draw("hist");

    ttext->DrawLatex(0.74, 0.88, ("Autocorrelation Analysis: "+channel).c_str());
    // leg->Draw();

    // mdb->Draw();
    // g1->Draw("same");

    // stringstream ss1;
    // ss1 << fixed << setprecision(1) << "#splitline{Double Bunch Injection:}{(" << g1->GetParameter(1)<<" #pm "<< g1->GetParError(2) << ") ns}";

    // tdb->DrawLatex(22, g1->GetMaximum()+8.5, ss1.str().c_str());

    //tdb->DrawLatex(22, g1->GetMaximum()+6.5, "Double Bunch Injection");

    mtrev->Draw();
    gtrev->Draw("same");
    stringstream ss2;
    ss2 << fixed << setprecision(0) << "T_{rev}: (" << gtrev->GetParameter(1) << " #pm "<< gtrev->GetParError(1) << ") ns";
    //ss2 << fixed << setprecision(1) << "T_{rev}: (" << gtrev->GetParameter(1)<<" #pm "<< gtrev->GetParError(2) << ") ns";
    // ttrev->DrawLatex(600, gtrev->GetMaximum()+6, ss2.str().c_str());

    monoff->Draw();
    gonoff->Draw("same");
    // tonoff->DrawLatex(9000, gonoff->GetMaximum()+9, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");

    mbeta->Draw();
//    gbeta->Draw("same");
    mbeta2->Draw();
//    gbeta2->Draw("same");
    mbeta3->Draw();
//    gbeta3->Draw("same");
    // tbeta->DrawLatex(70000, gbeta->GetMaximum()+9, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
    // tbeta->DrawLatex(48000, gbeta->GetMaximum()+10, "#splitline{#approx9#upointT_{rev}}{#approx10#upointT_{rev}}");
    // tbeta->DrawLatex(48000, gbeta->GetMaximum()+8, "#approx11#upointT_{rev}");


    //tsyn->DrawLatex(2.5e5, 3+10, "#splitline{Synchrotron}{Oscillations}");


            if(channel == "FWD1")
            {
                    // tdb->DrawLatex(22, g1->GetMaximum()+8.5, ss1.str().c_str());
                ttrev->DrawLatex(6500, gtrev->GetMaximum()+0.5, ss2.str().c_str());

                tonoff->DrawLatex(12000, gonoff->GetMaximum()+0.6, "Betatron oscillations: #approx 2#upointT_{rev}");
                // tbeta->DrawLatex(90000, gbeta->GetMaximum()+1.3, "#splitline{Long Betatron Pattern:}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
                    tbeta->DrawLatex(70000, 1.95, "Long Betatron Pattern:");
                    tbeta->DrawLatex(85000, 1.7, "#approx 9#upointT_{rev}");
                    tbeta->DrawLatex(90000, 1.3, "#approx 10#upointT_{rev}");
                    tbeta->DrawLatex(100000, 0.9, "#approx 11#upointT_{rev}");
            //    tsyn->DrawLatex(2.5e5, 3+10, "#splitline{Synchrotron}{Oscillations}");
                tsyn->DrawLatex(2e5, 1.5, "Synchrotron Oscillations: #approx 409 #mus");

            }
            else if(channel == "FWD2")
            {
                // tdb->DrawLatex(22, g1->GetMaximum()+3.5, ss1.str().c_str());
                ttrev->DrawLatex(600, gtrev->GetMaximum()+2.5, ss2.str().c_str());

                tonoff->DrawLatex(9000, gonoff->GetMaximum()+3, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");
                tbeta->DrawLatex(70000, gbeta->GetMaximum()+3, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
            //    tsyn->DrawLatex(2.5e5, 4, "#splitline{Synchrotron}{Oscillations}");

            }
            else if(channel == "FWD3")
            {
                    // tdb->DrawLatex(22, g1->GetMaximum()+1.5, ss1.str().c_str());
                ttrev->DrawLatex(600, gtrev->GetMaximum()+1, ss2.str().c_str());

                tonoff->DrawLatex(9000, gonoff->GetMaximum()+1.5, "#splitline{On/off-pattern:}{#approx2#upointT_{rev}}");
                tbeta->DrawLatex(70000, gbeta->GetMaximum()+1.5, "#splitline{#splitline{Long Beta-}{tron Pattern:}}{#splitline{      #approx  9#upointT_{rev}}{#splitline{      #approx10#upointT_{rev}}{      #approx11#upointT_{rev}}}}");
            //    tsyn->DrawLatex(2.5e5, 2, "#splitline{Synchrotron}{Oscillations}");
            }


    peaks_even->Draw("SAME P");
    peaks_odd->Draw("SAME P");


    TLatex* etext = new TLatex();
//    etext->SetNDC();
etext->SetTextFont(43);
etext->SetTextSize(gStyle->GetLegendTextSize());
etext->SetTextColor(ceven);


    cout << gaussians_even.size() << endl;

    gaussians_even.at(0)->Draw("same");

    // gaussians_even.at(1)->Draw("same");
    // gaussians_even.at(2)->Draw("same");
    // gaussians_even.at(3)->Draw("same");
   // gaussians_even.at(4)->Draw("same");
    stringstream ss;
    ss << fixed << setprecision(0) << "(" << gaussians_even.at(0)->GetParameter(1)/1e3 <<"#pm"<< gaussians_even.at(0)->GetParError(1)/1e3 << ") #mus";
    //
    // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");

   // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
    // etext->DrawLatex(gaussians_even.at(0)->GetParameter(1)-225,0.125+gaussians_even.at(0)->Eval(gaussians_even.at(0)->GetParameter(1)), ss.str().c_str() );
    etext->DrawLatex(550000,0.95, ss.str().c_str() );

    // TMarker* m_0 = new TMarker(gaussians_even.at(0)->GetParameter(1), gaussians_even.at(0)->Eval(gaussians_even.at(0)->GetParameter(1))+0.065, 23);
    TMarker* m_0 = new TMarker(gaussians_even.at(0)->GetParameter(1), 0.55, 23);
    m_0->SetMarkerSize(2);
    m_0->SetMarkerColor(ceven);

    m_0->Draw();


    // 1st peak
    gaussians_even.at(1)->Draw("same");
    // stringstream ss2;
    ss2.str("");
    ss2 << fixed << setprecision(0) << "(" << gaussians_even.at(1)->GetParameter(1)/1e3<<"#pm"<< gaussians_even.at(1)->GetParError(1)/1e3 << ") #mus";

    // etext->DrawLatex(gaussians_even.at(1)->GetParameter(1)-225,0.125+gaussians_even.at(1)->Eval(gaussians_even.at(1)->GetParameter(1)), ss2.str().c_str() );
    etext->DrawLatex(270000, 1.1, ss2.str().c_str() );

    TMarker* m_1 = new TMarker(gaussians_even.at(1)->GetParameter(1), 0.75, 23);
    m_1->SetMarkerSize(2);
    m_1->SetMarkerColor(ceven);

    m_1->Draw();

     // 2nd peak
    gaussians_even.at(2)->Draw("same");
    stringstream ss3;
    //ss3.str("");
    ss3 << fixed << setprecision(0) << "(" << gaussians_even.at(2)->GetParameter(1)/1e3<<"#pm"<< gaussians_even.at(2)->GetParError(1)/1e3 << ") #mus";

//    etext->DrawLatex(gaussians_even.at(2)->GetParameter(1)-225,0.125+gaussians_even.at(2)->Eval(gaussians_even.at(2)->GetParameter(1)), ss3.str().c_str() );
    etext->DrawLatex(850000,0.75, ss3.str().c_str() );

    // TMarker* m_2 = new TMarker(gaussians_even.at(2)->GetParameter(1), gaussians_even.at(2)->Eval(gaussians_even.at(2)->GetParameter(1))+0.065, 23);
    TMarker* m_2 = new TMarker(gaussians_even.at(2)->GetParameter(1), 0.4, 23);
    m_2->SetMarkerSize(2);
    m_2->SetMarkerColor(ceven);

    m_2->Draw();

    // 3nd peak
   gaussians_even.at(3)->Draw("same");
   stringstream ss4;
   ss4 << fixed << setprecision(0) << "#splitline{(" << gaussians_even.at(3)->GetParameter(1)/1e3<<"}{#pm"<< gaussians_even.at(3)->GetParError(1)/1e3 << ") #mus}";

   //etext->DrawLatex(gaussians_even.at(3)->GetParameter(1)-225,0.125+gaussians_even.at(3)->Eval(gaussians_even.at(3)->GetParameter(1)), ss4.str().c_str() );
   etext->DrawLatex(1.35e6,0.5, ss4.str().c_str() );

   // TMarker* m_3 = new TMarker(gaussians_even.at(3)->GetParameter(1), gaussians_even.at(3)->Eval(gaussians_even.at(3)->GetParameter(1))+0.065, 23);
   TMarker* m_3 = new TMarker(gaussians_even.at(3)->GetParameter(1), 0.3, 23);

   m_3->SetMarkerSize(2);
   m_3->SetMarkerColor(ceven);

   m_3->Draw();

   // 3nd peak
  // gaussians_even.at(4)->Draw("same");
  // stringstream ss5;
  // ss5 << fixed << setprecision(0) << "#splitline{(" << gaussians_even.at(4)->GetParameter(1)<<"}{#pm"<< gaussians_even.at(4)->GetParError(2) << ") #mus}";

  // etext->DrawLatex(gaussians_even.at(4)->GetParameter(1)-225,0.08+gaussians_even.at(4)->Eval(gaussians_even.at(4)->GetParameter(1)), ss5.str().c_str() );
  //etext->DrawLatex(1.5e6,0.45, ss5.str().c_str() );

  // TMarker* m_4 = new TMarker(gaussians_even.at(4)->GetParameter(1), gaussians_even.at(4)->Eval(gaussians_even.at(4)->GetParameter(1))+0.03, 23);
  // TMarker* m_4 = new TMarker(gaussians_even.at(4)->GetParameter(1), 0.2, 23);
  // m_4->SetMarkerSize(2);
  // m_4->SetMarkerColor(ceven);

  leg->AddEntry(set_line_width(peaks_even), "Even Multiples of T_{rev}", "p");
leg->AddEntry(set_line_width(peaks_odd), "Odd Multiples of T_{rev}", "p");

leg->Draw();
  //m_4->Draw();


 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".pdf").c_str());
 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".png").c_str());
 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".eps").c_str());
 can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_"+channel+".jpg").c_str());

}

void plt_peak()
{
    string chs[] = {"FWD1", "FWD2", "FWD3"};
    //string chs[] = {"FWD1"};
    for(auto& ch: chs)
    {
        ler_all(ch);
        // her_all(ch);
        //her_ref_zoom(ch);
        // ler_ref_zoom(ch);
         // her_synchrotron(ch);
         // ler_synchrotron(ch);
    }
}
