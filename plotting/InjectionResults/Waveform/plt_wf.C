// #include <boost/filesystem.hpp>
#include <TFile.h>

using namespace std;

TCanvas* make_waveform_canvas(Target target)
{
    // General stuff
    SetPhDStyle();

    // --- Canvas and Pads

    int width = round(vis_multi*textwidth*pt2pixel);
    // double height = 900;
    // double ratio = 16./9.;
    double ratio = 2./1.;

    string cantitle = target.typein+"_waveform";
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);

    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    //can->SetLeftMargin(0.08);

    double padsplit = 1-0.65;
    double leftmargin =0.06;
    double rightmargin =0.01;

    TPad* pad1 = new TPad("pad1", "top", 0.,padsplit, 1., 1.);
    pad1->SetLeftMargin(leftmargin);
    pad1->SetRightMargin(rightmargin);
    pad1->SetBottomMargin(0.03);

    TPad* pad2 = new TPad("pad2", "bottom", 0., 0., 1., padsplit);
    pad2->SetLeftMargin(leftmargin);
    pad2->SetRightMargin(rightmargin);
    pad2->SetTopMargin(0.03);
    pad2->SetBottomMargin(0.25);

    TPad* pad3 = new TPad("pad3", "cutout", 0.5, 0.5, 0.9725, 0.95);
    // pad2->SetLeftMargin(leftmargin);
    pad3->SetRightMargin(0.03);
    // pad2->SetTopMargin(0.035);
    pad3->SetBottomMargin(0.2);
    //pad2->SetLogy();
    pad1->Draw();
    pad2->Draw();
    pad3->Draw();

    // Do the axis and the range shit -------------------------------------------------------------------------------------------------------------------------

    int binshift = 0;
    if(target.aligned)
    {
        binshift = 500;
    }

    double xlow = 0;
    double xup = 2.4;

    if(target.n != -1 )
    {
        double dt = 0.8e-6;
        xlow = -dt/2.;
        xup = target.n*dt -dt/2.;

    }
    cout << "xlow:" << xlow<< ", xup" << xup<<endl;

    double ylow = 0;
    double yup = 3.5;

    double ylow2 = 0;
    double yup2 = 7;


    TH1F* axis_bottom = new TH1F("axis_bottom","axis_bottom",1000,xlow, xup-binshift*0.8e-6);

    axis_bottom->GetXaxis()->SetTitleOffset(2.78);
    // axis_bottom->GetXaxis()->SetTitleOffset(1.5);

    axis_bottom->GetYaxis()->SetTitleOffset(0.8);
    axis_bottom->GetXaxis()->SetLabelOffset(0.03);
    axis_bottom->GetYaxis()->SetRangeUser(ylow, yup);
    axis_bottom->GetYaxis()->SetRangeUser(ylow2, yup2);
    axis_bottom->GetXaxis()->SetNdivisions(510);

    axis_bottom->GetXaxis()->SetTitle("Time [ms]");

    axis_bottom->GetYaxis()->SetTitle("Particles [MIP]");

    axis_bottom->GetYaxis()->SetNdivisions(505);
    axis_bottom->GetYaxis()->SetTickLength( axis_bottom->GetXaxis()->GetTickLength()*padsplit/ratio);

    TLatex* tex = new TLatex();
    tex->SetNDC();
    tex->SetTextFont(63);

    // -----   Colors and Style   ---------------------
