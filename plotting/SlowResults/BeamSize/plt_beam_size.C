#include <vector>



void her(string channel)
{
	Target target = GetTarget("BEAMSIZE_HER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraphErrors* graph;
    TF1* function;
    double min_chi2 = 1e10;
    int min_i = 0;
    for(int i =0; i< 26; ++i)
    {
        TGraphErrors* gtmp = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(i)+"]").c_str());
        // cout << "(BS_Rate_+channel+_LP+to_string(i)).c_str(): " << ("BS_Rate_"+channel+"_LP["+to_string(i)+"]") << endl;
    //    cout << "Graph title: " << gtmp->GetTitle() << endl;

        TF1* ftmp = gtmp->GetFunction("f1");

        if(ftmp->GetChisquare()/ftmp->GetNDF() < min_chi2)
        {
            min_i = i;
            min_chi2 = ftmp->GetChisquare()/ftmp->GetNDF();
            graph = gtmp;
            function = ftmp;
        }
//        cout << "ftmp->GetChisquare()/ftmp->GetNDF(): " << ftmp->GetChisquare()/ftmp->GetNDF() << endl;

    }

        cout << "Min_i: " << min_i << ", min chi2/ndf: " << min_chi2 << endl;

        TGraphErrors* g_sigmay = (TGraphErrors*)rfile->Get((target.ring+"_sigmay").c_str());
        TGraphErrors* g_current = (TGraphErrors*)rfile->Get((target.ring+"_Current").c_str());
        TGraphErrors* g_zeff = (TGraphErrors*)rfile->Get((target.ring+"_Zeff").c_str());
        TGraphErrors* g_p = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
        TGraphErrors* g_obs = (TGraphErrors*)rfile->Get(("Rate "+ channel).c_str());

        double low[15];
        double up[15];

        if(target.ring == "HER"){
            low[0] = 1463446682; up[0] = 1463446972;
            low[1] = 1463446972; up[1] = 1463447382;
            low[2] = 1463447412; up[2] = 1463447603;
            low[3] = 1463447700; up[3] = 1463447880;
            low[4] = 1463447940; up[4] = 1463448112;
            low[5] = 1463449800; up[5] = 1463449980;
            low[6] = 1463450020; up[6] = 1463450410;
            low[7] = 1463450420; up[7] = 1463450800;
            low[8] = 1463450810; up[8] = 1463451090;
            low[9] = 1463451120; up[9] = 1463451300;
            low[10] = 1463452110; up[10] = 1463452350;
            low[11] = 1463452360; up[11] = 1463452620;
            low[12] = 1463452710; up[12] = 1463453060;
            low[13] = 1463453160; up[13] = 1463453710;
            low[14] = 1463453760; up[14] = 1463454000;
        }
        else if(target.ring == "LER")
        {
            // low[0] = 1463469869; up[0] = 1463470049;
            // low[1] = 1463470115; up[1] = 1463470295;
            // low[2] = 1463470361; up[2] = 1463470541;
            // low[3] = 1463470690; up[3] = 1463470870;
            // low[4] = 1463471041; up[4] = 1463471221;
            // low[5] = 1463471443; up[5] = 1463471623;
            // low[6] = 1463471688; up[6] = 1463471868;
            // low[7] = 1463472068; up[7] = 1463472248;
            // low[8] = 1463472308; up[8] = 1463472488;
            // low[9] = 1463472578; up[9] = 1463472758;
            // low[10] = 1463473067; up[10] = 1463473187;
            // low[11] = 1463473412; up[11] = 1463473532;
            // low[12] = 1463473643; up[12] = 1463473852;
            // low[13] = 1463473852; up[13] = 1463473972;
            // low[14] = 1463474064; up[14] = 1463474184;

            low[0] = 1463469869; up[0] = 1463470100;
            low[1] = up[0]; up[1] = 1463470400;
            low[2] = up[1]; up[2] = 1463470700;
            low[3] = up[2]; up[3] = 1463470910;
            low[4] = up[3]; up[4] = 1463471300;
            low[5] = up[4]; up[5] = 1463471640;
            low[6] = up[5]; up[6] = 1463471900;
            low[7] = up[6]; up[7] = 1463471960;
            low[8] = up[7]; up[8] = 1463472260;
            low[9] = up[8]; up[9] = 1463472800;
            low[10] = 1463472900; up[10] = 1463473300;
            low[11] = up[10]; up[11] = 1463473500;
            low[12] = up[11]; up[12] = 1463473700;
            low[13] = up[12]; up[13] = 1463474000;
            low[14] = up[13]; up[14] = 1463474184;
        }

        vector<TGraphErrors*> graphs;
        for(int i =0; i<16; ++i)
        {
            TGraphErrors* gtmp = new TGraphErrors();
            graphs.push_back(gtmp);
        }

        double xmin = 1e16;
        double xmax = 0;

        double ymin = 1e16;
        double ymax = 0;

        for(int i = 0 ; i<g_sigmay->GetN(); ++i)
        {
            double ts,sigmay, x,y, errx, erry;

            g_sigmay->GetPoint(i, ts, sigmay);
            // g_current->GetPoint(i, ts, I)
            // g_zeff->GetPoint(i, ts, zeff);
            // g_p->GetPoint(i, ts, p);
            // g_obs->GetPoint(i, ts, obs);
            // g_obs->GetPointError(i, errx erry);
             cout << fixed << setprecision(1) << "i: " << i << ", ts: " << ts << ", sigmay: " << sigmay << endl;
             graph->GetPoint(i,x,y);
             errx = graph->GetErrorX(i);
             erry = graph->GetErrorY(i);

             x /=1e6;
             errx /=1e6;

             y /=1e6;
             erry /=1e6;


                 if(sigmay>=35. && sigmay<=400)
                 {

                     for(int j = 0; j<15 ; ++j)
                     {
                        if(ts>=low[j] && ts <up[j])
                        {

                        int n = graphs[j]->GetN();
                        graphs[j]->SetPoint(n, x, y );

                        graphs[j]->SetPointError(n, errx, erry );


                        if(x<xmin) xmin=x;
                        if(x>xmax) xmax=x;

                        if(y-erry<ymin) ymin=y-erry;
                        if(y+erry>ymax) ymax=y+erry;
                    //}

                    n = graphs[15]->GetN();

                    graphs[15]->SetPoint(n, x, y );

                    graphs[15]->SetPointError(n, errx, erry );

                    }
                    }

                }

        }
        TF1* myfit = new TF1("fit", "[0]*x+[1]", 0.01*xmin, 100.*xmax);
        graphs[15]->Fit(myfit, "Q","");

        cout << "Final result: Chi2: " << function->GetChisquare() << ", ndf: " << fixed << setprecision(8) << function->GetNDF() << ", chi2/ndf: " <<  function->GetChisquare()/function->GetNDF()<< endl;

    //    gStyle->SetOptFit(1112);

        // --- Colors and Lines ---
        double markersize = 1.4;
        double markeralpha = 0.8;
        double linealpha = 0.8;
        double linewidth = 1;

        int shape1 = 22;
        int shape2 = 20;
        int shape3 = 21;

        int shape4 = 26;
        int shape5 = 4;
        int shape6 = 25;

        vector<int> colors;
        colors.push_back(tab20_orange->GetNumber());
        colors.push_back(tab20_blue->GetNumber());
        colors.push_back(tab20_green->GetNumber());
        colors.push_back(tab20_red->GetNumber());
        colors.push_back(tab20_lila->GetNumber());

        int cfit = kBlack;

        // Do the making the graph nice shit;

        for(int i =0; i<5; ++i)
        {
            graphs[i]->SetMarkerStyle(shape1);
            graphs[i]->SetMarkerSize(markersize);
            graphs[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
            graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
            graphs[i]->SetLineWidth(linewidth);

        }
        for(int i =5; i<10; ++i)
        {
            graphs[i]->SetMarkerStyle(shape2);
            graphs[i]->SetMarkerSize(markersize);
            graphs[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
            graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
            graphs[i]->SetLineWidth(linewidth);
        }
        for(int i =10; i<15; ++i)
        {
            graphs[i]->SetMarkerStyle(shape3);
            graphs[i]->SetMarkerSize(markersize);
            graphs[i]->SetMarkerColorAlpha(colors[i%5], markeralpha);
            graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
            graphs[i]->SetLineWidth(linewidth);
        }

        myfit->SetLineColor(cfit);
        myfit->SetLineWidth(2);
        myfit->SetNpx(4000);
    // --- Build Axis ---

    TGraphErrors* axis = graphs[15];
    // graphs[0]->Draw("ap");
    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
    TLegend* leg;
    if(target.ring == "HER")
    {
        leg  =  new TLegend(0.21, 0.14, 0.37, 0.34);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);

        if(channel == "FWD2"){
                axis->GetYaxis()->SetRangeUser(ymin*0.26, ymax*1.15);
        }
        else
        {
                axis->GetYaxis()->SetRangeUser(ymin*0.2, ymax*1.1);
        }
    }
    else if(target.ring == "LER")
    {
        if(channel == "FWD1"){
            //    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
                axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
        }
        else if(channel == "FWD2"){
            //    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
                axis->GetYaxis()->SetRangeUser(0, ymax*1.05);
        }
        else if(channel == "FWD3"){
                axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*2.5);
                axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
        }



                leg  =  new TLegend(0.45, 0.16, 0.61, 0.32);
                leg->SetBorderSize(0);
                leg->SetFillColor(0);
                leg->SetFillStyle(0);
                leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);;
    }


    // axis->GetYaxis()->SetRangeUser(ymin*0.4, ymax*1.15);
    axis->SetMarkerSize(0);
    axis->SetLineWidth(0);

    axis->GetXaxis()->SetTitle("I/(PZ_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
    axis->GetXaxis()->SetTitleOffset(1.15);
    axis->GetYaxis()->SetTitle("Particle Rate/(IPZ_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");
    // axis->GetXaxis()->SetTitle("#frac{I}{PZ_{e}^{2}#sigma_{y}} [mA#upointPa^{-1}#upoint#mum^{-1}]");
    // axis->GetYaxis()->SetTitle("#frac{Particle Rate}{IPZ_{e}^{2}} [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");

    // Legend and Text
    //
    // TLegend* leg =  new TLegend(0.21, 0.16, 0.37, 0.32);
    // leg->SetBorderSize(0);
    // leg->SetFillColor(0);
    // leg->SetFillStyle(0);
    // leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);
    // //leg->SetNColumns(3);

	leg->AddEntry(set_line_width(myfit), "Fit: #frac{Particle Rate}{IP_{e}Z_{e}^{2}} = S_{bg} + S_{t}#upoint#frac{I}{P_{e}Z_{e}^{2}#sigma_{y}}","l");
    stringstream ss;
    ss << fixed << setprecision(1) << "S_{bg} = (" << myfit->GetParameter(1) << " #pm "  << myfit->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
    leg->AddEntry((TObject*)0, ss.str().c_str(), "");
    ss.str("");

	ss << fixed << setprecision(1) << "S_{T} = (" << myfit->GetParameter(0) << " #pm "  << myfit->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-2}";
    //     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg->AddEntry((TObject*)0, ss.str().c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(gStyle->GetLegendFont()+20);
    text->SetTextSize(vis_multi*textsize*pt2pixel);

    // --- Get the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "her_injections_hit_energy_spectrum_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    // Do the Drawing shit!!!
    axis->Draw("AP");

	cout << "NPoints: " << axis->GetN();

    cout << "x = [" << xmin<<", "<<xmax<<"]"<<endl;
    cout << "y = [" << ymin<<", "<<ymax<<"]"<<endl;
    for(int i =0; i<15; ++i)
    {

        graphs[i]->Draw("p");
    }

    for(int i =0; i<5; ++i)
    {
        graphs[i] = (TGraphErrors*) graphs[i]->Clone("");
        graphs[i]->SetMarkerStyle(shape4);
        // graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
        graphs[i]->SetMarkerColorAlpha(kBlack, 1);
    //    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

    }
    for(int i =5; i<10; ++i)
    {
        graphs[i] = (TGraphErrors*) graphs[i]->Clone("");
        graphs[i]->SetMarkerStyle(shape5);
        // graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
        graphs[i]->SetMarkerColorAlpha(kBlack, 1);
    //    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
    }
    for(int i =10; i<15; ++i)
    {
        graphs[i] = (TGraphErrors*) graphs[i]->Clone("");
        graphs[i]->SetMarkerStyle(shape6);
        // graphs[i]->SetMarkerColorAlpha(colors[i%5], 1);
        graphs[i]->SetMarkerColorAlpha(kBlack, 1);
    //    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

    }

    for(int i =0; i<15; ++i)
    {

        graphs[i]->Draw("p");
    }

    cout << "f(xmin): " << function->Eval(xmin) << ",  f(xmax): " << function->Eval(xmax) << endl;
    myfit->Draw("same");
    // function->SetLineColor(kBlack);
    // function->Draw("same");
    // text->DrawLatex(0.21, 0.35, "HER Size Sweep");
    text->DrawLatex(0.21, 0.89, (target.typein+": "+channel).c_str());
    leg->Draw();


    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".pdf").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".png").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".eps").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".jpg").c_str());

    rfile->Close();
}

