
#include <TFile.h>

// TH1F* set_line_width4(TH1F* wf)
// {
//     TH1F* clone = (TH1F*) wf->Clone("clone");
//     clone->SetLineWidth(3);
//     return clone;
// }
//
// TF1* set_line_width4f(TF1* wf)
// {
//     TF1* clone = (TF1*) wf->Clone("clone");
//     clone->SetLineWidth(3);
//     return clone;
// }

void plt_avg1pe()
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

    can->SetLeftMargin(0.08);
    // can->SetBottomMargin(0.14);
    // can->SetRightMargin(0.02);

    // can->SetLeftMargin(0.08);
    // can->SetRightMargin(0.02);

    gStyle->SetEndErrorSize(2);
    // // claws default
    //int c1 = claws_red->GetNumber();
    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int c2 = tab20_orange->GetNumber();
    int c3 = tab20_blue->GetNumber();
    int cerr = tab20_lgray->GetNumber();
    double fillerr = 1.0;
    // // Katha 112
    // // int c1 = TColor::GetColor("#ee0000"); //rot
    // // int c2 = TColor::GetColor("#aaccff");
    // // int c3 = TColor::GetColor("#550144");
    //
    // // // root default
    // // int c1 = 2;
    // // int c2 = 4;
    // // int c3 = 1;
    int fillstyle = 1001;

    double linewidth = 2;

    double scale = 1e-9;
    double dt = 0.8;
    double length = 230;

    double ylow = -0.25;
    double yup = 4.99;

    double xlow = 155;
    double xup = 210;
    double ylow2 = -0.025;
    double yup2 = 0.38;

    string path = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401141/Calibration/Average1PE/";

    string fname = "run_401141_gainstate_extended_ph1v6.root";

    string runnr = fname.substr(4,6);


    TFile* rfile = new TFile((path + fname).c_str(), "open");

    TH1F* fwd1 = (TH1F*) rfile->Get("FWD1_avg");

    TH1F* axis = new TH1F("axis", "axis", 1000, fwd1->GetBinLowEdge(1)/scale, (fwd1->GetBinLowEdge(fwd1->GetNbinsX())+ fwd1->GetBinWidth(fwd1->GetNbinsX()))/scale);
    axis->GetYaxis()->SetRangeUser(ylow, yup);
    //axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ns]");
    // axis->GetYaxis()->SetTitleOffset(0.7);
    axis->GetYaxis()->SetTitleOffset(0.6);
    axis->GetYaxis()->SetTitle("Amplitude [mV]");
    axis->GetYaxis()->SetTickLength( axis->GetXaxis()->GetTickLength()/ratio );

    TH1F* saxis = (TH1F*) axis->Clone("saxis");
    saxis->GetXaxis()->SetLimits(xlow, xup);
    saxis->GetYaxis()->SetRangeUser(ylow2, yup2);
    // cout << saxis->GetXaxis()->GetLabelSize() << endl;
    // cout << saxis->GetXaxis()->GetTitleSize() << endl;
    double textsize = saxis->GetXaxis()->GetTitleSize()*0.8;
    saxis->GetXaxis()->SetLabelSize(textsize);
    saxis->GetXaxis()->SetTitleSize(textsize);

    saxis->GetYaxis()->SetLabelSize(textsize);
    saxis->GetYaxis()->SetTitleSize(textsize);

    saxis->GetXaxis()->SetTitleOffset(2.0);
    saxis->GetYaxis()->SetTitleOffset(1.4);

    saxis->GetYaxis()->SetNdivisions(505);

    TH1F* wf = new TH1F("wf", "wf", length, -dt/2., length*dt-dt/2. );
    TH1F* ext = new TH1F("ext", "ext", fwd1->GetNbinsX() - length, length*dt-dt/2., fwd1->GetNbinsX()*dt-dt/2. );
    TH1F* final = new TH1F("final", "final",fwd1->GetNbinsX() ,-dt/2., fwd1->GetNbinsX()*dt-dt/2.);

    for(int i = 1; i<=fwd1->GetNbinsX(); ++i)
    {
        double cont = fwd1->GetBinContent(i);
        if(i <= length)
        {
            wf->SetBinContent(i, cont);
            wf->SetBinError(i, fwd1->GetBinError(i));
        }
        else
        {
            ext->SetBinContent(i-length, cont);
        }

        final->SetBinContent(i, cont);
    }

    // WF:
    wf->SetLineColor(c1);
    //wf->SetFillColorAlpha(c1,1.0);
    wf->SetMarkerColor(c1);
    wf->SetMarkerSize(0.6);
    wf->SetLineWidth(linewidth);


    // FIT:
    TF1* fit = fwd1->GetFunction("FWD1_exp");

    double xmin = fit->GetXmin()/scale;
    double xmax = fit->GetXmax()/scale;

    double par0 = fit->GetParameter(0);
    double par1 = fit->GetParameter(1)*scale;
    double par2 = fit->GetParameter(2)/scale;

    fit->SetParameters(par0, par1, par2);

