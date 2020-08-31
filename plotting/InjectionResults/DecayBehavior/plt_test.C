

void plt_test()
{
    Target target = GetTarget("LER-ALL");

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraph* energy = (TGraph*) rfile->Get("FWD1_energy");
    TGraph* energy_bunch = (TGraph*) rfile->Get("FWD1_energy_bunch");
    TGraph* energy_bunch_1 = (TGraph*) rfile->Get("FWD1_energy_bunch_1");
    TGraph* energy_bunch_2 = (TGraph*) rfile->Get("FWD1_energy_bunch_2");

    TGraph* her_current = (TGraph*) rfile->Get("SKB_HER_current");
    TGraph* ler_current = (TGraph*) rfile->Get("SKB_LER_current");

    TGraph* low = new TGraph();
    low->SetTitle("low");
    low->SetMarkerColor(kBlue);

    TGraph* up = new TGraph();
    low->SetTitle("up");
    up->SetMarkerColor(kRed);

    for(int i = 0; i< energy->GetN(); i++)
    {
        double ts, e, eb, eb1, eb2, iher, iler;

        energy->GetPoint(i, ts, e);
        energy_bunch->GetPoint(i, ts, eb);
        energy_bunch_1->GetPoint(i, ts, eb1);
        energy_bunch_2->GetPoint(i, ts, eb2);

        her_current->GetPoint(i, ts, iher);
        ler_current->GetPoint(i, ts, iler);

        if(iler <= 2000)
        {
            low->SetPoint(low->GetN(), iler, eb1/e);
        }
        else if(iler > 500)
        {
            up->SetPoint(low->GetN(), iler, eb1);
        }

    }

//    low->GetXaxis()->SetRangeUser(0, 1200);
//    up->Draw("ap");
    low->Draw("ap");
    //up->Draw("same p");

}
