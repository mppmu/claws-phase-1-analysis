



void plt_t_below_her(string channel = "FWD1")
{

    double linewidth = 2;
    int linestyle    = 7;

    int cher         =  tab20_blue->GetNumber();
    int cler         =  tab20_red->GetNumber();
    int ccumulative  = tab20_red->GetNumber();
    // int cf1  = tab20_red->GetNumber();
    // int cf2  = tab20_green->GetNumber();
    // int cline = tab20_gray->GetNumber();
    //
    // double nsig = 5;

    // HER DATA SET
    Target target = GetTarget("HER-ALL");
    TFile* her_rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");
    TH1F* her = (TH1F*) her_rfile->Get((channel+"_t_energy_bunch_belowX_1").c_str());

    her->SetDirectory(0);
    her->SetLineColorAlpha( cher, 1.);
    her->SetLineWidth(linewidth);
    her->SetFillStyle(0);

    cout << her->GetXaxis()->GetXmin() << endl;
    cout << her->GetXaxis()->GetXmax() << endl;
    //cout << channel << " HER bin1: " << 1e2*her->GetBinContent(1)/her->Integral() << "\%" << ", bin last: " << 1e2*her->GetBinContent(her->GetNbinsX())/her->Integral() << "\%" << endl;
    cout << channel << " HER binwidth: " << her->GetBinWidth(1)*1e6 << " mus" << endl;
    double dt = 0.8e-3; // in mus
    double scaling = 1e3;
    double xlow, xup;

    xlow = her->GetXaxis()->GetXmin();
    xup = her->GetXaxis()->GetXmax();

    her->SetBins(her->GetNbinsX(), xlow*scaling, xup*scaling );
    // int rebin = 10;
    // her->Rebin(rebin);

    // cout << channel << " Entries in bin 0+1: " << her->GetBinContent(0)+her->GetBinContent(1)<< " percentage: " << (her->GetBinContent(0)+her->GetBinContent(1))/ (her->GetBinContent(0)+her->Integral(1, her->GetNbinsX())) << endl;
    cout << channel << " HER bin0: " << 1e2*her->GetBinContent(0)/her->GetEntries() <<"\%, bin1: " << 1e2*her->GetBinContent(1)/her->Integral() << "\%" << ", bin last: " << 1e2*her->GetBinContent(her->GetNbinsX())/her->Integral() << "\% bin last +1: " << 1e2*her->GetBinContent(her->GetNbinsX()+1)/her->GetEntries() << endl;
    // her->GetSetContent(1, 0);

    // DO THE CUMULATIVE:



    // cout << "her->GetXaxis()->GetXmin(): " << her->GetXaxis()->GetXmin() << endl;
    // cout << "her->GetXaxis()->GetXmax(): " << her->GetXaxis()->GetXmax() << endl;
    // cout << "her->GetBinWidth(1): " << her->GetBinWidth(1) << endl;
    // cout << "her->GetBinContent(0): " << her->GetBinContent(0)<< endl;
    // cout << "her->GetBinContent(1): " << her->GetBinContent(1)<< endl;
    // cout << "her->GetBinContent(2): " << her->GetBinContent(2)<< endl;
    // cout << "her->GetBinContent(3): " << her->GetBinContent(3)<< endl;
    //
    // cout << "Bin center: " << her->GetBinCenter(1)<< endl;
    //
    // cout << "her->GetBinContent(0): " << her->GetBinContent(0)<< endl;
    // cout << "cumulative->GetBinContent(0): " << cumulative->GetBinContent(0)<< endl;
    // cout << "cumulative->GetBinContent(1): " << cumulative->GetBinContent(1)<< endl;
    // cout << "cumulative->GetBinContent(2): " << cumulative->GetBinContent(2)<< endl;
    // cout << "cumulative->GetBinContent(3): " << cumulative->GetBinContent(3)<< endl;

    // Do the axis shit
    //TH1F* axis = new TH1F("axis", "axis",30001, -0.60005, 2.40005);

    //cout  << "her->Integral(): " << her->Integral(1, her->GetNbinsX()) <<  endl;
    her->GetXaxis()->SetRangeUser(0.5e-3,2.4);

    //cout  << "her->Integral(): " << her->Integral(1, her->GetNbinsX()) <<  endl;
    int ndiv = 506;
    her->GetXaxis()->SetNdivisions(ndiv);
    her->GetYaxis()->SetNdivisions(ndiv);

    her->GetXaxis()->SetTitle("Time [ms]");
    //    her->GetYaxis()->SetTitle()
    // her->GetYaxis()->SetRangeUser(0, 2500);
    her->GetYaxis()->SetRangeUser(0, 400);

    stringstream ss;
    ss << fixed << setprecision(0) << "Entries [1/" <<  her->GetBinWidth(1)*1e6 <<  " ns]";
    her->GetYaxis()->SetTitle(ss.str().c_str());
    her->GetYaxis()->SetTitleOffset(1.5);


    TMarker* single_marker = new TMarker(0, 293, 22);
    single_marker->SetMarkerSize(2.0);
    single_marker->SetMarkerColor(kBlack);


    // LEGEND
    TLegend* leg =  new TLegend(0.175, 0.78, 0.34, 0.87);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);

    // leg->AddEntry(make_box)
    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    leg->AddEntry(make_box(her, "EMPTY"), (target.data+"_{ib}").c_str(), "f");

    auto tmp = (TH1F*)her->Clone("tmp");
    tmp->SetLineColor(ccumulative);
    leg->AddEntry(make_box(tmp, "EMPTY"), "Cumulative Distribution Function", "f");
    //leg->AddEntry(make_box(ler, "EMPTY"), ler_target.data.c_str(), "f");

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
        can->SetTicky(0);
        can->SetRightMargin(0.09);
        can->SetLeftMargin(0.1);

    //    can->SetRightMargin(0.2);
    //    can->SetLogy();
        // axis->Draw();
        her->Draw();

        ttext->DrawLatex(0.16, 0.88, ("Time of last Hit (E_{ib} > 2 MIP): "+channel).c_str());
        leg->Draw();
        // single_marker->Draw();

        can->Update();


        // DO THE SECOND AXIS:
        // create hint1 filled with the bins integral of h1
        auto cumulative = (TH1F*)her->Clone("cumulative");

        double sum = her->GetBinContent(0);

        for (int i=1; i<=her->GetNbinsX(); i++)
        {
           sum += her->GetBinContent(i);
           cumulative->SetBinContent(i,sum);
        }
        cumulative->Scale(1./cumulative->GetBinContent(cumulative->GetNbinsX()));

        // cumulative->GetXaxis()->SetRangeUser(0.5e-3,2.4);
        float rightmax = 1.2*cumulative->GetMaximum();
        float scale = gPad->GetUymax()/rightmax;
        cumulative->Scale(scale);


        cumulative->SetLineColorAlpha(ccumulative, 1.);

        // her->SetLineColorAlpha( cher, 1.);
        cumulative->SetLineWidth(linewidth);

        cumulative->Draw("hist same");
        //cumulative->Draw();

       // draw an axis on the right side
       auto axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(), gPad->GetUxmax(), gPad->GetUymax(),0,rightmax,ndiv,"+L");
       axis->SetLineColor(ccumulative);
       axis->SetTextColor(ccumulative);
       axis->SetLabelColor(ccumulative);
       axis->SetTitle("Normalized Cumulative Entries [1/100 ns]");
       axis->SetTitleOffset(1.4);
       // axis->SetNdivisions(ndiv);

       double fontsize = her->GetYaxis()->GetTitleSize();
       int font     = her->GetYaxis()->GetTitleFont();

       axis->SetTitleFont(font);
       axis->SetLabelFont(font);
       axis->SetTitleSize(fontsize);
       axis->SetLabelSize(fontsize);
              //gStyle->SetPadTickY(1);
       axis->SetTickLength( her->GetXaxis()->GetTickLength()/ratio );
       axis->Draw();

       axis->Draw();

       can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_t_below_"+channel+".pdf").c_str());
       can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_t_below_"+channel+".png").c_str());
       can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_t_below_"+channel+".jpg").c_str());
       can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_t_below_"+channel+".eps").c_str());
}

