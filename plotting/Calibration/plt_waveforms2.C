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


void plt_waveforms2()
{
    // General stuff
    SetPhDStyle();

    // --- Canvas and Pads ------------------------------------------------------------------------------------------------------------------------------------

    int width = round(vis_multi*textwidth*pt2pixel);

    double ratio = 1./1.;

    TCanvas * can = new TCanvas("can", "can", width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    double padsplit = 0.6;
    double padsplit2 = 0.28;

    double rightmargin = 0.08;
    double leftmargin = 0.08;
    double topmargin = 0.02;
    double bottommargin = 0.14;
    double yoffset = 2;
    double xoffset = 2.4;

    TPad* pad1 = new TPad("pad1", "The pad 80% of the height", 0.0, padsplit, 1, 1);
    pad1->SetLeftMargin( leftmargin );
    pad1->SetBottomMargin( 0.15 );
    //pad1->SetLeftMargin( leftmargin );

    pad1->Draw();

    TPad* pad2 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.0, 1, padsplit);
    pad2->SetLeftMargin( leftmargin );
    pad2->SetBottomMargin(0.08);
    pad2->SetTopMargin(0.05);
    pad2->Draw();

    TPad* pad3 = new TPad("pad2", "The pad 20% of the height", padsplit2, 0.4*padsplit, 1.0, padsplit, kWhite);
//    TPad* pad3 = new TPad("pad2", "The pad 20% of the height", padsplit2, 0.33*padsplit, 0.985, padsplit-0.005, kWhite);
    pad3->SetRightMargin(0.005);
//    pad3->SetTopMargin(0.1);
    pad3->SetBottomMargin(0.14);
    pad3->SetLeftMargin(0.11);


    pad3->Draw();

    // --- Constants and colors etc. --------------------------------------------------------------------------------------------------------------------------

    double dt = 0.8;
    //double shift  = -1;
    int length = int(300/0.8);

    int c3 = kBlack;
    int c2 = tab20_lgray->GetNumber();
    int c1 = tab20_blue->GetNumber();

    int c6= kBlack;
    int c5 = tab20_lgray->GetNumber();
    int c4 = tab20_red->GetNumber();

    double alpha2 = 1;
    double alpha3 = 1;

    double linewidth = 2.0;


    // --- Top content -----------------------------------------------------------------------------------------------------------------------------------------

    TH1F* top_axis = new TH1F("calwf axis", "", length*0.5/double(1-padsplit2),  -dt/2., dt*length*0.5/double(1-padsplit2) - dt/2.);
    top_axis->GetXaxis()->SetTitle("Time [ns]");
    top_axis->GetYaxis()->SetTitle("Amplitude [mV]");
    top_axis->GetYaxis()->SetRangeUser( -3.99, 7.99);

    top_axis->GetXaxis()->SetTitleOffset(2.7);


    TLine *linetop = new TLine(-dt/2.,0,dt*length*0.5/double(1-padsplit2),0);
    linetop->SetLineColor(kGray);
    linetop->SetLineStyle(7);

    // Cal wf
    string path  = "/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/";

    string fname = "CalibrationEvent_401141-cal002_prepared.root";

    string calevtnr = fname.substr(17,13);

    TFile* rfile_cal = new TFile( (path + fname).c_str(), "open");

    TH1I* cal = (TH1I*) rfile_cal->Get("FWD1-INT");
    cal->SetDirectory(0);

    rfile_cal->Close("R");

    int cal_nbins = cal->GetNbinsX();

    TH1F* cal_line = new TH1F("calwf_line ", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);

    TH1F* cal_biterror = new TH1F("calwf_biterror", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);

    TH1F* cal_syserror = new TH1F("calwf_syserror", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);
    TH1F* cal_syserror_low = new TH1F("calwf_syserror_low", "", cal_nbins,  -dt/2., dt*cal_nbins - dt/2.);

    double bit_error = 50./127;
    double sys_error = 0.03*50.;

    for (size_t i = 1; i <= cal_nbins; i++)
    {
        double cont = cal->GetBinContent(i);

        cal_line->SetBinContent( i, cont );

        cal_biterror->SetBinContent(i, cont);
        cal_biterror->SetBinError(i, bit_error);

        cal_syserror->SetBinContent(i, cont+bit_error + sys_error);
        cal_syserror->SetBinError(i, bit_error + sys_error);

        cal_syserror_low->SetBinContent(i, cont-bit_error - sys_error);
    }

    cal_line->SetLineColor( c1 );
    cal_line->SetLineWidth(linewidth);
    // calwf->SetFillColor( claws_red2->GetNumber() );
    cal_line->SetMarkerSize(0.);

    //calwf_biterror->SetLineColor(1);
    cal_biterror->SetFillColorAlpha( c2, alpha2);
    cal_biterror->SetMarkerSize(0.);
    cal_biterror->SetLineWidth(0);

    cal_syserror->SetLineColor(c3);
    cal_syserror->SetLineStyle(1);
    cal_syserror->SetLineWidth(1);
    // calwf_syserror->SetFillColorAlpha( c3, alpha3);
    // calwf_syserror->SetMarkerSize(0.);
    // calwf_syserror->SetLineWidth(0);
    cal_syserror_low->SetLineColor(c3);
    cal_syserror_low->SetLineStyle(1);
    cal_syserror_low->SetLineWidth(1);

    // --- Legend ---

    //double legx1 = 0.77;
    double legx1 = 0.1;
    double legx2 = 0.22;
    double legy2 = 0.87;

    TLegend* ltop =  new TLegend(legx1, 0.61, legx2, 0.85);

    int legfontsize = ltop->GetTextSize();
    int legfont     = ltop->GetTextFont();

    ltop->SetBorderSize(0);
    ltop->SetFillColor(0);

    ltop->AddEntry(set_line_width4(cal_line), "Raw Calibration Waveform", "l");
    ltop->AddEntry(cal_biterror, "Bit Unc.", "f");

    TH1F* calsysdline = (TH1F*) cal_syserror->Clone("cal sys");
    //calsysdline->SetLineWidth(2);
    ltop->AddEntry(set_line_width4(calsysdline), "Bit + Sys. Unc.", "l");

    // Event number
    TLatex* ttop = new TLatex();
    ttop->SetNDC();
    ttop->SetTextFont(legfont+20);
    ttop->SetTextSize(gStyle->GetLegendTextSize());


    // --- Bottom content --------------------------------------------------------------------------------------------------------------------------------------

    TLine *linebot = new TLine(0,0,dt*length, 0);
    linebot->SetLineColor(kGray);
    linebot->SetLineStyle(7);
    //linebot->SetLineWidth(2);

    // --- Physics waveform ---

    fname = "PhysicsEvent_401141041_prepared.root";

    string evtnr = fname.substr(13,9);

    TFile *rfile = new TFile((path + fname).c_str(), "open");

    TH1I* ph = (TH1I*) rfile->Get("FWD1");
    ph->SetDirectory(0);

    rfile->Close("R");

    double dt2 = 0.8e-6;

    TH1F* ph_axis = new TH1F("ph_axis", "", ph->GetNbinsX(),  -dt2/2., dt2*ph->GetNbinsX() - dt2/2.);
    ph_axis->GetXaxis()->SetTitle("Time [ms]");
    ph_axis->GetYaxis()->SetTitle("Amplitude [mV]");
    ph_axis->GetYaxis()->SetRangeUser( -24.9, 499.99);
    ph_axis->GetXaxis()->SetTitleOffset(1.6);

    // TH1F* ph_axis_zoom = new TH1F("phwf axis", "", length,  -dt/2., dt*length - dt/2.);
    // ph_axis_zoom->GetXaxis()->SetTitle("Time [ns]");
    // ph_axis_zoom->GetYaxis()->SetTitle("Amplitude [mV]");
    // ph_axis_zoom->GetYaxis()->SetRangeUser( -52.9, 159.99);

    // --- Physics waveform ---

    TH1F* ph_line = new TH1F("ph_line ", "", ph->GetNbinsX(),  -dt2/2., dt2*ph->GetNbinsX() - dt2/2.);

    // TH1F* ph_biterror = new TH1F("ph_biterror ", "", ph->GetNbinsX(),  -dt/2., dt*ph->GetNbinsX() - dt/2.);
    //
    // TH1F* ph_syserror = new TH1F("ph_syserror ", "", ph->GetNbinsX(),  -dt/2., dt*ph->GetNbinsX() - dt/2.);
    // TH1F* ph_syserror_low = new TH1F("ph_syserror_low ", "", ph->GetNbinsX(),  -dt/2., dt*ph->GetNbinsX() - dt/2.);

    bit_error = 1000./127;
    sys_error = 0.03*1000.;

    for (size_t i = 1; i <=ph_axis->GetNbinsX(); i++)
    {
        double cont = ph->GetBinContent(i);

        ph_line->SetBinContent( i, cont );

        // ph_biterror->SetBinContent(i, cont);
        // ph_biterror->SetBinError(i, bit_error);
        //
        // ph_syserror->SetBinContent(i, cont + bit_error + sys_error);
        //
        // ph_syserror_low->SetBinContent(i, cont - bit_error - sys_error);
    }

    int shift = 0;

    for (size_t i = 1; i <= ph->GetNbinsX(); i++)
    // for (size_t i = 2000000; i <= ph->GetNbinsX(); i++)
    {
        if( ph->GetBinContent(i) > 100 )
        {
            shift = i-int(50/0.8);
            break;
        }
    }

    TH1F* ph_line_small = new TH1F("ph_line ", "", length,  -dt/2., dt*length - dt/2.);

    TH1F* ph_biterror = new TH1F("ph_biterror ", "", length,  -dt/2., dt*length - dt/2.);

    TH1F* ph_syserror = new TH1F("ph_syserror ", "", length,  -dt/2., dt*length - dt/2.);
    TH1F* ph_syserror_low = new TH1F("ph_syserror_low ", "", length,  -dt/2., dt*length - dt/2.);

    for (size_t i = 1; i <= length; i++)
    {
        double cont = ph->GetBinContent(i+shift);

        ph_line_small->SetBinContent( i, cont );

        ph_biterror->SetBinContent(i, cont);
        ph_biterror->SetBinError(i, bit_error);

        ph_syserror->SetBinContent(i, cont + bit_error + sys_error);

        ph_syserror_low->SetBinContent(i, cont - bit_error - sys_error);
    }

    TH1F* ph_axis_small = new TH1F("ph_axis_small", "", length,  -dt/2., dt*length - dt/2.);

    stringstream ss;
    ss << fixed << setprecision(3) << "Time - " << shift*dt/1000000. << " ms [ns]";
    ph_axis_small->GetXaxis()->SetTitle(ss.str().c_str());

    ph_axis_small->GetYaxis()->SetTitle("Amplitude [mV]");
    ph_axis_small->GetYaxis()->SetRangeUser( -54.9, 399.99);
    ph_axis_small->GetXaxis()->SetTitleOffset(2.8);
    ph_axis_small->GetYaxis()->SetTitleOffset( 2.1);
    ph_axis_small->GetYaxis()->SetNdivisions(505);

    ph_line->SetLineColor( c4 );
    ph_line->SetLineWidth(linewidth);
    // ph->SetFillColor( claws_red2->GetNumber() );
    ph_line->SetMarkerSize(0.);

    ph_line_small->SetLineColor( c4 );
    ph_line_small->SetLineWidth(linewidth);
    // ph->SetFillColor( claws_red2->GetNumber() );
    ph_line_small->SetMarkerSize(0.);

    //ph_biterror->SetLineColor(1);
    ph_biterror->SetFillColorAlpha( c5, alpha2);
    ph_biterror->SetMarkerSize(0.);
    ph_biterror->SetLineWidth(0);

    // ph_syserror->SetFillColorAlpha( c6, alpha3);
    // ph_syserror->SetMarkerSize(0.);
    // ph_syserror->SetLineWidth(0);

    ph_syserror->SetLineColor(c6);
    ph_syserror->SetLineStyle(1);
    ph_syserror->SetLineWidth(1);

    ph_syserror_low->SetLineColor(c6);
    ph_syserror_low->SetLineWidth(1);
    ph_syserror_low->SetLineStyle(1);

    TLatex* tlow = new TLatex();
    tlow->SetNDC();
    tlow->SetTextFont(legfont+20);
    cout << vis_multi*textsize*pt2pixel << endl;
    tlow->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* tmid = new TLatex();
    tmid->SetNDC();
    tmid->SetTextFont(legfont+20);
    tmid->SetTextSize(gStyle->GetLegendTextSize());


    TLegend* llow =  new TLegend(legx1, 0.76, legx2-0.01, legy2-0.02);
    llow->SetBorderSize(0);
    llow->SetFillColor(0);

    llow->AddEntry(set_line_width4(ph_line), "Raw Physics", "l");
    llow->AddEntry((TObject*)0, "Waveform", "");

    TLegend* l3 =  new TLegend(0.6, 0.55, 0.78, 0.83);

    l3->SetBorderSize(0);
    l3->SetFillColor(0);

    l3->AddEntry(set_line_width4(ph_line_small), "Raw Physics Waveform", "l");
    l3->AddEntry(ph_biterror, "Bit Unc.", "f");
    TH1F* phsysdline = (TH1F*) ph_syserror->Clone("ph sys");
    phsysdline->SetLineWidth(2);
    l3->AddEntry(set_line_width4(phsysdline), "Bit + Sys. Unc.", "l");

    // --- Top draw --------------------------------------------------------------------------------------------------------------------------------------------
    pad1->cd();

    top_axis->GetYaxis()->SetTickLength( top_axis->GetYaxis()->GetTickLength()*(1-padsplit) );
    top_axis->Draw("AXIS");

    linetop->Draw();

    cal_syserror->DrawCopy("hist L same ][");
    cal_syserror_low->DrawCopy("hist L same ][");
    cal_biterror->DrawCopy("E3 same");
    cal_line->DrawCopy("hist L same ][");

    ltop->Draw("same");
    ttop->DrawLatex(legx1+0.005, legy2, (calevtnr+": FWD1").c_str());

    top_axis->Draw("AXIS SAME");

    // --- Bottom draw -----------------------------------------------------------------------------------------------------------------------------------------
    pad2->cd();

    ph_axis->GetYaxis()->SetTickLength( ph_axis->GetYaxis()->GetTickLength()*(1-padsplit) );
    ph_axis->Draw("AXIS");

    // ph_syserror->DrawCopy("hist L same ][");
    // ph_syserror_low->DrawCopy("hist L same ][");
    // ph_biterror->DrawCopy("E3 same");
    ph_line->DrawCopy("hist L same ][");

    tlow->DrawLatex(legx1+0.005, legy2, (evtnr+": FWD1").c_str());
    llow->Draw();
    ph_axis->Draw("AXIS SAME");
    // --- Zoom draw -----------------------------------------------------------------------------------------------------------------------------------------
    pad3->cd();

    ph_axis_small->GetYaxis()->SetTickLength( ph_axis_small->GetYaxis()->GetTickLength()*(1-padsplit)*0.66 );
    ph_axis_small->Draw("AXIS");
    linebot->Draw();

    ph_syserror->DrawCopy("hist L same ][");
    ph_syserror_low->DrawCopy("hist L same ][");
    ph_biterror->DrawCopy("E3 same");
    ph_line_small->DrawCopy("hist L same ][");

    tlow->DrawLatex(0.61, 0.85, ("Zoom into " + evtnr+": FWD1").c_str());
    l3->Draw();
    ph_axis_small->Draw("AXIS SAME");
//    ph_axis_small->Draw("AXIS");

    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Data/waveforms.pdf");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Data/waveforms.png");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Data/waveforms.eps");
    can->SaveAs("~/Documents/PhD_Thesis/Software_Framework/Images/Data/waveforms.jpg");
}
