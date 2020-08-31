



void energy_vs_current_her(string channel = "FWD1")
{
    // SetPhDStyle();
    //string chs[] = {"FWD1", "FWD2", "FWD3"};
        // Set some standards


        double markersize = 0.8;
        double markeralpha = 0.4;
        double linealpha = 0.8;
        double linewidth = 1;


    int shape1 = 22;
    int shape2 = 20;
    int shape3 = 21;

    int shape4 = 26;
    int shape5 = 4;
    int shape6 = 25;

    int cher =  tab20_blue->GetNumber();
    int cler =  tab20_red->GetNumber();
    // int cf1  = tab20_red->GetNumber();
    // int cf2  = tab20_green->GetNumber();
    // int cline = tab20_gray->GetNumber();
    //
    // double nsig = 5;

    // HER DATA SET
    Target target = GetTarget("HER-ALL");

    TFile* her_rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraph* her = (TGraph*) her_rfile->Get((channel+"_energyb_vs_current_her").c_str());


    her->SetLineColorAlpha( cher, linealpha);
    her->SetLineWidth(linewidth);
    her->SetMarkerStyle(shape2);
    her->SetMarkerSize(markersize);
    her->SetMarkerColorAlpha( cher, markeralpha);

    TGraph* her2 = (TGraph*) her->Clone("her2");
    her2->SetMarkerStyle(shape5);
    her2->SetMarkerColorAlpha( cher, 1.0);
    // int rebin = 10;
    // her->Rebin(rebin);

        // Do the axis shit
        // her->GetXaxis()->SetMinimum(0);
        // her->GetXaxis()->SetMaximum(1050);
    TH1F* axis = new TH1F("axis","axis",1100, -1, 1050);
        // her->GetYaxis()->SetLimits(0, 1050);
        axis->GetXaxis()->SetNdivisions(506);
        axis->GetXaxis()->SetTitle("Beam Current [mA]");
        axis->GetYaxis()->SetTitle("E_{1st bunch} [MIP/2.4 ms]");
        axis->GetYaxis()->SetRangeUser(0, 400);
        axis->GetXaxis()->SetRangeUser(0, 1010);
    //    her->GetYaxis()->SetRangeUser(1, 5000);
        stringstream ss;
        // ss << fixed << setprecision(0) << "Entries [1/" <<  her->GetBinWidth(1)*1e6 <<  " ns]";
        // her->GetYaxis()->SetTitle(ss.str().c_str());

        // LEGEND
        TLegend* leg =  new TLegend(0.705, 0.81, 0.9, 0.85);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);

        TLegend* leg2 =  new TLegend(0.705, 0.81, 0.9, 0.85);
        leg2->SetBorderSize(0);
        leg2->SetFillColor(0);
        leg2->SetFillStyle(0);

        // leg->AddEntry(make_box)
        int legfontsize = leg->GetTextSize();
        int legfont     = leg->GetTextFont();

        TGraph* her_marker = (TGraph*) her->Clone("her_marker");
        her_marker->SetMarkerSize(1.6);
        TGraph* her_marker2 = (TGraph*) her2->Clone("her_marker2");
        her_marker2->SetMarkerSize(1.6);

        leg->AddEntry(her_marker, target.data.c_str(), "p");
        leg2->AddEntry(her_marker2, target.data.c_str(), "p");
        // //leg->AddEntry(make_box(her, "EMPTY"), her_target.data.c_str(), "f");
        //
        TLatex* ttext = new TLatex();
        ttext->SetNDC();
        ttext->SetTextFont(legfont+20);
        ttext->SetTextSize(gStyle->GetLegendTextSize());

        // Create the canvas
        int width = round(vis_multi*textwidth*pt2pixel*1./2.);
        //int width = round(vis_multi*.25**pt2pixel*1/2);
        double ratio = 3./3.;

        string cantitle = channel+ "_ebunch_per_etotal";
        TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
        can->SetFillColor(kWhite);
        can->SetBorderMode(0);
        can->SetBorderSize(2);
        can->SetFrameBorderMode(0);
        can->SetRightMargin(0.04);
    //    can->SetLogy();

        axis->Draw("AXIS");
        her->Draw("P");
        her2->Draw("P");
        ttext->DrawLatex(0.72, 0.88, "FWD1");
        leg->Draw();
        leg2->Draw();

        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_her_energy_vs_current.pdf").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_her_energy_vs_current.png").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_her_energy_vs_current.jpg").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_her_energy_vs_current.eps").c_str());
}

