


int her_synchrotron(string channel = "FWD1")
{
    SetPhDStyle();

    Target target = GetTarget("ALL_HER");

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

    double markersize = 0.9;
    double glinewidth = 2;

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
    axis->GetYaxis()->SetTitle(" Particle Rate^{2} [MIP^{2}/s^{2}]");

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

     gaus->Draw("same");

     ttext->DrawLatex(0.50, 0.88, ("PEAK Analysis: "+channel).c_str());
     leg->Draw();

     for(auto& gaus:gaussians_even)
     {
         gaus->Draw("same");

         stringstream ss;
         ss << fixed << setprecision(0) << gaus->GetParameter(1) << " #mus";
         //
         // 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");

        // etext->DrawLatexNDC(0.5,0.5, ss.str().c_str() );
         etext->DrawLatex(1.*gaus->GetParameter(1),0.05+gaus->Eval(gaus->GetParameter(1)), ss.str().c_str() );
     }
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

    Target target = GetTarget("ALL_LER");

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

    double markersize = 0.9;
    double glinewidth = 2;

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

    // TF1* gaus = new TF1("gaus", "gaus", 680, 820 );
    // gaus->SetLineColor(ceven);
    // gaus->SetLineWidth(3);
    // peaks_even->Fit(gaus,"QRS");
    //
    // gaussians_even.push_back((TF1*)gaus->Clone());
    //
    // gaus->SetRange(325,435);
    // peaks_even->Fit(gaus,"QRS");
    // gaussians_even.push_back((TF1*)gaus->Clone());
    //
    // gaus->SetRange(1090,1210);
    // peaks_even->Fit(gaus,"QRS");
    // gaussians_even.push_back((TF1*)gaus->Clone());
    //
    // gaus->SetRange(1500,1600);
    // peaks_even->Fit(gaus,"QRS");
    // gaussians_even.push_back((TF1*)gaus->Clone());
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
    etext->SetTextColor(ceven);
  // -----   Axis   ---------------------
    TH1F* axis = new TH1F("axis", "axis", 1000000, xlow, xup);
    axis->SetDirectory(0);

    axis->GetXaxis()->SetTitle("Peak to Peak Distance [#mus]");
//    axis->GetYaxis()->SetTitleOffset(1.7);
    axis->GetYaxis()->SetTitle(" Particle Rate^{2} [MIP^{2}/s^{2}]");

    // axis->GetXaxis()->SetRangeUser(-0.015, 4.05);
    //axis->GetYaxis()->SetRangeUser(0., her->GetMaximum()*0.8);
    axis->GetYaxis()->SetRangeUser(1e-9, 1e3);
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
     single_marker->Draw("same");

    // gaus->Draw("same");

     ttext->DrawLatex(0.50, 0.88, ("PEAK Analysis: "+channel).c_str());
     leg->Draw();

     // for(auto& gaus:gaussians_even)
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

    TH1F* ler = (TH1F*) rfile->Get((channel+"_fft_mag_h").c_str());
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
    ler->Scale(1./ler->Integral());
    // Make the axis nice

    ler->GetXaxis()->SetTitleOffset(1.25);
    ler->GetYaxis()->SetTitle("Relative Magnitude [a.u.]");
    ler->GetYaxis()->SetTitleOffset(0.8);
    ler->GetYaxis()->SetRangeUser(0, 40e-6);

    ler->GetXaxis()->SetTitle("Frequency [Hz]");
    // Make the legend


    TLegend* leg =  new TLegend(0.7, 0.76, 0.79, 0.81);
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

    double makerksize = 2;
    double nsig = 3;

    ler->SetLineWidth(linewidth);

    TF1* g1 = new TF1("gaus", "gaus(0)", 5.35e6, 1.55e7);
    // g1->SetParameters(2.06e-5,1.03609e7,3.55473e6);
        g1->SetParameters(1.88e-5,1.03609e7,3.55473e6);
    // g1->FixParameter(1, 1.03609e7);
    // g1->FixParameter(2, 3.76473e6);
    g1->SetNpx(2000);
    g1->SetLineColor(cdb);
    g1->SetLineWidth(linewidth+1);


    //ler->Fit(g1,"Q", "", 5.5e6, 1.5e7 );
//    ler->Fit(g1,"Q", "", g1->GetParameter(1) - 1*g1->GetParameter(2), g1->GetParameter(1) + 1*g1->GetParameter(2) );
    stringstream ss;
   //     ss << fixed << setprecision(0) << gaus->GetParameter(1) << " #mus";
    cout << "DB-Injection: " << fixed << setprecision(3) << g1->GetParameter(1) << " +- " << g1->GetParError(1) << endl;
    // g1->SetRange(g1->GetParameter(1) - nsig*g1->GetParameter(2),  g1->GetParameter(1) + nsig*g1->GetParameter(2) );

    TMarker* mdb = new TMarker(1.038e7, 21.75e-6, 23);
    mdb->SetMarkerSize(makerksize);
    mdb->SetMarkerColor(cdb);

    TLatex* tdb = new TLatex();
// /    deltatext->SetNDC();
    tdb->SetTextFont(leg->GetTextFont());
    tdb->SetTextSize(gStyle->GetLegendTextSize());
    tdb->SetTextColor(cdb);


    TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
    gtrev->SetParameters(120,9.939e4,100);
    gtrev->SetNpx(2000);
    gtrev->SetLineColor(ctrev);
    gtrev->SetLineWidth(linewidth+1);

     ler->Fit(gtrev,"NLQ", "", 9e4, 1.1e5 );
     ler->Fit(gtrev,"Q", "", gtrev->GetParameter(1) - 1.25*gtrev->GetParameter(2), gtrev->GetParameter(1) + 1.25*gtrev->GetParameter(2) );
     cout << "Trev: " << fixed << setprecision(3) << gtrev->GetParameter(1) << " +- " << gtrev->GetParError(1) << endl;
     gtrev->SetRange(gtrev->GetParameter(1) - 1.25*gtrev->GetParameter(2),  gtrev->GetParameter(1) + 1.25*gtrev->GetParameter(2) );

    TMarker* mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()*1.2, 23);
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

    ler->Fit(gonoff,"NLQ", "", 3.8e4, 4.5e4 );
    ler->Fit(gonoff,"NLQ", "", gonoff->GetParameter(1) - 2*gonoff->GetParameter(2), gonoff->GetParameter(1) + 2*gonoff->GetParameter(2) );
    // cout << "On/off: " << fixed << setprecision(3) << gonoff->GetParameter(1) << " +- " << gonoff->GetParError(1) << endl;
     gonoff->SetRange(gonoff->GetParameter(1) - 2*gonoff->GetParameter(2),  gonoff->GetParameter(1) + 2*gonoff->GetParameter(2) );

    TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()*1.1, 23);
    monoff->SetMarkerSize(makerksize);
    monoff->SetMarkerColor(conoff);

    TLatex* tonoff = new TLatex();