//    double linewidth = 2;

    // int c1 = claws_red->GetNumber();
    // int c2 = claws_gray->GetNumber();
    // int c3 = kBlack;

    int c1 =  tab20_red->GetNumber();
    int c2 = tab20_lgray->GetNumber();
    int c3 = kBlack;

    int linewidth = 3;
    int linewidth_c = 2;

    // ----- Get the hists     ---------------------------------------------------------

    TFile* file = new TFile( (target.pathin + "/" + target.filein).c_str(), "open");

    if( file->IsZombie() )
    {
        cout << "Error openning file" << endl;
        exit(-1);
    }
    int rebin1 = 1;
    int rebin = 5000;
    double scaling = 1e3;

    TH1F * fwd1 = (TH1F*) file->Get( "FWD1" );

    int nbins = 0;
    if(target.n != -1) nbins = target.n;
    else nbins = fwd1->GetNbinsX();


    fwd1->SetBins(nbins,(fwd1->GetBinLowEdge(1)-fwd1->GetBinWidth(1)*binshift)*scaling, (fwd1->GetBinLowEdge(nbins) + fwd1->GetBinWidth(nbins)*(1-binshift))*scaling  );
//    if(target.aligned) fwd1->SetBinContent(1,0);
    for(int bin = 1; bin< binshift; ++bin) fwd1->SetBinContent(bin,0);
    fwd1->SetLineColor(c1);
    fwd1->SetFillColor(c1);
    fwd1->SetLineWidth(linewidth);
    cout <<"fwd1 max: " << fwd1->GetMaximum() << endl;
//    fwd1->Rebin(rebin1);
    cout <<"fwd1 max: " << fwd1->GetMaximum() << endl;
    cout <<"FWD1 integral: " << fwd1->Integral() << endl;
    for(int bin = 1; bin < nbins; ++bin)
    {
        fwd1->SetBinError(bin, 0);
    }

    TH1F* fwd1c = (TH1F*) fwd1->Clone("FWD1c");
    fwd1c->Rebin(rebin);
    fwd1c->SetLineWidth(linewidth_c);
    //fwd1->Rebin(rebin);

    TH1F* fwd1cum = (TH1F*)fwd1c->GetCumulative();
    fwd1cum->SetDirectory(0);
    fwd1cum->SetFillStyle(0);

    TH1F * fwd2 = (TH1F*) file->Get( "FWD2" );

    fwd2->SetBins(nbins,(fwd2->GetBinLowEdge(1)-fwd2->GetBinWidth(1)*binshift)*scaling, (fwd2->GetBinLowEdge(nbins) + fwd2->GetBinWidth(nbins)*(1-binshift))*scaling  );
//    if(target.aligned) fwd2->SetBinContent(1,0);
    for(int bin = 1; bin< binshift; ++bin) fwd2->SetBinContent(bin,0);
    fwd2->SetLineColor(c2);
    fwd2->SetFillColor(c2);
    fwd2->SetLineWidth(linewidth);
    fwd2->Rebin(rebin1);

    TH1F* fwd2c = (TH1F*) fwd2->Clone("FWD2c");
    fwd2c->Rebin(rebin);
    fwd2c->SetLineWidth(linewidth_c);
    //fwd1->Rebin(rebin);

    TH1F* fwd2cum = (TH1F*)fwd2c->GetCumulative();
    fwd2cum->SetFillStyle(0);
//        fwd2c->Scale(100/fwd2->Integral());
    fwd2cum->SetDirectory(0);

    TH1F * fwd3 = (TH1F*) file->Get( "FWD3" );
    fwd3->SetBins(nbins,(fwd3->GetBinLowEdge(1)-fwd3->GetBinWidth(1)*binshift)*scaling, (fwd3->GetBinLowEdge(nbins) + fwd3->GetBinWidth(nbins)*(1-binshift))*scaling  );
//    if(target.aligned) fwd3->SetBinContent(1,0);
    for(int bin = 1; bin< binshift; ++bin) fwd3->SetBinContent(bin,0);
    fwd3->SetLineColor(c3);
    fwd3->SetFillColor(c3);
    fwd3->SetLineWidth(linewidth);
    fwd3->Rebin(rebin1);

    TH1F* fwd3c = (TH1F*) fwd3->Clone("FWD3c");
    fwd3c->Rebin(rebin);
    fwd3c->SetLineWidth(linewidth_c);
    //fwd1->Rebin(rebin);

    TH1F* fwd3cum = (TH1F*)fwd3c->GetCumulative();
    fwd3cum->SetFillStyle(0);
