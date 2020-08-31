#include <vector>

struct FitResult
{
	double s_bg;
	double s_bg_e;
	double s_bg_std;
	double s_t;
	double s_t_e;
	double s_t_std;
	int i_ccg;
	double p_ratio;
	double min_chi2;
	int min_ndf;
};

// gROOT->ProcessLine("gErrorIgnoreLevel = kFatal;");

FitResult scatter_her(string channel)
{
	Target target = GetTarget("BEAMSIZE_HER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    vector<TGraphErrors*> 	graphs;
	vector<TGraphErrors*> 	graphs_clean;
    TF1* 					function;

	std::vector<float> offsets;

	std::vector<float> slopes;


	std::vector<unsigned int> excluded_ccgs = {45,47,52,53,54,55,58,59,60};

	unsigned int n_P_HER = 50;

	for(int i =0; i< n_P_HER; ++i)
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
				   for(int k = 0; k< n_P_HER; ++k)
				   {
					   graphs.at(k)->GetPoint(i,x,y);
					   errx = graphs.at(k)->GetErrorX(i);
					   erry = graphs.at(k)->GetErrorY(i);

					   // x /=1e6;
					   // errx /=1e6;
					   //
					   // y /=1e6;
					   // erry /=1e6;

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

	for(int i = 0; i < n_P_HER; ++i)
	{
		bool is_nan = false;
		for(int j = 0 ;j < graphs_clean.at(i)->GetN(); ++j)
		{
			double x,y;
			graphs_clean.at(i)->GetPoint(j,x,y);

			if (x != x || y != y)
			{
				is_nan = true;
			}

			//cout << "i: " << i << ", x: " << x << ", y: " << y << endl;

		}


		if(is_nan )
		{
			cout << "NAN IS TRUE FOR i: " << i << endl;
			break;
		}

		 bool exclude = false;
		for(auto itExcludedCCG : excluded_ccgs) {
		  if(i ==itExcludedCCG) {
			exclude = true;
			break;
		  }
		}
		if(exclude)
		  continue;

		if(graphs_clean.at(i)->GetN()>5){

			TF1* myfit = new TF1("fit", "[0]+[1]*x", 0.01*xmin, 100.*xmax);
			int status = graphs_clean.at(i)->Fit(myfit, "Q","");

			// cout << "i: " << i << ", N_p: " << graphs_clean.at(i)->GetN() << ", chi2/ndf: " << myfit->GetChisquare()/myfit->GetNDF() << ", slope significance: " << myfit->GetParameter(1)/myfit->GetParError(1) << ", Status: " << status << endl;

			if(myfit->GetChisquare()/myfit->GetNDF() < min_chi2 && status == 0)
			{
				min_i = i;
				min_chi2 = myfit->GetChisquare()/myfit->GetNDF();
				graph_minp = graphs_clean.at(i);
				tf1_minp = myfit;
			}

			if( myfit->GetChisquare()/myfit->GetNDF() < 1.5 && myfit->GetParameter(1)/myfit->GetParError(1) > 1.0) {
			  slopes.push_back(myfit->GetParameter(1));
			  offsets.push_back(myfit->GetParameter(0));
			  if(channel == "FWD1")
			  {
				  cout << "i: " << i << ", offset: " << myfit->GetParameter(0) << ", offset error: " << myfit->GetParError(0) << ", slope: " << myfit->GetParameter(1) << ", slope error: " << myfit->GetParError(1) << ", slope sig: " <<  myfit->GetParameter(1)/myfit->GetParError(1) << endl;
			  }
	//  cout << "iChan: " << iChan << ", iPres: " << iPres <<  ", chisquared: " << chisquared << " , slope_significance: " << slope_significance << endl;
				}

			// if( )
			// {
			// 	min_i = i;
			// 	min_chi2 = myfit->GetChisquare()/myfit->GetNDF();
			// 	graph_minp = graphs_clean.at(i);
			// 	tf1_minp = myfit;
			// }
		}

	}

	cout << "\n" << "HER: for channel " << channel << endl;
	cout << "Min_i: " << min_i << ", min chi2/ndf: " << fixed << setprecision(5) << min_chi2 << ", fit prob: " << tf1_minp->GetProb() << endl;
	cout << "NPoints: " << graph_minp->GetN() << endl;

	cout << "OFFSET MEAN: " << TMath::Mean(offsets.size(),&offsets[0]) << endl;
	cout << "OFFSET STD: " << TMath::RMS(offsets.size(),&offsets[0]) << endl;

	cout << "SLOPES Mean: " << TMath::Mean(slopes.size(),&slopes[0]) << endl;
	cout << "SLOPES STD: " << TMath::RMS(slopes.size(),&slopes[0]) << endl;

	cout << fixed << setprecision(3) << "S_{bg} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}" << endl;;
	cout << fixed << setprecision(3) << "S_{T} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-2}" << "\n"<<  endl;;

	FitResult rtn;
	rtn.s_bg = tf1_minp->GetParameter(0);
	rtn.s_bg_e = tf1_minp->GetParError(0);
	rtn.s_t = tf1_minp->GetParameter(1);
	rtn.s_t_e = tf1_minp->GetParError(1);
	rtn.i_ccg = min_i;
	rtn.min_chi2 = tf1_minp->GetChisquare();
	rtn.min_ndf = tf1_minp->GetNDF();

	rtn.s_t_std = TMath::RMS(slopes.size(),&slopes[0]);
	rtn.s_bg_std = TMath::RMS(offsets.size(),&offsets[0]);


	// Get the pressure ratio for the respective CCG

	// Open and read file and dump into positions vector
	std::ifstream basefile("SlowResults/BeamSize/all_ccgs_claws_only.txt", std::ios_base::in);

	if(basefile) {
  		std::string line, substr;

	  	while(std::getline(basefile,line)) {
			if(line[0]=='#' || line.empty()) continue;
			std::istringstream iss(line);


			string branch;
			unsigned int tmp_ch;
			unsigned int tmp_i_ccg;
			double tmp_slope_significance;
			double tmp_pressure_ratio;
			double tmp_pressure_ratio_error;

			iss >> branch >> tmp_ch >> tmp_i_ccg >> tmp_slope_significance >> tmp_pressure_ratio >> tmp_pressure_ratio_error;

			if(channel == "FWD1")
			{
				if(tmp_ch == 0 && tmp_i_ccg == min_i)
				{
					rtn.p_ratio = tmp_pressure_ratio;
					break;
				}
			}
			else if(channel == "FWD2")
			{
				if(tmp_ch == 1 && tmp_i_ccg == min_i)
				{
					rtn.p_ratio = tmp_pressure_ratio;
					break;
				}
			}
			else if(channel == "FWD3")
			{
				if(tmp_ch == 2 && tmp_i_ccg == min_i)
				{
					rtn.p_ratio = tmp_pressure_ratio;
					break;
				}
			}
			else
			{
				rtn.p_ratio = -1;
			}
			//cout << "branch: " << branch << "\ntmp_ch: " << tmp_ch << "\ntmp_i_ccg: " << tmp_i_ccg << "\ntmp_slope_significance: " << tmp_slope_significance << "\ntmp_pressure_ratio: " << tmp_pressure_ratio << "\ntmp_pressure_ratio_error: " << tmp_pressure_ratio_error << endl;
	//		bool is_HER = tmp_xer.find("HER")!=std::string::npos;
	}
	   	// if(is_HER)
	  	// base_P_HER.push_back(tmp_baseP);
		//
		// std::cout << tmp_xer << " " << tmp_chan << " " << tmp_baseP << std::endl;
	  	// }
	  	// std::cout << "Base pressures loaded successfully" << std::endl;
	} // END if basefile
	else
	std::cout << "Base pressures not loaded! File not opened." << std::endl;


	// rtn.p_ratio = 3.24152;

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

	 	// graphs.at(min_i)->GetPoint(i,x,y);
	 	// errx = graphs.at(min_i)->GetErrorX(i);
	 	// erry = graphs.at(min_i)->GetErrorY(i);
	//	cout << "sigmay: " << sigmay << endl;

		graphs.at(min_i)->GetPoint(i,x,y);
		errx = graphs.at(min_i)->GetErrorX(i);
		erry = graphs.at(min_i)->GetErrorY(i);

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

	double s_bg = tf1_minp->GetParameter(0);
	double s_bg_e = tf1_minp->GetParError(0);

	// double s_t = tf1_minp->GetParameter(1);
	// double s_t_e = tf1_minp->GetParError(1);

	tf1_minp->SetParameter(0, s_bg/1e6);
	tf1_minp->SetParError(0, s_bg_e/1e6);

	// tf1_minp->SetParameter(1, s_t);

	tf1_minp->SetLineColor(cfit);
	tf1_minp->SetLineWidth(2);
	tf1_minp->SetNpx(4000);

	tf1_minp->SetRange(0, 1000);

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

	axis->GetXaxis()->SetTitle("I/(P_{e}Z_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
	axis->GetXaxis()->SetTitleOffset(1.15);
	axis->GetYaxis()->SetTitle("R/(IP_{e}Z_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");

	leg->AddEntry(set_line_width(tf1_minp), "Combined Parameterization:","l");
	//leg->AddEntry(set_line_width(tf1_minp), "Fit: #frac{Particle Rate}{IP_{e}Z_{e}^{2}} = S_{bg} + S_{t}#upoint#frac{I}{P_{e}Z_{e}^{2}#sigma_{y}}","l");
	stringstream ss;
	ss << fixed << setprecision(0) << "S_{bg} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
	ss.str("");

	ss << fixed << setprecision(1) << "S_{T} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointm#upointmA^{-2}";
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

		return rtn;
}

FitResult scatter_ler(string channel)
{
	Target target = GetTarget("BEAMSIZE_LER");

	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

    vector<TGraphErrors*> 	graphs;
	vector<TGraphErrors*> 	graphs_clean;

    TF1* 			function;

	for(int i =0; i< 26; ++i)
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
				   for(int k = 0; k< 26; ++k)
				   {
					   graphs.at(k)->GetPoint(i,x,y);
					   errx = graphs.at(k)->GetErrorX(i);
					   erry = graphs.at(k)->GetErrorY(i);


					   // x /=1e6;
					   // errx /=1e6;
					   //
					   // y /=1e6;
					   // erry /=1e6;

					   if(x < 1e50 && y < 1e50)
					   {

						   	if(x < 200000. && k ==3)
						   	{
						   			cout << "WAIT! k: " << k << ", x: " << x << ", y: " << y << endl;
						   		}

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




	double min_chi2 = 1e10;
	int min_i = 0;

	TGraphErrors* graph_minp;
	TF1* 		  tf1_minp;

	for(int i = 0; i < 26; ++i)
	{

		bool is_nan = false;
		for(int j = 0 ;j < graphs_clean.at(i)->GetN(); ++j)
		{
			double x,y;
			graphs_clean.at(i)->GetPoint(j,x,y);

			if (x != x || y != y)
			{
				is_nan = true;
			}

			//cout << "i: " << i << ", x: " << x << ", y: " << y << endl;

		}


		if(is_nan )
		{
			cout << "NAN IS TRUE FOR i: " << i << endl;
			break;
		}

		// if(graphs_clean.at(i)->GetN()>5){
		//
		// 	TF1* myfit = new TF1("fit", "[0]+[1]*x", 0.01*xmin, 100.*xmax);
		// 	graphs_clean.at(i)->Fit(myfit, "Q","");
		//
		// 	if(myfit->GetChisquare()/myfit->GetNDF() < min_chi2)
		// 	{
		// 		min_i = i;
		// 		min_chi2 = myfit->GetChisquare()/myfit->GetNDF();
		// 		graph_minp = graphs_clean.at(i);
		// 		tf1_minp = myfit;
		// 	}
		//
		// }
	}

	TF1* myfit = new TF1("fit", "[0]+[1]*x", 0.01*xmin, 100.*xmax);
	graphs_clean.at(0)->Fit(myfit, "Q","");
	graph_minp = graphs_clean.at(0);
	tf1_minp = myfit;
	min_i = 0;
	min_chi2 = myfit->GetChisquare()/myfit->GetNDF();

	cout << "\n" << "LER: for channel " << channel << endl;
	cout << "Min_i: " << min_i << ", min chi2/ndf: " << fixed << setprecision(5) << tf1_minp->GetChisquare()/tf1_minp->GetNDF() << ", fit prob: " << tf1_minp->GetProb()<< endl;
	cout << "NPoints: " << graph_minp->GetN() << endl;
	cout << fixed << setprecision(3) << "S_{bg} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}" << endl;;
	cout << fixed << setprecision(3) << "S_{T} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointmA^{-2}" << "\n"<<  endl;;

	FitResult rtn;
	rtn.s_bg = tf1_minp->GetParameter(0);
	rtn.s_bg_e = tf1_minp->GetParError(0);
	rtn.s_t = tf1_minp->GetParameter(1);
	rtn.s_t_e = tf1_minp->GetParError(1);
	rtn.i_ccg = 0;
	rtn.min_chi2 = tf1_minp->GetChisquare();
	rtn.min_ndf = tf1_minp->GetNDF();
	rtn.p_ratio = 3.24152;

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

// /	cout << "g_sigmay->GetN(): " <<g_sigmay->GetN() << ", graph_minp->GetN(): " << graph_minp->GetN() <<endl;

	for(int i = 0 ; i<g_sigmay->GetN(); ++i)
	{
		double ts,sigmay, x,y, errx, erry;

		g_sigmay->GetPoint(i, ts, sigmay);

	 	// graph_minp->GetPoint(i,x,y);
	 	// errx = graph_minp->GetErrorX(i);
	 	// erry = graph_minp->GetErrorY(i);

		graphs.at(min_i)->GetPoint(i,x,y);
		errx = graphs.at(min_i)->GetErrorX(i);
		erry = graphs.at(min_i)->GetErrorY(i);

	//	cout << "sigmay: " << sigmay << endl;

        x /=1e6;
        errx /=1e6;

        y /=1e6;
        erry /=1e6;

	//	cout << "x: " << x << ", y: " << y << endl;

		if(sigmay>=35. && sigmay<=400)
		{

			for(int j = 0; j<15 ; ++j)
			{

				if(ts>=low[j] && ts <up[j])
				{

					if(x < 0.05)
					{
							cout << "WAIT! i: " << i << ", x: " << x << ", y: " << y << endl;
					}


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
	//
	// int width2 = round(vis_multi*textwidth*pt2pixel*1/2.);
	// double ratio2 = 3./3.;
	//
	// string cantitle2 = "DEBUG" + channel;
	// TCanvas * can2 = new TCanvas(cantitle2.c_str(),cantitle2.c_str(), width2, width2/ratio2);
	// graphs_subruns[0]->Draw();
	// for(int i = 1 ; i<15; ++i)
	// {
	// 	graphs_subruns[i]->Draw("same");
	// }


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

	double s_bg = tf1_minp->GetParameter(0);
	double s_bg_e = tf1_minp->GetParError(0);

	// double s_t = tf1_minp->GetParameter(1);
	// double s_t_e = tf1_minp->GetParError(1);

	tf1_minp->SetParameter(0, s_bg/1e6);
	tf1_minp->SetParError(0, s_bg_e/1e6);

	// tf1_minp->SetParameter(1, s_t);

	tf1_minp->SetLineColor(cfit);
	tf1_minp->SetLineWidth(2);
	tf1_minp->SetNpx(4000);

	tf1_minp->SetRange(0, 1000);

	tf1_minp->SetLineColor(cfit);
	tf1_minp->SetLineWidth(2);
	tf1_minp->SetNpx(4000);


	TGraphErrors* axis = graph_minp;
	// graphs[0]->Draw("ap");
	axis->GetXaxis()->SetLimits(0, xmax+0.1);

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
				axis->GetYaxis()->SetRangeUser(0, ymax*1.15);
				leg  =  new TLegend(0.43, 0.16, 0.59, 0.33);
		}
		else if(channel == "FWD2"){
			//    axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*0.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.4);
				leg  =  new TLegend(0.21, 0.7, 0.37, 0.87);
		}
		else if(channel == "FWD3"){
			//	axis->GetXaxis()->SetLimits(xmin*0.5, xmax+xmin*2.5);
			axis->GetXaxis()->SetLimits(0, xmax+xmin*0.5);
				axis->GetYaxis()->SetRangeUser(0, ymax*1.45);
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

	axis->GetXaxis()->SetTitle("I/(P_{e}Z_{e}^{2}#sigma_{y}) [mA#upointPa^{-1}#upointm^{-1} ]");
	axis->GetXaxis()->SetTitleOffset(1.1);
	axis->GetYaxis()->SetTitle("R/(IP_{e}Z_{e}^{2}) [MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}]");

	leg->AddEntry(set_line_width(tf1_minp), "Combined Parameterization:","l");
	//leg->AddEntry(set_line_width(tf1_minp), "Fit: #frac{Particle Rate}{IP_{e}Z_{e}^{2}} = S_{bg} + S_{t}#upoint#frac{I}{P_{e}Z_{e}^{2}#sigma_{y}}","l");
	if(channel == "FWD2")
	{
		stringstream ss;
		ss << fixed << setprecision(2) << "S_{bg} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
		leg->AddEntry((TObject*)0, ss.str().c_str(), "");
		ss.str("");

		ss << fixed << setprecision(1) << "S_{T} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointm#upointmA^{-2}";
		//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
		leg->AddEntry((TObject*)0, ss.str().c_str(), "");
	}
	else if(channel == "FWD3")
	{
		stringstream ss;
		ss << fixed << setprecision(2) << "S_{bg} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
		leg->AddEntry((TObject*)0, ss.str().c_str(), "");
		ss.str("");

		ss << fixed << setprecision(1) << "S_{T} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointm#upointmA^{-2}";
		//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
		leg->AddEntry((TObject*)0, ss.str().c_str(), "");
	}
	else
	{
		stringstream ss;
		ss << fixed << setprecision(1) << "S_{bg} = (" << tf1_minp->GetParameter(0) << " #pm "  << tf1_minp->GetParError(0) << ") MMIP#upoints^{-1}#upointmA^{-1}#upointPa^{-1}";
		leg->AddEntry((TObject*)0, ss.str().c_str(), "");
		ss.str("");

		ss << fixed << setprecision(1) << "S_{T} = (" << tf1_minp->GetParameter(1) << " #pm "  << tf1_minp->GetParError(1) << ") MMIP#upoints^{-1}#upointm#upointmA^{-2}";
		//     sprintf(my_char,"#mu = %.2f mV",f1->GetParameter(1));
		leg->AddEntry((TObject*)0, ss.str().c_str(), "");
	}


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
		// graphs[i]->SetLineColorAlpha(colors[i%5], linealpha);
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

	return rtn;

}



void plt_beam_size_2()
{

    SetPhDStyle();

 	gErrorIgnoreLevel = 1001;

	TChain *ch_mc = new TChain("truth");
	ch_mc->Add("SlowResults/BeamSize/mc_v7.3.root");

	string chs[] = {"FWD1","FWD2","FWD3"};
//    string chs[] = {"FWD3"};

	std::string times = "*";
	std::string divide = "/";
	std::string vs = ":";
	std::string squared = "^2";
	std::string correction_factor = "1";

	double I_SAD = 1000.;
	double Z_SAD_LER = 1.;
	double Z_SAD_HER = 2.7;
	double sigma_y_SAD_HER = 59;
	double sigma_y_SAD_LER = 110;
	double P_SAD = 0.00000133322; // 10 nTorr in Pa (from paper)
	//  double P_SAD = 1.33322; // 10 mTorr in Pa (from Igal's email)

	//  double mc_factor = 500.*1.576; // Simu for 2ms plus Nbunch factor 1576/1000
	double mc_factor = 1;
	double brem_factor    = 0.935;  // f_B(Z)/f_B(Z_SAD) for Z=2.7
	double coulomb_factor = 1.0;    // f_C(Z)/f_B(Z_SAD) for Z=2.7

	std::vector<double> O_bg_data_LER, O_bg_e_data_LER, O_T_data_LER, O_T_e_data_LER;
	std::vector<double> O_bg_MC_LER, O_T_MC_LER;
	//
	// double r_ratio_ler = 3.4;
	// double r_ratio_her_1 = 3.4;

	vector<TGraphErrors*> ratios_bg;
	vector<TGraphErrors*> ratios_t;
	double gr_step = 0.27;
	double gr_offset = 1;



    for(int iCh = 0 ; iCh <3 ; ++iCh)
    {
		TGraphErrors* tmp_gr_bg = new TGraphErrors();
		tmp_gr_bg->SetName((chs[iCh]+"_bg").c_str());
		tmp_gr_bg->SetTitle((chs[iCh]+"_bg").c_str());

		TGraphErrors* tmp_gr_t = new TGraphErrors();
		tmp_gr_t->SetName((chs[iCh]+"_t").c_str());
		tmp_gr_t->SetTitle((chs[iCh]+"_t").c_str());
      // her(ch);
	  // ler(ch);
	  // scatter_her(ch);

	  // *** Get ratios for the LER ***
	  FitResult ler = scatter_ler(chs[iCh]);

	  if(true)
	  {
		  double O_bg_data   = ler.s_bg*I_SAD*P_SAD*ler.p_ratio*Z_SAD_LER*Z_SAD_LER;
		  double O_T_data    = ler.s_t*I_SAD*I_SAD/sigma_y_SAD_LER;
		  double O_bg_e_data = ler.s_bg_e*I_SAD*P_SAD*ler.p_ratio*Z_SAD_LER*Z_SAD_LER;
		  double O_T_e_data  = ler.s_t_e*I_SAD*I_SAD/sigma_y_SAD_LER;

		  std::cout << "  LER data: " << std::endl;
		  std::cout << "    O_bg:   " << O_bg_data << " +/- " << O_bg_e_data << std::endl;
		  std::cout << "    O_T:    " << O_T_data <<  " +/- " << O_T_e_data << std::endl;


		  // Now get the MC observable
		  double sum_lb = 0, sum_lc = 0, sum_lt = 0;

		  for(unsigned int iSec = 0; iSec<12; ++iSec)
		  {
			  std::stringstream plot_ss, cut_ss;
			  std::stringstream O_brem_ss, O_coulomb_ss, O_T_ss;

			  O_brem_ss    << "MC_LB_CLAWS_rate" << "_" << iSec << "[" << iCh << "]" << divide << correction_factor;
	          O_coulomb_ss << "MC_LC_CLAWS_rate" << "_" << iSec << "[" << iCh << "]" << divide << correction_factor;
	          O_T_ss       << "MC_LT_CLAWS_rate" << "[" << iCh << "]" << divide << correction_factor;

			  plot_ss << O_brem_ss.str() << vs << O_coulomb_ss.str() << vs << O_T_ss.str();
			  cut_ss << "Entry$==0";

			  //	  std::cout << plot_ss.str() << std::endl;

			  ch_mc->Draw(plot_ss.str().c_str(),cut_ss.str().c_str(),"NODRAW");
			  double *v1 = ch_mc->GetV1();
			  double *v2 = ch_mc->GetV2();
			  double *v3 = ch_mc->GetV3();

			  sum_lb += v1[0]*mc_factor*brem_factor;
			  sum_lc += v2[0]*mc_factor*coulomb_factor;
			  sum_lt  = v3[0]*mc_factor; // Touschek is not separated by section

		  } // END for iSec

		  std::cout << "  LER MC: " << std::endl;
		  std::cout << "    O_bg:   " << sum_lb+sum_lc << std::endl;
		  std::cout << "    O_T:    " << sum_lt << std::endl;

		  // O_bg_MC_LER.push_back(sum_lb+sum_lc);
		  // O_T_MC_LER.push_back(sum_lt);

		  double O_bg_mc = sum_lb+sum_lc;
		  double O_T_mc = sum_lt;

		  std::cout << "  LER Ratios: " << std::endl;
		  std::cout << "  O_bg_data/O_bg_mc:   " << O_bg_data/O_bg_mc << " +/- "<< O_bg_e_data/O_bg_mc << ", excess: " << (O_bg_data/O_bg_mc - 1.)/(O_bg_e_data/O_bg_mc) << " sigma" << std::endl;
		  std::cout << "  O_t_data/O_t_mc:    " << O_T_data/O_T_mc << " +/- "<< O_T_e_data/O_T_mc << ", excess: " << (O_T_data/O_T_mc - 1.)/(O_T_e_data/O_T_mc) << " sigma" << std::endl;
		   // ratios(ler);


		  int n = tmp_gr_bg->GetN();
		  tmp_gr_bg->SetPoint(n, O_bg_data/O_bg_mc, (iCh-1)*gr_step);
		  tmp_gr_bg->SetPointError(n, O_bg_e_data/O_bg_mc, gr_step/2.);

		  n = tmp_gr_t->GetN();
		 tmp_gr_t->SetPoint(n, O_T_data/O_T_mc, (iCh-1)*gr_step);
		 tmp_gr_t->SetPointError(n, O_T_e_data/O_T_mc, gr_step/2.);

		  cout << "For LER " << chs[iCh] << " x = " << O_bg_data/O_bg_mc << " +/- "<< O_bg_e_data/O_bg_mc << ", y: " <<  (iCh-1)*gr_step << std::endl;
  	  }

	  // *** Get ratios for the HER ***
	  FitResult her = scatter_her(chs[iCh]);

	  cout << "her.min_i_ccg: " << her.p_ratio << endl;
	  if(true)
	  {


		  double O_bg_data   = her.s_bg*I_SAD*P_SAD*her.p_ratio*Z_SAD_HER*Z_SAD_HER;
		  double O_bg_e_raw =  her.s_bg_e*I_SAD*P_SAD*her.p_ratio*Z_SAD_HER*Z_SAD_HER;
		  // double O_bg_e = TMath::Sqrt(her.s_bg_e*her.s_bg_e+her.s_bg_std*her.s_bg_std)*I_SAD*P_SAD*her.p_ratio*Z_SAD_HER*Z_SAD_HER;
		  double O_bg_e = O_bg_e_raw;

		  double O_T_data    = her.s_t*I_SAD*I_SAD/sigma_y_SAD_HER;
		  double O_T_e_raw  = her.s_t_e*I_SAD*I_SAD/sigma_y_SAD_HER;
		  //double O_T_e  = TMath::Sqrt(her.s_t_e*her.s_t_e+ her.s_t_std*her.s_t_std)*I_SAD*I_SAD/sigma_y_SAD_HER;
		  double O_T_e = O_T_e_raw;

		  cout << "S_bg: " <<her.s_bg << endl;
		  cout << "S_t: " << her.s_t << endl;
		  cout << "p_ratio: " << her.p_ratio << endl;

		  std::cout << "  HER data: " << std::endl;
		  std::cout << "    O_bg:   " << O_bg_data << " +/- " << O_bg_e << std::endl;
		  std::cout << "    O_T:    " << O_T_data <<  " +/- " << O_T_e << std::endl;


		  // Now get the MC observable
		  double sum_hb = 0, sum_hc = 0, sum_ht = 0;

		  for(unsigned int iSec = 0; iSec<12; ++iSec)
		  {
			  std::stringstream plot_ss, cut_ss;
			  std::stringstream O_brem_ss, O_coulomb_ss, O_T_ss;

			  O_brem_ss    << "MC_HB_CLAWS_rate_" << iSec << "[" << iCh << "]" << divide << correction_factor;
			  O_coulomb_ss << "MC_HC_CLAWS_rate_" << iSec << "[" << iCh << "]" << divide << correction_factor;
			  O_T_ss       << "MC_HT_CLAWS_rate" << "[" << iCh << "]" << divide << correction_factor;

			  plot_ss << O_brem_ss.str() << vs << O_coulomb_ss.str() << vs << O_T_ss.str();
			  cut_ss << "Entry$==0";

			  //	  std::cout << plot_ss.str() << std::endl;

			  ch_mc->Draw(plot_ss.str().c_str(),cut_ss.str().c_str(),"NODRAW");
			  double *v1 = ch_mc->GetV1();
			  double *v2 = ch_mc->GetV2();
			  double *v3 = ch_mc->GetV3();

			  sum_hb += v1[0]*mc_factor*brem_factor;
			  sum_hc += v2[0]*mc_factor*coulomb_factor;
			  sum_ht = v3[0]*mc_factor; // Touschek is not separated by section

		  } // END for iSec

		  double O_bg_mc = sum_hb+sum_hc;
		  double O_T_mc = sum_ht;

			std::cout << "  HER MC: " << std::endl;
			std::cout << "    O_bg:   " << sum_hb+sum_hc << std::endl;
			std::cout << "    O_T:    " << sum_ht << std::endl;

			std::cout << "  HER Ratios: " << std::endl;
			std::cout << "  O_bg_data/O_bg_mc:   " << O_bg_data/O_bg_mc << " +/- "<< O_bg_e/O_bg_mc << ", excess: " << (O_bg_data/O_bg_mc - 1.)/(O_bg_e/O_bg_mc) << " sigma" <<  std::endl;
			std::cout << "  O_t_data/O_t_mc:    " << O_T_data/O_T_mc << " +/- "<< O_T_e/O_T_mc << ", excess: " << (O_T_data/O_T_mc - 1.)/(O_T_e/O_T_mc) << " sigma" <<  std::endl;

			int n = tmp_gr_bg->GetN();
			tmp_gr_bg->SetPoint(n, O_bg_data/O_bg_mc, (iCh-1)*gr_step+gr_offset);
			tmp_gr_bg->SetPointError(n, O_bg_e/O_bg_mc, gr_step/2.);

			n = tmp_gr_t->GetN();
			tmp_gr_t->SetPoint(n, O_T_data/O_T_mc, (iCh-1)*gr_step+gr_offset);
			tmp_gr_t->SetPointError(n, O_T_e/O_T_mc, gr_step/2.);


			cout << "For HER " << chs[iCh] << " x = " << O_bg_data/O_bg_mc << " +/- "<< O_bg_e/O_bg_mc << ", y: " <<  (iCh-1)*gr_step+gr_offset <<  std::endl;
	  }

	  ratios_bg.push_back(tmp_gr_bg);
	  ratios_t.push_back(tmp_gr_t);

    }

	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
	//double ratio = 16./9.;
	double ratio = 1./1.;

	string cantitle = "beamgas_ratio";
	TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	can->SetFillColor(kWhite);
	can->SetBorderMode(0);
	can->SetBorderSize(2);
	can->SetFrameBorderMode(0);
	can->SetBottomMargin(0.15);

	can->SetLogx();

	int cfwd1 = tab20_blue->GetNumber();
	int cfwd2 = tab20_red->GetNumber();
	int cfwd3 = tab20_green->GetNumber();

	double markersize = 0.8;
	int mfwd1 = 4;

	TH2D *axis_bg = new TH2D("axis_bg", "axis_bg", 1000, 7e-3, 3e2, 3, -0.5, 2.5);

	axis_bg->Draw("axis");

	axis_bg->GetXaxis()->SetTitle("R_{bg}^{Data}/R_{bg}^{MC}");
	axis_bg->GetXaxis()->SetTitleOffset(1.4);
//	axis_bg->GetXaxis()->SetLimits(5e-3, 5e2);

	axis_bg->GetYaxis()->SetBinLabel(1, "LER ");
	axis_bg->GetYaxis()->SetBinLabel(2, "HER ");
	axis_bg->GetYaxis()->SetBinLabel(3, "" );

	ratios_bg[0]->SetMarkerColor(cfwd1);
	ratios_bg[0]->SetMarkerSize(markersize);
	ratios_bg[0]->SetMarkerStyle(20);
	ratios_bg[0]->SetFillColorAlpha(cfwd1,0.25);
	ratios_bg[0]->SetLineWidth(0);
	ratios_bg[0]->SetLineColor(kRed);
	ratios_bg[0]->SetFillStyle(1001);


	// fwd1_mu->SetFillStyle(1001);
	// ratios_bg[0]->SetBorderSize(0);


	ratios_bg[1]->SetMarkerColor(cfwd2);
	ratios_bg[1]->SetFillColorAlpha(cfwd2,0.25);
	ratios_bg[1]->SetMarkerSize(markersize);
	ratios_bg[1]->SetMarkerStyle(20);
	ratios_bg[1]->SetLineWidth(0);

	ratios_bg[2]->SetMarkerColor(cfwd3);
	ratios_bg[2]->SetFillColorAlpha(cfwd3,0.25);
	ratios_bg[2]->SetMarkerSize(markersize);
	ratios_bg[2]->SetMarkerStyle(20);
	ratios_bg[2]->SetLineWidth(0);


	TLegend* leg =  new TLegend(0.21, 0.79, 0.94, 0.85);
//leg->SetHeader("");
    leg->SetNColumns(3);
	leg->SetBorderSize(0);
	leg->SetFillColor(0);
	leg->SetFillStyle(0);
	leg->SetMargin(0.4);
	leg->SetColumnSeparation(0.1);
    leg->AddEntry(ratios_bg[0], "FWD1", "F");
	leg->AddEntry(ratios_bg[1], "FWD2", "F");
	leg->AddEntry(ratios_bg[2], "FWD3", "F");


	TLegend* leg2 = (TLegend*) leg->Clone("leg2");
	leg2->Clear();

	leg2->AddEntry(ratios_bg[0], "FWD1", "p");
	leg2->AddEntry(ratios_bg[1], "FWD2", "p");
	leg2->AddEntry(ratios_bg[2], "FWD3", "p");

// leg->AddEntry(err, "Sys. + Stat. Unc.", "f");

	int legfontsize = leg->GetTextSize();
	int legfont     = leg->GetTextFont();

	TLatex* text = new TLatex();
	text->SetNDC();
	text->SetTextFont(legfont+20);
	text->SetTextSize(gStyle->GetLegendTextSize());

	TPaveText *pt = new TPaveText(7.15e-3,1.975,1e-2,2.075);
	pt->SetBorderSize(0);
	pt->SetFillColor(0);
	pt->SetFillStyle(1001);
//	pt->Draw("same");

	TPaveText *pt2 = new TPaveText(2e2,1.975,2.925e2,2.075);
	pt2->SetBorderSize(0);
	pt2->SetFillColor(0);
	pt2->SetFillStyle(1001);

	ratios_bg[0]->Draw("P2");

	ratios_bg[1]->Draw("P2");

	ratios_bg[2]->Draw("P2");

	text->DrawLatex(0.22, 0.87, "Beam-gas Ratios");
	leg->Draw();
	leg2->Draw();

	can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/beam_gas_ratios.pdf");
	can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/beam_gas_ratios.png");
	can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/beam_gas_ratios.jpg");
	can->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/beam_gas_ratios.eps");



	string cantitle2 = "touschek_ratio";
	TCanvas * can2 = new TCanvas(cantitle2.c_str(),cantitle2.c_str(), width, width/ratio);
	can2->SetFillColor(kWhite);
	can2->SetBorderMode(0);
	can2->SetBorderSize(2);
	can2->SetFrameBorderMode(0);
	can2->SetLogx();
	can2->SetBottomMargin(0.15);



	axis_bg->GetXaxis()->SetTitle("R_{T}^{Data}/R_{T}^{MC}");

	ratios_t[0]->GetXaxis()->SetLimits(5e-3, 5e2);
	ratios_t[0]->GetYaxis()->SetRangeUser(-gr_step-0.2, gr_step+gr_offset+0.2);

	// ratios_bg[0]->SetFillStyle(3001);
	ratios_t[0]->SetMarkerColor(cfwd1);
	ratios_t[0]->SetFillColorAlpha(cfwd1,0.25);
	ratios_t[0]->SetMarkerSize(markersize);
	ratios_t[0]->SetMarkerStyle(20);


	ratios_t[1]->SetMarkerColor(cfwd2);
	ratios_t[1]->SetFillColorAlpha(cfwd2,0.25);
	ratios_t[1]->SetMarkerSize(markersize);
	ratios_t[1]->SetMarkerStyle(20);

	ratios_t[2]->SetMarkerColor(cfwd3);
	ratios_t[2]->SetFillColorAlpha(cfwd3,0.25);
	ratios_t[2]->SetMarkerSize(markersize);
	ratios_t[2]->SetMarkerStyle(20);



	axis_bg->Draw("axis");
	ratios_t[0]->Draw("P2");
// /	ratios_bg[0]->Draw("P");

	ratios_t[1]->Draw("P2");

	ratios_t[2]->Draw("P2");

	text->DrawLatex(0.22, 0.87, "Touschek Ratios");
	leg->Draw();
	leg2->Draw();
	//sleep(1);
		// ratios_bg[1]->Draw("P");
	// ratios_bg[2]->Draw("P");

	can2->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/touschek_ratios.pdf");
	can2->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/touschek_ratios.png");
	can2->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/touschek_ratios.jpg");
	can2->SaveAs("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize/touschek_ratios.eps");


//	  FitResult ler = scatter_ler("FWD1");
}
