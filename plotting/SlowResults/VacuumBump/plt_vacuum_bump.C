
int values_vs_time(Target target, string channel = "FWD1")
{
    SetPhDStyle();

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraphErrors* g_ip_vs_t = (TGraphErrors*)rfile->Get("I_LER x SKB_LER_pressures_local[1]");
    TGraphErrors* g_ip_vs_t_w = (TGraphErrors*)rfile->Get("I_LER x SKB_LER_pressures_local[1]_w");
    TGraphErrors* g_fwd1_vs_t = (TGraphErrors*)rfile->Get("Rate FWD1");
    TGraphErrors* g_fwd2_vs_t = (TGraphErrors*)rfile->Get("Rate FWD2");
    TGraphErrors* g_fwd3_vs_t = (TGraphErrors*)rfile->Get("Rate FWD3");
    TGraphErrors* g_z_vs_t = (TGraphErrors*)rfile->Get("SKB_LER_Zeff_D02");

    // --- Get the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./6.;


    string cantitle = "values_over_time_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);


    TPad * pad = new TPad("pad1", "The pad 20% of the height", 0.0, 0.66, 1.0, 1.0, kWhite);
    pad->Draw();

    // pad->SetRightMargin(0.035);
    // pad->SetTopMargin(0.05);
    // pad->SetBottomMargin(0.19);
    // pad->SetLeftMargin(0.12);

    TPad * pad2 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.33, 1.0, 0.66, kWhite);
    pad2->Draw();

    // pad2->SetRightMargin(0.035);
    // pad2->SetTopMargin(0.05);
    // pad2->SetBottomMargin(0.19);
    // pad2->SetLeftMargin(0.12);


    TPad * pad3 = new TPad("pad2", "The pad 20% of the height", 0.0, 0.0, 1.0, 0.33, kWhite);
    pad3->Draw();



    // pad3->SetRightMargin(0.035);
    // pad3->SetTopMargin(0.05);
    // pad3->SetBottomMargin(0.19);
    // pad3->SetLeftMargin(0.12);



    // ----- Do the drawing shit -----
    pad->cd();
    g_ip_vs_t->Draw("AP");

    pad2->cd();
    g_fwd1_vs_t->Draw("AP");
    g_fwd2_vs_t->Draw("P");
    g_fwd3_vs_t->Draw("P");

    pad3->cd();
    g_z_vs_t->Draw("AP");

    return 1;
}