// /    deltatext->SetNDC();
    tonoff->SetTextFont(leg->GetTextFont());
    tonoff->SetTextSize(gStyle->GetLegendTextSize());
    tonoff->SetTextColor(conoff);

    TF1* gonoff2 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gonoff2->SetParameters(60,5.4e4,1000);
    gonoff2->SetNpx(2000);
    gonoff2->SetLineColor(conoff);
    gonoff2->SetLineWidth(linewidth+1);

    ler->Fit(gonoff2,"Q", "", 4.5e4, 6.0e4 );
    ler->Fit(gonoff2,"NLQ", "", gonoff2->GetParameter(1) - 2*gonoff2->GetParameter(2), gonoff2->GetParameter(1) + 2*gonoff2->GetParameter(2) );
    cout << "On/off: " << fixed << setprecision(3) << gonoff->GetParameter(1) << " +- " << gonoff->GetParError(1) << endl;
    gonoff2->SetRange(gonoff2->GetParameter(1) - 2*gonoff2->GetParameter(2),  gonoff2->GetParameter(1) + 2*gonoff2->GetParameter(2) );

    TMarker* monoff2 = new TMarker(gonoff2->GetParameter(1), gonoff2->GetMaximum()*1.1, 23);
    monoff2->SetMarkerSize(makerksize);
    monoff2->SetMarkerColor(conoff);

    TLatex* tonoff2 = new TLatex();
