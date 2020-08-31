// #include <boost/filesystem.hpp>
#include <TFile.h>

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


void plt_waveforms()
{
    // General stuff
    SetPhDStyle();

    // int c1 = TColor::GetColor( 148, 103, 189);
    // int c1 = claws_black->GetNumber();
    // int c2 = tab20_blue->GetNumber();
    // int c3 = claws_gray->GetNumber();

    int c1 = kBlack;
    int c2 = kGray;
    int c3 = tab20_blue->GetNumber();

    // int c2 = TColor::GetColor(31,119,180);
    // int c3 = TColor::GetColor(174,199,232 );

    // int c2 = TColor::GetColor(236,111,46);
    // int c3 = TColor::GetColor(247,206, 160 );


    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_gray->GetNumber();

    // int c4 = claws_black->GetNumber();
    // int c5 = claws_red2->GetNumber();
    // int c6 = claws_gray->GetNumber();

    int c4 = kBlack;
    int c5 = kGray;
    int c6 = claws_red2->GetNumber();

    // double alpha2 = 0.6;
    // double alpha3 = 0.3;

    double alpha2 = 1;
    double alpha3 = 1;

    double linewidth = 2.0;

    double dt = 0.8;
    //double shift  = -1;
    double length = 300;


    // --- Calibration waveform ---
    string path  = "/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/";

    string fname = "CalibrationEvent_401141-cal002_prepared.root";

    string calevtnr = fname.substr(17,13);

    TFile* rfile_cal = new TFile( (path + fname).c_str(), "open");

    TH1I* calwf = (TH1I*) rfile_cal->Get("FWD1-INT");
    calwf->SetDirectory(0);

    rfile_cal->Close("R");

    int cal_nbins = calwf->GetNbinsX();

    TH1F* calwf_axis = new TH1F("calwf axis", "", length,  -dt/2., dt*length - dt/2.);
    calwf_axis->GetXaxis()->SetTitle("Time [ns]");
    calwf_axis->GetYaxis()->SetTitle("Amplitude [mV]");
    calwf_axis->GetYaxis()->SetRangeUser( -3.99, 7.99);


    TH1F* calwf_line = new TH1F("calwf_line ", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);

    TH1F* calwf_biterror = new TH1F("calwf_biterror", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);

    TH1F* calwf_syserror = new TH1F("calwf_syserror", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);
    TH1F* calwf_syserror_low = new TH1F("calwf_syserror_low", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);

    double bit_error = 50./127;
    double sys_error = 0.03*50.;

    for (size_t i = 1; i <= cal_nbins; i++)
    {
        double cont = calwf->GetBinContent(i);

        calwf_line->SetBinContent( i, cont );

        calwf_biterror->SetBinContent(i, cont);
        calwf_biterror->SetBinError(i, bit_error);

        calwf_syserror->SetBinContent(i, cont+bit_error + sys_error);
        calwf_syserror->SetBinError(i, bit_error + sys_error);

        calwf_syserror_low->SetBinContent(i, cont-bit_error - sys_error);
    }

    calwf_line->SetLineColor( c1 );
    calwf_line->SetLineWidth(linewidth);
    // calwf->SetFillColor( claws_red2->GetNumber() );
    calwf_line->SetMarkerSize(0.);

    //calwf_biterror->SetLineColor(1);
    calwf_biterror->SetFillColorAlpha( c2, alpha2);
    calwf_biterror->SetMarkerSize(0.);
    calwf_biterror->SetLineWidth(0);

    calwf_syserror->SetLineColor(c3);
    calwf_syserror->SetLineStyle(2);
    calwf_syserror->SetLineWidth(1);
    // calwf_syserror->SetFillColorAlpha( c3, alpha3);
    // calwf_syserror->SetMarkerSize(0.);
    // calwf_syserror->SetLineWidth(0);
    calwf_syserror_low->SetLineColor(c3);
    calwf_syserror_low->SetLineStyle(2);
    calwf_syserror_low->SetLineWidth(1);

    TH1F* calwf_axis_small =  (TH1F*) calwf_axis->Clone("phwf_axis_small");
    calwf_axis_small->GetXaxis()->SetTitle("");
    calwf_axis_small->GetXaxis()->SetLabelSize( 0.7*calwf_axis_small->GetXaxis()->GetLabelSize());
    // phwf_axis_small->GetXaxis()->SetTitleOffset(0);
    calwf_axis_small->GetYaxis()->SetTitle("");

    calwf_axis_small->GetYaxis()->SetLabelSize( 0.7*calwf_axis_small->GetYaxis()->GetLabelSize());

    calwf_axis_small->GetYaxis()->SetTickLength( calwf_axis_small->GetYaxis()->GetTickLength()*0.6);

    calwf_axis_small->GetXaxis()->SetRangeUser(116,143);
    calwf_axis_small->GetYaxis()->SetRangeUser(15.01,59);

    calwf_axis_small->GetXaxis()->SetNdivisions(505);
    calwf_axis_small->GetYaxis()->SetNdivisions(505);


    // --- Physics waveform ---

    fname = "PhysicsEvent_401141041_prepared.root";

    string evtnr = fname.substr(13,9);

    TFile *rfile = new TFile((path + fname).c_str(), "open");

    TH1I* phwf = (TH1I*) rfile->Get("FWD1");
    phwf->SetDirectory(0);

    rfile->Close("R");
    // delete rfile;

    int shift = 0;

    for (size_t i = 2000000; i <= phwf->GetNbinsX(); i++)
    {
        if( phwf->GetBinContent(i) > 50 )
        {
            shift = i-132;
            break;
        }
    }

    TH1F* phwf_axis = new TH1F("phwf axis", "", length,  -dt/2., dt*length - dt/2.);
    phwf_axis->GetXaxis()->SetTitle("Time [ns]");
    phwf_axis->GetYaxis()->SetTitle("Amplitude [mV]");
    phwf_axis->GetYaxis()->SetRangeUser( -52.9, 159.99);

    TH1F* phwf_line = new TH1F("phwf_line ", "", length,  -dt/2., dt*length - dt/2.);

    TH1F* phwf_biterror = new TH1F("phwf_biterror", "", length,  -dt/2., dt*length - dt/2.);

    TH1F* phwf_syserror = new TH1F("phwf_syserror", "", length,  -dt/2., dt*length - dt/2.);
    TH1F* phwf_syserror_low = new TH1F("phwf_syserror_low", "", length,  -dt/2., dt*length - dt/2.);

    bit_error = 1000./127;
    sys_error = 0.03*1000.;

    for (size_t i = 1; i <=length; i++)
    {
        double cont = phwf->GetBinContent(i+shift);

        phwf_line->SetBinContent( i, cont );

        phwf_biterror->SetBinContent(i, cont);
        phwf_biterror->SetBinError(i, bit_error);

        phwf_syserror->SetBinContent(i, cont + bit_error + sys_error);

        phwf_syserror_low->SetBinContent(i, cont - bit_error - sys_error);
    }

    phwf_line->SetLineColor( c4 );
    phwf_line->SetLineWidth(linewidth);
    // phwf->SetFillColor( claws_red2->GetNumber() );
    phwf_line->SetMarkerSize(0.);

    //phwf_biterror->SetLineColor(1);
    phwf_biterror->SetFillColorAlpha( c5, alpha2);
    phwf_biterror->SetMarkerSize(0.);
    phwf_biterror->SetLineWidth(0);

    // phwf_syserror->SetFillColorAlpha( c6, alpha3);
    // phwf_syserror->SetMarkerSize(0.);
    // phwf_syserror->SetLineWidth(0);

    phwf_syserror->SetLineColor(c6);
    phwf_syserror->SetLineStyle(2);
    phwf_syserror->SetLineWidth(1);

    phwf_syserror_low->SetLineColor(c6);
    phwf_syserror_low->SetLineWidth(1);
    phwf_syserror_low->SetLineStyle(2);

    TH1F* phwf_axis_small =  (TH1F*) phwf_axis->Clone("phwf_axis_small");
    phwf_axis_small->GetXaxis()->SetTitle("");
    phwf_axis_small->GetXaxis()->SetLabelSize( 0.7*phwf_axis_small->GetXaxis()->GetLabelSize());
    // phwf_axis_small->GetXaxis()->SetTitleOffset(0);
    phwf_axis_small->GetYaxis()->SetTitle("");

    phwf_axis_small->GetYaxis()->SetLabelSize( 0.7*phwf_axis_small->GetYaxis()->GetLabelSize());

    phwf_axis_small->GetYaxis()->SetTickLength( phwf_axis_small->GetYaxis()->GetTickLength()*0.6);

    phwf_axis_small->GetXaxis()->SetRangeUser(116,143);
    phwf_axis_small->GetYaxis()->SetRangeUser(15.01,59);

    phwf_axis_small->GetXaxis()->SetNdivisions(505);
    phwf_axis_small->GetYaxis()->SetNdivisions(505);

//    phwf_axis->GetYaxis()->SetRangeUser( -52.9, 159.99);

    // // --- Create the line ---
    TLine *line = new TLine(-dt/2.,0,dt*length - dt/2.,0);
    line->SetLineColor(kGray);
    line->SetLineStyle(7);

    // --- Canvas and Pads

    int width = round(vis_multi*textwidth*pt2pixel);
    // double height = 900;
    // double ratio = 16./9.;
    double ratio = 1./1.;
    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    double padsplit = 0.5;
    TPad* pad1 = new TPad("pad1", "The pad 80% of the height", 0.0, padsplit, 1, 1);
    TPad* pad2 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.0, 1, padsplit);
    TPad* pad3 = new TPad("pad2", "The pad 20% of the height", 0.65, 0.8, 0.99, 1.0, kWhite);
    TPad* pad4 = new TPad("pad2", "The pad 20% of the height", 0.65, 0.29, 1.0, 0.5, kWhite);

    pad1->Draw();
    pad2->Draw();
