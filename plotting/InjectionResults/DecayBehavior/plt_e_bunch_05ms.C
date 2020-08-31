



void plt_e_bunch_05ms()
{
    SetPhDStyle();
    string chs[] = {"FWD1", "FWD2", "FWD3"};
    //string chs[] = {"FWD1"};

    for(auto channel : chs)
    {
        // Set some standards
    int rebin = 1;
    double linewidth = 2;
    int linestyle =7;

    int cher =  tab20_blue->GetNumber();
    int cler =  tab20_red->GetNumber();
    int cler2 =  tab20_green->GetNumber();
    // int cf1  = tab20_red->GetNumber();
    // int cf2  = tab20_green->GetNumber();
    // int cline = tab20_gray->GetNumber();
    //
    // double nsig = 5;

    // HER DATA SET
    Target her_target = GetTarget("HER-ALL");

    TFile* her_rfile = new TFile((her_target.pathin+"/"+her_target.filein).c_str(), "open");

    TH1F* her = (TH1F*) her_rfile->Get((channel+"_e_bunch_5mus_1").c_str());

    her->SetDirectory(0);
    her->SetLineColorAlpha( cher, 1.);
    her->SetLineWidth(linewidth);
    her->SetFillStyle(0);
    cout << her->GetXaxis()->GetXmin() << endl;
    cout << her->GetXaxis()->GetXmax() << endl;
        cout << channel << " HER bin0: " << 1e2*her->GetBinContent(0)/her->GetEntries() <<"\%, bin1: " << 1e2*her->GetBinContent(1)/her->Integral() << "\%" << ", bin last: " << 1e2*her->GetBinContent(her->GetNbinsX())/her->Integral() << "\% bin last +1: " << 1e2*her->GetBinContent(her->GetNbinsX()+1)/her->GetEntries() << endl;

        her->Rebin(rebin);

        // Do the axis shit
        her->GetXaxis()->SetRangeUser(-5e-5,1.00005);
        her->GetXaxis()->SetNdivisions(506);
        her->GetXaxis()->SetTitle("E_{ib} (t < 500 #mus)/E_{ib}");
    //    her->GetYaxis()->SetTitle()
    //    her->GetYaxis()->SetRangeUser(0, 600);
        her->GetYaxis()->SetRangeUser(0.8, 9999);
        stringstream ss;
        ss << fixed << setprecision(2) << "Entries [1/" <<  her->GetBinWidth(1) <<  "]";
        her->GetYaxis()->SetTitle(ss.str().c_str());

        TH1F *her_err = (TH1F*) her->Clone("her_err");
        her_err->SetMarkerSize(0);
        her_err->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        her_err->SetFillColorAlpha(cher,1);
        her_err->SetLineWidth(0);

        // LER DATA SET
        Target ler_target = GetTarget("LER-ALL");

        TFile* ler_rfile = new TFile((ler_target.pathin+"/"+ler_target.filein).c_str(), "open");

        TH1F* ler = (TH1F*) ler_rfile->Get((channel+"_e_bunch_5mus_1").c_str());

        ler->SetDirectory(0);
        ler->SetLineColorAlpha( cler, 1.);
        ler->SetLineWidth(linewidth);
        ler->SetFillStyle(0);

        cout << channel << " LER bin0: " << 1e2*ler->GetBinContent(0)/ler->GetEntries() <<"\%, bin1: " << 1e2*ler->GetBinContent(1)/ler->Integral() << "\%" << ", bin last: " << 1e2*ler->GetBinContent(ler->GetNbinsX())/ler->Integral() << "\% bin last +1: " << 1e2*ler->GetBinContent(ler->GetNbinsX()+1)/ler->GetEntries() << endl;
        ler->Rebin(rebin);

        TH1F *ler_err = (TH1F*) ler->Clone("ler_err");
        ler_err->SetMarkerSize(0);
        ler_err->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        ler_err->SetFillColorAlpha(cler,1);
        ler_err->SetLineWidth(0);


        TH1F* ler2 = (TH1F*) ler_rfile->Get((channel+"_e_bunch_5mus").c_str());

        ler2->SetDirectory(0);
        ler2->SetLineColorAlpha( cler2, 1.);
        ler2->SetLineWidth(linewidth);
        ler2->SetFillStyle(0);

        cout << channel << " ler bin1: " << 1e2*ler2->GetBinContent(1)/ler2->Integral() << "\%" << ", bin last: " << 1e2*ler2->GetBinContent(ler2->GetNbinsX())/ler2->Integral() << "\%" << endl;

        ler2->Rebin(rebin);

        TH1F *ler2_err = (TH1F*) ler->Clone("ler2_err");
        ler2_err->SetMarkerSize(0);
        ler2_err->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        ler2_err->SetFillColorAlpha(cler2,1);
        ler2_err->SetLineWidth(0);



        TLegend* leg =  new TLegend(0.195, 0.65, 0.39, 0.86);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);

        // leg->AddEntry(make_box)
        int legfontsize = leg->GetTextSize();
        int legfont     = leg->GetTextFont();

        leg->AddEntry(make_box(her, "EMPTY"), (her_target.data+"_{ib}").c_str(), "f");
        leg->AddEntry(her_err, "Stat. Unc.", "f");
        leg->AddEntry(make_box(ler, "EMPTY"), (ler_target.data+"_{ib}").c_str(), "f");
        leg->AddEntry(ler_err, "Stat. Unc.", "f");
        // leg->AddEntry(make_box(ler2, "EMPTY"), (ler_target.data+"_{ib1 + ib2}").c_str(), "f");
        // leg->AddEntry(ler2_err, "Stat. Unc.", "f");


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

        can->SetLogy();

        her->Draw("AXIS");
        // ler2->Draw("HIST SAME");
        // ler2_err->Draw("same E2");

        ler->Draw("HIST SAME");
        ler_err->Draw("same E2");
        her->Draw("HIST SAME");
        her_err->Draw("same E2");
        ttext->DrawLatex(0.2, 0.88, ("Energy Ratio: "+channel).c_str());
        leg->Draw();


        // can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".pdf").c_str());
        // can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".png").c_str());
        // can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".jpg").c_str());
        // can->SaveAs((target.pathout+"/DecayStudy/"+target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".eps").c_str());


        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".pdf").c_str());
        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".png").c_str());
        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".jpg").c_str());
        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_05mus_per_ebunch_"+channel+".eps").c_str());
    }
}