// /    deltatext->SetNDC();
    tonoff2->SetTextFont(leg->GetTextFont());
    tonoff2->SetTextSize(gStyle->GetLegendTextSize());
    tonoff2->SetTextColor(conoff);


    TF1* gbeta = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta->SetParameters(8e-6,9900,2000);
    gbeta->SetNpx(2000);
    gbeta->SetLineColor(cbeta);
    gbeta->SetLineWidth(linewidth+1);

    ler->Fit(gbeta,"LQ", "", 0.75e4, 1.4e4 );
    ler->Fit(gbeta,"L", "", gbeta->GetParameter(1) - 2*gbeta->GetParameter(2), gbeta->GetParameter(1) + 2*gbeta->GetParameter(2) );
    gbeta->SetRange(gbeta->GetParameter(1) - 1.25*gbeta->GetParameter(2),  gbeta->GetParameter(1) + 1.25*gbeta->GetParameter(2) );


    TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()*1.2, 23);
    mbeta->SetMarkerSize(makerksize);
    mbeta->SetMarkerColor(cbeta);

    TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta2->SetParameters(5,100615,2);
    gbeta2->SetNpx(2000);
    gbeta2->SetLineColor(cbeta);
    gbeta2->SetLineWidth(linewidth+1);

    // ler->Fit(gbeta2,"NLQ", "", 100610, 100620 );
    // ler->Fit(gbeta2,"NLQ", "", gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
    // gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );

    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+2, 23);
    mbeta2->SetMarkerSize(makerksize);
    mbeta2->SetMarkerColor(cbeta);

    TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta3->SetParameters(25,110675,2);
    gbeta3->SetNpx(2000);
    gbeta3->SetLineColor(cbeta);
    gbeta3->SetLineWidth(linewidth+1);

    // ler->Fit(gbeta3,"NLQ", "", 110670, 110680 );
    // ler->Fit(gbeta3,"NLQ", "", gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    // gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );

    TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+2, 23);
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
    int width = round(vis_multi*textwidth*pt2pixel);
    double ratio = 2./1.;

    string cantitle = "ler_all_fft_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetRightMargin(0.01);
    can->SetLeftMargin(0.08);
    can->SetTopMargin(0.06);


	can->SetLogx();
//    can->SetLogy();
//
//   // ---- DRAWING ----
    can->cd();
    ler->Draw("hist");

    ttext->DrawLatex(0.7, 0.84, ("Frequency Spectrum: "+channel).c_str());
    leg->Draw();

    mdb->Draw();
    g1->Draw("same");
    tdb->DrawLatex(2.7e6, 24e-6, "#splitline{Double Bunch Injection:}{10.4 MHz}");

    mtrev->Draw();
    gtrev->Draw("same");
    stringstream ss2;
    // ss1 << fixed << setprecision(1) << "#splitline{#splitline{Long Beta-}{tron Pattern:}}{(" << gbeta->GetParameter(1)<<" #pm "<< gbeta->GetParError(2) << ") Hz}";
    ss2 << fixed << setprecision(1) << "T_{rev}: " << gtrev->GetParameter(1)*1e-3<<" kHz " ;

    ttrev->DrawLatex(6e4, gtrev->GetMaximum()*1.3, ss2.str().c_str());
    //
    monoff->Draw();
    gonoff->Draw("same");

    stringstream ss3;
    // ss1 << fixed << setprecision(1) << "#splitline{#splitline{Long Beta-}{tron Pattern:}}{(" << gbeta->GetParameter(1)<<" #pm "<< gbeta->GetParError(2) << ") Hz}";
    ss3 << fixed << setprecision(1) << "#splitline{On/off-pattern:}{#splitline{        " << gonoff->GetParameter(1)*1e-3<<" kHz}{        "<<  gonoff2->GetParameter(1)*1e-3<<" kHz}}" ;

    tonoff->DrawLatex(1.25e4, gonoff->GetMaximum()+7e-6, ss3.str().c_str());

    monoff2->Draw();
    gonoff2->Draw("same");