void her_new(string channel)
{
	Target target = GetTarget("BEAMSIZE_HER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    vector<TGraphErrors*> 	graphs;
	vector<TGraphErrors*> 	graphs_clean;
    TF1* 			function;

	for(int i =0; i< 76; ++i)
    {
	 	TGraphErrors* gtmp = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(i)+"]").c_str());
	 	graphs.push_back(gtmp);
	 	graphs_clean.push_back(new TGraphErrors());
	}

	TGraphErrors* g_sigmay = (TGraphErrors*)rfile->Get((target.ring+"_sigmay").c_str());
	TGraphErrors* g_current = (TGraphErrors*)rfile->Get((target.ring+"_Current").c_str());
	TGraphErrors* g_zeff = (TGraphErrors*)rfile->Get((target.ring+"_Zeff").c_str());
	// TGraphErrors* g_p = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
	TGraphErrors* g_obs = (TGraphErrors*)rfile->Get(("Rate "+ channel).c_str());

	double low[15];
	double up[15];

	if(target.ring == "HER"){
		low[0] = 1463446682; up[0] = 1463446972;
		low[1] = 1463446972; up[1] = 1463447382;
		low[2] = 1463447412; up[2] = 1463447603;
		low[3] = 1463447700; up[3] = 1463447880;
		low[4] = 1463447940; up[4] = 1463448112;
		low[5] = 1463449800; up[5] = 1463449980;
		low[6] = 1463450020; up[6] = 1463450410;
		low[7] = 1463450420; up[7] = 1463450800;
		low[8] = 1463450810; up[8] = 1463451090;
		low[9] = 1463451120; up[9] = 1463451300;
		low[10] = 1463452110; up[10] = 1463452350;
		low[11] = 1463452360; up[11] = 1463452620;
		low[12] = 1463452710; up[12] = 1463453060;
		low[13] = 1463453160; up[13] = 1463453710;
		low[14] = 1463453760; up[14] = 1463454000;
	}
	else if(target.ring == "LER")
	{
		// low[0] = 1463469869; up[0] = 1463470049;
		// low[1] = 1463470115; up[1] = 1463470295;
		// low[2] = 1463470361; up[2] = 1463470541;
		// low[3] = 1463470690; up[3] = 1463470870;
		// low[4] = 1463471041; up[4] = 1463471221;
		// low[5] = 1463471443; up[5] = 1463471623;
		// low[6] = 1463471688; up[6] = 1463471868;
		// low[7] = 1463472068; up[7] = 1463472248;
		// low[8] = 1463472308; up[8] = 1463472488;
		// low[9] = 1463472578; up[9] = 1463472758;
		// low[10] = 1463473067; up[10] = 1463473187;
		// low[11] = 1463473412; up[11] = 1463473532;
		// low[12] = 1463473643; up[12] = 1463473852;
		// low[13] = 1463473852; up[13] = 1463473972;
		// low[14] = 1463474064; up[14] = 1463474184;

		low[0] = 1463469869; up[0] = 1463470100;
		low[1] = up[0]; up[1] = 1463470400;
		low[2] = up[1]; up[2] = 1463470700;
		low[3] = up[2]; up[3] = 1463470910;
		low[4] = up[3]; up[4] = 1463471300;
		low[5] = up[4]; up[5] = 1463471640;
		low[6] = up[5]; up[6] = 1463471900;
		low[7] = up[6]; up[7] = 1463471960;
		low[8] = up[7]; up[8] = 1463472260;
		low[9] = up[8]; up[9] = 1463472800;
		low[10] = 1463472900; up[10] = 1463473300;
		low[11] = up[10]; up[11] = 1463473500;
		low[12] = up[11]; up[12] = 1463473700;
		low[13] = up[12]; up[13] = 1463474000;
		low[14] = up[13]; up[14] = 1463474184;
	}

	double xmin = 1e16;
	double xmax = 0;

	double ymin = 1e16;
	double ymax = 0;

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

		// cout << "xmax: " << xmax << endl;
		// cout << "ymax: " << ymax << endl;
		if(sigmay>=35. && sigmay<=400)
		{
			for(int j = 0; j<15 ; ++j)
			{
			   if(ts>=low[j] && ts <up[j])
			   {
				   for(int k = 0; k< 76; ++k)
				   {
					   graphs.at(k)->GetPoint(i,x,y);
					   errx = graphs.at(k)->GetErrorX(i);
					   erry = graphs.at(k)->GetErrorY(i);


					   x /=1e6;
					   errx /=1e6;

					   y /=1e6;
					   erry /=1e6;

					   if(x < 1e50 && y < 1e50)
					   {
						   int n = graphs_clean.at(k)->GetN();

						   graphs_clean.at(k)->SetPoint(n, x, y );

						   graphs_clean.at(k)->SetPointError(n, errx, erry );

						//   cout<< "k: " << k<< ", x: " << x << ", y: " << y << endl;
						   if(x<xmin) xmin=x;
						   if(x>xmax) xmax=x;

							if(y-erry<ymin) ymin=y-erry;
							if(y+erry>ymax) ymax=y+erry;
						}
				   }

			   }
			}
		}
	}

	cout << "xmax: " << xmax << endl;
	cout << "ymax: " << ymax << endl;
	// Find the minimum Chi2

	TGraphErrors* graph_minp;
	TF1* 		  tf1_minp;

	double min_chi2 = 1e10;
	int min_i = 0;

	for(int i = 0; i < 26; ++i)
	{
		TF1* myfit = new TF1("fit", "[0]+[1]*x", 0.01*xmin, 100.*xmax);
		graphs_clean.at(i)->Fit(myfit, "Q","");

		cout << "i: " << i << ", chi2/ndf: " << myfit->GetChisquare()/myfit->GetNDF() << ", slope significance: " << myfit->GetParameter(0)/myfit->GetParError(0) <<  endl;
		if(myfit->GetChisquare()/myfit->GetNDF() < min_chi2)
		{
			min_i = i;
			min_chi2 = myfit->GetChisquare()/myfit->GetNDF();
			graph_minp = graphs_clean.at(i);
			tf1_minp = myfit;
		}
	}

	cout << "\n" << "HER: for channel " << channel << endl;
	cout << "Min_i: " << min_i << ", min chi2/ndf: " << fixed << setprecision(5) << min_chi2 << endl;
	cout << "NPoints: " << graph_minp->GetN() << endl;
	cout << fixed << setprecision(3) << "S_{bg} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}" << endl;;
	cout << fixed << setprecision(3) << "S_{T} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-2}" << "\n"<<  endl;;


	vector<TGraphErrors*> graphs_subruns;
	for(int i =0; i<15; ++i)
    {
        TGraphErrors* gtmp = new TGraphErrors();
        graphs_subruns.push_back(gtmp);
    }

	xmin = 1e16;
	xmax = 0;

	ymin = 1e16;
	ymax = 0;

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

	 	graphs.at(min_i)->GetPoint(i,x,y);
	 	errx = graphs.at(min_i)->GetErrorX(i);
	 	erry = graphs.at(min_i)->GetErrorY(i);
	//	cout << "sigmay: " << sigmay << endl;


        x /=1e6;
        errx /=1e6;

        y /=1e6;
        erry /=1e6;

		if(sigmay>=35. && sigmay<=400)
		{

			for(int j = 0; j<15 ; ++j)
			{

				if(ts>=low[j] && ts <up[j])
				{

					int n = graphs_subruns[j]->GetN();
					graphs_subruns[j]->SetPoint(n, x, y );

					graphs_subruns[j]->SetPointError(n, errx, erry );

					if(x<xmin) xmin=x;
					if(x>xmax) xmax=x;

					if(y-erry<ymin) ymin=y-erry;
					if(y+erry>ymax) ymax=y+erry;

				}
			}

		}

	}

	int npoints_subruns = 0;

	for(int j = 0; j<15 ; ++j)
	{
		npoints_subruns += graphs_subruns[j]->GetN();
	}
	cout << "npoints_subruns: " << npoints_subruns << endl;
	// --- Colors and Lines ---
	double markersize = 1.4;
	double markeralpha = 0.8;
	double linealpha = 0.8;
	double linewidth = 1;

	int shape1 = 22;
	int shape2 = 20;
	int shape3 = 21;

	int shape4 = 26;
	int shape5 = 4;
	int shape6 = 25;

	vector<int> colors;
	colors.push_back(tab20_orange->GetNumber());
	colors.push_back(tab20_blue->GetNumber());
	colors.push_back(tab20_green->GetNumber());
	colors.push_back(tab20_red->GetNumber());
	colors.push_back(tab20_lila->GetNumber());

	int cfit = kBlack;

	// Do the making the graph nice shit;

	for(int i =0; i<5; ++i)
	{
		graphs_subruns[i]->SetMarkerStyle(shape1);
		graphs_subruns[i]->SetMarkerSize(markersize);
		graphs_subruns[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
		graphs_subruns[i]->SetLineColorAlpha(colors[i%5], linealpha);
		graphs_subruns[i]->SetLineWidth(linewidth);

	}
	for(int i =5; i<10; ++i)
	{
		graphs_subruns[i]->SetMarkerStyle(shape2);
		graphs_subruns[i]->SetMarkerSize(markersize);
		graphs_subruns[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
		graphs_subruns[i]->SetLineColorAlpha(colors[i%5], linealpha);
		graphs_subruns[i]->SetLineWidth(linewidth);
	}
	for(int i =10; i<15; ++i)
	{
		graphs_subruns[i]->SetMarkerStyle(shape3);
		graphs_subruns[i]->SetMarkerSize(markersize);
		graphs_subruns[i]->SetMarkerColorAlpha(colors[i%5], markeralpha);
		graphs_subruns[i]->SetLineColorAlpha(colors[i%5], linealpha);
		graphs_subruns[i]->SetLineWidth(linewidth);
	}

	tf1_minp->SetLineColor(cfit);
	tf1_minp->SetLineWidth(2);
	tf1_minp->SetNpx(4000);


	TGraphErrors* axis = graph_minp;
	// graphs[0]->Draw("ap");
	axis->GetXaxis()->SetLimits(0, xmax+xmin);

	TLegend* leg;
	if(target.ring == "HER")
	{
		leg  =  new TLegend(0.21, 0.16, 0.37, 0.33);
		leg->SetBorderSize(0);
		leg->SetFillColor(0);
		leg->SetFillStyle(0);
		leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);

		if(channel == "FWD2"){
				axis->GetYaxis()->SetRangeUser(ymin*0.18, ymax*1.15);
		}
		else if(channel == "FWD3"){
				axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
		}
		else
		{
				axis->GetYaxis()->SetRangeUser(ymin*0.2, ymax*1.2);
		}
	}
	else if(target.ring == "LER")
	{
		if(channel == "FWD1"){
			//    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
		}
		else if(channel == "FWD2"){
			//    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.05);
		}
		else if(channel == "FWD3"){
				axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*2.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
		}

				leg  =  new TLegend(0.45, 0.16, 0.61, 0.32);
				leg->SetBorderSize(0);
				leg->SetFillColor(0);
				leg->SetFillStyle(0);
				leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);;
	}


	// axis->GetYaxis()->SetRangeUser(ymin*0.4, ymax*1.15);
	axis->SetMarkerSize(0);
	axis->SetLineWidth(0);

	axis->GetXaxis()->SetTitle("I/(PZ_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
	axis->GetXaxis()->SetTitleOffset(1.15);
	axis->GetYaxis()->SetTitle("Particle Rate/(IPZ_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");

	leg->AddEntry(set_line_width(tf1_minp), "Combined Parameterization","l");
	//leg->AddEntry(set_line_width(tf1_minp), "Fit: #frac{Particle Rate}{IP_{e}Z_{e}^{2}} = S_{bg} + S_{t}#upoint#frac{I}{P_{e}Z_{e}^{2}#sigma_{y}}","l");
	stringstream ss;
	ss << fixed << setprecision(1) << "S_{bg} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
	ss.str("");

	ss << fixed << setprecision(1) << "S_{T} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-2}";
	//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
	leg->AddEntry((TObject*)0, ss.str().c_str(), "");

	TLatex* text = new TLatex();
	text->SetNDC();
	text->SetTextFont(gStyle->GetLegendFont()+20);
	text->SetTextSize(vis_multi*textsize*pt2pixel);

	// --- Get the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
	double ratio = 3./3.;

	string cantitle = "her_size_sweep" + channel;
	TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	can->SetFillColor(kWhite);
	can->SetBorderMode(0);
	can->SetBorderSize(2);
	can->SetFrameBorderMode(0);

	// Do the Drawing shit!!!
	axis->Draw("AP");

	// cout << "NPoints: " << axis->GetN() << endl;;

	cout << "x = [" << xmin<<", "<<xmax<<"]"<<endl;
	cout << "y = [" << ymin<<", "<<ymax<<"]"<<endl;
	for(int i =0; i<15; ++i)
	{
		graphs_subruns[i]->Draw("p");
	}

	for(int i =0; i<5; ++i)
	{
		graphs_subruns[i] = (TGraphErrors*) graphs_subruns[i]->Clone("");
		graphs_subruns[i]->SetMarkerStyle(shape4);
		// graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
		graphs_subruns[i]->SetMarkerColorAlpha(kBlack, 1);
	//    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

	}
	for(int i =5; i<10; ++i)
	{
		graphs_subruns[i] = (TGraphErrors*) graphs_subruns[i]->Clone("");
		graphs_subruns[i]->SetMarkerStyle(shape5);
		// graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
		graphs_subruns[i]->SetMarkerColorAlpha(kBlack, 1);
	//    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
	}
	for(int i =10; i<15; ++i)
	{
		graphs_subruns[i] = (TGraphErrors*) graphs_subruns[i]->Clone("");
		graphs_subruns[i]->SetMarkerStyle(shape6);
		// graphs[i]->SetMarkerColorAlpha(colors[i%5], 1);
		graphs_subruns[i]->SetMarkerColorAlpha(kBlack, 1);
	//    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

	}

	for(int i =0; i<15; ++i)
	{

		graphs_subruns[i]->Draw("p");
	}

	// cout << "f(xmin): " << tf1_minp->Eval(xmin) << ",  f(xmax): " << tf1_minp->Eval(xmax) << endl;
	tf1_minp->Draw("same");
	// function->SetLineColor(kBlack);
	// function->Draw("same");
	// text->DrawLatex(0.21, 0.35, "HER Size Sweep");
	text->DrawLatex(0.21, 0.89, (target.typein+": "+channel).c_str());
	leg->Draw();


	    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".pdf").c_str());
	    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".png").c_str());
	    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".eps").c_str());
	    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".jpg").c_str());

	    rfile->Close();

}

