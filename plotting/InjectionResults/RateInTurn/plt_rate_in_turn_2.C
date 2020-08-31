

int her_ref(string channel = "FWD1")
{

    // Set some standards
    double linewidth = 2;
    int linestyle =7;

    int chist = kBlack;
    int cmarker  = cher->GetNumber();

    // Get the hist

    Target target = GetTarget("HER-REF");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* hist = (TH1F*) rfile->Get((channel+"_rate_in_turn").c_str());
    hist->SetDirectory(0);
    hist->SetLineColorAlpha( chist, 1.);
    // hist->SetFillColorAlpha( c1, 1.);
    // hist->SetMarkerColorAlpha(c1, 1);
    // hist->SetMarkerSize(1.0);
    // hist->SetLineWidth(linewidth);
    hist->SetLineWidth(linewidth);
    hist->SetFillStyle(0);

    // Normalize to the length of the waveform
    if(target.n != -1)
    {
        double n_turns = target.n*0.8e-9/10061.4e-9;
        double scale = n_turns*1e-3;
        hist->Scale(1./scale);
    }

    // Do the X Axis
    int nbinsx = hist->GetNbinsX();
    double xlow = hist->GetXaxis()->GetBinLowEdge(0);
    double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

    double scale = 1e6;
    hist->SetBins(nbinsx, xlow*scale, xup*scale);

    hist->GetXaxis()->SetTitle("t_{turn} [#mus]");

    double maxX = hist->GetBinCenter(hist->GetMaximumBin());
    double max = hist->GetMaximum();
    cout << "Max: " << max << ", @: "<< maxX <<  endl;


    // Set Y Scale
//    hist->GetYaxis()->SetRangeUser(0, max*1.6);
    hist->GetYaxis()->SetRangeUser(1e-2, max*50);
    hist->GetYaxis()->SetTitle("Particle Rate [mMIP/0.8 ns]");

    // Create the marker
    TMarker* single_marker = new TMarker(maxX, max*1.25, 23);
    single_marker->SetMarkerSize(1.6);
    single_marker->SetMarkerColor(cmarker);

    // Legend
    TLegend* leg =  new TLegend(0.215, 0.8, 0.39, 0.85);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);

    leg->AddEntry(make_box(hist), target.data.c_str(), "f");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* tbunch = new TLatex();
    //tbunch->SetNDC();
    tbunch->SetTextFont(leg->GetTextFont()+20);
    tbunch->SetTextSize(gStyle->GetLegendTextSize());
    tbunch->SetTextColor(cmarker);

    // Create the canvas
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
//    can->SetLogx();

    hist->Draw("HIST");
    single_marker->Draw();
    string mytext = "Rate in Turn: " + channel;
    text->DrawLatex(0.22, 0.88, mytext.c_str() );
    tbunch->DrawLatex(maxX*0.75, max*1.6,  "Injection Bunch");
    leg->Draw();

    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".pdf").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".png").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".jpg").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".eps").c_str());

    rfile->Close();
    return 0;
// -----   Colors and Style   ---------------------
}