//    tonoff2->DrawLatex(9000, gonoff->GetMaximum()+5, "on/off-pattern");
    //
    mbeta->Draw();
    gbeta->Draw("same");

    // mbeta2->Draw();
    // gbeta2->Draw("same");
    // mbeta3->Draw();
    // gbeta3->Draw("same");
    cout << gbeta->GetParError(1) << endl;
    stringstream ss1;
    // ss1 << fixed << setprecision(1) << "#splitline{#splitline{Long Beta-}{tron Pattern:}}{(" << gbeta->GetParameter(1)<<" #pm "<< gbeta->GetParError(2) << ") Hz}";
        ss1 << fixed << setprecision(1) << "#splitline{#splitline{Long Beta-}{tron Pattern:}}{"<< gbeta->GetParameter(1)*1e-3<<" kHz}";

    tbeta->DrawLatex(5000, gbeta->GetMaximum()+4e-6, ss1.str().c_str());
    //
    //
    // msyn->Draw();
    // tsyn->DrawLatex(2.5e5, 3+5, "#splitline{Synchrotron}{Oscillations}");
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

 can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_peak_"+channel+".pdf").c_str());
 can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_peak_"+channel+".png").c_str());
 can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_peak_"+channel+".eps").c_str());
 can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_peak_"+channel+".jpg").c_str());

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
    her->GetYaxis()->SetTitle("Particle Rate^{2} [(MIP/0.8 ns)^{2}]");
//    her->GetYaxis()->SetTitleOffset(0.65);
    her->GetYaxis()->SetRangeUser(0, 1.05);
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

    double makerksize = 1.8;
    double nsig = 3;

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
        her->Fit(gtrev,"NL", "", gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2), gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
        gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );

        TMarker* mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 0.04, 23);
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
        her->Fit(gonoff,"NL", "", gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2), gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );
        gonoff->SetRange(gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2),  gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );

        TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 0.1, 23);
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
        her->Fit(gbeta,"NL", "", gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2), gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );
        gbeta->SetRange(gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2),  gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );

        TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+0.03, 23);
        mbeta->SetMarkerSize(makerksize);
        mbeta->SetMarkerColor(cbeta);

        TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
        gbeta2->SetParameters(5,100615,2);
        gbeta2->SetNpx(2000);
        gbeta2->SetLineColor(cbeta);
        gbeta2->SetLineWidth(linewidth+1);

        her->Fit(gbeta2,"NL", "", 100.610, 100.620 );
        her->Fit(gbeta2,"NL", "", gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
        gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );

    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+0.03, 23);
    mbeta2->SetMarkerSize(makerksize);
    mbeta2->SetMarkerColor(cbeta);

    TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    gbeta3->SetParameters(25,110675,2);
    gbeta3->SetNpx(2000);
    gbeta3->SetLineColor(cbeta);
    gbeta3->SetLineWidth(linewidth+1);

    her->Fit(gbeta3,"NL", "", 110.670, 110.680 );
    her->Fit(gbeta3,"NL", "", gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );

    TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+0.03, 23);
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
    ttrev->DrawLatex(9.25, gtrev->GetMaximum()+0.075, "T_{rev}");

    monoff->Draw();
    gonoff->Draw("same");
    tonoff->DrawLatex(14.000, gonoff->GetMaximum()+0.13, "on/off-pattern");

    mbeta->Draw();
    gbeta->Draw("same");
    mbeta2->Draw();
    gbeta2->Draw("same");
    mbeta3->Draw();
    gbeta3->Draw("same");
    tbeta->DrawLatex(70.000, 0.245, "#splitline{Betatron}{Oscillations}");


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
    ler->GetYaxis()->SetTitle("Particle Rate^{2} [(MIP/0.8 ns)^{2}]");
//    ler->GetYaxis()->SetTitleOffset(0.65);
    ler->GetYaxis()->SetRangeUser(0, 320);
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

    double makerksize = 1.8;
    double nsig = 3;

    ler->SetLineWidth(linewidth);

        TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
        gtrev->SetParameters(14,10060,4);
        gtrev->SetNpx(2000);
        gtrev->SetLineColor(ctrev);
        gtrev->SetLineWidth(linewidth+1);

        ler->Fit(gtrev,"NL", "", 10.050, 10.070 );
        ler->Fit(gtrev,"NL", "", gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2), gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
        gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );

        TMarker* mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 6, 23);
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

        TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 6, 23);
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

        TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+6, 23);
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

    TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+6, 23);
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

    TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+ 6, 23);
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
    ttrev->DrawLatex(9.25, gtrev->GetMaximum()+16, "T_{rev}");

    monoff->Draw();
    gonoff->Draw("same");
    tonoff->DrawLatex(14.000, gonoff->GetMaximum()+16, "on/off-pattern");

    mbeta->Draw();
    gbeta->Draw("same");
    mbeta2->Draw();
    gbeta2->Draw("same");
    mbeta3->Draw();
    gbeta3->Draw("same");
    tbeta->DrawLatex(70.000, 187, "#splitline{Betatron}{Oscillations}");


    msyn->Draw();
    tsyn->DrawLatex(3.5e5, 3+5, "#splitline{Synchrotron}{Oscillations}");

     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".pdf").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".png").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".eps").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_peak_ler_zoom_"+channel+".jpg").c_str());

}

