#include <vector>

using namespace std;

TCanvas* rate_in_turn(TFile* rfile, Target target, string channel = "FWD1")
{
    int width = round(vis_multi*textwidth*pt2pixel*1./2.);
    //int width = round(vis_multi*.25**pt2pixel*1/2);
    double ratio = 3./3.;

    string cantitle = target.typein+"_"+channel+ "_rate_in_turn";
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

	can->SetLogy();

	// -----   Colors and Style   ---------------------
	double linewidth = 2;

   int c1 = kBlack;
   // int c2 = claws_gray->GetNumber();
   // int c3 = claws_black->GetNumber();
   // int cfit = TColor::GetColor("#2ca02c");
   // int cfit = kAzure+1;
   int cfit = claws_red->GetNumber();
   // // Katha 112


    // -----   Axis   ---------------------
    TH1F* hist = (TH1F*) rfile->Get((channel+"_rate_in_turn").c_str());
    hist->SetDirectory(0);
    hist->SetLineColorAlpha( c1, 1.);
    hist->SetFillColorAlpha( c1, 1.);
    hist->SetMarkerColorAlpha(c1, 1);
    hist->SetMarkerSize(1.0);
    // hist->SetLineWidth(linewidth);
	hist->SetLineWidth(1);
    hist->SetFillStyle(0);

	if(target.n != -1)
	{
		double scale = target.n*0.8e-9;
		hist->Scale(1./scale);
	}

	int nbinsx = hist->GetNbinsX();
	double xlow = hist->GetXaxis()->GetBinLowEdge(0);
	double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

	//     int nbinsy = hist->GetNbinsY();
	//     double ylow = hist->GetYaxis()->GetBinLowEdge(0)*1e6;
	//     double yup = (hist->GetYaxis()->GetBinLowEdge(nbinsy)+hist->GetYaxis()->GetBinWidth(nbinsy))*1e6;
	double scale = 1e6;
	 hist->SetBins(nbinsx, xlow*scale, xup*scale);


    hist->Sumw2();

	// ----- AXIS -----

//	TH1F* axis = new TH1F("axis", "axis", 1000, xlow, xup);
  // 	axis->SetDirectory(0);
   hist->GetXaxis()->SetTitle("Time in Turn [#mus]");
   hist->GetYaxis()->SetTitle("Rate in Turn a.u.]");


	double min=1000000;
	for(int bin = 1; bin< 12000; ++bin)
	{
		if(hist->GetBinContent(bin) < min) min = hist->GetBinContent(bin);
	}
	cout << "Minimum: " << min << endl;
	hist->GetYaxis()->SetRangeUser(min, hist->GetMaximum()*12.);

	// ---- Legened ----

	TLegend* leg =  new TLegend(0.24, 0.7, 0.44, 0.76);
	leg->SetBorderSize(0);
	leg->SetFillColor(0);
	leg->SetFillStyle(0);

	leg->AddEntry(make_box(hist), channel.c_str(), "f");

	TLatex* text = new TLatex();
	text->SetNDC();
	text->SetTextFont(leg->GetTextFont()+20);
	text->SetTextSize(gStyle->GetLegendTextSize());

     // ---- Drawing ----
	hist->Draw("HIST ][");

	leg->Draw();

	if(target.data != "")
	{
		//text->DrawLatex(0.22, 0.89, string("LER Double Bunch Injection").c_str());
		text->DrawLatex(0.22, 0.89,  target.data.c_str());
		text->DrawLatex(0.22, 0.82,  string("LER Double Bunch Injection").c_str());
	}
	else
	{
		text->DrawLatex(0.5, 0.88,  string("Time In Turn").c_str());
	}

    return can;
}