//    fwd3c->Scale(100/fwd3->Integral());
    fwd3cum->SetDirectory(0);

    double max = 0;
    int maxbin = 1;
    for(int bin =1 +binshift; bin < fwd1->FindBin(xup); ++bin)
    {

        if( (fwd1->GetBinContent(bin)+fwd2->GetBinContent(bin)+fwd3->GetBinContent(bin)) > max)
        {
            maxbin = bin;
            max = (fwd1->GetBinContent(bin)+fwd2->GetBinContent(bin)+fwd3->GetBinContent(bin));
        }

    }

    double markersize = 0.;
    // double
    fwd1->SetMarkerStyle(20);
    fwd1->SetMarkerSize(markersize);

    fwd2->SetMarkerStyle(20);
    fwd2->SetMarkerSize(markersize);

    fwd3->SetMarkerStyle(20);
    fwd3->SetMarkerSize(markersize);


    if(max<=0.025)
    {
        fwd1->Scale(1000.);
        fwd2->Scale(1000.);
        fwd3->Scale(1000.);
    }
    // ----- Do the Stack     ---------------------------------------------------------

    THStack *stack = new THStack("stack","");

    cout << "fwd1->GetBinContent(1): " << fwd1->GetBinContent(2) << ", fwd1->GetBinCenter(1): " << fwd1->GetBinCenter(2) << endl;
    stack->Add(fwd1);
    stack->Add(fwd2);
    stack->Add(fwd3);

    // int bin1 = 100000;
    // int bin2 = 220000;
    int bin1 = 116000;
    int bin2 = 240000;
    double dt =0.8;
    //int dist = bin2 - bin1;
    int dist = 250000;

    if(target.data == "NI-VACS")
    {
        dist = 250;
        maxbin = 100000./0.8;
    }

    bin1 = maxbin - dist/2.+10000;
    bin2 = maxbin + dist/2.+10000;
    if(target.typein == "LER_Reference_WAVEFORM_PEAK_FFT_INJ_LERONLY")
    {
        double shift =10000;
        bin1 += shift;
        bin2 += shift;
    }

     fwd1c->Reset();
     fwd2c->Reset();
     fwd3c->Reset();
     fwd1c->SetLineWidth(linewidth);
     fwd2c->SetLineWidth(linewidth);
     fwd3c->SetLineWidth(linewidth);
    //TH1F* fwd1s = new TH1F("fwd1s","fwd1s",bin2-bin1, bin1*dt-dt/2., bin2*dt-dt/2);
    double scale = 1e-3;
    if(target.data == "NI-VACS")
    {
        fwd1c->SetBins(bin2-bin1, -dt/2., (bin2-bin1)*dt-dt/2);
        fwd2c->SetBins(bin2-bin1, -dt/2., (bin2-bin1)*dt-dt/2);
        fwd3c->SetBins(bin2-bin1, -dt/2., (bin2-bin1)*dt-dt/2);
    }
    else
    {
        fwd1c->SetBins(bin2-bin1, (bin1*dt-dt/2.)*scale, (bin2*dt-dt/2)*scale);
        fwd2c->SetBins(bin2-bin1, (bin1*dt-dt/2.)*scale, (bin2*dt-dt/2)*scale);
        fwd3c->SetBins(bin2-bin1, (bin1*dt-dt/2.)*scale, (bin2*dt-dt/2)*scale);
    }

    for(int bin = bin1; bin<=bin2; ++bin)
    {
        fwd1c->SetBinContent(bin-bin1+1, fwd1->GetBinContent(bin));

        fwd2c->SetBinContent(bin-bin1+1, fwd2->GetBinContent(bin));

        fwd3c->SetBinContent(bin-bin1+1, fwd3->GetBinContent(bin));
    }


    // if(max> 0.02 && max<= 1.)
    // {
    //     fwd1c->Scale(1000.);
    //     fwd2c->Scale(1000.);
    //     fwd3c->Scale(1000.);
    // }


    cout<< "fwd1s bin widht: " << fwd1c->GetBinWidth(10)<< endl;
    cout <<"fwd1s max: " << fwd1c->GetMaximum() << endl;
    cout <<"fwd1cum max: " << fwd1cum->GetMaximum() << endl;
    THStack *s_stack = new THStack("s_stack","");
    s_stack->Add(fwd1c);
    s_stack->Add(fwd2c);
    s_stack->Add(fwd3c);
    cout << "s_stack max: " << s_stack->GetMaximum() << endl;

    // Legend and Text -------------------------------------------------------------


    // TLegend* leg =  new TLegend(0.1, 0.645, 0.39, 0.715);
    TLegend* leg =  new TLegend(0.1, 0.73, 0.35, 0.8);