void ler_db( string channel = "FWD1")
{

    SetPhDStyle();

    Target target = GetTarget("LER-ALL");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* ler = (TH1F*) rfile->Get((channel+"_fft_mag_h").c_str());
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

//    ler->SetBins(target.n, ler->GetBinLowEdge(1)*scaling, (ler->GetBinLowEdge(target.n) + ler->GetBinWidth(target.n))*scaling );

    ler->GetXaxis()->SetTitleOffset(1.2);
//    ler->GetXaxis()->SetRangeUser(1e4, 1e8);
//    ler->GetXaxis()->SetTitle("Peak to Peak Distance [#mus]");
    ler->GetYaxis()->SetTitle("Relative Magnitude [a.u.]");
//    ler->GetYaxis()->SetTitleOffset(0.65);
//    ler->GetYaxis()->SetRangeUser(0, 320);
    // Make the legend

    // TLegend* leg =  new TLegend(0.525, 0.795, 0.72, 0.845);
    // leg->SetBorderSize(0);
    // leg->SetFillColor(0);
    //
    // // leg->AddEntry(make_box)
    // int legfontsize = leg->GetTextSize();
    // int legfont     = leg->GetTextFont();
    //
    // leg->AddEntry(make_box(ler, "EMPTY"), target.data.c_str(), "f");
    //
    // TLatex* ttext = new TLatex();
    // ttext->SetNDC();
    // ttext->SetTextFont(legfont+20);
    // ttext->SetTextSize(gStyle->GetLegendTextSize());
    //
    // // Make the rest look nice and set notes
    // double linewidth = 2;
    //
    // int cdb = tab20_red->GetNumber();
    // int ctrev = tab20_green->GetNumber();
    // int conoff = tab20_orange->GetNumber();
    // int cbeta = tab20_blue->GetNumber();
    // int csyn = tab20_orange->GetNumber();
    //
    // double makerksize = 1.8;
    // double nsig = 3;
    //
    // ler->SetLineWidth(linewidth);
    //
    //     TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
    //     gtrev->SetParameters(14,10060,4);
    //     gtrev->SetNpx(2000);
    //     gtrev->SetLineColor(ctrev);
    //     gtrev->SetLineWidth(linewidth+1);
    //
    //     ler->Fit(gtrev,"NL", "", 10.050, 10.070 );
    //     ler->Fit(gtrev,"NL", "", gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2), gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
    //     gtrev->SetRange(gtrev->GetParameter(1) - nsig*gtrev->GetParameter(2),  gtrev->GetParameter(1) + nsig*gtrev->GetParameter(2) );
    //
    //     TMarker* mtrev = new TMarker(gtrev->GetParameter(1) , gtrev->GetMaximum() + 6, 23);
    //     mtrev->SetMarkerSize(makerksize);
    //     mtrev->SetMarkerColor(ctrev);
    //
    //     TLatex* ttrev = new TLatex();
    // // /    deltatext->SetNDC();
    //     ttrev->SetTextFont(leg->GetTextFont());
    //     ttrev->SetTextSize(gStyle->GetLegendTextSize());
    //     ttrev->SetTextColor(ctrev);
    //
    //     TF1* gonoff = new TF1("gaus", "gaus(0)", 0, 1e6);
    //     gonoff->SetParameters(60,20.125,4);
    //     gonoff->SetNpx(2000);
    //     gonoff->SetLineColor(conoff);
    //     gonoff->SetLineWidth(linewidth+1);
    //
    //     ler->Fit(gonoff,"NL", "", 20.110, 20.130 );
    //     ler->Fit(gonoff,"NL", "", gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2), gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );
    //     gonoff->SetRange(gonoff->GetParameter(1) - nsig*gonoff->GetParameter(2),  gonoff->GetParameter(1) + nsig*gonoff->GetParameter(2) );
    //
    //     TMarker* monoff = new TMarker(gonoff->GetParameter(1), gonoff->GetMaximum()+ 6, 23);
    //     monoff->SetMarkerSize(makerksize);
    //     monoff->SetMarkerColor(conoff);
    //
    //     TLatex* tonoff = new TLatex();
    // // /    deltatext->SetNDC();
    //     tonoff->SetTextFont(leg->GetTextFont());
    //     tonoff->SetTextSize(gStyle->GetLegendTextSize());
    //     tonoff->SetTextColor(conoff);
    //
    //     TF1* gbeta = new TF1("gaus", "gaus(0)", 0, 1e6);
    //     gbeta->SetParameters(40,90552,2);
    //     gbeta->SetNpx(2000);
    //     gbeta->SetLineColor(cbeta);
    //     gbeta->SetLineWidth(linewidth+1);
    //
    //     ler->Fit(gbeta,"NL", "", 90.545, 90.560 );
    //     ler->Fit(gbeta,"NL", "", gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2), gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );
    //     gbeta->SetRange(gbeta->GetParameter(1) - nsig*gbeta->GetParameter(2),  gbeta->GetParameter(1) + nsig*gbeta->GetParameter(2) );
    //
    //     TMarker* mbeta = new TMarker(gbeta->GetParameter(1), gbeta->GetMaximum()+6, 23);
    //     mbeta->SetMarkerSize(makerksize);
    //     mbeta->SetMarkerColor(cbeta);
    //
    //     TF1* gbeta2 = new TF1("gaus", "gaus(0)", 0, 1e6);
    //     gbeta2->SetParameters(5,100615,2);
    //     gbeta2->SetNpx(2000);
    //     gbeta2->SetLineColor(cbeta);
    //     gbeta2->SetLineWidth(linewidth+1);
    //
    //     ler->Fit(gbeta2,"NL", "", 100.610, 100.620 );
    //     ler->Fit(gbeta2,"NL", "", gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2), gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
    //     gbeta2->SetRange(gbeta2->GetParameter(1) - nsig*gbeta2->GetParameter(2),  gbeta2->GetParameter(1) + nsig*gbeta2->GetParameter(2) );
    //
    // TMarker* mbeta2 = new TMarker(gbeta2->GetParameter(1), gbeta2->GetMaximum()+6, 23);
    // mbeta2->SetMarkerSize(makerksize);
    // mbeta2->SetMarkerColor(cbeta);
    //
    // TF1* gbeta3 = new TF1("gaus", "gaus(0)", 0, 1e6);
    // gbeta3->SetParameters(25,110675,2);
    // gbeta3->SetNpx(2000);
    // gbeta3->SetLineColor(cbeta);
    // gbeta3->SetLineWidth(linewidth+1);
    //
    // ler->Fit(gbeta3,"NL", "", 110.670, 110.680 );
    // ler->Fit(gbeta3,"NL", "", gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2), gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    // gbeta3->SetRange(gbeta3->GetParameter(1) - nsig*gbeta3->GetParameter(2),  gbeta3->GetParameter(1) + nsig*gbeta3->GetParameter(2) );
    //
    // TMarker* mbeta3 = new TMarker(gbeta3->GetParameter(1), gbeta3->GetMaximum()+ 6, 23);
    // mbeta3->SetMarkerSize(makerksize);
    // mbeta3->SetMarkerColor(cbeta);
    //
    // TLatex* tbeta = new TLatex();
    // // /    deltatext->SetNDC();
    // tbeta->SetTextFont(leg->GetTextFont());
    // tbeta->SetTextSize(gStyle->GetLegendTextSize());
    // tbeta->SetTextColor(cbeta);
    //
    //
    // TMarker* msyn = new TMarker(8e5, 3, 23);
    // msyn->SetMarkerSize(makerksize);
    // msyn->SetMarkerColor(csyn);
    //
    // TLatex* tsyn = new TLatex();
    // // /    deltatext->SetNDC();
    // tsyn->SetTextFont(leg->GetTextFont());
    // tsyn->SetTextSize(gStyle->GetLegendTextSize());
    // tsyn->SetTextColor(csyn);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1./2.);
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


//    can->SetLogx();
    //    can->SetLogy();
    //
    //   // ---- DRAWING ----
    can->cd();
    ler->Draw("hist");

    // ttext->DrawLatex(0.525, 0.88, ("PEAK Analysis: "+channel).c_str());
    // leg->Draw();
    //
    // // mdb->Draw();
    // // g1->Draw("same");
    // // tdb->DrawLatex(50, g1->GetMaximum()+7, "#splitline{Double Bunch}{Injection}");
    //
    // mtrev->Draw();
    // gtrev->Draw("same");
    // ttrev->DrawLatex(9.25, gtrev->GetMaximum()+16, "T_{rev}");
    //
    // monoff->Draw();
    // gonoff->Draw("same");
    // tonoff->DrawLatex(14.000, gonoff->GetMaximum()+16, "on/off-pattern");
    //
    // mbeta->Draw();
    // gbeta->Draw("same");
    // mbeta2->Draw();
    // gbeta2->Draw("same");
    // mbeta3->Draw();
    // gbeta3->Draw("same");
    // tbeta->DrawLatex(70.000, 187, "#splitline{Betatron}{Oscillations}");
    //
    //
    // msyn->Draw();
    // tsyn->DrawLatex(3.5e5, 3+5, "#splitline{Synchrotron}{Oscillations}");

     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_fft_ler_db_"+channel+".pdf").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_fft_ler_db_"+channel+".png").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_fft_ler_db_"+channel+".eps").c_str());
     can->SaveAs((target.pathout+"/PEAK/"+target.fileout+"_fft_ler_db_"+channel+".jpg").c_str());

}