void plt_t_below_ler(string channel = "FWD1")
{
    // SetPhDStyle();
    //string chs[] = {"FWD1", "FWD2", "FWD3"};
        // Set some standards

    double linewidth = 2;
    int linestyle =7;

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

    TH1F* ler = (TH1F*) ler_rfile->Get((channel+"_t_energy_bunch_belowX_1").c_str());

    ler->SetDirectory(0);
    ler->SetLineColorAlpha( cler, 1.);
    ler->SetLineWidth(linewidth);
    ler->SetFillStyle(0);
    cout << ler->GetXaxis()->GetXmin() << endl;
    cout << ler->GetXaxis()->GetXmax() << endl;
    //cout << channel << " ler bin1: " << 1e2*ler->GetBinContent(1)/ler->Integral() << "\%" << ", bin last: " << 1e2*ler->GetBinContent(ler->GetNbinsX())/ler->Integral() << "\%" << endl;
    cout << channel << " ler binwidth: " << ler->GetBinWidth(1)*1e6 << " mus" << endl;
    double dt = 0.8e-3; // in mus
    double scaling = 1e3;
    double xlow, xup;

    xlow = ler->GetXaxis()->GetXmin();
    xup = ler->GetXaxis()->GetXmax();

    ler->SetBins(ler->GetNbinsX(), xlow*scaling, xup*scaling );
    // int rebin = 10;
    // ler->Rebin(rebin);

        // Do the axis shit
        // ler->GetXaxis()->SetRangeUser(-5e-5,1.00005);
        ler->GetXaxis()->SetNdivisions(506);
        ler->GetXaxis()->SetTitle("Time [ms]");
    //    ler->GetYaxis()->SetTitle()
        if(channel == "FWD1")
        {
            ler->GetYaxis()->SetRangeUser(0, 750);
        }
        else
        {
            ler->GetYaxis()->SetRangeUser(0, 1050);
        }

    //    ler->GetYaxis()->SetRangeUser(1, 5000);
        stringstream ss;
        ss << fixed << setprecision(0) << "Entries [1/" <<  ler->GetBinWidth(1)*1e6 <<  " ns]";
        ler->GetYaxis()->SetTitle(ss.str().c_str());


        // LEGEND
        TLegend* leg =  new TLegend(0.67, 0.795, 0.865, 0.85);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);

        // leg->AddEntry(make_box)
        int legfontsize = leg->GetTextSize();
        int legfont     = leg->GetTextFont();

        leg->AddEntry(make_box(ler, "EMPTY"), (target.data+"_{ib1}").c_str(), "f");
        //leg->AddEntry(make_box(ler, "EMPTY"), ler_target.data.c_str(), "f");

        TLatex* ttext = new TLatex();
        ttext->SetNDC();
        ttext->SetTextFont(legfont+20);
        ttext->SetTextSize(gStyle->GetLegendTextSize());

        TMarker* single_marker;


        if(channel == "FWD1")
        {
            single_marker = new TMarker(0, 740, 22);
        }
        else
        {
            single_marker = new TMarker(0, 1000, 22);
        }

        single_marker->SetMarkerSize(2.0);
        single_marker->SetMarkerColor(kBlack);
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
    //    can->SetRightMargin(0.2);
    //    can->SetLogy();

        ler->Draw();
        // ler->Draw("HIST SAME");
        // ler_err->Draw("same E2");
        // ler->Draw("HIST SAME");
        // ler_err->Draw("same E2");
        ttext->DrawLatex(0.21, 0.88, ("Time of last Hit (E_{ib} > 2 MIP): "+channel).c_str());
        leg->Draw();
        single_marker->Draw();

        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_t_below.pdf").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_t_below.png").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_t_below.jpg").c_str());
        can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ler_t_below.eps").c_str());
}

void plt_t_below()
{
    SetPhDStyle();

    string chs[] = {"FWD1", "FWD2", "FWD3"};
    // string chs[] = {"FWD1"};
    for(auto channel : chs)
    {
        plt_t_below_her(channel);
        //plt_t_below_ler(channel);
    }
}