//    double par0err = fit->GetParError(0);
    double par1err = fit->GetParError(1)*scale;
    double par2err = fit->GetParError(2)/scale;

    fit->SetParError(1, par1err);
    fit->SetParError(2, par2err);

    fit->SetRange(xmin, xmax);

    fit->SetLineColor(c2);
    fit->SetLineWidth(3);

    // EXT:

    ext->SetLineColor(c3);
//    ext->SetFillColorAlpha(c3,1.0);
    ext->SetMarkerColor(c3);
    ext->SetMarkerSize(0.6);
    ext->SetLineWidth(3);

//     fwd1->SetLineColorAlpha( c1, 1.);
//     fwd1->SetFillColorAlpha(c3, 1.0);
//     fwd1->SetLineWidth(linewidth);
//     fwd1->SetMarkerSize(0.);
//
//     //fwd1->GetXaxis()->SetRangeUser(xlow, xup);

// //    fwd1->GetXaxis()->SetNdivisions(505);
//
//     reco->SetLineColorAlpha(c2, 1.0);
//
//
    TH1I *err = (TH1I*) wf->Clone("err");
    err->SetMarkerSize(0);
    //err->SetFillStyle(3353);
    err->SetFillStyle(fillstyle);
    err->SetFillColorAlpha(cerr,fillerr);
    //err->SetFillColorAlpha(cerr,1.0);
    err->SetLineWidth(0);

     // --- Create the line ---
    TLine *line = new TLine(axis->GetXaxis()->GetXmin(),0,axis->GetXaxis()->GetXmax(),0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    TLine *line2 = new TLine(saxis->GetXaxis()->GetXmin(),0,saxis->GetXaxis()->GetXmax(),0);
    line2->SetLineColor(kGray);
    line2->SetLineStyle(7);


    cout<< saxis->GetXaxis()->GetXmin() << endl;

    TLegend* leg =  new TLegend(0.5, 0.35, 0.62, 0.48);
    //leg->SetHeader("");
//    leg->SetNColumns(2);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->AddEntry(set_line_width(wf), "Averaged Waveform", "l");

    leg->AddEntry(err, "Sys. + Stat. Unc.", "f");

    cout << fit->GetParameter(0) << endl;
    cout << fit->GetParameter(1) << endl;
    cout << fit->GetParameter(2) << endl;

    //ss << fixed << setprecision(2) << fit->GetParameter(0) << "e^{"<< fit->GetParameter(1) <<" ns^{-1}(x-" <<fit->GetParameter(2)<<" ns)}"<<" mV";

    TLegend* leg2 =  new TLegend(0.71, 0.35, 0.85, 0.48);
    //leg->SetHeader("");
//    leg->SetNColumns(2);
    leg2->SetBorderSize(0);
    leg2->SetFillColor(0);
    leg2->AddEntry(set_line_width(ext), "Waveform Extension", "l");
    leg2->AddEntry(set_line_width(fit), "Fit: f(t) = p_{1}#upointexp(p_{2}(t-p_{3} ))", "l");

    TLegend* leg3 =  new TLegend(0.72, 0.2, 0.86, 0.34);
    //leg->SetHeader("");
//    leg->SetNColumns(2);
    leg3->SetBorderSize(0);
    leg3->SetFillColor(0);
    leg3->SetTextSize(gStyle->GetLegendTextSize()*0.8);

    stringstream ss;
    ss << fixed << setprecision(1) << "p_{1} = (" << fit->GetParameter(0) << "    #pm "  << fit->GetParError(0) << ") mV";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg3->AddEntry((TObject*)0, ss.str().c_str(), "");

    stringstream ss2;
    ss2 << fixed << setprecision(1) << "p_{2} = (-" << 1e3*fit->GetParameter(1) << " #pm "  << 1e3*fit->GetParError(1) << ")/(10^{3} ns)";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg3->AddEntry((TObject*)0, ss2.str().c_str(), "");
    //leg->AddEntry((TObject*)0, "", "");

    stringstream ss3;
    ss3 << fixed << setprecision(1) << "p_{3} = (" << fit->GetParameter(2) << " #pm "  << fit->GetParError(2) << ") ns";
//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg3->AddEntry((TObject*)0, ss3.str().c_str(), "");

    //leg->AddEntry(fwd1, "Final 1 pe waveform");

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();
    //
    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* text2 = new TLatex();
    text2->SetNDC();
    text2->SetTextFont(legfont+20);
    text2->SetTextSize(gStyle->GetLegendTextSize());

    // Real plotting...

    axis->Draw("axis");
    line->Draw("same");

    //final->Draw("hist L same ][");

    err->Draw("same E3");

    wf->Draw("hist L same ][");
    // final->DrawCopy("AXIS");

    ext->Draw("hist L same ][");

    fit->Draw("same");

    text->DrawLatex(0.115, 0.88, ("Run " + runnr+": FWD1").c_str());

    axis->Draw("same axis");

    // wf->DrawCopy("same p E1");
    //
    // fit->DrawCopy("same");
    // //
    // // ext->Draw("same hist ][");

    //leg->Draw("same");

    TPad * pad2 = new TPad("pad2", "The pad 20% of the height", 0.48, 0.48, 1.0, 1.0, kWhite);

    pad2->Draw();
    pad2->cd();
    pad2->SetRightMargin(0.035);
    pad2->SetTopMargin(0.025);
    pad2->SetBottomMargin(0.19);
    pad2->SetLeftMargin(0.12);

    saxis->Draw("axis");
    line2->Draw("same");
    err->Draw("same E3");
    wf->Draw("hist L same ][");
    fit->Draw("same");

    ext->Draw("hist L same ][");
    saxis->Draw("same axis");

    text2->DrawLatex(0.82, 0.82, "Zoom");

    can->cd();
    leg->Draw();
    leg2->Draw();
    leg3->Draw();


    // final->GetXaxis()->SetRangeUser(150, 200);
    // final->GetYaxis()->SetRangeUser(-0.01, 0.4);
    //
    // final->GetXaxis()->SetTitleOffset(1.8);
    // final->GetYaxis()->SetTitleOffset(1.8);
    //
    // final->DrawCopy("axis");
    // //line->Draw("same");
    //
    // final->Draw("same hist");
    // wf->Draw("same p E1");
    //
    // fit->Draw("same");
    //
    // // // ext->Draw("same hist ][");
    // ext->Draw("same hist");
    // // //

    // leg->Draw("same");


    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/avg1pe.pdf");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/avg1pe.png");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/avg1pe.eps");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/avg1pe.jpg");

}
