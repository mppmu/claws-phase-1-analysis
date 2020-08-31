


void plt_datasets()
{
//    string datasets[] = {"NJ-ALL", "NJ-HER", "NJ-LER", "NJ-VACS", "", "HER-ALL", "LER-ALL", "LER-VACS", "", "HER-REF" , "HER-PS" , "HER-VS1", "HER-VS2", "", "LER-REF", "LER-PS" , "LER-VS" , "LER-SA"};
    //string datasets[] = {"HER-ALL"};

    ofstream txtfile;
    txtfile.open ("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/DataSets/datasets_overview.txt");

    for(auto dataset : GetDataSets())
    {
        if(dataset == "")
        {
            stringstream ss;
            ss << "\n" <<"\\" << "\\";
            cout << ss.str()<< endl;
            txtfile << ss.str();
        }
        else
        {
        Target target = GetTarget(dataset);

        TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");
        TGraph* her_current = (TGraph*) rfile->Get("SKB_HER_current");
        TGraph* ler_current = (TGraph*) rfile->Get("SKB_LER_current");

        stringstream ss;
        ss << "\n" << dataset << fixed << setprecision(0) << " & " << target.ring << " & " << TMath::MinElement(her_current->GetN(),her_current->GetY()) << " - " << TMath::MaxElement(her_current->GetN(),her_current->GetY());
        ss << " & " << TMath::MinElement(ler_current->GetN(),ler_current->GetY()) << " - " << TMath::MaxElement(ler_current->GetN(),ler_current->GetY());

        ss << " & - ";

        TGraph* vs1;
        TGraph* vs2;
        TGraph* inj_charge;
        TGraph* inj_efficiency;
        if(target.ring == "HER")
        {
            vs1 = (TGraph*) rfile->Get("SKB_HER_BTePS_VM27E_KRB_vs_her_1");
            vs2 = (TGraph*) rfile->Get("SKB_HER_BTePS_VM28E_KRB_vs_her_2");
            inj_charge= (TGraph*) rfile->Get("SKB_HER_injectionCharge");
            inj_efficiency= (TGraph*) rfile->Get("SKB_HER_injectionEfficiency");
        }
        else
        {
            vs1 = (TGraph*) rfile->Get("SKB_LER_BTpPS_VM32P_KRB_vs_ler_1");
            vs2 = (TGraph*) rfile->Get("SKB_LER_BTpPS_VM33P_KRB_vs_ler_1");
            inj_charge= (TGraph*) rfile->Get("SKB_LER_injectionCharge");
            inj_efficiency= (TGraph*) rfile->Get("SKB_LER_injectionEfficiency");
        }

        ss << fixed << setprecision(0) <<  " & " << TMath::MinElement(vs1->GetN(),vs1->GetY())*1e6 << " - " << TMath::MaxElement(vs1->GetN(),vs1->GetY())*1e6;
        ss << " & " << TMath::MinElement(vs2->GetN(),vs2->GetY())*1e6 << " - " << TMath::MaxElement(vs2->GetN(),vs2->GetY())*1e6;
        ss << " & - ";

        // TGraph* inj_charge= (TGraph*) rfile->Get(("SKB_"+target.ring + "_injectionCharge").c_str());
        ss << fixed << setprecision(0) << " & " << inj_charge->GetMean(2)*1e3 << " $\\pm$ " <<inj_charge->GetRMS(2)*1e3;

        // TGraph* inj_efficiency= (TGraph*) rfile->Get(("SKB_"+target.ring + "_injectionEfficiency").c_str());
        ss << fixed << setprecision(0) << " & " << inj_efficiency->GetMean(2) << " $\\pm$ " <<inj_efficiency->GetRMS(2);
        ss << " & " << her_current->GetN();
        ss <<"\\" << "\\";
        cout << ss.str()<< endl;
        txtfile << ss.str();

        rfile->Close();
        }
    }

    txtfile.close();
}