//    pad3->Draw();
    pad4->Draw();

    double rightmargin = 0.08;
    double leftmargin = 0.08;
    double topmargin = 0.02;
    double bottommargin = 0.14;
    double yoffset = 1.7;
    double xoffset = 2.4;

    calwf_axis->GetXaxis()->SetTitleOffset(xoffset);
    phwf_axis->GetXaxis()->SetTitleOffset(xoffset);

    // --- pad1 ---
    pad1->SetLeftMargin( leftmargin );
    //pad1->SetTopMargin( topmargin );
    //pad1->SetBottomMargin( bottommargin +0.025);
//    pad1->SetRightMargin( rightmargin );

    // --- pad2 ---
    pad2->SetLeftMargin( leftmargin );
    //pad2->SetTopMargin( topmargin );
    //pad2->SetBottomMargin( bottommargin );
//    pad2->SetRightMargin( rightmargin );

    pad3->SetRightMargin(0.02);
    //pad3->SetTopMargin(0.0);
    pad3->SetBottomMargin(0.08);
    pad3->SetLeftMargin(0.08);

    pad4->SetRightMargin(0.02);
    //pad4->SetTopMargin(0.0);
    pad4->SetBottomMargin(0.08);
    pad4->SetLeftMargin(0.08);

    // --- Legend ---

    //double legx1 = 0.77;
    double legx1 = 0.11;
    double legx2 = 0.23;
    double legy2 = 0.87;

    TLegend* lup =  new TLegend(legx1, 0.68, legx2, legy2-0.01);

    int legfontsize = lup->GetTextSize();
    int legfont     = lup->GetTextFont();

    lup->SetBorderSize(0);
    lup->SetFillColor(0);

    lup->AddEntry(set_line_width4(calwf_line), "Raw Calibration Waveform", "l");
    lup->AddEntry(calwf_biterror, "Bit Unc.", "f");

    TH1F* calsysdline = (TH1F*) calwf_syserror->Clone("cal sys");
    //calsysdline->SetLineWidth(2);
    lup->AddEntry(set_line_width4(calsysdline), "Bit + Sys. Unc.", "l");

    TLegend* llow =  (TLegend*) lup->Clone("llow");
    llow->Clear();
    llow->AddEntry(set_line_width4(phwf_line), "Raw Physics Waveform", "l");
    llow->AddEntry(phwf_biterror, "Bit Unc.", "f");
    TH1F* phsysdline = (TH1F*) phwf_syserror->Clone("ph sys");
    phsysdline->SetLineWidth(2);
    llow->AddEntry(set_line_width4(phsysdline), "Bit + Sys. Unc.", "l");

    TLatex* tup = new TLatex();
    tup->SetNDC();
    tup->SetTextFont(legfont+20);
    tup->SetTextSize(vis_multi*textsize*pt2pixel*0.8);

    TLatex* tlow = new TLatex();
    tlow->SetNDC();
    tlow->SetTextFont(legfont+20);
    cout << vis_multi*textsize*pt2pixel << endl;
    tlow->SetTextSize(vis_multi*textsize*pt2pixel*0.8);

    // --- Draw first pad ---
    pad1->cd();

    calwf_axis->GetYaxis()->SetTickLength( calwf_axis->GetYaxis()->GetTickLength()*(1-padsplit) );


    calwf_axis->Draw("AXIS");

    //pad1->Update();
    //calwf_axis->Draw("AXIS");

    line->Draw();
    //lup->Draw("same");
    //calwf_syserror->DrawCopy("E3 same");
    calwf_syserror->DrawCopy("hist L same ][");
    calwf_syserror_low->DrawCopy("hist L same ][");

    calwf_biterror->DrawCopy("E3 same");

    calwf_line->DrawCopy("hist L same ][");

    lup->Draw("same");
    tup->DrawLatex(legx1+0.005, legy2, (calevtnr+": FWD1").c_str());

    //calwf_axis->Draw("same axis");

    // pad3->cd();
    //
    // calwf_axis_small->Draw("axis");
    //
    // line->Draw();
    //
    // calwf_syserror->DrawCopy("hist L same ][");
    // calwf_syserror_low->DrawCopy("hist L same ][");
    //e
    // calwf_biterror->DrawCopy("E3 same");
    //
    // calwf_line->DrawCopy("hist L same ][");
    //
    // calwf_axis_small->Draw("same axis");
    //

    // --- Second pad ---
    pad2->cd();
    // -52.9, 159.99
    // 116,143)
    TLine *line2 = new TLine(116,-52.9,116, 159.99);
    line2->SetLineColor(kBlack);
    line2->SetLineStyle(9);
    line2->SetLineWidth(2);

    TLine *line3 = new TLine(143,-52.9,143, 159.99);
    line3->SetLineColor(kBlack);
    line3->SetLineStyle(9);
    line3->SetLineWidth(2);
    // TLine *line = new TLine(-dt/2.,0,dt*length - dt/2.,0);
    // line->SetLineColor(kGray);
    // line->SetLineStyle(7);

    phwf_axis->GetYaxis()->SetTickLength( phwf_axis->GetYaxis()->GetTickLength()*(1-padsplit) );


    phwf_axis->Draw("AXIS");
    line2->Draw();
    line3->Draw();
    line->Draw();
    llow->Draw("same");
    tlow->DrawLatex(legx1+0.005, legy2, (evtnr+": FWD1").c_str());

    phwf_syserror->DrawCopy("hist L same ][");
    phwf_syserror_low->DrawCopy("hist L same ][");

    phwf_biterror->DrawCopy("E3 same");

    phwf_line->DrawCopy("hist L same ][");

    // lup->Draw("same");
    // tup->DrawLatex(legx1, legy2, evtnr.c_str());

    //phwf_axis->DrawCopy("same axis");


    pad4->cd();

    phwf_axis_small->DrawCopy("axis");
    line->Draw();

    phwf_syserror->DrawCopy("hist L same ][");
    phwf_syserror_low->DrawCopy("hist L same ][");

    phwf_biterror->DrawCopy("E3 same");

    phwf_line->DrawCopy("hist L same ][");

    //phwf_axis_small->Draw("same axis");


    can->SaveAs("./Calibration/waveforms.pdf");
    can->SaveAs("./Calibration/waveforms.png");
    can->SaveAs("./Calibration/waveforms.eps");
    can->SaveAs("./Calibration/waveforms.jpg");

    // int width = round(vis_multi*textwidth*pt2pixel);
    // double height = 900;
    // double ratio = 16./9.;
    // ratio = 1./1.;
    // TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    // can->SetFillColor(kWhite);
    // can->SetBorderMode(0);
    // can->SetBorderSize(2);
    // can->SetFrameBorderMode(0);


}
