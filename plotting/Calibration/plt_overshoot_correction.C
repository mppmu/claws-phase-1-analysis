


TF1* kde;

Double_t osfunc(Double_t *x, Double_t *par)
{
    // cout<< "Kde("<<x[0]<<"): " << kde->Eval(x[0]*1e-9) << endl;

    double number = par[0]*(kde->Eval((x[0] - par[1])*1e-9) - 53500.) + par[2];
    return number;
}


int plt_overshoot_correction()
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

    // Butt ugly

    TFile* fkde = new TFile("./Calibration/Data/overshoot_template_function.root", "OPEN");
    kde = (TF1*) fkde->Get("kde");
    cout << kde->GetMinimumX() << endl;
    //gStyle->SetEndErrorSize(2);
    // // claws default
    //int c1 = claws_red->GetNumber();
    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int c2 = tab20_orange->GetNumber();
    int c3 = tab20_blue->GetNumber();
    int cerr = kGray;
    // int cfit = tab20_green->GetNumber();
    int cfit = tab20_orange->GetNumber();
    //int cfit = TColor::GetColor("#C7C7C7");
    int cosfit = tab20_blue->GetNumber();

    double fillerr = 1.0;

    int fillstyle = 1001;

    double linewidth = 2;

    double scale = 1e-9;
    double dt = 0.8;

    double ylow = -29;
    double yup = 89;

    double xlow = 550;
    double xup = 1450;
    double ylow2 = -24;
    double yup2 = 24;

    string path  = "/remote/ceph/group/ilc/claws/phase1/RAW/connecticut/2016-05-25/Run-401145/Calibration/PDS_Physics/Waveforms/";
    string fname = "PhysicsEvent_401145033_pd_subtracted.root";
    string runnr = fname.substr(13,9);

    TFile* rfile = new TFile((path + fname).c_str(), "open");

    TH1D* fwd1 = (TH1D*) rfile->Get("FWD1");

    //int start = 423515;
    int start = 422950;
    int length = 25500;

    TH1D *fwd1m = new TH1D("fwd1m","fwd1m", length, - 0.4, length*0.8 - 0.4);
    for(int i = 1; i<=length; ++i )
    {
        double cont = fwd1->GetBinContent(i+start);
        double err  = fwd1->GetBinError(i+start);
        fwd1m->SetBinContent(i,cont);
        fwd1m->SetBinError(i, err);
    }

    TH1I *err = (TH1I*) fwd1m->Clone("err");
    err->SetMarkerSize(0);
    err->SetFillStyle(fillstyle);
    err->SetFillColorAlpha(cerr,fillerr);
    err->SetLineWidth(1);

    TF1 *fit_line = new TF1("fit_line","[0]*(x-[1])", 575, 700);
    fit_line->SetParameters(-0.1, 625);

    fwd1m->Fit(fit_line, "SR+");
//    fwd1m->SetLineColor(tab20_red->GetNumber());

    fit_line->SetLineColor(cfit);
    fit_line->SetLineWidth(linewidth+2);
    fit_line->SetNpx(5000);


    TF1 *osfit = new TF1("osfit", osfunc, fit_line->GetX( 0. ), 20000,3, 1);

    osfit->SetParameter(0, 1);
    osfit->SetParameter(1, 640);
    osfit->SetParameter(2, 0);

    fwd1m->Fit(osfit, "RS+");

    osfit->SetLineColor(cosfit);
    osfit->SetLineWidth(linewidth+1);
    osfit->SetNpx(5000);

    TH1D* axis = (TH1D*) fwd1m->Clone("axis");
    axis->GetYaxis()->SetRangeUser(ylow, yup);
//    axis->GetXaxis()->SetTitleOffset(1.3);
    axis->GetXaxis()->SetTitle("Time [ns]");
    axis->GetYaxis()->SetTitleOffset(0.7);
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
    saxis->GetYaxis()->SetTitleOffset(1.5);
    saxis->GetXaxis()->SetNdivisions(505);
    saxis->GetYaxis()->SetNdivisions(505);

    TLine *line = new TLine(axis->GetXaxis()->GetXmin(),0,axis->GetXaxis()->GetXmax(),0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    TLine *line2 = new TLine(saxis->GetXaxis()->GetXmin(),0,saxis->GetXaxis()->GetXmax(),0);
    line2->SetLineColor(kGray);
    line2->SetLineStyle(7);

    TLegend* leg =  new TLegend(0.275, 0.67, 0.415, 0.87);
    //leg->SetHeader("");
//    leg->SetNColumns(2);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->AddEntry(set_line_width(fwd1m), "Raw Physics Waveform", "l");
    //leg->AddEntry(err, "Sys. Unc.", "f");
    leg->AddEntry(set_line_width(fit_line), "Linear Fit", "l");
    leg->AddEntry(set_line_width(osfit), "KDE", "l");

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());


    // Plotting
    axis->Draw("AXIS");
    line->Draw("same");
    //err->Draw("same E3");
    fwd1m->Draw("hist L same ][");
    fit_line->Draw("same");
    osfit->Draw("same");

    leg->Draw();
    text->DrawLatex(0.28, 0.88, ("Evt. " + runnr+": FWD1").c_str());

    TPad * pad2 = new TPad("pad2", "The pad 20% of the height", 0.52, 0.45, 1.0, 1.0, kWhite);

    pad2->Draw();
    pad2->cd();
    pad2->SetRightMargin(0.035);
    pad2->SetTopMargin(0.025);
    pad2->SetBottomMargin(0.16);
    pad2->SetLeftMargin(0.13);

    saxis->Draw("axis");
    line2->Draw("same");
    fwd1m->Draw("hist L same ][");
    fit_line->Draw("same");
    osfit->Draw("same");

    //osfit->Draw();
    //kde->Draw();
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/osfit.pdf");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/osfit.png");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/osfit.eps");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Reconstruction_Procedure/osfit.jpg");
    return 0;
}