int her_ref_zoom(string channel = "FWD1")
{

    // Set some standards
    double linewidth = 2;
    int linestyle =7;

    int chist = kBlack;
    int cfit  = cher->GetNumber();
    int cline = tab20_gray->GetNumber();

    double nsig = 5;
    // Get the hist

    Target target = GetTarget("HER-REF");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* hist = (TH1F*) rfile->Get((channel+"_rate_in_turn").c_str());
    hist->SetDirectory(0);
    hist->SetLineColorAlpha( chist, 1.);
    // hist->SetFillColorAlpha( c1, 1.);
    // hist->SetMarkerColorAlpha(c1, 1);
    // hist->SetMarkerSize(1.0);
    // hist->SetLineWidth(linewidth);
    hist->SetLineWidth(linewidth);
    hist->SetFillStyle(0);

    // Normalize to the length of the waveform
    if(target.n != -1)
    {
        double n_turns = target.n*0.8e-9/10061.4e-9;
        double scale = n_turns*1e-3;
        hist->Scale(1./scale);
    }

    // Do the X Axis
    int nbinsx = hist->GetNbinsX();
    double xlow = hist->GetXaxis()->GetBinLowEdge(0);
    double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

    double scale = 1e6;
    hist->SetBins(nbinsx, xlow*scale, xup*scale);

    hist->GetXaxis()->SetTitle("t_{turn} [#mus]");

    double maxX = hist->GetBinCenter(hist->GetMaximumBin());
    double max = hist->GetMaximum();
    cout << "Max: " << max << ", @: "<< maxX <<  endl;

    // Fit the bitch
    TF1* gaus = new TF1("gaus", "gaus(0)", 0, 1e6);
    gaus->SetNpx(2000);
    gaus->SetLineColor(cfit);
    gaus->SetLineWidth(linewidth+1);
    gaus->SetParameters(max, maxX, 1e-2);

    hist->Fit(gaus,"NL", "", maxX- 2e-2, maxX+2e-2 );

    TFitResultPtr r = hist->Fit(gaus,"NL","",gaus->GetParameter(1)-nsig*gaus->GetParameter(2), gaus->GetParameter(1)+nsig*gaus->GetParameter(2));

    cout << channel << ": " << gaus->GetProb() << endl;
    double frmin = gaus->GetParameter(1)-nsig*gaus->GetParameter(2);
    double frmax = gaus->GetParameter(1)+nsig*gaus->GetParameter(2);

    gaus->SetRange(frmin, frmax);

    hist->GetXaxis()->SetRangeUser(gaus->GetParameter(1)-3*nsig*gaus->GetParameter(2), gaus->GetParameter(1)+3*nsig*gaus->GetParameter(2));
    // Do the axis naming and scaling

    hist->GetXaxis()->SetNdivisions(506);
    hist->GetYaxis()->SetRangeUser(0., max*2.3);
//    hist->GetYaxis()->SetRangeUser(0.03, max*200);
    hist->GetYaxis()->SetTitle("Particle Rate [mMIP/0.8 ns]");


    // // Create the marker
    // TMarker* single_marker = new TMarker(maxX, max*1.05, 23);
    // single_marker->SetMarkerSize(1.6);
    // single_marker->SetMarkerColor(cmarker);

    // Legend
    TLegend* leg1 =  new TLegend(0.215, 0.8, 0.39, 0.85);
    //new TLegend(0.215, 0.8, 0.39, 0.85);
    leg1->SetBorderSize(0);
    leg1->SetFillColor(0);
    leg1->SetFillStyle(0);

    leg1->AddEntry(make_box(hist), target.data.c_str(), "f");
    //leg1->AddEntry((TObject*)0, channel.c_str(), "");

    TLegend* leg2 =  new TLegend(0.555, 0.8, 0.73, 0.85);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
    //
    // leg->AddEntry(make_box(hist), target.data.c_str(), "f");
    leg2->AddEntry(set_line_width(gaus, 3), "Gaussian Fit:", "l");

    TLegend* leg3 =  new TLegend(0.555, 0.67, 0.73, 0.8);
    leg3->SetBorderSize(0);
    leg3->SetFillColor(0);
    leg3->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss1;
	ss1 << fixed << setprecision(1) << "c. = (" << gaus->GetParameter(0) << " #pm "<< gaus->GetParError(0)<<  ") MIP/ns";
	leg3->AddEntry((TObject*)0, ss1.str().c_str(), "");

	stringstream ss2;
	ss2 << fixed << setprecision(1) << "#mu = (" << gaus->GetParameter(1)*1e3 << " #pm "<< gaus->GetParError(1)*1e3<< ") ns";
	leg3->AddEntry((TObject*)0, ss2.str().c_str(), "");

	stringstream ss3;
	ss3 << fixed << setprecision(1) << "#sigma = (" << gaus->GetParameter(2)*1e3 << " #pm "<< gaus->GetParError(2)*1e3<< ") ns";
	leg3->AddEntry((TObject*)0, ss3.str().c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg1->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLine *line = new TLine(frmin, 0, frmin, max*1.3);
    line->SetLineColor(cline);
    line->SetLineStyle(linestyle);
    line->SetLineWidth(linewidth);

    TLine *line2 = new TLine(frmax, 0, frmax, max*1.3);
    line2->SetLineColor(cline);
    line2->SetLineStyle(linestyle);
    line2->SetLineWidth(linewidth);


    TLatex* tbunch = new TLatex();
    //tbunch->SetNDC();
    tbunch->SetTextFont(leg1->GetTextFont());
    tbunch->SetTextSize(gStyle->GetLegendTextSize()*0.8);
    tbunch->SetTextColor(cline);

    // Create the canvas
    int width = round(vis_multi*textwidth*pt2pixel*1./2.);
    //int width = round(vis_multi*.25**pt2pixel*1/2);
    double ratio = 3./3.;

    string cantitle = target.typein+"_"+channel+ "_rate_in_turn_zoom";
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//    can->SetLogy();
//    can->SetLogx();

    hist->Draw("HIST");
    gaus->Draw("same");
    // single_marker->Draw();
    string mytext = "Zoom into Rate in Turn: " + channel;
    text->DrawLatex(0.22, 0.88, mytext.c_str() );
    tbunch->DrawLatex(frmin+0.0008, max*1.15, "#splitline{Region assigned to}{Injection Bunch}");
    //tbunch->DrawLatex(frmin+0.00075, max*1.15, "#splitline{Energy deposits caused}{by the Injeciton Bunch}");
    leg1->Draw();
    leg2->Draw();
    leg3->Draw();
    line->Draw("same");
    line2->Draw("same");

    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".pdf").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".png").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".jpg").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".eps").c_str());

    rfile->Close();
    return 0;
