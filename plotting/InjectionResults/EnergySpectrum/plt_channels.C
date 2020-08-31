
void plt_channels()
{
    // SetPhDStyle();
    //
    // gStyle->SetOptStat(1111111);
    // gStyle->SetOptFit(1111111);
    TGraph* both = new TGraph();
    both->SetMarkerStyle(21);
    TGraph* her = new TGraph();
    her->SetMarkerStyle(23);
    TGraph* ler = new TGraph();
    ler->SetMarkerStyle(24);


    both->SetPoint(0, 0.1, 4471);
    both->SetPoint(1, 0.2, 1757);
    both->SetPoint(2, 0.3, 1382);

    her->SetPoint(0, 0.1, 1670);
    her->SetPoint(1, 0.2, 641);
    her->SetPoint(2, 0.3, 544);

    ler->SetPoint(0, 0.1, 2408);
    ler->SetPoint(1, 0.2, 853);
    ler->SetPoint(2, 0.3, 471);

    TF1* fit1 = new TF1("fit1", "[0]/pow(x+[1],2)",0, 10);
    fit1->SetParameters(179, 0.1);

    both->Fit(fit1, "", "", 0, 10);

    TF1* fit2 = new TF1("fit2", "[0]/pow(x+[1],2)",0, 10);
    fit2->SetParameters(17, 0);

    her->Fit(fit2, "", "", 0, 10);

    TF1* fit3 = new TF1("fit3", "[0]/pow(x+[1],2)",0, 10);
    fit3->SetParameters(54.9, 0.05);

    ler->Fit(fit3, "", "", 0, 10);

    both->GetYaxis()->SetRangeUser(0,5000);
    both->GetXaxis()->SetRangeUser(0,0.4);

    both->Draw("AP");
    her->Draw("P");
    ler->Draw("P");
}