void nj_bs( string channel = "FWD1")
{

    SetPhDStyle();

    Target target = GetTarget("NJ-VACS");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* ler = (TH1F*) rfile->Get((channel+"_fft_mag_h").c_str());
    ler->SetDirectory(0);

    // if(target.n != -1)
    // {
    //     double scale = target.n*0.8e-9;
    //     ler->Scale(1./scale);
    // }

    double dt = 0.8e-3; // in mus
    double scaling = 1e-6;
    double xlow, xup;

    xlow = -dt/2.;
    xup = target.n*dt - dt/2.;

    ler->SetBins(target.n/2., ler->GetBinLowEdge(1)*scaling, (ler->GetBinLowEdge(target.n/2.) + ler->GetBinWidth(target.n/2.))*scaling );

    ler->Scale(1/ler->Integral());
    ler->GetXaxis()->SetTitleOffset(1.25);
    ler->GetYaxis()->SetTitleOffset(0.8);
    ler->GetYaxis()->SetRangeUser(5e-9, 1e-3);
    ler->GetXaxis()->SetTitle("Frequency [MHz]");
    ler->GetYaxis()->SetTitle("Relative Magnitude [a.u.]");
//    ler->GetYaxis()->SetTitleOffset(0.65);
//    ler->GetYaxis()->SetRangeUser(0, 320);
    // Make the legend

    TLegend* leg =  new TLegend(0.7, 0.76, 0.79, 0.81);
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
    //
    // // Make the rest look nice and set notes
     double linewidth = 3;
    //
    int cdb = tab20_red->GetNumber();
    int ctrev = tab20_cyan->GetNumber();
    int conoff = tab20_orange->GetNumber();
    int cbeta = tab20_blue->GetNumber();
    int csyn = tab20_orange->GetNumber();

    double makerksize = 2;
    double nsig = 3;
    //
    // ler->SetLineWidth(linewidth);
    //
        TF1* gtrev = new TF1("gaus", "gaus(0)", 0, 1e6);
        gtrev->SetParameters(14,166.18,0.001);
        gtrev->SetNpx(2000);
        gtrev->SetLineColor(ctrev);
        gtrev->SetLineWidth(linewidth);

    ler->Fit(gtrev,"", "", 166.178, 166.181 );
    ler->Fit(gtrev,"NL", "", gtrev->GetParameter(1) - 2*gtrev->GetParameter(2), gtrev->GetParameter(1) + 2*gtrev->GetParameter(2) );
    gtrev->SetRange(gtrev->GetParameter(1) - 2*gtrev->GetParameter(2),  gtrev->GetParameter(1) + 2*gtrev->GetParameter(2) );

    TMarker* mtrev ;
    if (channel == "FWD1")
    {
            mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()*1.65, 23);
    }
    else if(channel == "FWD2")
    {
        mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()*1.85, 23);
    }
    else if(channel == "FWD3")
    {
            mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()*1.85, 23);
    }

    //TMarker* mtrev = new TMarker(gtrev->GetParameter(1), gtrev->GetMaximum()*1.85, 23);
    mtrev->SetMarkerSize(makerksize);
    mtrev->SetMarkerColor(ctrev);

    TLatex* ttrev = new TLatex();
    // /    deltatext->SetNDC();
    ttrev->SetTextFont(leg->GetTextFont());
    ttrev->SetTextSize(gStyle->GetLegendTextSize());
    ttrev->SetTextColor(ctrev);

    // ---- Do the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel);
    double ratio = 2./1.;

    string cantitle = "_peak_ler_ref_zoom" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetRightMargin(0.01);
    can->SetLeftMargin(0.08);
    can->SetTopMargin(0.06);

    // can->SetRightMargin(0.01);
    // can->SetLeftMargin(0.08);
    // can->SetTopMargin(0.02);