void ler_new(string channel)
{
	Target target = GetTarget("BEAMSIZE_LER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    vector<TGraphErrors*> 	graphs;
	vector<TGraphErrors*> 	graphs_clean;
    TF1* 			function;

	for(int i =0; i< 76; ++i)
    {
	 	TGraphErrors* gtmp = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(i)+"]").c_str());
	 	graphs.push_back(gtmp);
	 	graphs_clean.push_back(new TGraphErrors());
	}

	TGraphErrors* g_sigmay = (TGraphErrors*)rfile->Get((target.ring+"_sigmay").c_str());
	TGraphErrors* g_current = (TGraphErrors*)rfile->Get((target.ring+"_Current").c_str());
	TGraphErrors* g_zeff = (TGraphErrors*)rfile->Get((target.ring+"_Zeff").c_str());
	// TGraphErrors* g_p = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
	TGraphErrors* g_obs = (TGraphErrors*)rfile->Get(("Rate "+ channel).c_str());

	double low[15];
	double up[15];

	if(target.ring == "HER"){
		low[0] = 1463446682; up[0] = 1463446972;
		low[1] = 1463446972; up[1] = 1463447382;
		low[2] = 1463447412; up[2] = 1463447603;
		low[3] = 1463447700; up[3] = 1463447880;
		low[4] = 1463447940; up[4] = 1463448112;
		low[5] = 1463449800; up[5] = 1463449980;
		low[6] = 1463450020; up[6] = 1463450410;
		low[7] = 1463450420; up[7] = 1463450800;
		low[8] = 1463450810; up[8] = 1463451090;
		low[9] = 1463451120; up[9] = 1463451300;
		low[10] = 1463452110; up[10] = 1463452350;
		low[11] = 1463452360; up[11] = 1463452620;
		low[12] = 1463452710; up[12] = 1463453060;
		low[13] = 1463453160; up[13] = 1463453710;
		low[14] = 1463453760; up[14] = 1463454000;
	}
	else if(target.ring == "LER")
	{
		low[0] = 1463469869; up[0] = 1463470100;
		low[1] = up[0]; up[1] = 1463470400;
		low[2] = up[1]; up[2] = 1463470700;
		low[3] = up[2]; up[3] = 1463470910;
		low[4] = up[3]; up[4] = 1463471300;
		low[5] = up[4]; up[5] = 1463471640;
		low[6] = up[5]; up[6] = 1463471900;
		low[7] = up[6]; up[7] = 1463471960;
		low[8] = up[7]; up[8] = 1463472260;
		low[9] = up[8]; up[9] = 1463472800;
		low[10] = 1463472900; up[10] = 1463473300;
		low[11] = up[10]; up[11] = 1463473500;
		low[12] = up[11]; up[12] = 1463473700;
		low[13] = up[12]; up[13] = 1463474000;
		low[14] = up[13]; up[14] = 1463474184;
	}

	double xmin = 1e16;
	double xmax = 0;

	double ymin = 1e16;
	double ymax = 0;

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

		// cout << "xmax: " << xmax << endl;
		// cout << "ymax: " << ymax << endl;
		if(sigmay>=35. && sigmay<=400)
		{
			for(int j = 0; j<15 ; ++j)
			{
			   if(ts>=low[j] && ts <up[j])
			   {
				   for(int k = 0; k< 76; ++k)
				   {
					   graphs.at(k)->GetPoint(i,x,y);
					   errx = graphs.at(k)->GetErrorX(i);
					   erry = graphs.at(k)->GetErrorY(i);


					   x /=1e6;
					   errx /=1e6;

					   y /=1e6;
					   erry /=1e6;

					   if(x < 1e50 && y < 1e50)
					   {
						   int n = graphs_clean.at(k)->GetN();

						   graphs_clean.at(k)->SetPoint(n, x, y );

						   graphs_clean.at(k)->SetPointError(n, errx, erry );

						//   cout<< "k: " << k<< ", x: " << x << ", y: " << y << endl;
						   if(x<xmin) xmin=x;
						   if(x>xmax) xmax=x;

							if(y-erry<ymin) ymin=y-erry;
							if(y+erry>ymax) ymax=y+erry;
						}
				   }

			   }
			}
		}
	}

	cout << "xmax: " << xmax << endl;
	cout << "ymax: " << ymax << endl;
	// Find the minimum Chi2

	TGraphErrors* graph_minp;
	TF1* 		  tf1_minp;

	double min_chi2 = 1e10;
	int min_i = 0;

	for(int i = 0; i < 26; ++i)
	{
		TF1* myfit = new TF1("fit", "[0]+[1]*x", 0.01*xmin, 100.*xmax);
		graphs_clean.at(i)->Fit(myfit, "Q","");

		if(myfit->GetChisquare()/myfit->GetNDF() < min_chi2)
		{
			min_i = i;
			min_chi2 = myfit->GetChisquare()/myfit->GetNDF();
			graph_minp = graphs_clean.at(i);
			tf1_minp = myfit;
		}
	}

	cout << "\n" << "LER: for channel " << channel << endl;
	cout << "Min_i: " << min_i << ", min chi2/ndf: " << fixed << setprecision(5) << min_chi2 << endl;
	cout << "NPoints: " << graph_minp->GetN() << endl;
	cout << fixed << setprecision(3) << "S_{bg} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}" << endl;;
	cout << fixed << setprecision(3) << "S_{T} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-2}" << "\n"<<  endl;;


	vector<TGraphErrors*> graphs_subruns;
	for(int i =0; i<15; ++i)
    {
        TGraphErrors* gtmp = new TGraphErrors();
        graphs_subruns.push_back(gtmp);
    }

	xmin = 1e16;
	xmax = 0;

	ymin = 1e16;
	ymax = 0;

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

	 	graphs.at(min_i)->GetPoint(i,x,y);
	 	errx = graphs.at(min_i)->GetErrorX(i);
	 	erry = graphs.at(min_i)->GetErrorY(i);
	//	cout << "sigmay: " << sigmay << endl;


        x /=1e6;
        errx /=1e6;

        y /=1e6;
        erry /=1e6;

		if(sigmay>=35. && sigmay<=400)
		{

			for(int j = 0; j<15 ; ++j)
			{

				if(ts>=low[j] && ts <up[j])
				{

					int n = graphs_subruns[j]->GetN();
					graphs_subruns[j]->SetPoint(n, x, y );

					graphs_subruns[j]->SetPointError(n, errx, erry );

					if(x<xmin) xmin=x;
					if(x>xmax) xmax=x;

					if(y-erry<ymin) ymin=y-erry;
					if(y+erry>ymax) ymax=y+erry;

				}
			}

		}

	}

	int npoints_subruns = 0;

	for(int j = 0; j<15 ; ++j)
	{
		npoints_subruns += graphs_subruns[j]->GetN();
	}
	cout << "npoints_subruns: " << npoints_subruns << endl;
	// --- Colors and Lines ---
	double markersize = 1.4;
	double markeralpha = 0.8;
	double linealpha = 0.8;
	double linewidth = 1;

	int shape1 = 22;
	int shape2 = 20;
	int shape3 = 21;

	int shape4 = 26;
	int shape5 = 4;
	int shape6 = 25;

	vector<int> colors;
	colors.push_back(tab20_orange->GetNumber());
	colors.push_back(tab20_blue->GetNumber());
	colors.push_back(tab20_green->GetNumber());
	colors.push_back(tab20_red->GetNumber());
	colors.push_back(tab20_lila->GetNumber());

	int cfit = kBlack;

	// Do the making the graph nice shit;

	for(int i =0; i<5; ++i)
	{
		graphs_subruns[i]->SetMarkerStyle(shape1);
		graphs_subruns[i]->SetMarkerSize(markersize);
		graphs_subruns[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
		graphs_subruns[i]->SetLineColorAlpha(colors[i%5], linealpha);
		graphs_subruns[i]->SetLineWidth(linewidth);

	}
	for(int i =5; i<10; ++i)
	{
		graphs_subruns[i]->SetMarkerStyle(shape2);
		graphs_subruns[i]->SetMarkerSize(markersize);
		graphs_subruns[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
		graphs_subruns[i]->SetLineColorAlpha(colors[i%5], linealpha);
		graphs_subruns[i]->SetLineWidth(linewidth);
	}
	for(int i =10; i<15; ++i)
	{
		graphs_subruns[i]->SetMarkerStyle(shape3);
		graphs_subruns[i]->SetMarkerSize(markersize);
		graphs_subruns[i]->SetMarkerColorAlpha(colors[i%5], markeralpha);
		graphs_subruns[i]->SetLineColorAlpha(colors[i%5], linealpha);
		graphs_subruns[i]->SetLineWidth(linewidth);
	}

	tf1_minp->SetLineColor(cfit);
	tf1_minp->SetLineWidth(2);
	tf1_minp->SetNpx(4000);


	TGraphErrors* axis = graph_minp;
	// graphs[0]->Draw("ap");
	axis->GetXaxis()->SetLimits(0, xmax+xmin*0.5);

	TLegend* leg;
	if(target.ring == "HER")
	{
		leg  =  new TLegend(0.21, 0.16, 0.37, 0.33);
		leg->SetBorderSize(0);
		leg->SetFillColor(0);
		leg->SetFillStyle(0);
		leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);

		if(channel == "FWD2"){
				axis->GetYaxis()->SetRangeUser(ymin*0.26, ymax*1.15);
		}
		else
		{
				axis->GetYaxis()->SetRangeUser(ymin*0.2, ymax*1.2);
		}
	}
	else if(target.ring == "LER")
	{
		if(channel == "FWD1"){
			//    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.2);
				leg  =  new TLegend(0.41, 0.16, 0.57, 0.33);
		}
		else if(channel == "FWD2"){
			//    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.4);
				leg  =  new TLegend(0.21, 0.7, 0.37, 0.87);
		}
		else if(channel == "FWD3"){
			//	axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*2.5);
			axis->GetXaxis()->SetLimits(0, xmax+xmin*0.25);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
				leg  =  new TLegend(0.19, 0.7, 0.35, 0.87);
		}

				//leg  =  new TLegend(0.41, 0.16, 0.57, 0.33);
				leg->SetBorderSize(0);
				leg->SetFillColor(0);
				leg->SetFillStyle(0);
				leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);;
	}


	// axis->GetYaxis()->SetRangeUser(ymin*0.4, ymax*1.15);
	axis->SetMarkerSize(0);
	axis->SetLineWidth(0);

	axis->GetXaxis()->SetTitle("I/(PZ_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
	axis->GetXaxis()->SetTitleOffset(1.15);
	axis->GetYaxis()->SetTitle("Particle Rate/(IPZ_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");

	leg->AddEntry(set_line_width(tf1_minp), "Combined Parameterization","l");
	//leg->AddEntry(set_line_width(tf1_minp), "Fit: #frac{Particle Rate}{IP_{e}Z_{e}^{2}} = S_{bg} + S_{t}#upoint#frac{I}{P_{e}Z_{e}^{2}#sigma_{y}}","l");
	stringstream ss;
	ss << fixed << setprecision(1) << "S_{bg} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
	ss.str("");

	ss << fixed << setprecision(1) << "S_{T} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-2}";
	//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
	leg->AddEntry((TObject*)0, ss.str().c_str(), "");

	TLatex* text = new TLatex();
	text->SetNDC();
	text->SetTextFont(gStyle->GetLegendFont()+20);
	text->SetTextSize(vis_multi*textsize*pt2pixel);

	// --- Get the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
	double ratio = 3./3.;

	string cantitle = "ler_size_sweep" + channel;
	TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	can->SetFillColor(kWhite);
	can->SetBorderMode(0);
	can->SetBorderSize(2);
	can->SetFrameBorderMode(0);

	// Do the Drawing shit!!!
	axis->Draw("AP");

	// cout << "NPoints: " << axis->GetN() << endl;;

	cout << "x = [" << xmin<<", "<<xmax<<"]"<<endl;
	cout << "y = [" << ymin<<", "<<ymax<<"]"<<endl;
	for(int i =0; i<15; ++i)
	{
		graphs_subruns[i]->Draw("p");
	}

	for(int i =0; i<5; ++i)
	{
		graphs_subruns[i] = (TGraphErrors*) graphs_subruns[i]->Clone("");
		graphs_subruns[i]->SetMarkerStyle(shape4);
		// graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
		graphs_subruns[i]->SetMarkerColorAlpha(kBlack, 1);
	//    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

	}
	for(int i =5; i<10; ++i)
	{
		graphs_subruns[i] = (TGraphErrors*) graphs_subruns[i]->Clone("");
		graphs_subruns[i]->SetMarkerStyle(shape5);
		// graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
		graphs_subruns[i]->SetMarkerColorAlpha(kBlack, 1);
	//    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
	}
	for(int i =10; i<15; ++i)
	{
		graphs_subruns[i] = (TGraphErrors*) graphs_subruns[i]->Clone("");
		graphs_subruns[i]->SetMarkerStyle(shape6);
		// graphs[i]->SetMarkerColorAlpha(colors[i%5], 1);
		graphs_subruns[i]->SetMarkerColorAlpha(kBlack, 1);
	//    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

	}

	for(int i =0; i<15; ++i)
	{

		graphs_subruns[i]->Draw("p");
	}

	// cout << "f(xmin): " << tf1_minp->Eval(xmin) << ",  f(xmax): " << tf1_minp->Eval(xmax) << endl;
	tf1_minp->Draw("same");
	// function->SetLineColor(kBlack);
	// function->Draw("same");
	// text->DrawLatex(0.21, 0.35, "HER Size Sweep");

		if(channel == "FWD3")
		{
			text->DrawLatex(0.19, 0.89, (target.typein+": "+channel).c_str());
		}
		else
		{
			text->DrawLatex(0.21, 0.89, (target.typein+": "+channel).c_str());
		}

	leg->Draw();

	can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".pdf").c_str());
	can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".png").c_str());
	can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".eps").c_str());
	can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".jpg").c_str());

	rfile->Close();

}