//    TLegend* leg =  new TLegend(0.1, 0.72, 0.39, 0.80);
    //leg->SetHeader("");
    leg->SetNColumns(3);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    leg->AddEntry(make_box(fwd1), "FWD1", "f");
    leg->AddEntry(make_box(fwd2), "FWD2", "f");
    leg->AddEntry(make_box(fwd3), "FWD3", "f");

    //leg->AddEntry(err, "Sys. + Stat. Unc.", "f");
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(legfont+20);
    text->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* stext = new TLatex();
    stext->SetNDC();
    stext->SetTextFont(legfont+20);
    stext->SetTextSize(gStyle->GetLegendTextSize());

    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont-10);
    ttext->SetTextSize(gStyle->GetLegendTextSize()*0.9);
    ttext->SetTextColor(kGray+1);

    // Do the drawing shit -------------------------------------------------------------
    pad1->cd();

    stack->Draw("HIST ][ ");


    // Trigger delay line

    int linestyle = 9;

    TLine *line;

    if(target.data == "HER-ALL")
    {
        line = new TLine(0.10744, 0,0.10744,1);
    }
    else if(target.data == "LER-ALL")
    {
        line = new TLine(0.10743, 0,0.10743, 5.3);
    }
    else
    {
        line = new TLine(0.10744, 0,0.10744,1);
    }
 //	TArrow* line = new TArrow(fcombined->GetMaximumX(rup, 5.), 5,fcombined->GetMaximumX(rup, 5.),fcombined->GetMaximum(rup, 5.), 0.015, "-|>");
    line->SetLineColor(kGray+1);
 // /	line->SetFillStyle(0);
    line->SetLineStyle(linestyle);
    line->SetLineWidth(1);

    // TLine *line2 = new TLine(0.10748, 0,0.10748, 5.3);
    //
    // line2->Draw("same");

    line->Draw("same");



    if(target.data == "HER-ALL")
    {
        ttext->DrawLatex(0.11, 0.65, "T_{trg} = 107.44 #mus");
    }
    else if(target.data == "LER-ALL")
    {
        ttext->DrawLatex(0.11, 0.65, "T_{trg} = 107.38 #mus, 107.48 #mus");
    }


    stack->Draw("HIST ][ same");

    stack->GetYaxis()->SetTitle("Particle Rate [MIP/0.8 ns]");
    if(max<=0.02) stack->GetYaxis()->SetTitle("Particle Rate [mMIP/0.8 ns]");

    stack->GetXaxis()->SetTitle();
    stack->GetXaxis()->SetLabelSize(0);
    stack->GetYaxis()->SetTitleOffset(0.8);

    // cout << "Stack1 max: " << stack->GetMaximum() << endl;
    // stack->GetYaxis()->SetLimits(0,stack->GetMaximum()*1.4);
    // if(target.n != -1)
    // {
    //     double dt = 0.8e-6;
    //     stack->GetXaxis()->SetRangeUser(-dt/2., target.n*dt -dt/2.);
    // }

    if(maxbin >= fwd1->FindBin(xup)/2.)
    {
        stack->SetMinimum(0.);
        if(max <=0.02) stack->SetMaximum(max*1000.*4);
        else stack->SetMaximum(max*4);
    }
    else
    {
        stack->SetMinimum(0.);
        if(max <=0.02) stack->SetMaximum(max*1000.*1.7);
        else stack->SetMaximum(max*1.7);

    }
    cout << "stack max: " << max <<endl;
    cout << "fwd1->FindBin(xup): " << fwd1->FindBin(xup) << endl;
    cout << "Maxbin: " << maxbin << endl;
    stack->GetXaxis()->SetRangeUser(xlow,xup-binshift*fwd1->GetBinWidth(1));
    // stack->SetMinimum(0.);
    // stack->SetMaximum(max*1.6);

    stack->GetYaxis()->SetTickLength( stack->GetXaxis()->GetTickLength()*(1-padsplit)/ratio );
    // For runs 401141 and 401140 [0,14]
    // For runs 401145 and 401131 [0,250]

    text->DrawLatex(0.1, 0.84, ("Averaged Reconstructed Waveform: " + target.data).c_str());
    // text->DrawLatex(0.1, 0.75, );
    pad1->Modified();
    can->Update();

    leg->Draw();

    pad2->cd();

    axis_bottom->GetXaxis()->SetTickLength( stack->GetXaxis()->GetTickLength()/(1-padsplit) );
    axis_bottom->GetYaxis()->SetTickLength( stack->GetYaxis()->GetTickLength()/(1-padsplit) );

    // if(target.n != -1)
    // {
    //     axis_bottom->GetYaxis()->SetRangeUser(0,fwd1cum->GetBinContent(fwd1cum->FindBin(target.n*dt -dt/2.))*1.2);
    // }
    // else
    // {
    //     axis_bottom->GetYaxis()->SetRangeUser(0,fwd1cum->GetMaximum()*1.2);
    // }


    axis_bottom->GetYaxis()->SetRangeUser(0,fwd1cum->GetBinContent(fwd1cum->FindBin(xup) -1 )*1.4);
    cout << "fwd1cum->GetBinContent(fwd1cum->FindBin(xup))*1.2: " << fwd1cum->GetBinContent(fwd1cum->FindBin(xup) -1 )*1.2 << endl;

    axis_bottom->Draw("AXIS");


    fwd1cum->Draw("hist L same ][");
    fwd2cum->Draw("hist L same ][");
    fwd3cum->Draw("hist L same ][");
    pad2->Modified();
    stext->DrawLatex(0.1, 0.8, "Cumulative Distribution Function");
    pad2->Modified();
    can->Update();

    pad3->cd();