int rate_vs_ip(Target target, string channel = "FWD1")
{
    SetPhDStyle();

    TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraphErrors* g_rate_vs_p = (TGraphErrors*)rfile->Get("Rate FWD1 vs I_LER x SKB_LER_pressures_local[1]");
    TGraphErrors* g_rate_vs_p_weighted = (TGraphErrors*)rfile->Get("Rate FWD1 vs I_LER x SKB_LER_pressures_local[1]_w");

    TGraphErrors* g_rate_vs_p_1 = new TGraphErrors();
    g_rate_vs_p_1->SetLineColor(kBlue);
    g_rate_vs_p_1->SetMarkerColor(kBlue);
    TGraphErrors* g_rate_vs_p_2 = new TGraphErrors();
    g_rate_vs_p_2->SetLineColor(kRed);
    g_rate_vs_p_2->SetMarkerColor(kRed);

    TGraphErrors* g_rate_vs_p_weighted_1 = new TGraphErrors();
    g_rate_vs_p_weighted_1->SetLineColor(kBlue);
    g_rate_vs_p_weighted_1->SetMarkerColor(kBlue);
    TGraphErrors* g_rate_vs_p_weighted_2 = new TGraphErrors();
    g_rate_vs_p_weighted_2->SetLineColor(kRed);
    g_rate_vs_p_weighted_2->SetMarkerColor(kRed);

    TGraphErrors* g_z_vs_t = (TGraphErrors*)rfile->Get("SKB_LER_Zeff_D02");

    double t1 = 1463.9712e6;
    double t2 = 1463.9713e6;
    double t3 = 1463.9719e6;
    double t4 = 1463.9725e6;

    int n1 = 0;
    int n2 = 0;
    int n3 = 0;
    int n4 = 0;

    double x_prev, y_prev;
    g_z_vs_t->GetPoint(0, x_prev, y_prev);

    for(int i = 1; i<g_rate_vs_p->GetN(); ++i)
    {
            double x,y, errx, erry;
            g_z_vs_t->GetPoint(i, x, y);
        //    cout <<fixed<<setprecision(6)<< "x: " << x << " x_prev: " << x_prev<< ", delta1: " << x-t1  << ", delta2: " << x-t2 << ", delta3: " << x-t3 << ", i: " << i <<endl;
            if(x_prev < t1 && t1 <= x)
            {
                n1 = i;

            }
            else if(x_prev < t2 && t2 <= x)
            {
                n2 = i;

            }
            else if(x_prev < t3 && t3 <= x)
            {
                n3 = i;
            }
            else if(x_prev < t4 && t4 <= x)
            {
                n4 = i;
            }

                x_prev = x;

    }

    cout << "n1: " << n1 << " n2: " << n2 << " n3: " << n3 << endl;

    for(int i = 0; i<g_rate_vs_p->GetN(); ++i)
    {
        double x,y, errx, erry;
        g_rate_vs_p->GetPoint(i, x, y);
        errx = g_rate_vs_p->GetErrorX(i);
        erry = g_rate_vs_p->GetErrorY(i);

        if(n1<=i && i<=n2)
        {
            int n = g_rate_vs_p_1->GetN();
            g_rate_vs_p_1->SetPoint(n, x, y);
            g_rate_vs_p_1->SetPointError(n, errx, erry);
        }
        else if(n3<=i && i<=n4)
        {
            int n = g_rate_vs_p_2->GetN();
            g_rate_vs_p_2->SetPoint(n, x, y);
            g_rate_vs_p_2->SetPointError(n, errx, erry);
        }
    }

    for(int i = 0; i<g_rate_vs_p_weighted->GetN(); ++i)
    {
        double x,y, errx, erry;
        g_rate_vs_p_weighted->GetPoint(i, x, y);
        errx = g_rate_vs_p_weighted->GetErrorX(i);
        erry = g_rate_vs_p_weighted->GetErrorY(i);

        if(n1<=i && i<=n2)
        {
            int n = g_rate_vs_p_weighted_1->GetN();
            g_rate_vs_p_weighted_1->SetPoint(n, x, y);
            g_rate_vs_p_weighted_1->SetPointError(n, errx, erry);
        }
        else if(n3<=i && i<=n4)
        {
            int n = g_rate_vs_p_weighted_2->GetN();
            g_rate_vs_p_weighted_2->SetPoint(n, x, y);
            g_rate_vs_p_weighted_2->SetPointError(n, errx, erry);
        }
    }
    // --- Get the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;


    string cantitle = "unweighted_rate_vs_ip_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

//    g_rate_vs_p->Draw("AP");
    g_rate_vs_p_2->Draw("AP");
    g_rate_vs_p_1->Draw("P");

    // --- Get the 2nd canvas --

    cantitle = "weighted_rate_vs_ip_" + channel;
    TCanvas * canw = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    canw->SetFillColor(kWhite);
    canw->SetBorderMode(0);
    canw->SetBorderSize(2);
    canw->SetFrameBorderMode(0);

    // g_rate_vs_p_weighted->Draw("AP");
    g_rate_vs_p_weighted_2->Draw("AP");
    g_rate_vs_p_weighted_1->Draw("P");
    return 1;
}
void plt_vacuum_bump()
{

    //string chs[] = {"FWD1","FWD2","FWD3"};
    string channels[] = {"FWD1"};

    Target target = GetTarget("VacuumBump");
//    values_vs_time(target);

    for(auto& channel: channels)
    {
        rate_vs_ip(target);
    }
}