void her_tr(string channel)
{
	// first get the minimum CCP
	Target target = GetTarget("BEAMSIZE_LER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

	vector<TGraphErrors*> 	graphs;
	vector<TGraphErrors*> 	graphs_clean;
	TF1* 			function;

	for(int i =0; i< 76; ++i)
	{
		TGraphErrors* gtmp = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(i)+"]").c_str());
		graphs.push_back(gtmp);
		graphs_clean.push_back(new TGraphErrors());
	}

	TGraphErrors* g_sigmay = (TGraphErrors*)rfile->Get((target.ring+"_sigmay").c_str());
	TGraphErrors* g_current = (TGraphErrors*)rfile->Get((target.ring+"_Current").c_str());
	TGraphErrors* g_zeff = (TGraphErrors*)rfile->Get((target.ring+"_Zeff").c_str());
	// TGraphErrors* g_p = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
	TGraphErrors* g_obs = (TGraphErrors*)rfile->Get(("Rate "+ channel).c_str());

	double low[15];
	double up[15];

	if(target.ring == "HER"){
		low[0] = 1463446682; up[0] = 1463446972;
		low[1] = 1463446972; up[1] = 1463447382;
		low[2] = 1463447412; up[2] = 1463447603;
		low[3] = 1463447700; up[3] = 1463447880;
		low[4] = 1463447940; up[4] = 1463448112;
		low[5] = 1463449800; up[5] = 1463449980;
		low[6] = 1463450020; up[6] = 1463450410;
		low[7] = 1463450420; up[7] = 1463450800;
		low[8] = 1463450810; up[8] = 1463451090;
		low[9] = 1463451120; up[9] = 1463451300;
		low[10] = 1463452110; up[10] = 1463452350;
		low[11] = 1463452360; up[11] = 1463452620;
		low[12] = 1463452710; up[12] = 1463453060;
		low[13] = 1463453160; up[13] = 1463453710;
		low[14] = 1463453760; up[14] = 1463454000;
	}
	else if(target.ring == "LER")
	{
		// low[0] = 1463469869; up[0] = 1463470049;
		// low[1] = 1463470115; up[1] = 1463470295;
		// low[2] = 1463470361; up[2] = 1463470541;
		// low[3] = 1463470690; up[3] = 1463470870;
		// low[4] = 1463471041; up[4] = 1463471221;
		// low[5] = 1463471443; up[5] = 1463471623;
		// low[6] = 1463471688; up[6] = 1463471868;
		// low[7] = 1463472068; up[7] = 1463472248;
		// low[8] = 1463472308; up[8] = 1463472488;
		// low[9] = 1463472578; up[9] = 1463472758;
		// low[10] = 1463473067; up[10] = 1463473187;
		// low[11] = 1463473412; up[11] = 1463473532;
		// low[12] = 1463473643; up[12] = 1463473852;
		// low[13] = 1463473852; up[13] = 1463473972;
		// low[14] = 1463474064; up[14] = 1463474184;

		low[0] = 1463469869; up[0] = 1463470100;
		low[1] = up[0]; up[1] = 1463470400;
		low[2] = up[1]; up[2] = 1463470700;
		low[3] = up[2]; up[3] = 1463470910;
		low[4] = up[3]; up[4] = 1463471300;
		low[5] = up[4]; up[5] = 1463471640;
		low[6] = up[5]; up[6] = 1463471900;
		low[7] = up[6]; up[7] = 1463471960;
		low[8] = up[7]; up[8] = 1463472260;
		low[9] = up[8]; up[9] = 1463472800;
		low[10] = 1463472900; up[10] = 1463473300;
		low[11] = up[10]; up[11] = 1463473500;
		low[12] = up[11]; up[12] = 1463473700;
		low[13] = up[12]; up[13] = 1463474000;
		low[14] = up[13]; up[14] = 1463474184;
	}

	double xmin = 1e16;
	double xmax = 0;

	double ymin = 1e16;
	double ymax = 0;

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

		// cout << "xmax: " << xmax << endl;
		// cout << "ymax: " << ymax << endl;
		if(sigmay>=35. && sigmay<=400)
		{
			for(int j = 0; j<15 ; ++j)
			{
			   if(ts>=low[j] && ts <up[j])
			   {
				   for(int k = 0; k< 76; ++k)
				   {
					   graphs.at(k)->GetPoint(i,x,y);
					   errx = graphs.at(k)->GetErrorX(i);
					   erry = graphs.at(k)->GetErrorY(i);


					   x /=1e6;
					   errx /=1e6;

					   y /=1e6;
					   erry /=1e6;

					   if(x < 1e25 && y < 1e25)
					   {
						   int n = graphs_clean.at(k)->GetN();

						   graphs_clean.at(k)->SetPoint(n, x, y );

						   graphs_clean.at(k)->SetPointError(n, errx, erry );

						//   cout<< "k: " << k<< ", x: " << x << ", y: " << y << endl;
							if(x<xmin) xmin=x;
							if(x>xmax) xmax=x;

							if(y-erry<ymin) ymin=y-erry;
							if(y+erry>ymax) ymax=y+erry;
						}
				   }

			   }
			}
		}
	}

	cout << "xmax: " << xmax << endl;
	cout << "ymax: " << ymax << endl;
	// Find the minimum Chi2

	TGraphErrors* graph_minp;
	TF1* 		  tf1_minp;

	double min_chi2 = 1e10;
	int min_i = 0;

	for(int i = 0; i < 16; ++i)
	{
		TF1* myfit = new TF1("fit", "[0]+[1]*x", 0.01*xmin, 100.*xmax);
		graphs_clean.at(i)->Fit(myfit, "Q","");

		if(myfit->GetChisquare()/myfit->GetNDF() < min_chi2)
		{
			min_i = i;
			min_chi2 = myfit->GetChisquare()/myfit->GetNDF();
			graph_minp = graphs_clean.at(i);
			tf1_minp = myfit;
		}
	}

	xmin = 1e16;
	xmax = 0;

	ymin = 1e16;
	ymax = 0;

	for(int i = 0 ; i<graph_minp->GetN(); ++i)
	{
		double x,y, errx, erry;

		graph_minp->GetPoint(i,x,y);
		errx = graph_minp->GetErrorX(i);
		erry = graph_minp->GetErrorY(i);

		if(x<xmin) xmin=x;
		if(x>xmax) xmax=x;

		if(y-erry<ymin) ymin=y-erry;
		if(y+erry>ymax) ymax=y+erry;
	}

	cout << "Min_i: " << min_i << ", min chi2/ndf: " << fixed << setprecision(5) << min_chi2 << endl;
	cout << "S_bg: " << tf1_minp->GetParameter(0) << " +- " <<  tf1_minp->GetParError(0) << ", S_T: "<< tf1_minp->GetParameter(1) << " +- " <<  tf1_minp->GetParError(1) << endl;
	cout << "NPoints: " << graph_minp->GetN() << endl;

    //rfile->Close();

	// Now get the plots for above and below
	Target target_below = GetTarget("BEAMSIZE_LER_BELOW");
	TFile* rfile_below = new TFile((target_below.pathin+"/"+target_below.filein).c_str(), "open");

	TGraphErrors* graph_below = (TGraphErrors*)rfile_below->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
	TGraphErrors* graph_below_clean = new TGraphErrors();


	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

		if(sigmay>=35. && sigmay<=400)
		{
			for(int j = 0; j<15 ; ++j)
			{
			   if(ts>=low[j] && ts <up[j])
			   {
				   graph_below->GetPoint(i,x,y);
					errx = graph_below->GetErrorX(i);
					erry = graph_below->GetErrorY(i);

					   x /=1e6;
					   errx /=1e6;

					   y /=1e6;
					   erry /=1e6;

						int n = graph_below_clean->GetN();

						graph_below_clean->SetPoint(n, x, y );

						graph_below_clean->SetPointError(n, errx, erry );

				   }

			   }
			}
		}

	Target target_above = GetTarget("BEAMSIZE_LER_ABOVE");
	TFile* rfile_above = new TFile((target_above.pathin+"/"+target_above.filein).c_str(), "open");

	TGraphErrors* graph_above  = (TGraphErrors*)rfile_above->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
	TGraphErrors* graph_above_clean = new TGraphErrors();

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

		if(sigmay>=35. && sigmay<=400)
		{
			for(int j = 0; j<15 ; ++j)
			{
			   if(ts>=low[j] && ts <up[j])
			   {
				   graph_above->GetPoint(i,x,y);
					errx = graph_above->GetErrorX(i);
					erry = graph_above->GetErrorY(i);

					   x /=1e6;
					   errx /=1e6;

					   y /=1e6;
					   erry /=1e6;

						int n = graph_above_clean->GetN();

						graph_above_clean->SetPoint(n, x, y );

						graph_above_clean->SetPointError(n, errx, erry );

				   }

			   }
			}
		}

		double mean_above = graph_above_clean->GetMean(2);

		for(int i = 0; i < graph_above_clean->GetN(); ++i)
		{
			double x,y, errx, erry;

			graph_above_clean->GetPoint(i,x,y);
			 errx = graph_above_clean->GetErrorX(i);
			 erry = graph_above_clean->GetErrorY(i);

			 graph_above_clean->SetPoint(i, x, y/mean_above);
			 graph_above_clean->SetPointError(i, errx, erry/mean_above);

		}

	//Now draw the whole shit

	graph_below_clean->SetMarkerSize(0);
	graph_below_clean->SetLineWidth(0);

	graph_below_clean->GetXaxis()->SetTitle("I/(PZ_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
	graph_below_clean->GetXaxis()->SetTitleOffset(1.15);
	graph_below_clean->GetYaxis()->SetTitle("Particle Rate/(IPZ_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");


	//graph_below_clean->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
	graph_below_clean->GetXaxis()->SetLimits(0, xmax+xmin*0.5);

	TLegend* leg;

	if(target.ring == "HER")
	{
		leg  =  new TLegend(0.21, 0.14, 0.37, 0.34);
		leg->SetBorderSize(0);
		leg->SetFillColor(0);
		leg->SetFillStyle(0);
		leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);

		if(channel == "FWD2"){
				graph_below_clean->GetYaxis()->SetRangeUser(ymin*0.26, ymax*1.15);
		}
		else
		{
				graph_below_clean->GetYaxis()->SetRangeUser(0, ymax*1.1);
		}
	}
	else{
		leg  =  new TLegend(0.21, 0.14, 0.37, 0.34);
		leg->SetBorderSize(0);
		leg->SetFillColor(0);
		leg->SetFillStyle(0);
		leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);

		graph_below_clean->GetYaxis()->SetRangeUser(0, ymax*1.1);

	}

	double mean_below = graph_below_clean->GetMean(2);
	cout << "mean_below: " << mean_below << endl;

	for(int i = 0; i < graph_below_clean->GetN(); ++i)
	{
		double x,y, errx, erry;

		graph_below_clean->GetPoint(i,x,y);
		errx = graph_below_clean->GetErrorX(i);
		erry = graph_below_clean->GetErrorY(i);

		graph_below_clean->SetPoint(i, x, y/mean_below);
		graph_below_clean->SetPointError(i, errx, erry/mean_below);

	}

	TF1* fit_below = new TF1("fit", "[0]*x+[1]", 0.01*xmin, 100.*xmax);
	TF1* fit_above = new TF1("fit", "[0]*x+[1]", 0.01*xmin, 100.*xmax);



	TLatex* ttext = new TLatex();
	ttext->SetNDC();
	ttext->SetTextFont(gStyle->GetLegendFont()+20);
	ttext->SetTextSize(gStyle->GetLegendTextSize());

	double markersize = 1.4;
	double markeralpha = 0.8;
	double linealpha = 0.8;
	double linewidth = 1;

	graph_below_clean->SetMarkerSize(markersize);
	graph_below_clean->SetMarkerColorAlpha(tab20_blue->GetNumber(), markeralpha);
	graph_below_clean->SetMarkerStyle(21);
	graph_below_clean->SetLineColorAlpha(tab20_blue->GetNumber(), linealpha);
	graph_below_clean->SetLineWidth(linewidth);
	fit_below->SetLineColorAlpha(tab20_blue->GetNumber(), linealpha);
	fit_below->SetLineWidth(2);
	fit_below->SetNpx(5000);

	graph_above_clean->SetMarkerSize(markersize);
	graph_above_clean->SetMarkerColorAlpha(tab20_red->GetNumber(), markeralpha);
	graph_above_clean->SetMarkerStyle(22);
	graph_above_clean->SetLineColorAlpha(tab20_red->GetNumber(), linealpha);
	graph_above_clean->SetLineWidth(linewidth);
	fit_above->SetLineColorAlpha(tab20_red->GetNumber(), linealpha);
	fit_above->SetLineWidth(2);
	fit_above->SetNpx(5000);

	graph_below_clean->Fit(fit_below, "N","");
	graph_above_clean->Fit(fit_above, "N","");

	// --- Get the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
	double ratio = 3./3.;

	string cantitle = "her_size_sweep" + channel;
	TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	can->SetFillColor(kWhite);
	can->SetBorderMode(0);
	can->SetBorderSize(2);
	can->SetFrameBorderMode(0);

	graph_below_clean->Draw("AP");

	TGraphErrors* graph_below_clean_clone  = (TGraphErrors*) graph_below_clean->Clone("");
	graph_below_clean_clone->SetMarkerStyle(25);
	graph_below_clean_clone->SetMarkerColorAlpha(kBlack, 1);

	graph_below_clean_clone->Draw("P");
	fit_below->Draw("same");

	graph_above_clean->Draw("P");

	TGraphErrors* graph_above_clean_clone  = (TGraphErrors*) graph_above_clean->Clone("");
	graph_above_clean_clone->SetMarkerStyle(26);
	graph_above_clean_clone->SetMarkerColorAlpha(kBlack, 1);

	graph_above_clean_clone->Draw("P");
	fit_above->Draw("same");

	ttext->DrawLatex(0.21, 0.89, (target_below.typein+": "+channel).c_str());


	can->SaveAs((target_below.pathout+"/"+target_below.fileout+"_"+channel+".pdf").c_str());
	can->SaveAs((target_below.pathout+"/"+target_below.fileout+"_"+channel+".png").c_str());
	can->SaveAs((target_below.pathout+"/"+target_below.fileout+"_"+channel+".eps").c_str());
	can->SaveAs((target_below.pathout+"/"+target_below.fileout+"_"+channel+".jpg").c_str());

}