//    s_stack->Draw("AXIS");
    s_stack->Draw();

    double textsize = s_stack->GetXaxis()->GetTitleSize()*0.9;
    s_stack->GetXaxis()->SetLabelSize(textsize);
    s_stack->GetXaxis()->SetTitleSize(textsize);
    s_stack->GetXaxis()->SetLabelOffset(0.015);

    s_stack->GetXaxis()->SetTitle("Time [#mus]");
    if(target.data == "NI-VACS")
    {
        stringstream ss;
        ss << fixed << setprecision(0) << "Time - " << maxbin*0.8*1e-3 <<" #mus [ns]";
        s_stack->GetXaxis()->SetTitle(ss.str().c_str());

    }
//    s_stack->GetXaxis()->SetRangeUser(0.5,0.25);

    s_stack->GetYaxis()->SetLabelSize(textsize);
    s_stack->GetYaxis()->SetTitleSize(textsize);
    s_stack->GetYaxis()->SetLabelOffset(0.01);

    if(max<=0.25)
    {
        s_stack->GetYaxis()->SetTitle("Part. Rate [mMIP/0.8 ns]");
    }
    else
    {
        s_stack->GetYaxis()->SetTitle("Part. Rate [MIP/0.8 ns]");
    }

    s_stack->SetMaximum(0.);
    s_stack->SetMaximum(90);

    s_stack->GetXaxis()->SetTitleOffset(2.3);

    if(target.data == "HER-ALL")
    {
        s_stack->GetYaxis()->SetTitleOffset(1.9);
    }
    else
    {
        s_stack->GetYaxis()->SetTitleOffset(1.3);
    }


    s_stack->GetXaxis()->SetNdivisions(506);
    s_stack->GetYaxis()->SetNdivisions(506);

    // s_stack->GetYaxis()->