//    can->SetLogx();
        can->SetLogy();
    //
    //   // ---- DRAWING ----
    can->cd();
    ler->Draw("hist");



    ttext->DrawLatex(0.7, 0.84, ("Frequency Spectrum: "+channel).c_str());
    leg->Draw();
    //
    // // mdb->Draw();
    // // g1->Draw("same");
    // // tdb->DrawLatex(50, g1->GetMaximum()+7, "#splitline{Double Bunch}{Injection}");
    //
    // mtrev->Draw();
    mtrev->Draw();
    gtrev->Draw("same");

    stringstream ss;
    ss << fixed << setprecision(1) << "Bunch Spacing: " << gtrev->GetParameter(1)<<" MHz";


    if (channel == "FWD1")
    {
            ttrev->DrawLatex(gtrev->GetParameter(1)-75, gtrev->GetMaximum()*2.5, ss.str().c_str());
    }
    else if(channel == "FWD2")
    {
            ttrev->DrawLatex(gtrev->GetParameter(1)-75, gtrev->GetMaximum()*3, ss.str().c_str());
    }
    else if(channel == "FWD3")
    {
            ttrev->DrawLatex(gtrev->GetParameter(1)-75, gtrev->GetMaximum()*3, ss.str().c_str());
    }
    // ttrev->DrawLatex(gtrev->GetParameter(1)-75, gtrev->GetMaximum()*2.5, ss.str().c_str());
