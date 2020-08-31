
string quantiles(TH1F* h, int precision, double scale = 1) {
   // demo for quantiles
   const Int_t nq = 3;


   Double_t xq[nq];  // position where to compute the quantiles in [0,1]
   Double_t yq[nq];  // array to contain the quantiles

   xq[0] = 0.16;
   xq[1] = 0.5;
   xq[2] = 0.84;

   h->GetQuantiles(nq,yq,xq);

   stringstream res;
   res << fixed << setprecision(precision) << "$"<< abs(yq[1]*scale)  << "_{-" << abs(yq[0]-yq[1])*scale << "}^{+" << abs(yq[2]-yq[1])*scale << "}" << "$" ;

   return res.str();

}

string quantiles(TGraph* g, int precision , double scale = 1) {
   // demo for quantiles
   double min = 0;
   double max = 0;

   for(int i = 0; i < g->GetN(); ++i){
       double x,y;
       g->GetPoint(i,x,y);

       if(min>y) min = y;
       if(max<y) max = y;
   }

   TH1F *h = new TH1F("tmp","demo quantiles",10000,0.5*min,1.5*max);

   for(int i = 0; i < g->GetN(); ++i){
       double x,y;
       g->GetPoint(i,x,y);

       h->Fill(y);
   }

   const Int_t nq = 3;

   Double_t xq[nq];  // position where to compute the quantiles in [0,1]
   Double_t yq[nq];  // array to contain the quantiles

   xq[0] = 0.16;
   xq[1] = 0.5;
   xq[2] = 0.84;

   h->GetQuantiles(nq,yq,xq);

   // yq[1] = h->GetMean();

   stringstream res;
   res << fixed << setprecision(precision) << "$" << abs(yq[1]*scale) << "_{-" << abs(yq[1]-yq[0])*scale << "}^{+" << abs(yq[2]-yq[1])*scale << "}"<< "$";

   delete h;

   return res.str();

}

void plt_decay_study()
{
//    //string datasets[] = {"NJ-ALL", "NJ-HER", "NJ-LER", "NJ-VACS", "", "HER-ALL", "LER-ALL", "LER-VACS", "", "HER-REF" , "HER-PS" , "HER-VS1", "HER-VS2", "", "LER-REF", "LER-PS" , "LER-VS" , "LER-SA"};
    //vector<string> datasets = {"LER-ALL", "LER-VACS", "", "HER-REF" , "HER-PS" , "HER-VS1", "HER-VS2", "", "LER-REF", "LER-PS" , "LER-VS" , "LER-SA"};
    vector<string> datasets = {"HER-ALL", "LER-ALL","LER-VACS", "", "HER-REF" , "HER-PS" , "HER-VS1", "HER-VS2","", "LER-REF","LER-PS","LER-VS","LER-SA"};
    // /vector<string> datasets = GetDataSets();
    string channels[] = {"FWD1"};
    ofstream txtfile;
    txtfile.open ("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/DecayStudy/decay_study.txt");
    for(auto channel : channels)
    {
        for(auto dataset : datasets )
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
            ss << "\n" << dataset << fixed << setprecision(0) << " && ";

            // TGraph* energy= (TGraph*) rfile->Get((channel + "_energy").c_str());
            // ss << energy->GetMean(2) << " $\\pm$ " <<energy->GetRMS(2);
            //
            // TGraph* eb1= (TGraph*) rfile->Get((channel + "_energy_bunch_1").c_str());
            // ss  << " & " << eb1->GetMean(2) << " $\\pm$ " <<eb1->GetRMS(2);
            //
            // TGraph* eb2= (TGraph*) rfile->Get((channel + "_energy_bunch_2").c_str());
            // ss  << " & " << eb2->GetMean(2) << " $\\pm$ " <<eb2->GetRMS(2);
            //
            // TH1F* eb1_per_e = (TH1F*) rfile->Get((channel + "_e_bunch_1").c_str());
            // ss << fixed << setprecision(0) << " & " << eb1_per_e->GetMean()*1e2 << " $\\pm$ " <<eb1_per_e->GetRMS()*1e2;
            //
            // TH1F* eb2_per_e= (TH1F*) rfile->Get((channel + "_e_bunch_2").c_str());
            // ss  << " & " << eb2_per_e->GetMean()*1e2 << " $\\pm$ " <<eb2_per_e->GetRMS()*1e2;
            //
            // TH1F* eb1_in05ms = (TH1F*) rfile->Get((channel + "_e_bunch_5mus_1").c_str());
            // ss << fixed << setprecision(1) << " & " << eb1_in05ms->GetMean()*1e2 << " $\\pm$ " <<eb1_in05ms->GetRMS()*1e2;
            //
            // TH1F* eb2_in05ms= (TH1F*) rfile->Get((channel + "_e_bunch_5mus_2").c_str());
            // ss  << " & " << eb2_in05ms->GetMean()*1e2 << " $\\pm$ " <<eb2_in05ms->GetRMS()*1e2;
            //
            // TH1F* t1_belowX = (TH1F*) rfile->Get((channel + "_t_energy_bunch_belowX_1").c_str());
            // ss << fixed << setprecision(1) << " & " << t1_belowX->GetMean()*1e6 << " $\\pm$ " <<t1_belowX->GetRMS()*1e6;
            //
            // TH1F* t2_belowX= (TH1F*) rfile->Get((channel + "_t_energy_bunch_belowX_2").c_str());
            // ss  << " & " << t2_belowX->GetMean()*1e6 << " $\\pm$ " <<t2_belowX->GetRMS()*1e6;

            // --- NEW VERSION ---
            TGraph* energy= (TGraph*) rfile->Get((channel + "_energy").c_str());
            ss << quantiles(energy, 0);

            TGraph* eb1= (TGraph*) rfile->Get((channel + "_energy_bunch_1").c_str());
            ss  << " && " << quantiles(eb1, 0);

            TH1F* eb1_per_e = (TH1F*) rfile->Get((channel + "_e_bunch_1_only").c_str());
            ss <<  " && " << quantiles(eb1_per_e, 0, 1e2);
            // ss << fixed << setprecision(0) << " & " << eb1_per_e->GetMean()*1e2 << " " << quantiles(eb1_per_e, 1e2);

            TH1F* eb1_in05ms = (TH1F*) rfile->Get((channel + "_e_bunch_5mus_1").c_str());
            ss << " & " << quantiles(eb1_in05ms, 0, 1e2);
            // ss << fixed << setprecision(1) << " & " << eb1_in05ms->GetMean()*1e2 << " $\\pm$ " <<eb1_in05ms->GetRMS()*1e2;

            TH1F* t1_belowX = (TH1F*) rfile->Get((channel + "_t_energy_bunch_belowX_1").c_str());
            ss << " & " << quantiles(t1_belowX, 0, 1e6);
            // ss << fixed << setprecision(1) << " & " << t1_belowX->GetMean()*1e6 << " $\\pm$ " <<t1_belowX->GetRMS()*1e6;

            ss <<"\\" << "\\" << " \\addlinespace[6pt]";
            cout << ss.str()<< endl;
            txtfile << ss.str();

            rfile->Close();
            }
    }
}
    txtfile.close();
}