TCanvas* rate_in_turn_zoom(TFile* rfile, Target target, string channel = "FWD1")
{
    int width = round(vis_multi*textwidth*pt2pixel*1./2.);
    //int width = round(vis_multi*.25**pt2pixel*1/2);
    double ratio = 3./3.;

    string cantitle = target.typein+"_"+channel+ "_can_rate_in_turn_zoom";
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

	can->SetLeftMargin(0.17);
	can->SetRightMargin(0.045);
	// -----   Colors and Style   ---------------------
	double linewidth = 2;
	int linestyle =7;

   int c1 = kBlack;
   // int c2 = claws_gray->GetNumber();
   // int c3 = claws_black->GetNumber();
   // int cfit = TColor::GetColor("#2ca02c");
   // int cfit = kAzure+1;
   int cfit1 = tab20_blue->GetNumber();
   int cfit2 = tab20_orange->GetNumber();
   // // Katha 112
	int cl1 = claws_gray->GetNumber();
	int cl2 = cl1;
    // -----   Axis   ---------------------
    TH1F* hist = (TH1F*) rfile->Get((channel+"_rate_in_turn").c_str());
    hist->SetDirectory(0);
    hist->SetLineColorAlpha( c1, 1.);
    hist->SetFillColorAlpha( c1, 1.);
    hist->SetMarkerColorAlpha(c1, 1);
    hist->SetMarkerSize(1.0);
    // hist->SetLineWidth(linewidth);
	hist->SetLineWidth(linewidth);
    hist->SetFillStyle(0);

	if(target.n != -1)
	{
		double scale = target.n*0.8e-9;
		hist->Scale(1./scale);
	}

	int nbinsx = hist->GetNbinsX();
	double xlow = hist->GetXaxis()->GetBinLowEdge(0);
	double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

	//     int nbinsy = hist->GetNbinsY();
	//     double ylow = hist->GetYaxis()->GetBinLowEdge(0)*1e6;
	//     double yup = (hist->GetYaxis()->GetBinLowEdge(nbinsy)+hist->GetYaxis()->GetBinWidth(nbinsy))*1e6;
	double scale = 1e6;
	 hist->SetBins(nbinsx, xlow*scale, xup*scale);
	//


    hist->Sumw2();

	// ----- AXIS -----

//	TH1F* axis = new TH1F("axis", "axis", 1000, xlow, xup);
  // 	axis->SetDirectory(0);
   hist->GetXaxis()->SetTitle("Time in Turn [#mus]");
   hist->GetXaxis()->SetRangeUser(6.735,7.05);
  // hist->SetNdivisions(506);
   hist->GetYaxis()->SetTitle("Rate in Turn [a.u.]");
   hist->GetYaxis()->SetTitleOffset(1.7);

   double histmax = hist->GetMaximum();

   hist->GetYaxis()->SetRangeUser(0., histmax*1.6);

  //
  //     stringstream ss;
  //     double binwidth = hist->GetBinWidth(3);
  //     ss << fixed << setprecision(2) << "Entries [1/" << binwidth << " MIP #times s]";
  //     axis->GetYaxis()->SetTitle(ss.str().c_str());
  //
  //


     // ---- Fit ----

	//Use TSpectrum to find the peak candidates
		double range = 6;
		double dt = 0.8e-3;
	 TSpectrum *s = new TSpectrum(2);
	 Int_t nfound = s->Search(hist,2,"",0.10);
	 Double_t *xpeaks = s->GetPositionX();
	 printf("Found %d candidate peaks to fit\n",nfound);
	 // vector<TF1*> fits;
	 // for(int i = 0; i< nfound; ++i)
	 // {
		 cout<< "Found peak at: " << xpeaks[0] << endl;

		 TF1 *fit1 = new TF1("fa1","gaus",xpeaks[0]-range*dt,xpeaks[0]+range*dt);
		 hist->Fit(fit1, "SLR");

		 TF1 *fit2 = new TF1("fa2","gaus",xpeaks[1]-range*dt,xpeaks[1]+range*dt);
		 hist->Fit(fit2, "SLR");
	  //
		 // fits.push_back(gaus);
	 // }
	 double t_diff = (fit2->GetParameter(1) - fit1->GetParameter(1))*1e3;
	 cout<<  "TDiff: "<< t_diff << endl;
	 cout<<  "Deviation: "<< 1.965*49-t_diff << endl;

	// for(auto&fit:fits)
	// {
		fit1->SetLineWidth(linewidth);
		fit1->SetLineColor(cfit1);
		fit1->SetNpx(1000);

		fit2->SetLineWidth(linewidth);
		fit2->SetLineColor(cfit2);
	    fit2->SetNpx(1000);

	// ----- Lines -----


	double y2 = histmax*1.15;

	cout << "hist->GetMaximum(): "<< hist->GetMaximum() << endl;
//	TLine *line = new TLine(fit1->GetParameter(1),0,fit1->GetParameter(1),y2);
	TLine *line = new TLine(fit1->GetParameter(1), hist->GetBinContent(hist->FindBin(fit1->GetParameter(1))),fit1->GetParameter(1),y2);
	line->SetLineColor(cl1);
	line->SetLineStyle(linestyle);
	line->SetLineWidth(linewidth);

	// TLine *line2 = new TLine( fit2->GetParameter(1),0,fit2->GetParameter(1),y2);
	TLine *line2 = new TLine( fit2->GetParameter(1), hist->GetBinContent(hist->FindBin(fit2->GetParameter(1))),fit2->GetParameter(1),y2);
	line2->SetLineColor(cl1);
	line2->SetLineStyle(linestyle);
	line2->SetLineWidth(linewidth);

	TArrow* ar1 = new TArrow(fit1->GetParameter(1),y2,fit2->GetParameter(1),y2, 0.015, "<|>");
	ar1->SetLineColor(cl1);
	ar1->SetFillColorAlpha(cl1,0);
	ar1->SetLineWidth(linewidth);


	TLegend* leg =  new TLegend(0.6, 0.58, 0.8, 0.69);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);

	leg->AddEntry(make_box(hist), channel.c_str(), "f");
	leg->AddEntry(set_line_width_f(fit1, 3), "Gaussian Fit:", "l");

	TLegend* leg2 =  new TLegend(0.6, 0.43, 0.8, 0.58);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

	stringstream ss1;
	ss1 << fixed << setprecision(2) << "c. = " << fit1->GetParameter(0) << " a.u.";
	leg2->AddEntry((TObject*)0, ss1.str().c_str(), "");

	stringstream ss2;
	ss2 << fixed << setprecision(4) << "#mu = " << fit1->GetParameter(1) << " #mus";
	leg2->AddEntry((TObject*)0, ss2.str().c_str(), "");

	stringstream ss3;
	ss3 << fixed << setprecision(1) << "#sigma = " << fit1->GetParameter(2)*1e3 << " ns";
	leg2->AddEntry((TObject*)0, ss3.str().c_str(), "");

	TLegend* leg3 =  new TLegend(0.6, 0.30, 0.8, 0.49);
	leg3->SetBorderSize(0);
	leg3->SetFillColor(0);
	leg3->SetFillStyle(0);

	leg3->AddEntry(set_line_width_f(fit2, 3), "Gaussian Fit:", "l");

	TLegend* leg4 =  new TLegend(0.6, 0.215, 0.8, 0.365);
	leg4->SetBorderSize(0);
	leg4->SetFillColor(0);
	leg4->SetTextSize(gStyle->GetLegendTextSize()*0.8);

	stringstream ss4;
	ss4 << fixed << setprecision(2) << "c. = " << fit2->GetParameter(0) << " a.u.";
	leg4->AddEntry((TObject*)0, ss4.str().c_str(), "");

	stringstream ss5;
	ss5 << fixed << setprecision(4) << "#mu = " << fit2->GetParameter(1) << " #mus";
	leg4->AddEntry((TObject*)0, ss5.str().c_str(), "");

	stringstream ss6;
	ss6 << fixed << setprecision(1) << "#sigma = " << fit2->GetParameter(2)*1e3 << " ns";
	leg4->AddEntry((TObject*)0, ss6.str().c_str(), "");

	TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());


     // ---- Drawing ----