void ler(string channel)
{
	Target target = GetTarget("BEAMSIZE_LER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    TGraphErrors* graph;
    TF1* function;
    double min_chi2 = 1e10;
    int min_i = 0;
    for(int i =0; i< 26; ++i)
    {
        TGraphErrors* gtmp = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(i)+"]").c_str());
        // cout << "(BS_Rate_+channel+_LP+to_string(i)).c_str(): " << ("BS_Rate_"+channel+"_LP["+to_string(i)+"]") << endl;
    //    cout << "Graph title: " << gtmp->GetTitle() << endl;

        TF1* ftmp = gtmp->GetFunction("f1");
		// cout << "i: " << i << ", chi2/ndf: " << ftmp->GetChisquare()/ftmp->GetNDF() << endl;

        if(ftmp->GetChisquare()/ftmp->GetNDF() < min_chi2)
        {
            min_i = i;
            min_chi2 = ftmp->GetChisquare()/ftmp->GetNDF();
            graph = gtmp;
            function = ftmp;

        }
//        cout << "ftmp->GetChisquare()/ftmp->GetNDF(): " << ftmp->GetChisquare()/ftmp->GetNDF() << endl;

    }

        cout << "Min_i: " << min_i << ", min chi2/ndf: " << min_chi2 << endl;

        TGraphErrors* g_sigmay = (TGraphErrors*)rfile->Get((target.ring+"_sigmay").c_str());
        TGraphErrors* g_current = (TGraphErrors*)rfile->Get((target.ring+"_Current").c_str());
        TGraphErrors* g_zeff = (TGraphErrors*)rfile->Get((target.ring+"_Zeff").c_str());
        TGraphErrors* g_p = (TGraphErrors*)rfile->Get(("BS_Rate_"+channel+"_LP["+to_string(min_i)+"]").c_str());
        TGraphErrors* g_obs = (TGraphErrors*)rfile->Get(("Rate "+ channel).c_str());

        double low[15];
        double up[15];

        if(target.ring == "HER"){
            low[0] = 1463446682; up[0] = 1463446972;
            low[1] = 1463446972; up[1] = 1463447382;
            low[2] = 1463447412; up[2] = 1463447603;
            low[3] = 1463447700; up[3] = 1463447880;
            low[4] = 1463447940; up[4] = 1463448112;
            low[5] = 1463449800; up[5] = 1463449980;
            low[6] = 1463450020; up[6] = 1463450410;
            low[7] = 1463450420; up[7] = 1463450800;
            low[8] = 1463450810; up[8] = 1463451090;
            low[9] = 1463451120; up[9] = 1463451300;
            low[10] = 1463452110; up[10] = 1463452350;
            low[11] = 1463452360; up[11] = 1463452620;
            low[12] = 1463452710; up[12] = 1463453060;
            low[13] = 1463453160; up[13] = 1463453710;
            low[14] = 1463453760; up[14] = 1463454000;
        }
        else if(target.ring == "LER")
        {
            // low[0] = 1463469869; up[0] = 1463470049;
            // low[1] = 1463470115; up[1] = 1463470295;
            // low[2] = 1463470361; up[2] = 1463470541;
            // low[3] = 1463470690; up[3] = 1463470870;
            // low[4] = 1463471041; up[4] = 1463471221;
            // low[5] = 1463471443; up[5] = 1463471623;
            // low[6] = 1463471688; up[6] = 1463471868;
            // low[7] = 1463472068; up[7] = 1463472248;
            // low[8] = 1463472308; up[8] = 1463472488;
            // low[9] = 1463472578; up[9] = 1463472758;
            // low[10] = 1463473067; up[10] = 1463473187;
            // low[11] = 1463473412; up[11] = 1463473532;
            // low[12] = 1463473643; up[12] = 1463473852;
            // low[13] = 1463473852; up[13] = 1463473972;
            // low[14] = 1463474064; up[14] = 1463474184;

            low[0] = 1463469869; up[0] = 1463470100;
            low[1] = up[0]; up[1] = 1463470400;
            low[2] = up[1]; up[2] = 1463470700;
            low[3] = up[2]; up[3] = 1463470910;
            low[4] = up[3]; up[4] = 1463471300;
            low[5] = up[4]; up[5] = 1463471640;
            low[6] = up[5]; up[6] = 1463471900;
            low[7] = up[6]; up[7] = 1463471960;
            low[8] = up[7]; up[8] = 1463472260;
            low[9] = up[8]; up[9] = 1463472800;
            low[10] = 1463472900; up[10] = 1463473300;
            low[11] = up[10]; up[11] = 1463473500;
            low[12] = up[11]; up[12] = 1463473700;
            low[13] = up[12]; up[13] = 1463474000;
            low[14] = up[13]; up[14] = 1463474184;
        }

        vector<TGraphErrors*> graphs;
        for(int i =0; i<16; ++i)
        {
            TGraphErrors* gtmp = new TGraphErrors();
            graphs.push_back(gtmp);
        }

        double xmin = 1e16;
        double xmax = 0;

        double ymin = 1e16;
        double ymax = 0;

        for(int i = 0 ; i<g_sigmay->GetN(); ++i)
        {
            double ts,sigmay, x,y, errx, erry;

            g_sigmay->GetPoint(i, ts, sigmay);
            // g_current->GetPoint(i, ts, I)
            // g_zeff->GetPoint(i, ts, zeff);
            // g_p->GetPoint(i, ts, p);
            // g_obs->GetPoint(i, ts, obs);
            // g_obs->GetPointError(i, errx erry);
             cout << fixed << setprecision(1) << "i: " << i << ", ts: " << ts << ", sigmay: " << sigmay << endl;
             graph->GetPoint(i,x,y);
             errx = graph->GetErrorX(i);
             erry = graph->GetErrorY(i);

             x /=1e6;
             errx /=1e6;

             y /=1e6;
             erry /=1e6;


                 if(sigmay>=35. && sigmay<=400)
                 {

                     for(int j = 0; j<15 ; ++j)
                     {
                        if(ts>=low[j] && ts <up[j])
                        {

                        int n = graphs[j]->GetN();
                        graphs[j]->SetPoint(n, x, y );

                        graphs[j]->SetPointError(n, errx, erry );


                        if(x<xmin) xmin=x;
                        if(x>xmax) xmax=x;

                        if(y-erry<ymin) ymin=y-erry;
                        if(y+erry>ymax) ymax=y+erry;
                    //}

                    n = graphs[15]->GetN();

                    graphs[15]->SetPoint(n, x, y );

                    graphs[15]->SetPointError(n, errx, erry );

                    }
                    }

                }

        }
        TF1* myfit = new TF1("fit", "[0]*x+[1]", 0.01*xmin, 100.*xmax);
        graphs[15]->Fit(myfit, "N","");

        cout << "Final result: Chi2: " << function->GetChisquare() << ", ndf: "<< function->GetNDF() << ", chi2/ndf: " <<  function->GetChisquare()/function->GetNDF()<< endl;

    //    gStyle->SetOptFit(1112);

        // --- Colors and Lines ---
        double markersize = 1.4;
        double markeralpha = 0.8;
        double linealpha = 0.8;
        double linewidth = 1;

        int shape1 = 22;
        int shape2 = 20;
        int shape3 = 21;

        int shape4 = 26;
        int shape5 = 4;
        int shape6 = 25;

        vector<int> colors;
        colors.push_back(tab20_orange->GetNumber());
        colors.push_back(tab20_blue->GetNumber());
        colors.push_back(tab20_green->GetNumber());
        colors.push_back(tab20_red->GetNumber());
        colors.push_back(tab20_lila->GetNumber());

        int cfit = kBlack;

        // Do the making the graph nice shit;

        for(int i =0; i<5; ++i)
        {
            graphs[i]->SetMarkerStyle(shape1);
            graphs[i]->SetMarkerSize(markersize);
            graphs[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
            graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
            graphs[i]->SetLineWidth(linewidth);

        }
        for(int i =5; i<10; ++i)
        {
            graphs[i]->SetMarkerStyle(shape2);
            graphs[i]->SetMarkerSize(markersize);
            graphs[i]->SetMarkerColorAlpha(colors[i%5],markeralpha);
            graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
            graphs[i]->SetLineWidth(linewidth);
        }
        for(int i =10; i<15; ++i)
        {
            graphs[i]->SetMarkerStyle(shape3);
            graphs[i]->SetMarkerSize(markersize);
            graphs[i]->SetMarkerColorAlpha(colors[i%5], markeralpha);
            graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
            graphs[i]->SetLineWidth(linewidth);
        }

        myfit->SetLineColor(cfit);
        myfit->SetLineWidth(2);
        myfit->SetNpx(4000);
    // --- Build Axis ---

    TGraphErrors* axis = graphs[15];
    // graphs[0]->Draw("ap");
    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
    TLegend* leg;
    if(target.ring == "HER")
    {
        leg  =  new TLegend(0.21, 0.16, 0.37, 0.32);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);

        if(channel == "FWD2"){
                axis->GetYaxis()->SetRangeUser(ymin*0.26, ymax*1.15);
        }
        else
        {
                axis->GetYaxis()->SetRangeUser(ymin*0.38, ymax*1.15);
        }
    }
    else if(target.ring == "LER")
    {
        if(channel == "FWD1"){
            //    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
                axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
        }
        else if(channel == "FWD2"){
            //    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
                axis->GetYaxis()->SetRangeUser(0, ymax*1.05);
        }
        else if(channel == "FWD3"){
                axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*2.5);
                axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
        }

        leg  =  new TLegend(0.37, 0.16, 0.53, 0.32);
        leg->SetBorderSize(0);
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);;
    }


    // axis->GetYaxis()->SetRangeUser(ymin*0.4, ymax*1.15);
    axis->SetMarkerSize(0);
    axis->SetLineWidth(0);

    axis->GetXaxis()->SetTitle("I/(PZ_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
    axis->GetXaxis()->SetTitleOffset(1.15);
    axis->GetYaxis()->SetTitle("Particle Rate/(IPZ_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");
    // axis->GetXaxis()->SetTitle("#frac{I}{PZ_{e}^{2}#sigma_{y}} [mA#upointPa^{-1}#upoint#mum^{-1}]");
    // axis->GetYaxis()->SetTitle("#frac{Particle Rate}{IPZ_{e}^{2}} [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");

    // Legend and Text
    //
    // TLegend* leg =  new TLegend(0.21, 0.16, 0.37, 0.32);
    // leg->SetBorderSize(0);
    // leg->SetFillColor(0);
    // leg->SetFillStyle(0);
    // leg->SetTextSize(gStyle->GetLegendTextSize()*0.8);
    // //leg->SetNColumns(3);

    leg->AddEntry(set_line_width(myfit), "Fit: #frac{Particle Rate}{IP_{e}Z_{e}^{2}} = S_{bg} + S_{t}#upoint#frac{I}{P_{e}Z_{e}^{2}#sigma_{y}}","l");
    stringstream ss;
    ss << fixed << setprecision(1) << "S_{bg} = (" << myfit->GetParameter(1) << " #pm "  << myfit->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
    leg->AddEntry((TObject*)0, ss.str().c_str(), "");
    ss.str("");

	ss << fixed << setprecision(1) << "S_{T} = (" << myfit->GetParameter(0) << " #pm "  << myfit->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-2}";
    //     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
    leg->AddEntry((TObject*)0, ss.str().c_str(), "");

    TLatex* text = new TLatex();
    text->SetNDC();
    text->SetTextFont(gStyle->GetLegendFont()+20);
    text->SetTextSize(vis_multi*textsize*pt2pixel);

    // --- Get the canvas ---
    int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "ler_size_sweep_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    // Do the Drawing shit!!!
    axis->Draw("AP");

    cout << "x = [" << xmin<<", "<<xmax<<"]"<<endl;
    cout << "y = [" << ymin<<", "<<ymax<<"]"<<endl;
    for(int i =0; i<15; ++i)
    {

        graphs[i]->Draw("p");
    }

    for(int i =0; i<5; ++i)
    {
        graphs[i] = (TGraphErrors*) graphs[i]->Clone("");
        graphs[i]->SetMarkerStyle(shape4);
        // graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
        graphs[i]->SetMarkerColorAlpha(kBlack, 1);
    //    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

    }
    for(int i =5; i<10; ++i)
    {
        graphs[i] = (TGraphErrors*) graphs[i]->Clone("");
        graphs[i]->SetMarkerStyle(shape5);
        // graphs[i]->SetMarkerColorAlpha(colors[i%5],1);
        graphs[i]->SetMarkerColorAlpha(kBlack, 1);
    //    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
    }
    for(int i =10; i<15; ++i)
    {
        graphs[i] = (TGraphErrors*) graphs[i]->Clone("");
        graphs[i]->SetMarkerStyle(shape6);
        // graphs[i]->SetMarkerColorAlpha(colors[i%5], 1);
        graphs[i]->SetMarkerColorAlpha(kBlack, 1);
    //    graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);

    }

    for(int i =0; i<15; ++i)
    {

        graphs[i]->Draw("p");
    }

    cout << "f(xmin): " << function->Eval(xmin) << ",  f(xmax): " << function->Eval(xmax) << endl;
    myfit->Draw("same");
    // function->SetLineColor(kBlack);
    // function->Draw("same");
    // text->DrawLatex(0.21, 0.35, "HER Size Sweep");
    text->DrawLatex(0.21, 0.89, (target.typein+": "+channel).c_str());
    leg->Draw();


    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".pdf").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".png").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".eps").c_str());
    can->SaveAs((target.pathout+"/"+target.fileout+"_"+channel+".jpg").c_str());

    rfile->Close();
}

void plt_beam_size()
{

    SetPhDStyle();

     string chs[] = {"FWD1","FWD2","FWD3"};
//    string chs[] = {"FWD1"};
    // Target target = GetTarget("BEAMSIZE_LER");
    for(auto& ch: chs)
    {
      // her(ch);
	  // ler(ch);
	  // her_new(ch);
	   ler_new(ch);
	 //	her_tr(ch);
	 //  ler_tr(ch);
    }

}