void energy_vs_current_ler(string channel = "FWD1")
{
    // SetPhDStyle();
    //string chs[] = {"FWD1", "FWD2", "FWD3"};
        // Set some standards


    double markersize = 0.8;
    double markeralpha = 0.4;
    double linealpha = 0.8;
    double linewidth = 1;

    int shape1 = 22;
    int shape2 = 20;
    int shape3 = 21;

    int shape4 = 26;
    int shape5 = 4;
    int shape6 = 25;

    int cher =  tab20_blue->GetNumber();
    int cler =  tab20_red->GetNumber();
    // int cf1  = tab20_red->GetNumber();
    // int cf2  = tab20_green->GetNumber();
    // int cline = tab20_gray->GetNumber();
    //
    // double nsig = 5;

    // HER DATA SET
    Target target = GetTarget("LER-ALL");

    TFile* ler_rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraph* ler = (TGraph*) ler_rfile->Get((channel+"_energyb_vs_current_ler_1").c_str());


    ler->SetLineColorAlpha( cler, linealpha);
    ler->SetLineWidth(linewidth);
    ler->SetMarkerStyle(shape2);
    ler->SetMarkerSize(markersize);
    ler->SetMarkerColorAlpha( cler, markeralpha);

    TGraph* ler2 = (TGraph*) ler->Clone("ler2");
    ler2->SetMarkerStyle(shape5);
    ler2->SetMarkerColorAlpha( cler, 1.0);
    // int rebin = 10;
    // ler->Rebin(rebin);

        // Do the axis shit
        ler->GetXaxis()->SetRangeUser(0,1010);
        ler->GetXaxis()->SetNdivisions(506);
        ler->GetXaxis()->SetTitle("Beam Current [mA]");
        ler->GetYaxis()->SetTitle("E_{1st bunch} [MIP/2.4 ms]");
        ler->GetYaxis()->SetRangeUser(0, 400);
    //    ler->GetYaxis()->SetRangeUser(1, 5000);
        stringstream ss;
        // ss << fixed << setprecision(0) << "Entries [1/" <<  ler->GetBinWidth(1)*1e6 <<  " ns]";
        // ler->GetYaxis()->SetTitle(ss.str().c_str());


        // LEGEND
        TLegend* leg =  new TLegend(0.705, 0.81, 0.9, 0.85);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);

        TLegend* leg2 =  new TLegend(0.705, 0.81, 0.9, 0.85);
        leg2->SetBorderSize(0);
        leg2->SetFillColor(0);
        leg2->SetFillStyle(0);

        // leg->AddEntry(make_box)
        int legfontsize = leg->GetTextSize();
        int legfont     = leg->GetTextFont();

        TGraph* ler_marker = (TGraph*) ler->Clone("ler_marker");
        ler_marker->SetMarkerSize(1.6);
        TGraph* ler_marker2 = (TGraph*) ler2->Clone("ler_marker2");
        ler_marker2->SetMarkerSize(1.6);

        leg->AddEntry(ler_marker, target.data.c_str(), "p");
        leg2->AddEntry(ler_marker2, target.data.c_str(), "p");
        // //leg->AddEntry(make_box(ler, "EMPTY"), ler_target.data.c_str(), "f");
        //
        TLatex* ttext = new TLatex();
        ttext->SetNDC();
        ttext->SetTextFont(legfont+20);
        ttext->SetTextSize(gStyle->GetLegendTextSize());


        // Create the canvas
        int width = round(vis_multi*textwidth*pt2pixel*1./2.);
        //int width = round(vis_multi*.25**pt2pixel*1/2);
        double ratio = 3./3.;

        string cantitle = channel+ "_ebunch_per_etotal_ler";
        TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
        can->SetFillColor(kWhite);
        can->SetBorderMode(0);
        can->SetBorderSize(2);
        can->SetFrameBorderMode(0);
        can->SetRightMargin(0.04);
    //    can->SetRightMargin(0.2);
    //    can->SetLogy();

        ler->Draw("AP");
        ler2->Draw("P");
        // ler->Draw("HIST SAME");
        // ler_err->Draw("same E2");
        // her->Draw("HIST SAME");
        // her_err->Draw("same E2");
        ttext->DrawLatex(0.72, 0.88, "FWD1");
        leg->Draw();
        leg2->Draw();
        // single_marker->Draw();

        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_energy_vs_current.pdf").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_energy_vs_current.png").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_energy_vs_current.jpg").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_energy_vs_current.eps").c_str());
}

void plt_energy_vs_current()
{
    SetPhDStyle();
    //string chs[] = {"FWD1", "FWD2", "FWD3"};
    string chs[] = {"FWD1"};
    for(auto channel : chs)
    {
        energy_vs_current_her(channel);
        energy_vs_current_ler(channel);
    }
}