//     can->cd();
	hist->Draw("HIST ][");
	line->Draw();
	line2->Draw();
	ar1->Draw();

	stringstream ss;
	ss << fixed << setprecision(2) << "#Deltat = " << t_diff << " ns";
  	text->DrawLatex(0.295, 0.745, ss.str().c_str());


	fit1->Draw("same");
	fit2->Draw("same");

	leg->Draw();
	leg2->Draw();
	leg3->Draw();
	leg4->Draw();

	if(target.data != "")
	{
	//	text->DrawLatex(0.22, 0.89,  string("Zoom into LER Double Bunch Injection").c_str());
		text->DrawLatex(0.22, 0.89,  target.data.c_str());
		text->DrawLatex(0.22, 0.82,  string("Zoom into LER Double Bunch Injection").c_str());
	}
	else
	{
		text->DrawLatex(0.5, 0.88,  string("Time In Turn").c_str());
	}

    return can;
}

void plt_rateinturn()
{
    SetPhDStyle();

    string filepath = "/remote/ceph/group/ilc/claws/phase1/Analysis/Run-401141/WAVEFORM_PEAK_FFT_INJ_HER";

    string type = "WAVEFORM_PEAK_FFT_INJ_HERONLY";
    string savepath = filepath;
    string filein = "AnalysisEvent.root";
    // string prefix =
    //string text = "Run-400991-401196, Non-Inj., Vac. Scrub.";
//    string text2 = "Non-Injection, Vacuum Scrubbing";


    for(auto & target: targets)
    {
        TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

        string chs[] = {"FWD1", "FWD2", "FWD3"};
    //    string chs[] = {"FWD1"};
        for(int i =0; i<3; ++i)
        {
            TCanvas * can;
            can = rate_in_turn(rfile,target, chs[i]);
            can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn"+".pdf").c_str());
            can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn"+".png").c_str());
            can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn"+".jpg").c_str());
            can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn"+".eps").c_str());

			can = rate_in_turn_zoom(rfile,target, chs[i]);
			can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn_zoom"+".pdf").c_str());
			can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn_zoom"+".png").c_str());
			can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn_zoom"+".jpg").c_str());
			can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_time_in_turn_zoom"+".eps").c_str());
        }

    //    rfile->Close();
    }

}