//     0.5, 0.5, 0.9725, 0.95

    s_stack->GetYaxis()->SetTickLength( stack->GetYaxis()->GetTickLength()/(1-padsplit) );
    s_stack->GetXaxis()->SetTickLength( s_stack->GetYaxis()->GetTickLength()/0.5 );

    // s_stack->SetMaximum(0.);
    if(target.data == "HER-REF")
    {
        s_stack->SetMaximum(1);
    }
    else if(target.data == "HER-ALL")
    {
        s_stack->SetMaximum(0.2);
    }
    else if(target.data == "LER-REF")
    {
        s_stack->SetMaximum(1.0);
    }
    else if(maxbin >= 1 )
    {
        s_stack->SetMaximum(5);
    }

    stext->DrawLatex(0.15, 0.84, "Zoom");

    pad3->Modified();
    can->Update();

//    file->Close();

    return can;
}

void plt_wf()
{
    TCanvas* can;

    // HER Ref
    Target all_non_target = GetTarget("LER-ALL");
    can = make_waveform_canvas(all_non_target);
    // can->SaveAs((all_non_target.pathout+"/"+all_non_target.fileout+"_waveform"+".pdf").c_str());
    can->SaveAs((all_non_target.pathout+"/Waveforms/"+all_non_target.fileout+"_waveform"+".png").c_str());
    can->SaveAs((all_non_target.pathout+"/Waveforms/"+all_non_target.fileout+"_waveform"+".eps").c_str());
    can->SaveAs((all_non_target.pathout+"/Waveforms/"+all_non_target.fileout+"_waveform"+".jpg").c_str());

    // // HER PHASE SHIFT
    // Target all_non_target = GetTarget("ALIGNED_ALL_NON");
    // can = make_waveform_canvas(all_non_target);
    // //    can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_waveform"+".pdf").c_str());
    // can->SaveAs((all_non_target.pathout+"/"+all_non_target.fileout+"_waveform"+".png").c_str());
    // can->SaveAs((all_non_target.pathout+"/"+all_non_target.fileout+"_waveform"+".eps").c_str());
    // can->SaveAs((all_non_target.pathout+"/"+all_non_target.fileout+"_waveform"+".jpg").c_str());


    // // HER PHASE SHIFT
    // Target her_ps_target = GetTarget("HER_PS");
    // TCanvas* can = make_waveform_canvas(her_ps_target);
    // //    can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_waveform"+".pdf").c_str());
    // can->SaveAs((her_ps_target.pathout+"/"+her_ps_target.fileout+"_waveform"+".png").c_str());
    // can->SaveAs((her_ps_target.pathout+"/"+her_ps_target.fileout+"_waveform"+".eps").c_str());
    // can->SaveAs((her_ps_target.pathout+"/"+her_ps_target.fileout+"_waveform"+".jpg").c_str());

    // // LER PHASE SHIFT
    // Target ler_ps_target = GetTarget("LER_PS");
    // TCanvas* can = make_waveform_canvas(ler_ps_target);
    // //    can->SaveAs((target.pathout+"/"+target.fileout+"_"+chs[i]+"_waveform"+".pdf").c_str());
    // can->SaveAs((ler_ps_target.pathout+"/"+ler_ps_target.fileout+"_waveform"+".png").c_str());
    // can->SaveAs((ler_ps_target.pathout+"/"+ler_ps_target.fileout+"_waveform"+".eps").c_str());
    // can->SaveAs((ler_ps_target.pathout+"/"+ler_ps_target.fileout+"_waveform"+".jpg").c_str());

    //return 0;
}