//    ttrev->DrawLatex(9.25, gtrev->GetMaximum()+16, "T_{rev}");
    //
    // monoff->Draw();
    // gonoff->Draw("same");
    // tonoff->DrawLatex(14.000, gonoff->GetMaximum()+16, "on/off-pattern");
    //
    // mbeta->Draw();
    // gbeta->Draw("same");
    // mbeta2->Draw();
    // gbeta2->Draw("same");
    // mbeta3->Draw();
    // gbeta3->Draw("same");
    // tbeta->DrawLatex(70.000, 187, "#splitline{Betatron}{Oscillations}");
    //
    //
    // msyn->Draw();
    // tsyn->DrawLatex(3.5e5, 3+5, "#splitline{Synchrotron}{Oscillations}");

     can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_fft_bs_"+channel+".pdf").c_str());
     can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_fft_bs_"+channel+".png").c_str());
     can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_fft_bs_"+channel+".eps").c_str());
     can->SaveAs((target.pathout+"/FFT/"+target.fileout+"_fft_bs_"+channel+".jpg").c_str());

}

void plt_fft()
{
    //string chs[] = {"FWD1", "FWD2", "FWD3"};
    string chs[] = {"FWD3"};
    for(auto& ch: chs)
    {
       ler_all(ch);
    //    ler_db(ch);
    //    nj_bs(ch);
    //    her_ref_zoom(ch);
    //    ler_ref_zoom(ch);
        //her_synchrotron(ch);
        //ler_synchrotron(ch);
    }
}
