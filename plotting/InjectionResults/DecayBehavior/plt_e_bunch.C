



void plt_e_bunch()
{
    SetPhDStyle();
    string chs[] = {"FWD1", "FWD2", "FWD3"};
    //string chs[] = {"FWD1"};
    for(auto channel : chs)
    {
            // Set some standards
        int rebin        = 1;
        double linewidth =   2;
        int linestyle    =   7;

        int cher   =  tab20_blue->GetNumber();
        int cler   =  tab20_red->GetNumber();
        int cler_1 =  tab20_green->GetNumber();
        int cler_2 =  tab20_orange->GetNumber();
        int cler_1_only =  tab20_red->GetNumber();
        int cler_2_only =  tab20_cyan->GetNumber();
        // int cf1  = tab20_red->GetNumber();
        // int cf2  = tab20_green->GetNumber();
        // int cline = tab20_gray->GetNumber();
        //
        // double nsig = 5;

        // HER DATA SET
        Target her_target = GetTarget("HER-ALL");

        TFile* her_rfile = new TFile((her_target.pathin+"/"+her_target.filein).c_str(), "open");

        TH1F* her = (TH1F*) her_rfile->Get((channel+"_e_bunch_1").c_str());

        her->SetDirectory(0);
        her->SetLineColorAlpha( cher, 1.);
        her->SetLineWidth(linewidth);
        her->SetFillStyle(0);
        cout << her->GetXaxis()->GetXmin() << endl;
        cout << her->GetXaxis()->GetXmax() << endl;
        cout << her->GetNbinsX() << endl;
        cout << her->Integral()<< " " << her->GetEntries() << endl;
        cout << channel << " HER bin0: " << 1e2*her->GetBinContent(0)/her->GetEntries() <<"\%, bin1: " << 1e2*her->GetBinContent(1)/her->Integral() << "\%" << ", bin last: " << 1e2*her->GetBinContent(her->GetNbinsX())/her->Integral() << "\% bin last +1: " << 1e2*her->GetBinContent(her->GetNbinsX()+1)/her->GetEntries() << endl;


        // her->Rebin(rebin);

        // Do the axis shit
        // her->GetXaxis()->SetRangeUser(-5e-5,1.00005);
        her->GetXaxis()->SetNdivisions(506);
        her->GetXaxis()->SetTitle("E_{ib} /E");
    //    her->GetYaxis()->SetTitle()
        if(channel == "FWD1"){
                        her->GetYaxis()->SetRangeUser(0, 850);
        }
        else if(channel == "FWD2")
        {
            her->GetYaxis()->SetRangeUser(0, 1250);

        }
        else
        {
            her->GetYaxis()->SetRangeUser(0, 1350);

        }
    //    her->GetYaxis()->SetRangeUser(1, 5000);
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

        // DO BOTH LER BUNCHES
        TH1F* ler = (TH1F*) ler_rfile->Get((channel+"_e_bunch").c_str());

        ler->SetDirectory(0);
        ler->SetLineColorAlpha( cler, 1.);
        ler->SetLineWidth(linewidth);
        ler->SetFillStyle(0);

        cout << channel << " ler bin0: " << 1e2*ler->GetBinContent(0)/ler->GetEntries() <<"\%, bin1: " << 1e2*ler->GetBinContent(1)/ler->Integral() << "\%" << ", bin last: " << 1e2*ler->GetBinContent(ler->GetNbinsX())/ler->Integral() << "\% bin last +1: " << 1e2*ler->GetBinContent(ler->GetNbinsX()+1)/ler->GetEntries() << endl;

        ler->Rebin(rebin);

        TH1F *ler_err = (TH1F*) ler->Clone("ler_err");
        ler_err->SetMarkerSize(0);
        ler_err->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        ler_err->SetFillColorAlpha(cler,1);
        ler_err->SetLineWidth(0);

        // DO FIRST LER BUNCH
        TH1F* ler_1 = (TH1F*) ler_rfile->Get((channel+"_e_bunch_1").c_str());

        ler_1->SetDirectory(0);
        ler_1->SetLineColorAlpha( cler_1, 1.);
        ler_1->SetLineWidth(linewidth);
        ler_1->SetFillStyle(0);

        cout << channel << " ler_1 bin0: " << 1e2*ler_1->GetBinContent(0)/ler_1->GetEntries() <<"\%, bin1: " << 1e2*ler_1->GetBinContent(1)/ler_1->Integral() << "\%" << ", bin last: " << 1e2*ler_1->GetBinContent(ler_1->GetNbinsX())/ler_1->Integral() << "\% bin last +1: " << 1e2*ler_1->GetBinContent(ler_1->GetNbinsX()+1)/ler_1->GetEntries() << endl;

        // ler_1->Rebin(rebin);

        TH1F *ler_1_err = (TH1F*) ler->Clone("ler_err");
        ler_1_err->SetMarkerSize(0);
        ler_1_err->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        ler_1_err->SetFillColorAlpha(cler_1,1);
        ler_1_err->SetLineWidth(0);

        // DO SECOND LER BUNCH
        TH1F* ler_2 = (TH1F*) ler_rfile->Get((channel+"_e_bunch_2").c_str());

        ler_2->SetDirectory(0);
        ler_2->SetLineColorAlpha( cler_2, 1.);
        ler_2->SetLineWidth(linewidth);
        ler_2->SetFillStyle(0);

        cout << channel << " ler_2 bin0: " << 1e2*ler_2->GetBinContent(0)/ler_2->GetEntries() <<"\%, bin1: " << 1e2*ler_2->GetBinContent(1)/ler_2->Integral() << "\%" << ", bin last: " << 1e2*ler_2->GetBinContent(ler_2->GetNbinsX())/ler_2->Integral() << "\% bin last +1: " << 1e2*ler_2->GetBinContent(ler_2->GetNbinsX()+1)/ler_2->GetEntries() << endl;

        // ler_2->Rebin(rebin);

        TH1F *ler_2_err = (TH1F*) ler_2->Clone("ler_2_err");
        ler_2_err->SetMarkerSize(0);
        ler_2_err->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        ler_2_err->SetFillColorAlpha(cler_2,1);
        ler_2_err->SetLineWidth(0);


        // // DO CLEAN FIRST LER BUNCH
        TH1F* ler_1_only = (TH1F*) ler_rfile->Get((channel+"_e_bunch_1_only").c_str());

        ler_1_only->SetDirectory(0);
        ler_1_only->SetLineColorAlpha( cler_1_only, 1.);
        ler_1_only->SetLineWidth(linewidth);
        ler_1_only->SetFillStyle(0);

        cout << channel << " ler_1_only bin0: " << 1e2*ler_1_only->GetBinContent(0)/ler_1_only->GetEntries() <<"\%, bin1: " << 1e2*ler_1_only->GetBinContent(1)/ler_1_only->Integral() << "\%" << ", bin last: " << 1e2*ler_1_only->GetBinContent(ler_1_only->GetNbinsX())/ler_1_only->Integral() << "\% bin last +1: " << 1e2*ler_1_only->GetBinContent(ler_1_only->GetNbinsX()+1)/ler_1_only->GetEntries() << endl;

        // ler_1_only->Rebin(rebin);


        TH1F *ler_err_1_only = (TH1F*) ler_1_only->Clone("ler_err");
        ler_err_1_only->SetMarkerSize(0);
        ler_err_1_only->SetFillStyle(3353);
        //err->SetFillStyle(1001);
        //err->SetFillColorAlpha(cerr,1);
        ler_err_1_only->SetFillColorAlpha(cler_1_only,1);
        ler_err_1_only->SetLineWidth(0);

        // // DO CLEAN FIRST LER BUNCH
        TH1F* ler_2_only = (TH1F*) ler_rfile->Get((channel+"_e_bunch_2_only").c_str());

        ler_2_only->SetDirectory(0);
        ler_2_only->SetLineColorAlpha( cler_2_only, 1.);
        ler_2_only->SetLineWidth(linewidth);
        ler_2_only->SetFillStyle(0);

        cout << channel << " ler_2_only bin0: " << 1e2*ler_2_only->GetBinContent(0)/ler_2_only->GetEntries() <<"\%, bin1: " << 1e2*ler_2_only->GetBinContent(1)/ler_2_only->Integral() << "\%" << ", bin last: " << 1e2*ler_2_only->GetBinContent(ler_2_only->GetNbinsX())/ler_2_only->Integral() << "\% bin last +1: " << 1e2*ler_2_only->GetBinContent(ler_2_only->GetNbinsX()+1)/ler_2_only->GetEntries() << endl;

        // ler_2_only->Rebin(rebin);

        // cler_3->SetDirectory(0);
        // cler_3->SetLineColorAlpha( cler_3, 1.);
        // cler_3->SetLineWidth(linewidth);
        // cler_3->SetFillStyle(0);

        // LEGEND
        TLegend* leg =  new TLegend(0.195, 0.65, 0.39, 0.86);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);

        // leg->AddEntry(make_box)
        int legfontsize = leg->GetTextSize();
        int legfont     = leg->GetTextFont();

        leg->AddEntry(make_box(her, "EMPTY"), (her_target.data+"_{ib}").c_str(), "f");
        leg->AddEntry(her_err, "Stat. Unc.", "f");
        // leg->AddEntry(make_box(ler, "EMPTY"), (ler_target.data+"_{ib1 + ib2}").c_str(), "f");
        // leg->AddEntry(make_box(ler_1, "EMPTY"), (ler_target.data+"_{ib1}").c_str(), "f");
        // // leg->AddEntry(ler_err, "Stat. Unc.", "f");
        // leg->AddEntry(make_box(ler_2, "EMPTY"), (ler_target.data+"_{ib2}").c_str(), "f");
        // leg->AddEntry(ler_2_err, "Stat. Unc.", "f");
        leg->AddEntry(make_box(ler_1_only, "EMPTY"), (ler_target.data+"_{ib}").c_str(), "f");
        leg->AddEntry(ler_err_1_only, "Stat. Unc.", "f");
        // leg->AddEntry(make_box(ler_2_only, "EMPTY"), (ler_target.data+"_2_only").c_str(), "f");

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

    //    can->SetLogy();

        her->Draw("AXIS");
        her->Draw("HIST SAME");
        her_err->Draw("same E2");

        // ler_2_err->Draw("same E2");
        // ler->Draw("HIST SAME");
        // ler_1->Draw("HIST SAME");
        // ler_2->Draw("HIST SAME");
        // ler_err->Draw("same E2");

        ler_1_only->Draw("HIST SAME");
        ler_err_1_only->Draw("same E2");
        // ler_2_only->Draw("HIST SAME");

        ttext->DrawLatex(0.2, 0.88, ("Energy Ratio: "+channel).c_str());
        leg->Draw();

        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_per_etotal_"+channel+".pdf").c_str());
        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_per_etotal_"+channel+".png").c_str());
        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_per_etotal_"+channel+".jpg").c_str());
        can->SaveAs((her_target.pathout+"/DecayStudy/"+her_target.fileout+"_ebunch_per_etotal_"+channel+".eps").c_str());

        // can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ebunch_per_etotal.pdf").c_str());
        // can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ebunch_per_etotal.png").c_str());
        // can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ebunch_per_etotal.jpg").c_str());
        // can->SaveAs(("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DecayStudy/"+channel+"_ebunch_per_etotal.eps").c_str());
    }
}