// -----   Colors and Style   ---------------------
}

int ler_vs(string channel = "FWD1")
{

    // Set some standards
    double linewidth = 2;
    int linestyle =7;

    int chist = kBlack;
    int cmarker  = cler->GetNumber();

    // Get the hist

    Target target = GetTarget("LER-VACS");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* hist = (TH1F*) rfile->Get((channel+"_rate_in_turn").c_str());
    hist->SetDirectory(0);
    hist->SetLineColorAlpha( chist, 1.);
    // hist->SetFillColorAlpha( c1, 1.);
    // hist->SetMarkerColorAlpha(c1, 1);
    // hist->SetMarkerSize(1.0);
    // hist->SetLineWidth(linewidth);
    hist->SetLineWidth(linewidth);
    hist->SetFillStyle(0);

    // Normalize to the length of the waveform
    if(target.n != -1)
    {
        double n_turns = target.n*0.8e-9/10061.4e-9;
        double scale = n_turns*1e-3;
        hist->Scale(1./scale);
    }

    // Do the X Axis
    int nbinsx = hist->GetNbinsX();
    double xlow = hist->GetXaxis()->GetBinLowEdge(0);
    double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

    double scale = 1e6;
    hist->SetBins(nbinsx, xlow*scale, xup*scale);

    hist->GetXaxis()->SetTitle("t_{turn} [#mus]");

    double maxX = hist->GetBinCenter(hist->GetMaximumBin());
    double max = hist->GetMaximum();
    cout << "Max: " << max << ", @: "<< maxX <<  endl;


    // Set Y Scale
//    hist->GetYaxis()->SetRangeUser(0, max*1.6);
    hist->GetYaxis()->SetRangeUser(6e-4, max*50);
    hist->GetYaxis()->SetTitle("Particle Rate [mMIP/0.8 ns]");

    // Create the marker
    TMarker* single_marker = new TMarker(maxX-0.025, max*1.25, 23);
    single_marker->SetMarkerSize(1.6);
    single_marker->SetMarkerColor(cmarker);

    TMarker* single_marker2 = new TMarker(maxX+0.15-0.025, max*1.0, 23);
    single_marker2->SetMarkerSize(1.6);
    single_marker2->SetMarkerColor(cmarker);

    // Legend
    TLegend* leg =  new TLegend(0.215, 0.8, 0.39, 0.85);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);

    leg->AddEntry(make_box(hist), target.data.c_str(), "f");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* tbunch = new TLatex();
    //tbunch->SetNDC();
    tbunch->SetTextFont(leg->GetTextFont()+20);
    tbunch->SetTextSize(gStyle->GetLegendTextSize());
    tbunch->SetTextColor(cmarker);

    // Create the canvas
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
//    can->SetLogx();

    hist->Draw("HIST");
    single_marker->Draw();
    single_marker2->Draw();
    string mytext = "Rate in Turn: " + channel;
    text->DrawLatex(0.22, 0.88, mytext.c_str() );
    tbunch->DrawLatex(maxX*0.75, max*1.6,  "Injection Bunches");
    leg->Draw();

    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".pdf").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".png").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".jpg").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn"+".eps").c_str());

    rfile->Close();
    return 0;
// -----   Colors and Style   ---------------------
}

int ler_vacs_zoom(string channel = "FWD1")
{

    // Set some standards
    double linewidth = 2;
    int linestyle =7;

    int chist = kBlack;
    int cf1  = tab20_red->GetNumber();
    int cf2  = tab20_green->GetNumber();
    int cline = tab20_gray->GetNumber();

    double nsig = 5;
    // Get the hist

    Target target = GetTarget("LER-VACS");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TH1F* hist = (TH1F*) rfile->Get((channel+"_rate_in_turn").c_str());
    hist->SetDirectory(0);
    hist->SetLineColorAlpha( chist, 1.);
    // hist->SetFillColorAlpha( c1, 1.);
    // hist->SetMarkerColorAlpha(c1, 1);
    // hist->SetMarkerSize(1.0);
    // hist->SetLineWidth(linewidth);
    hist->SetLineWidth(linewidth);
    hist->SetFillStyle(0);

    // Normalize to the length of the waveform
    if(target.n != -1)
    {
        double n_turns = target.n*0.8e-9/10061.4e-9;
        double scale = n_turns*1e-3;
        hist->Scale(1./scale);
    }

    // Do the X Axis
    int nbinsx = hist->GetNbinsX();
    double xlow = hist->GetXaxis()->GetBinLowEdge(0);
    double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

    double scale = 1e6;
    hist->SetBins(nbinsx, xlow*scale, xup*scale);

    hist->GetXaxis()->SetTitle("t_{turn} [#mus]");

    double maxX = hist->GetBinCenter(hist->GetMaximumBin());
    double max = hist->GetMaximum();
    cout << "Max: " << max << ", @: "<< maxX <<  endl;

    // Fit the bitch

    TF1* g1 = new TF1("gaus", "gaus(0)", 0, 1e6);
    g1->SetParameters(10,6.779,2.5e-3);
    g1->SetNpx(2000);
    g1->SetLineColor(cf1);
    g1->SetLineWidth(linewidth+1);

    hist->Fit(g1,"NL", "", 6.774, 6.784 );
    hist->Fit(g1,"NL", "", g1->GetParameter(1) - nsig*g1->GetParameter(2), g1->GetParameter(1) + nsig*g1->GetParameter(2) );
    g1->SetRange(g1->GetParameter(1) - nsig*g1->GetParameter(2),  g1->GetParameter(1) + nsig*g1->GetParameter(2) );

    TF1* g2 = new TF1("gaus", "gaus(0)", 0, 1e6);
    g2->SetParameters(10,6.879,2.5e-3);
    g2->SetNpx(2000);
    g2->SetLineColor(cf2);
    g2->SetLineWidth(linewidth+1);

    hist->Fit(g2,"NL", "", 6.874, 6.884 );
    hist->Fit(g2,"NL", "", g2->GetParameter(1) - nsig*g2->GetParameter(2), g2->GetParameter(1) + nsig*g2->GetParameter(2) );
    g2->SetRange(g2->GetParameter(1) - nsig*g2->GetParameter(2),  g2->GetParameter(1) + nsig*g2->GetParameter(2) );

    hist->GetXaxis()->SetRangeUser(6.73, 6.92);
    // Do the axis naming and scaling

    hist->GetXaxis()->SetNdivisions(506);
    hist->GetYaxis()->SetRangeUser(0., max*2.5);
//    hist->GetYaxis()->SetRangeUser(0.03, max*200);
    hist->GetYaxis()->SetTitle("Particle Rate [mMIP/0.8 ns]");


    // // Create the marker
    // TMarker* single_marker = new TMarker(maxX, max*1.05, 23);
    // single_marker->SetMarkerSize(1.6);
    // single_marker->SetMarkerColor(cmarker);

    // Legend
    TLegend* leg1 =  new TLegend(0.215, 0.76, 0.385, 0.85);
    //new TLegend(0.215, 0.8, 0.39, 0.85);
    leg1->SetBorderSize(0);
    leg1->SetFillColor(0);
    leg1->SetFillStyle(0);

    leg1->AddEntry(make_box(hist, "EMPTY"), target.data.c_str(), "f");
    leg1->AddEntry(set_line_width(g1,3), "Gaussian Fit:", "l");
    //leg1->AddEntry((TObject*)0, channel.c_str(), "");

    TLegend* leg2 =  new TLegend(0.215, 0.62, 0.385, 0.755);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);

    leg2->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss1;
	ss1 << fixed << setprecision(1) << "c. = (" << g1->GetParameter(0) << " #pm "<< g1->GetParError(0)<<  ") MIP/ns";
	leg2->AddEntry((TObject*)0, ss1.str().c_str(), "");

	stringstream ss2;
	ss2 << fixed << setprecision(1) << "#mu = (" << g1->GetParameter(1)*1e3 << " #pm "<< g1->GetParError(1)*1e3<< ") ns";
	leg2->AddEntry((TObject*)0, ss2.str().c_str(), "");

	stringstream ss3;
	ss3 << fixed << setprecision(1) << "#sigma = (" << g1->GetParameter(2)*1e3 << " #pm "<< g1->GetParError(2)*1e3<< ") ns";
	leg2->AddEntry((TObject*)0, ss3.str().c_str(), "");

    TLegend* leg3 =  new TLegend(0.575, 0.765, 0.75, 0.795);
    leg3->SetBorderSize(0);
    leg3->SetFillColor(0);
    leg3->SetFillStyle(0);

    leg3->AddEntry(set_line_width(g2,3), "Gaussian Fit:", "l");

    TLegend* leg4 =  new TLegend(0.575, 0.62, 0.75, 0.755);
    leg4->SetBorderSize(0);
    leg4->SetFillColor(0);
    leg4->SetFillStyle(0);

    leg4->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss4;
	ss4 << fixed << setprecision(1) << "c. = (" << g2->GetParameter(0) << " #pm "<< g2->GetParError(0)<<  ") MIP/ns";
	leg4->AddEntry((TObject*)0, ss4.str().c_str(), "");

	stringstream ss5;
	ss5 << fixed << setprecision(1) << "#mu = (" << g2->GetParameter(1)*1e3 << " #pm "<< g2->GetParError(1)*1e3<< ") ns";
	leg4->AddEntry((TObject*)0, ss5.str().c_str(), "");

	stringstream ss6;
	ss6 << fixed << setprecision(1) << "#sigma = (" << g2->GetParameter(2)*1e3 << " #pm "<< g2->GetParError(2)*1e3<< ") ns";
	leg4->AddEntry((TObject*)0, ss6.str().c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(leg1->GetTextFont()+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* deltatext = new TLatex();
// /    deltatext->SetNDC();
    deltatext->SetTextFont(leg1->GetTextFont());
    deltatext->SetTextSize(gStyle->GetLegendTextSize());
    deltatext->SetTextColor(cline);

    double linemax = g1->GetMaximum()*1.05;
    TLine *line = new TLine(g1->GetParameter(1), g1->GetMaximum(), g1->GetParameter(1), linemax);
    line->SetLineColor(cline);
    line->SetLineStyle(linestyle);
    line->SetLineWidth(linewidth);

    TLine *line2 = new TLine(g2->GetParameter(1), g2->GetMaximum(), g2->GetParameter(1), linemax);
    line2->SetLineColor(cline);
    line2->SetLineStyle(linestyle);
    line2->SetLineWidth(linewidth);

    TArrow *arr = new TArrow(g1->GetParameter(1),linemax,g2->GetParameter(1),linemax,0.02,"<|>");
    arr->SetLineColor(cline);
//    arr->SetLineStyle(linestyle);
    arr->SetLineWidth(linewidth);
    arr->SetFillStyle(0);
    TLatex* tbunch = new TLatex();
    //tbunch->SetNDC();
    tbunch->SetTextFont(leg1->GetTextFont());
    tbunch->SetTextSize(gStyle->GetLegendTextSize()*0.8);
    tbunch->SetTextColor(cline);

    // Create the canvas
    int width = round(vis_multi*textwidth*pt2pixel*1./2.);
    //int width = round(vis_multi*.25**pt2pixel*1/2);
    double ratio = 3./3.;

    string cantitle = target.typein+"_"+channel+ "_rate_in_turn_zoom";
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//    can->SetLogy();
//    can->SetLogx();

    hist->Draw("HIST");
    g1->Draw("same");
    g2->Draw("same");
    // single_marker->Draw();
    string mytext = "Zoom into Rate in Turn: " + channel;
    text->DrawLatex(0.22, 0.88, mytext.c_str() );

    stringstream ss7;
    ss7 << fixed << setprecision(1) << "#Deltat = " << (g2->GetParameter(1) - g1->GetParameter(1))*1e3 <<  " ns";
    deltatext->DrawLatex(g1->GetParameter(1) + 2.5e-2 , linemax*1.03,ss7.str().c_str());
//    tbunch->DrawLatex(frmin+0.0008, max*1.15, "#splitline{Region assigned to}{Injection Bunch}");
    //tbunch->DrawLatex(frmin+0.00075, max*1.15, "#splitline{Energy deposits caused}{by the Injeciton Bunch}");
    leg1->Draw();
    leg2->Draw();
    leg3->Draw();
    leg4->Draw();

    line->Draw("same");
    line2->Draw("same");
    arr->Draw();
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".pdf").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".png").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".jpg").c_str());
    can->SaveAs((target.pathout+"/RateInTurn/"+target.fileout+"_"+channel+"_rate_in_turn_zoom"+".eps").c_str());

    rfile->Close();
    return 0;
// -----   Colors and Style   ---------------------
}

void plt_rate_in_turn_2()
{
    SetPhDStyle();
    string chs[] = {"FWD1", "FWD2", "FWD3"};
    //string chs[] = {"FWD1"};
    for(auto ch : chs)
    {
        her_ref(ch);
        her_ref_zoom(ch);
        ler_vs(ch);
        ler_vacs_zoom(ch);
    }


}
