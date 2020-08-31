#include <math.h>

TF1 *fexp, *flangaus;
double finter(double *x, double*par) {
    return TMath::Abs(fexp->EvalPar(x,par) - flangaus->EvalPar(x,par));
}


Double_t langaufun(Double_t *x, Double_t *par) {

		//Fit parameters:
		//par[0]=Width (scale) parameter of Landau density
		//par[1]=Most Probable (MP, location) parameter of Landau density
		//par[2]=Total area (integral -inf to inf, normalization constant)
		//par[3]=Width (sigma) of convoluted Gaussian function
		//
		//In the Landau distribution (represented by the CERNLIB approximation),
		//the maximum is located at x=-0.22278298 with the location parameter=0.
		//This shift is corrected within this function, so that the actual
		//maximum is identical to the MP parameter.

		// Numeric constants
		Double_t invsq2pi = 0.3989422804014; // (2 pi)^(-1/2)
		Double_t mpshift  = -0.22278298;     // Landau maximum location

		// Control constants
		Double_t np = 100.0;    // number of convolution steps
		Double_t sc =   5.0;    // convolution extends to +-sc Gaussian sigmas

		// Variables
		Double_t xx;
		Double_t mpc;
		Double_t fland;
		Double_t sum = 0.0;
		Double_t xlow,xupp;
		Double_t step;
		Double_t i;


		// MP shift correction
		mpc = par[1] - mpshift * par[0];

		// Range of convolution integral
		xlow = x[0] - sc * par[3];
		xupp = x[0] + sc * par[3];

		step = (xupp-xlow) / np;

		// Convolution integral of Landau and Gaussian by sum
		for(i=1.0; i<=np/2; i++) {
				xx = xlow + (i-.5) * step;
				fland = TMath::Landau(xx,mpc,par[0]) / par[0];
				sum += fland * TMath::Gaus(x[0],xx,par[3]);

				xx = xupp - (i-.5) * step;
				fland = TMath::Landau(xx,mpc,par[0]) / par[0];
				sum += fland * TMath::Gaus(x[0],xx,par[3]);
		}

		return (par[2] * step * sum * invsq2pi / par[3]);
}

// Double_t exponential(Double_t *x, Double_t *par) {
//     return par[0]*exp(par[1]+par[2]*x[0]);
//     //return 1./((par[1]+x[0])*par[2]) + par[0];
// };

Double_t exponential(Double_t *x, Double_t *par) {
    // return par[0]*sqrt(x[0]*par[1])*exp(-par[1]*x[0]);
    return par[0]*exp(-par[1]*x[0]);
	//return par[0]/(par[1]*x[0]);
};

// Double_t exponential(Double_t *x, Double_t *par) {
//
//     return par[0]*par[1]*sqrt(x[0]/par[2])*exp(-x[0]/par[2]);
//     //return 1./((par[1]+x[0])*par[2]) + par[0];
// };

// Sum of background and peak function
Double_t combined(Double_t *x, Double_t *par) {
   return exponential(x,par) + langaufun(x,&par[2]);
}



void hitenergy_noinj( string channel = "FWD1")
{
	double xlow = 0;
	double xup 	= 150;

	// ---- Do the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "her_injections_hit_energy_spectrum_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->SetLogy();

    // -----   Colors and Style   ---------------------
    double linewidth = 2;
	double falpha = 1;

	int cboth = kBlack;

	int call = kBlack;

    int c1 = kBlack;
    int cfit = claws_red->GetNumber();
	int cfitexp = kCyan;
	int cfitlangaus = kMagenta;
    int cexpo = tab20_blue->GetNumber();
    int clangaus = tab20_orange->GetNumber();
	int cinter = kBlue;


	int cher = tab20_blue->GetNumber();
	int cfref = cher;

	int cler = tab20_red->GetNumber();
	int cfall = call;

	int cnon = kBlack;
	int cfnon = cnon;

	int cl = kGray+2;// int cref = tab20_blue->GetNumber();
	// -----  HISTOS   ---------------------

	// Targets
	Target target = GetTarget("NJ-ALL");
	//Target target = GetTarget("VC_STAT_FULL_INJ_NON_3e6_HER_5_LER_5");

	Target her_target = GetTarget("NJ-HER");
	//Target her_target = GetTarget("VC_STAT_HER_INJ_NON_3e6_HER_5_LER_-5_5");
	//
	Target ler_target = GetTarget("NJ-LER");
	//Target ler_target = GetTarget("VC_STAT_LER_INJ_NON_3e6_HER_-5_5_LER_5");

	// Both rings
	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

	TH1F* both = (TH1F*) rfile->Get((channel+"_hit_energy_spectrum").c_str());
	both->SetDirectory(0);

		if(target.n != -1)
		{
			double scale = target.n*0.8e-9*3.*3.;
			both->Scale(1./scale);
		}

	both->SetLineColor(cboth);
	both->SetLineWidth(linewidth);


	TFile* her_rfile = new TFile((her_target.pathin+"/"+her_target.filein).c_str(), "open");

	TH1F* her = (TH1F*) her_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	her->SetDirectory(0);

	if(target.n != -1)
	{
		double scale = her_target.n*0.8e-9*3.*3.;
		her->Scale(1./scale);
	}

	her->SetLineColor(cher);
	her->SetLineWidth(linewidth);


	TFile* ler_rfile = new TFile((ler_target.pathin+"/"+ler_target.filein).c_str(), "open");

	TH1F* ler = (TH1F*) ler_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	ler->SetDirectory(0);

	if(target.n != -1)
	{
		double scale = ler_target.n*0.8e-9*3.*3.;
		ler->Scale(1./scale);
	}

	ler->SetLineColor(cler);
	ler->SetLineWidth(linewidth);

	// Open text file
	ofstream txtfile;
	txtfile.open ((target.pathout+"/HitEnergy/"+target.fileout+"_hit_energy.txt").c_str(), ios::app);

	// -----   Fits   ---------------------

	// ---- Fit ----
    string funcname = "combined_fit_" + channel;

    double rlow = 0;
    for(int bin =1; bin< both->GetNbinsX(); ++bin)
    {
        if(both->GetBinContent(bin)> 1e-2)
        {
            rlow = both->GetBinLowEdge(bin);
            break;
        }
    }

    double min = 10000;
    int minbin = both->FindBin(rlow);

    for(int bin = both->FindBin(rlow); bin < both->FindBin(1); ++bin)
    {
        if(both->GetBinContent(bin) <= min)
        {
            min = both->GetBinContent(bin);
            minbin = bin;
        }
    }

    double rup = both->GetBinCenter(minbin);
//	double rup = both->GetBinLowEdge(minbin) + both->GetBinWidth(minbin);
    double rend = 2.5;
	TF1 *fcombined = new TF1(funcname.c_str(),combined, rlow,rend,6);

	fcombined->SetLineColorAlpha(cboth, falpha);
	fcombined->SetLineWidth(linewidth);
	fcombined->SetNpx(2000);

	funcname = "exp_fit_" + channel;
	fexp = new TF1(funcname.c_str(),exponential, 0, 150,2);

	funcname = "langaus_fit_" + channel;
	flangaus = new TF1(funcname.c_str(),langaufun, 0, 150,4);

	both->Fit(fexp, "O", "", rlow, rup);
	cout << "rlow: " << rlow << ", rup: " << rup << endl;

	for ( int j = 0; j < 2; ++j) fcombined->SetParameter (j, fexp->GetParameter(j) );
	flangaus->SetParameter(0, 0.12);
	flangaus->SetParameter(1, 0.76);
	flangaus->SetParameter(2, 150.);
	flangaus->SetParameter(3, 0.16);

	both->Fit(flangaus, "O", "", rup,2);


	for ( int j = 0; j < 4; ++j) fcombined->SetParameter (j+2, flangaus->GetParameter(j) );

	both->Fit(fcombined, "L", "", rlow,3);

	for ( int j = 0; j < 6; ++j)
	{
			cout << "Param: " << fcombined->GetParameter(j) << endl;
	}

	// Now feed the fitresults back in the independent functions
	for ( int j = 0; j < 2; ++j) fexp->SetParameter (j, fcombined->GetParameter(j) );
	for ( int j = 0; j < 4; ++j) flangaus->SetParameter (j, fcombined->GetParameter(j+2) );
	//both->Fit(flangaus, "ON", "", rup,2);

	cout << target.data << ", ch: " << channel <<  fixed << setprecision(2) << "Chi2: " << both->Chisquare(fcombined)<< ", NDF: " << fcombined->GetNDF() << ", Chi2/NDF: " << fcombined->GetChisquare()/fcombined->GetNDF() << endl;
	cout << target.data << ", ch: " << channel << fixed << setprecision(10) << "Prob: " << fcombined->GetProb()<< endl;
	// Exponential


	fexp->SetLineColorAlpha(cfitexp, falpha);
	fexp->SetLineWidth(linewidth);

	// for(int i = 0; i < 3; ++i) fexp->SetParameter(i, fcombined->GetParameter(i));

	// Langaus
	flangaus->SetLineColorAlpha(cfitlangaus, falpha);
	flangaus->SetLineWidth(linewidth);

	// for(int i = 0; i < 4; ++i) flangaus->SetParameter(i, fcombined->GetParameter(i+3));

	// Intersection
	TF1 *fint = new TF1("fint",finter,0,3,0);
	fint->SetLineColorAlpha(cinter, falpha);
	fint->SetLineWidth(linewidth);

	double xint = fint->GetMinimumX(rlow,2);
	double xint_4_line = xint;
	// Write the shit to file

	double bucketsize = 3*1e-9*1.965;
	double area = 9.;

	stringstream ss;
	ss << "& \\multicolumn{12}{c}{"<< channel << "} \\"<<"\\ \n " << "\\cmidrule{2-13} \n" << target.data;
	double n_hits = both->Integral();
	ss << fixed << setprecision(0) << " & " << n_hits;
	ss << fixed << scientific << setprecision(1)<< " & " << both->Integral()*area*bucketsize;
	ss << fixed << setprecision(2) << " & " << fcombined->GetMaximumX(rup, 2.);
	ss  << " & " << xint;

	double p_small_gamma = fexp->Integral(rlow, xint)/fcombined->Integral(rlow, xint);
	double p_small_mip = flangaus->Integral(rlow, xint)/fcombined->Integral(rlow, xint);
	double p_large_gamma = fexp->Integral(xint, rend)/fcombined->Integral(xint, rend);
	double p_large_mip = flangaus->Integral(xint, rend)/fcombined->Integral(xint, rend);
	ss << fixed << setprecision(1) << " & " << 100.*p_small_gamma;
	ss  << " & " << 100.*p_small_mip;
	ss  << " & ";
	ss  << " & " << 100.*p_large_gamma;
	ss  << " & " << 100.*p_large_mip;
	ss  << " & ";
	// Get the hits
	// N_hits
	int xint_bin = both->GetXaxis()->FindBin(xint);
	int three_bin = both->GetXaxis()->FindBin(rend);
	// N_hits_gamma
	//int xup = both->GetXaxis()->GetXup();
	double xint_bin_share = (xint-both->GetBinLowEdge(xint_bin))/both->GetBinWidth(xint_bin);
	double n_gammas = (both->Integral(1, xint_bin-1)+xint_bin_share*both->GetBinContent(xint_bin))*p_small_gamma;
	n_gammas += ((1.-xint_bin_share)*both->GetBinContent(xint_bin) + both->Integral(xint_bin+1, three_bin))*p_large_gamma;
	ss << fixed << setprecision(1) << " & " << 100*n_gammas/n_hits;
	double n_mip = (both->Integral(1, xint_bin-1)+xint_bin_share*both->GetBinContent(xint_bin))*p_small_mip;
	n_mip += ((1.-xint_bin_share)*both->GetBinContent(xint_bin) + both->Integral(xint_bin+1, three_bin))*p_large_mip + both->Integral(three_bin+1, both->GetNbinsX());
	ss << fixed << setprecision(1) << " & " << 100*n_mip/n_hits<<"\\"<<"\\";

	txtfile << ss.str();

	ss.str("");

	// No do the HER data set
	//TF1* fcombined_her = (TF1*) fcombined->Clone("fcombined_her");
	funcname = "combined_fit_her_" + channel;

	TF1 *fcombined_her = new TF1(funcname.c_str(),combined, rlow, rend,6);
//	fcombined_her->SetParameters(1,953, 0.057, 0.186,0.914, 0.260, 0.21);
	fcombined_her->SetLineColor(cher);
	fcombined_her->SetLineWidth(linewidth);
	fcombined_her->SetNpx(2000);
	// fcombined->FixParameter (0, 1);
	her->Fit(fexp, "OQ", "",rlow, rup);
	for ( int j = 0; j < 2; ++j) fcombined_her->SetParameter (j, fexp->GetParameter(j) );

	her->Fit(flangaus, "QO", "", rup,2);
	for ( int j = 0; j < 4; ++j) fcombined_her->SetParameter (j+2, flangaus->GetParameter(j) );

	her->Fit(fcombined_her, "QL", "", rlow,rend);

	// Now feed the fitresults back in the independent functions
	for ( int j = 0; j < 2; ++j) fexp->SetParameter (j, fcombined_her->GetParameter(j) );
	for ( int j = 0; j < 4; ++j) flangaus->SetParameter (j, fcombined_her->GetParameter(j+2) );

	xint = fint->GetMinimumX(rlow,2);

	ss <<" \n " << her_target.data;
	n_hits = her->Integral();
	ss << fixed << setprecision(0) << " & " << n_hits;
	ss << fixed << scientific << setprecision(1)<< " & " << her->Integral()*area*bucketsize;
	ss << fixed << setprecision(2) << " & " << fcombined_her->GetMaximumX(rup, 5.);
	ss  << " & " << xint;

	p_small_gamma = fexp->Integral(rlow, xint)/fcombined_her->Integral(rlow, xint);
	p_small_mip = flangaus->Integral(rlow, xint)/fcombined_her->Integral(rlow, xint);
	p_large_gamma = fexp->Integral(xint, rend)/fcombined_her->Integral(xint, rend);
	p_large_mip = flangaus->Integral(xint, rend)/fcombined_her->Integral(xint, rend);
	ss << fixed << setprecision(1) << " & " << 100.*p_small_gamma;
	ss  << " & " << 100.*p_small_mip;
	ss  << " & ";
	ss  << " & " << 100.*p_large_gamma;
	ss  << " & " << 100.*p_large_mip;
	ss  << " & ";
	// Get the hits
	// N_hits
	xint_bin = her->GetXaxis()->FindBin(xint);
	three_bin = her->GetXaxis()->FindBin(rend);
	// N_hits_gamma
	//int xup = her->GetXaxis()->GetXup();
	xint_bin_share = (xint-her->GetBinLowEdge(xint_bin))/her->GetBinWidth(xint_bin);
	n_gammas = (her->Integral(1, xint_bin-1)+xint_bin_share*her->GetBinContent(xint_bin))*p_small_gamma;
	n_gammas += ((1.-xint_bin_share)*her->GetBinContent(xint_bin) + her->Integral(xint_bin+1, three_bin))*p_large_gamma;
	ss << fixed << setprecision(1) << " & " << 100*n_gammas/n_hits;
	n_mip = (her->Integral(1, xint_bin-1)+xint_bin_share*her->GetBinContent(xint_bin))*p_small_mip;
	n_mip += ((1.-xint_bin_share)*her->GetBinContent(xint_bin) + her->Integral(xint_bin+1, three_bin))*p_large_mip + her->Integral(three_bin+1, her->GetNbinsX());
	ss << fixed << setprecision(1) << " & " << 100*n_mip/n_hits <<"\\"<<"\\";

	txtfile << ss.str();

	ss.str("");

	//TF1* fcombined_ler = (TF1*) fcombined->Clone("fcombined_ler");
	funcname = "combined_fit_ler_" + channel;

	TF1 *fcombined_ler = new TF1(funcname.c_str(),combined, rlow, rend,6);
//	fcombined_ler->SetParameters(1,953, 0.057, 0.186,0.914, 0.260, 0.21);
	fcombined_ler->SetLineColor(cler);
	fcombined_ler->SetLineWidth(linewidth);
	fcombined_ler->SetNpx(2000);
	// fcombined->FixParameter (0, 1);
	ler->Fit(fexp, "OQ", "",rlow, rup);
	for ( int j = 0; j < 2; ++j) fcombined_ler->SetParameter (j, fexp->GetParameter(j) );

	ler->Fit(flangaus, "QO", "", rup,2);
	for ( int j = 0; j < 4; ++j) fcombined_ler->SetParameter (j+2, flangaus->GetParameter(j) );

	ler->Fit(fcombined_ler, "QL", "", rlow,rend);

	// Now feed the fitresults back in the independent functions
	for ( int j = 0; j < 2; ++j) fexp->SetParameter (j, fcombined_ler->GetParameter(j) );
	for ( int j = 0; j < 4; ++j) flangaus->SetParameter (j, fcombined_ler->GetParameter(j+2) );

	xint = fint->GetMinimumX(rlow,2);

	ss <<" \n " << ler_target.data;
	n_hits = ler->Integral();
	ss << fixed << setprecision(0) << " & " << n_hits;
	ss << fixed << scientific << setprecision(1)<< " & " << ler->Integral()*area*bucketsize;
	ss << fixed << setprecision(2) << " & " << fcombined_ler->GetMaximumX(rup, 5.);
	ss  << " & " << xint;

	p_small_gamma = fexp->Integral(rlow, xint)/fcombined_ler->Integral(rlow, xint);
	p_small_mip = flangaus->Integral(rlow, xint)/fcombined_ler->Integral(rlow, xint);
	p_large_gamma = fexp->Integral(xint, rend)/fcombined_ler->Integral(xint, rend);
	p_large_mip = flangaus->Integral(xint, rend)/fcombined_ler->Integral(xint, rend);
	ss << fixed << setprecision(1) << " & " << 100.*p_small_gamma;
	ss  << " & " << 100.*p_small_mip;
	ss  << " & ";
	ss  << " & " << 100.*p_large_gamma;
	ss  << " & " << 100.*p_large_mip;
	ss  << " & ";
	// Get the hits
	// N_hits
	xint_bin = ler->GetXaxis()->FindBin(xint);
	three_bin = ler->GetXaxis()->FindBin(rend);
	// N_hits_gamma
	//int xup = ler->GetXaxis()->GetXup();
	xint_bin_share = (xint-ler->GetBinLowEdge(xint_bin))/ler->GetBinWidth(xint_bin);
	n_gammas = (ler->Integral(1, xint_bin-1)+xint_bin_share*ler->GetBinContent(xint_bin))*p_small_gamma;
	n_gammas += ((1.-xint_bin_share)*ler->GetBinContent(xint_bin) + ler->Integral(xint_bin+1, three_bin))*p_large_gamma;
	ss << fixed << setprecision(1) << " & " << 100*n_gammas/n_hits;
	n_mip = (ler->Integral(1, xint_bin-1)+xint_bin_share*ler->GetBinContent(xint_bin))*p_small_mip;
	n_mip += ((1.-xint_bin_share)*ler->GetBinContent(xint_bin) + ler->Integral(xint_bin+1, three_bin))*p_large_mip + ler->Integral(three_bin+1, ler->GetNbinsX());
	ss << fixed << setprecision(1) << " & " << 100*n_mip/n_hits<<"\\"<<"\\" << "\n \\"<<"\\ \n";

	txtfile << ss.str();

	ss.str("");

	// --- Error ---

	TH1F *err = (TH1F*) both->Clone("err");
	err->SetMarkerSize(0);
	err->SetFillStyle(3353);
	err->SetFillColorAlpha(kBlack,1);

	TH1F *her_err = (TH1F*) her->Clone("her_err");
	her_err->SetMarkerSize(0);
	her_err->SetFillStyle(3353);
	her_err->SetFillColorAlpha(cher,1);

    // -----   Axis   ---------------------
	TH1F* axis = new TH1F("axis", "axis", 1000, xlow, xup);
    axis->SetDirectory(0);
//     axis->SetLineColorAlpha( c1, 1.);
//     axis->SetFillColorAlpha( c1, 1.);
//     axis->SetMarkerColorAlpha(c1, 1);
//     axis->SetMarkerSize(1.0);
// //	axis->SetLineWidth(1.5);
//     axis->SetFillStyle(0);

	axis->GetXaxis()->SetTitle("Hit Energy [MIP]");
	axis->GetXaxis()->SetTitleOffset(1.2);
	axis->GetYaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]");

	axis->GetXaxis()->SetRangeUser(-0.015, 4.05);
	axis->GetYaxis()->SetRangeUser(0.1, 2e4);

	//axis->GetYaxis()->SetRangeUser(0, 1e4);
 	// stringstream ss;
    //  double binwidth = her_ref_r->GetBinWidth(3);
    //  ss << fixed << setprecision(2) << "Entries [1/" << binwidth << " MIP #times s]";
    //  axis->GetYaxis()->SetTitle(ss.str().c_str());

	int linestyle = 9;


	TLine *line = new TLine(fcombined->GetMaximumX(rup, 5.), 6,fcombined->GetMaximumX(rup, 5.),3e3);
//	TArrow* line = new TArrow(fcombined->GetMaximumX(rup, 5.), 5,fcombined->GetMaximumX(rup, 5.),fcombined->GetMaximum(rup, 5.), 0.015, "-|>");
	line->SetLineColor(cl);
// /	line->SetFillStyle(0);
	line->SetLineStyle(linestyle);
	line->SetLineWidth(linewidth);

	TLine *line4 = new TLine(fcombined->GetMaximumX(rup, 5.), 0.1, fcombined->GetMaximumX(rup, 5.), 2);
	// TArrow* line2 = new TArrow(xint_4_line, 0.5,xint_4_line,fcombined->Eval(xint_4_line), 0.015, "|>");
	line4->SetLineColor(cl);
//	line2->SetFillStyle(0);
	line4->SetLineStyle(linestyle);
	line4->SetLineWidth(linewidth);

	TLine *line2 = new TLine(xint_4_line, 0.6,xint_4_line,3e3);
	// TArrow* line2 = new TArrow(xint_4_line, 0.5,xint_4_line,fcombined->Eval(xint_4_line), 0.015, "|>");
	line2->SetLineColor(cl);
//	line2->SetFillStyle(0);
	line2->SetLineStyle(linestyle);
	line2->SetLineWidth(linewidth);

	TLine *line3 = new TLine(xint_4_line, 0.1, xint_4_line, 0.2);
	// TArrow* line2 = new TArrow(xint_4_line, 0.5,xint_4_line,fcombined->Eval(xint_4_line), 0.015, "|>");
	line3->SetLineColor(cl);
//	line2->SetFillStyle(0);
	line3->SetLineStyle(linestyle);
	line3->SetLineWidth(linewidth);

//	TArrow* ar1 = new TArrow(6,y1,6+intwindow,y1, 0.015, "<|>");
	// ar1->SetLineColor(cl1);
	// ar1->SetFillColorAlpha(cl1,0);
	// ar1->SetLineWidth(linewidth);

	// -----   Text and legend ----

//	TLegend* leg =  new TLegend(0.63, 0.66, 0.82, 0.82);
	// TLegend* leg =  new TLegend(0.55, 0.56, 0.73, 0.85);
    TLegend* leg =  new TLegend(0.64, 0.56, 0.84, 0.85);
	    //leg->SetHeader("");
	//    leg->SetNColumns(2);
	leg->SetBorderSize(0);
	leg->SetFillColor(0);
	leg->SetTextSize(gStyle->GetLegendTextSize());
	// leg->AddEntry(make_box)

	int legfontsize = leg->GetTextSize();
	int legfont     = leg->GetTextFont();

	leg->AddEntry(make_box(both, "EMPTY"), target.data.c_str(), "f");
	leg->AddEntry(set_line_width(fcombined,3), "Exp. + L*G Fit", "l");

	leg->AddEntry(make_box(her, "EMPTY"),her_target.data.c_str(), "f");
	leg->AddEntry(set_line_width(fcombined_her,3), "Exp. + L*G Fit", "l");

	leg->AddEntry(make_box(ler, "EMPTY"), ler_target.data.c_str(), "f");
	leg->AddEntry(set_line_width(fcombined_ler,3), "Exp. + L*G Fit", "l");

	// leg->AddEntry(make_box(hist), channel.c_str(), "f");
	// leg->AddEntry(set_line_width(err,0), "Stat. Unc.", "f");
	// leg->AddEntry(set_line_width_f(fit,3), "Exp. + Landau*Gaus Fit", "l");

// 	leg->AddEntry(make_box(both,"EMPTY"), "Non-Injection Events", "f");
//     leg->AddEntry(set_line_width(err,0), "Stat. Unc.", "f");
//     leg->AddEntry(set_line_width_f(fcombined,3), "Exp. + Landau*Gaus Fit", "l");
//
// //	stringstream ss;
// 	ss << fixed << setprecision(2) << "MPV_{L.*G.} = " << fcombined->GetMaximumX(rup, 5.) << " MIP";

    //leg->AddEntry((TObject*)0, ss.str().c_str(), "");


	TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

	TLatex* mtext = new TLatex();
	mtext->SetNDC();
//	mtext->SetTextFont(legfont+20);
	mtext->SetTextSize(gStyle->GetLegendTextSize());
//	mtext->SetFillColor(kWhite);

	TLatex* atext = new TLatex();
	atext->SetNDC();
	atext->SetTextColor(cl);
	atext->SetTextFont(legfont+20);
	atext->SetTextSize(gStyle->GetLegendTextSize()*0.8);

	// TPaveText *pt = new TPaveText(0.3,.2,0.6,.3,"brNDC");
	// pt->SetBorderSize(0);
	// pt->SetFillColor(0);
	// pt->AddText(ss.str().c_str());
	// pt->SetTextSize(gStyle->GetLegendTextSize());
	// pt->SetTextColor(cl);
	// ---- Drawing ----
    can->cd();
    axis->Draw("axis");
	line->Draw();
	line2->Draw();
	line3->Draw();
	line4->Draw();
//	err->Draw("same E2");
	both->Draw("same hist");
	fcombined->Draw("same");

//	her_err->Draw("same E2");
	ler->Draw("same hist");
	fcombined_ler->Draw("same");

	her->Draw("same hist");
	fcombined_her->Draw("same");
	// fexp->Draw("same");
	// flangaus->Draw("same");
	// fint->Draw("same");
	ttext->DrawLatex(0.22, 0.88, ("Hit Energy Spectrum: " + channel).c_str());
//	mtext->DrawLatex(0.25, 0.3, ss.str().c_str());

	if(channel == "FWD1") atext->DrawLatex(0.305, 0.36, "MPV_{L*G}");
	else if(channel == "FWD2") atext->DrawLatex(0.345, 0.36, "MPV_{L*G}");
	else if(channel == "FWD3") atext->DrawLatex(0.335, 0.36, "MPV_{L*G}");

	if(channel == "FWD1") atext->DrawLatex(0.24, 0.205, "E_{#gamma/c}");
	else if(channel == "FWD2") atext->DrawLatex(0.285, 0.205, "E_{#gamma/c}");
	else if(channel == "FWD3") atext->DrawLatex(0.265, 0.205, "E_{#gamma/c}");

	leg->Draw();


	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_energy_"+channel+".pdf").c_str());
    can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_energy_"+channel+".png").c_str());
    can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_energy_"+channel+".eps").c_str());
    can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_energy_"+channel+".jpg").c_str());

	txtfile.close();
}




void hitenergy_her(string channel = "FWD1")
{
	double xlow = 0;
	double xup 	= 150;

	// ---- Do the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "her_injections_hit_energy_spectrum_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

	can->SetLogx();
    can->SetLogy();

    // -----   Colors and Style   ---------------------
    double linewidth = 2;
	double falpha = 1;

	int cref = tab20_green->GetNumber();
	int cfref = cref;

	int call = tab20_blue->GetNumber();
	int cfher = call;

    int cbunch = tab20_red->GetNumber();
    int cfbunch = cbunch;

	int cnon = kBlack;
	int cfnon = cnon;

	// -----  HISTOS   ---------------------
	// HREF

	Target her_ref_target = GetTarget("HER-REF");
	TFile* her_ref_rfile = new TFile((her_ref_target.pathin+"/"+her_ref_target.filein).c_str(), "open");

	TH1F* her_ref = (TH1F*) her_ref_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	her_ref->SetDirectory(0);

	if(her_ref_target.n != -1)
	{
		double scale = her_ref_target.n*0.8e-9*3*3;
		her_ref->Scale(1./scale);
	}

	// Black Magic shit for the logarithmic binning
	int nbins = her_ref->GetNbinsX();
	double lowedge = her_ref->GetBinLowEdge(1);
	double binwidth = her_ref->GetBinWidth(1);
	double upedge = her_ref->GetBinLowEdge(nbins) + binwidth;

	int nbins_new = -1;
	double prev_xlow = lowedge;
	double step = 1.;
	double step2 = 2.5;

	int lowbin = 1;

	for(int bin =1; bin< her_ref->GetNbinsX(); ++bin)
	{
		if(her_ref->GetBinContent(bin)> 1e-10)
		{
			lowbin = bin;
			break;
		}
	}

	int quadbin = -1;
	for(int i = lowbin; i<nbins; ++i)
	{
		if(her_ref->GetBinContent(i) < 0.001 )
		{
			quadbin = i;
			break;
		}
	}

    quadbin = 25;

	for(int i = 1; i<nbins; ++i)
	{
		if(i>=quadbin)
		{
			if(prev_xlow + pow(step,(i-1))*pow(step2,(i-quadbin))*binwidth>upedge)
			{
				nbins_new = i;
				break;
			}
			prev_xlow = prev_xlow + pow(step,(i-1))*pow(step2,(i-quadbin))*binwidth;
		}
		else
		{
			if(prev_xlow + pow(step,(i-1))*binwidth>upedge)
			{
				nbins_new = i;
				break;
			}
			prev_xlow = prev_xlow + pow(step,(i-1))*binwidth;
		}

	}

	Double_t xbins[nbins_new+1];

	xbins[0] = lowedge;
	for(int i = 1; i<nbins_new+1; ++i)
	{
		if(i>=quadbin)
		{
			xbins[i] = xbins[i-1] + pow(step,(i-1))*pow(step2,(i-quadbin))*binwidth;
		}
		else
		{
			xbins[i] = xbins[i-1] + pow(step,(i-1))*binwidth;
		}

	}

	TH1F* her_ref_r = (TH1F*) her_ref->Rebin(nbins_new,"hnew",xbins);
    her_ref_r->Reset();

    double mintmp = 1e10;
    for(int bin = 1 ; bin < her_ref->GetNbinsX(); ++bin)
    {
        if(her_ref->GetBinContent(bin)<mintmp && her_ref->GetBinContent(bin) != 0) mintmp = her_ref->GetBinContent(bin);
    }

    for(int bin = 1 ; bin < her_ref->GetNbinsX(); ++bin)
    {
        if(her_ref->GetBinContent(bin) != 0)
        {
            int n = round(her_ref->GetBinContent(bin)/mintmp);
            double x = her_ref->GetBinCenter(bin);
            double widthscale = her_ref->GetBinWidth(bin)/her_ref_r->GetBinWidth(her_ref_r->FindBin(x));
            her_ref_r->Fill(x, n*widthscale);
        }
    }


    her_ref_r->Scale(her_ref_r->Integral()/her_ref->Integral());


	her_ref_r->SetLineColor(cref);
	her_ref_r->SetLineWidth(linewidth);

	// HER All inj
	Target all_her_target = GetTarget("HER-ALL");
	TFile* all_her_rfile = new TFile((all_her_target.pathin+"/"+all_her_target.filein).c_str(), "open");

	TH1F* all_her = (TH1F*) all_her_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	all_her->SetDirectory(0);
    cout << "Max HER-ALL: " << all_her->GetMaximum() << endl;
	if(all_her_target.n != -1)
	{
		double scale = all_her_target.n*0.8e-9*3*3;
		all_her->Scale(1./scale);
	}

	all_her->SetLineColor(call);
	all_her->SetLineWidth(linewidth);

    TH1F* all_her_bunch = (TH1F*) all_her_rfile->Get((channel+"_bunch_hit_energy_spectrum").c_str());
    all_her_bunch->SetDirectory(0);
    cout << "Max HER-ALL: " << all_her->GetMaximum() << endl;
    if(all_her_target.n != -1)
    {
        double scale = all_her_target.n*0.8e-9*3*3;
        all_her_bunch->Scale(1./scale);
    }

    all_her_bunch->SetLineColor(cbunch);
    all_her_bunch->SetLineWidth(linewidth);

	// Non INJ
	Target all_non_target = GetTarget("NJ-HER");
	TFile* all_non_rfile = new TFile((all_non_target.pathin+"/"+all_non_target.filein).c_str(), "open");

	TH1F* all_non = (TH1F*) all_non_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	all_non->SetDirectory(0);
    cout << "Max NJ-ALL: " << all_non->GetMaximum() << endl;
		if(all_non_target.n != -1)
	{
		double scale = all_non_target.n*0.8e-9*3*3;
		all_non->Scale(1./scale);
	}

	all_non->SetLineColor(cnon);
	all_non->SetLineWidth(linewidth);

	// -----   Fits   ---------------------

	// ---- Fit ----
    string funcname = channel +  "_ref";

    double rlow = 0;
    for(int bin =1; bin< her_ref_r->GetNbinsX(); ++bin)
    {
        if(her_ref_r->GetBinContent(bin)> 1e-10)
        {
            rlow = her_ref_r->GetBinLowEdge(bin);
            break;
        }
    }
    cout << channel << " rlow: " << rlow << endl;;
        //double rup = her_ref_r->GetBinLowEdge( her_ref_r->GetNbinsX() ) + her_ref_r->GetBinWidth(2);

    double min = 10000;
    int minbin = her_ref_r->FindBin(rlow);

    for(int bin = her_ref_r->FindBin(rlow); bin < her_ref_r->FindBin(1); ++bin)
    {
        if(her_ref_r->GetBinContent(bin) <= min)
        {
            min = her_ref_r->GetBinContent(bin);
            minbin = bin;
        }
    }
    double rup = her_ref_r->GetBinCenter(minbin);
    double rend = 1.5;
	TF1 *fit_ref = new TF1(funcname.c_str(),combined, rlow,rend,6);

	fit_ref->SetLineColorAlpha(cfref, falpha);
	fit_ref->SetLineWidth(linewidth);
    fit_ref->SetNpx(2000);

    funcname = "exp_fit_" + channel;
	fexp = new TF1(funcname.c_str(),exponential, 0, 150,2);

	funcname = "langaus_fit_" + channel;
	flangaus = new TF1(funcname.c_str(),langaufun, 0, 150,4);

    her_ref_r->Fit(fexp, "OQ", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_ref->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(2, 0.2);
    flangaus->SetParameter(3, 0.65);
    flangaus->SetParameter(4, 20.);
    flangaus->SetParameter(5, 0.16);

    her_ref_r->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_ref->SetParameter (j+2, flangaus->GetParameter(j) );

    her_ref_r->Fit(fit_ref, "LQ", "", rlow,rend);
    cout << "HER-REF: " << fit_ref->GetMaximumX(rup, 5) << endl;

    // Fit NJ-HER
    funcname = channel +  "_nj_her";

    TF1 *fit_non = new TF1(funcname.c_str(),combined, rlow,rend,6);

	fit_non->SetLineColorAlpha(cfnon, falpha);
	fit_non->SetLineWidth(linewidth);
    fit_non->SetNpx(2000);

    all_non->Fit(fexp, "OQ", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_non->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(2, 0.12);
    flangaus->SetParameter(3, 1);
    flangaus->SetParameter(4, 20.);
    flangaus->SetParameter(5, 0.16);

    all_non->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_non->SetParameter (j+2, flangaus->GetParameter(j) );

    all_non->Fit(fit_non, "LQ", "", rlow,rend);
    cout << "NJ-HER: " << fit_non->GetMaximumX(rup, 5) << endl;

    // Fit HER-ALL
    funcname = channel +  "_her_all";

    TF1 *fit_her = new TF1(funcname.c_str(),combined, rlow,rend,6);

    fit_her->SetLineColorAlpha(cfher, falpha);
    fit_her->SetLineWidth(linewidth);
    fit_her->SetNpx(2000);

    all_her->Fit(fexp, "OQ", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_her->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(2, 0.12);
    flangaus->SetParameter(3, 1);
    flangaus->SetParameter(4, 20.);
    flangaus->SetParameter(5, 0.16);

    all_her->Fit(flangaus, "O", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_her->SetParameter (j+2, flangaus->GetParameter(j) );

    all_her->Fit(fit_her, "LQ", "", rlow,rend);
    cout << "HER-ALL: " << fit_her->GetMaximumX(rup, 5) << endl;

    // Fit HER-ALL BUNCH
    funcname = channel +  "_her_all_bunch";

    TF1 *fit_her_bunch = new TF1(funcname.c_str(),combined, rlow,rend,6);

    fit_her_bunch->SetLineColorAlpha(cfbunch, falpha);
    fit_her_bunch->SetLineWidth(linewidth);
    fit_her_bunch->SetNpx(2000);

    all_her_bunch->Fit(fexp, "OQ", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_her_bunch->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(2, 0.16);
    flangaus->SetParameter(3, 0.85);
    flangaus->SetParameter(4, 8.);
    flangaus->SetParameter(5, 0.1);

    all_her_bunch->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_her_bunch->SetParameter (j+2, flangaus->GetParameter(j) );

    all_her_bunch->Fit(fit_her_bunch, "LQ", "", rlow,rend);
    cout << "HER-ALL BUNCH: " << fit_her_bunch->GetMaximumX(rup, 5) << endl;
    // -----   Axis   ---------------------
	TH1F* axis = new TH1F("axis", "axis", 151000, -0.5, 150.5);


    //TH1F* axis = (TH1F*) rfile->Get((channel+"_hit_energy_spectrum").c_str());
    //axis->SetDirectory(0);
    // axis->SetLineColorAlpha( c1, 1.);
    // axis->SetFillColorAlpha( c1, 1.);
    // axis->SetMarkerColorAlpha(c1, 1);
    // axis->SetMarkerSize(1.0);
    // // axis->SetLineWidth(linewidth);
	// axis->SetLineWidth(1.5);
    // axis->SetFillStyle(0);

	all_non->GetXaxis()->SetTitle("Hit Energy [MIP]");
	all_non->GetXaxis()->SetTitleOffset(1.2);
	all_non->GetYaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]");

	// axis->GetXaxis()->SetRangeUser(0.14, 150);
	// axis->GetYaxis()->SetRangeUser(1e-2, 5e3);

 	// stringstream ss;
    //  double binwidth = her_ref_r->GetBinWidth(3);
    //  ss << fixed << setprecision(2) << "Entries [1/" << binwidth << " MIP #times s]";
    //  axis->GetYaxis()->SetTitle(ss.str().c_str());

	// -----   Text and legend ----

//	TLegend* leg =  new TLegend(0.5, 0.64, 0.7, 0.85);

	TLegend* leg =  new TLegend(0.68, 0.64, 0.88, 0.85);
	    //leg->SetHeader("");
	//    leg->SetNColumns(2);
	leg->SetBorderSize(0);
	leg->SetFillColor(0);

	// leg->AddEntry(make_box)

	int legfontsize = leg->GetTextSize();
	int legfont     = leg->GetTextFont();
//
    leg->AddEntry(make_box(all_non, "EMPTY"), all_non_target.data.c_str(), "f");
    leg->AddEntry(make_box(all_her, "EMPTY"), all_her_target.data.c_str(), "f");
    leg->AddEntry(make_box(all_her_bunch, "EMPTY"), (all_her_target.data+"_{ib}").c_str(), "f");
//    leg->AddEntry((TObject*) 0, "(E_{b} only)", "");

    leg->AddEntry(make_box(her_ref_r, "EMPTY"), her_ref_target.data.c_str(), "f");


// 	// leg->AddEntry(set_line_width(err,0), "Stat. Unc.", "f");
// 	// leg->AddEntry(set_line_width_f(fit,3), "Exp. + Landau*Gaus Fit", "l");
//
// 	leg->AddEntry(make_box(all_non), "Non-Injection", "f");
//
// 	stringstream ss;
//     ss << fixed << setprecision(2) << "MPV_{NI.} = " << fit_non->GetMaximumX(rup, 5) << " MIP";
//
// 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
//
// 	leg->AddEntry(make_box(all_her), "HER Injection", "f");
// 	ss.str("");
// 	ss << fixed << setprecision(2) << "MPV_{HI.} = " << fit_all->GetMaximumX(rup, 5) << " MIP";
// 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
//
// 	leg->AddEntry(make_box(her_ref_r), "HER Ref. Inj.", "f");
// 	ss.str("");
// 	ss << fixed << setprecision(2) << "MPV_{HRI.} = " << fit_ref->GetMaximumX(rup, 5) << " MIP";
// 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
// //	leg->AddEntry((TObject*)0, "Injection Events", "");
//
	TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

	// ---- Drawing ----

    all_non->GetXaxis()->SetRangeUser(0.11,150);
    all_non->GetYaxis()->SetRangeUser(1e-2, 5e3);
    //can->cd();
    all_non->Draw("axis");


//     //
 	all_non->Draw("same hist");
 	fit_non->Draw("same");
//
// //    axis->GetXaxis()->SetLimits(0.14, 150);
// //    axis->GetXaxis()->SetRange(0.15, 150);
//
//
     all_her->Draw("same hist");
     fit_her->Draw("same");
//     // can->Update();
//     //
//     //
     all_her_bunch->Draw("same hist");
     fit_her_bunch->Draw("same");
//     // can->Update();
//     //
 	her_ref_r->Draw("same hist");
 	fit_ref->Draw("same");
//     // can->Update();

    //fexp->Draw("same");
    // flangaus->Draw("same");
	// her_ref_r->DrawCopy("same hist");
	//
	// all_non->SetFillColorAlpha(kWhite, 0);
	// all_non->Draw("same hist");
	//
	// all_her->SetFillColorAlpha(kWhite, 0);
	// all_her->Draw("same hist");
	//
	// her_ref_r->SetFillColorAlpha(kWhite, 0);
	// her_ref_r->Draw("same hist");

    //
    // axis->GetXaxis()->SetRange(axis->FindBin(0.15), axis->FindBin(100));
    // axis->GetYaxis()->SetRangeUser(1e-2, 5e3);
    // can->Modified();
    // can->Update();

//	ttext->DrawLatex(0.22, 0.88, ("Hit Energy Spectrum: "+channel).c_str());
    ttext->DrawLatex(0.43, 0.88, ("Hit Energy Spectrum: "+channel).c_str());
	leg->Draw();

	can->SaveAs((her_ref_target.pathout+"/HitEnergy/"+her_ref_target.fileout+"_hit_energy_"+channel+".pdf").c_str());
    can->SaveAs((her_ref_target.pathout+"/HitEnergy/"+her_ref_target.fileout+"_hit_energy_"+channel+".png").c_str());
    can->SaveAs((her_ref_target.pathout+"/HitEnergy/"+her_ref_target.fileout+"_hit_energy_"+channel+".eps").c_str());
    can->SaveAs((her_ref_target.pathout+"/HitEnergy/"+her_ref_target.fileout+"_hit_energy_"+channel+".jpg").c_str());
}

void hitenergy_ler(string channel = "FWD1")
{
	double xlow = 0;
	double xup 	= 150;

	// ---- Do the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "ler_injections_hit_energy_spectrum_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

	can->SetLogx();
    can->SetLogy();

    // -----   Colors and Style   ---------------------
    double linewidth = 2;
	double falpha = 1;

	int cref = tab20_green->GetNumber();
	int cfref = cref;

	int call = tab20_blue->GetNumber();
	int cfler = call;

    int cbunch = tab20_red->GetNumber();
    int cfbunch = cbunch;

	int cnon = kBlack;
	int cfnon = cnon;

	// -----  HISTOS   ---------------------
	// HREF

	Target ler_ref_target = GetTarget("LER-REF");
	TFile* ler_ref_rfile = new TFile((ler_ref_target.pathin+"/"+ler_ref_target.filein).c_str(), "open");

	TH1F* ler_ref = (TH1F*) ler_ref_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	ler_ref->SetDirectory(0);

	if(ler_ref_target.n != -1)
	{
		double scale = ler_ref_target.n*0.8e-9*3*3;
		ler_ref->Scale(1./scale);
	}

	// Black Magic shit for the logarithmic binning
	int nbins = ler_ref->GetNbinsX();
	double lowedge = ler_ref->GetBinLowEdge(1);
	double binwidth = ler_ref->GetBinWidth(1);
	double upedge = ler_ref->GetBinLowEdge(nbins) + binwidth;

	int nbins_new = -1;
	double prev_xlow = lowedge;
	double step = 1.;
	double step2 = 2.5;

	int lowbin = 1;

	for(int bin =1; bin< ler_ref->GetNbinsX(); ++bin)
	{
		if(ler_ref->GetBinContent(bin)> 1e-10)
		{
			lowbin = bin;
			break;
		}
	}

	int quadbin = ler_ref->GetNbinsX();
	for(int i = lowbin; i<nbins; ++i)
	{
        if(ler_ref->GetBinContent(i) < 20 &&i >= 13 )
	//	if(ler_ref->GetBinContent(i) < -1 )
		{
			quadbin = i;
			break;
		}
	}

    quadbin =36;

	for(int i = 1; i<nbins; ++i)
	{
		if(i>=quadbin)
		{
			if(prev_xlow + pow(step,(i-1))*pow(step2,(i-quadbin))*binwidth>upedge)
			{
				nbins_new = i;
				break;
			}
			prev_xlow = prev_xlow + pow(step,(i-1))*pow(step2,(i-quadbin))*binwidth;
		}
		else
		{
			if(prev_xlow + pow(step,(i-1))*binwidth>upedge)
			{
				nbins_new = i;
				break;
			}
			prev_xlow = prev_xlow + pow(step,(i-1))*binwidth;
		}

	}

	Double_t xbins[nbins_new+1];

	xbins[0] = lowedge;
	for(int i = 1; i<nbins_new+1; ++i)
	{
		if(i>=quadbin)
		{
			xbins[i] = xbins[i-1] + pow(step,(i-1))*pow(step2,(i-quadbin))*binwidth;
		}
		else
		{
			xbins[i] = xbins[i-1] + pow(step,(i-1))*binwidth;
		}

	}

	TH1F* ler_ref_r = (TH1F*) ler_ref->Rebin(nbins_new,"hnew",xbins);
    ler_ref_r->Reset();

    double mintmp = 1e10;
    for(int bin = 1 ; bin < ler_ref->GetNbinsX(); ++bin)
    {
        if(ler_ref->GetBinContent(bin)<mintmp && ler_ref->GetBinContent(bin) != 0) mintmp = ler_ref->GetBinContent(bin);
    }
    mintmp /=2.;
    for(int bin = 1 ; bin < ler_ref->GetNbinsX(); ++bin)
    {
        if(ler_ref->GetBinContent(bin) != 0)
        {
            //int n = round(ler_ref->GetBinContent(bin)/mintmp);
            double n = ler_ref->GetBinContent(bin);
            double x = ler_ref->GetBinCenter(bin);
            double widthscale = ler_ref->GetBinWidth(bin)/ler_ref_r->GetBinWidth(ler_ref_r->FindBin(x));
        //    cout << "bin: " << bin << ", n: " << n<< ", x: " << x << ", widthscale: " << widthscale<< endl;
            ler_ref_r->Fill(x, n*widthscale);
        }
    }
    //ler_ref_r->Scale(ler_ref->Integral()/ler_ref_r->Integral());


	ler_ref_r->SetLineColor(cref);
	ler_ref_r->SetLineWidth(linewidth);

	// ler All inj
	Target all_ler_target = GetTarget("LER-ALL");
	TFile* all_ler_rfile = new TFile((all_ler_target.pathin+"/"+all_ler_target.filein).c_str(), "open");

	TH1F* all_ler = (TH1F*) all_ler_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	all_ler->SetDirectory(0);
    cout << "Max ler-ALL: " << all_ler->GetMaximum() << endl;
	if(all_ler_target.n != -1)
	{
		double scale = all_ler_target.n*0.8e-9*3*3;
		all_ler->Scale(1./scale);
	}

	all_ler->SetLineColor(call);
	all_ler->SetLineWidth(linewidth);

    TH1F* all_ler_bunch = (TH1F*) all_ler_rfile->Get((channel+"_bunch_hit_energy_spectrum").c_str());
    all_ler_bunch->SetDirectory(0);
    cout << "Max ler-ALL: " << all_ler->GetMaximum() << endl;
    if(all_ler_target.n != -1)
    {
        double scale = all_ler_target.n*0.8e-9*3*3;
        all_ler_bunch->Scale(1./scale);
    }

    all_ler_bunch->SetLineColor(cbunch);
    all_ler_bunch->SetLineWidth(linewidth);

	// Non INJ
	Target all_non_target = GetTarget("NJ-LER");
	TFile* all_non_rfile = new TFile((all_non_target.pathin+"/"+all_non_target.filein).c_str(), "open");

	TH1F* all_non = (TH1F*) all_non_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	all_non->SetDirectory(0);
    cout << "Max NJ-ALL: " << all_non->GetMaximum() << endl;
		if(all_non_target.n != -1)
	{
		double scale = all_non_target.n*0.8e-9*3*3;
		all_non->Scale(1./scale);
	}

	all_non->SetLineColor(kBlack);
	all_non->SetLineWidth(linewidth);

	// -----   Fits   ---------------------

	// ---- Fit ----
    string funcname = channel +  "_ref";

    double rlow = 0;
    for(int bin =1; bin< ler_ref_r->GetNbinsX(); ++bin)
    {
        if(ler_ref_r->GetBinContent(bin)> 1e-10)
        {
            rlow = ler_ref_r->GetBinLowEdge(bin);
            break;
        }
    }
    cout << channel << " rlow: " << rlow << endl;;
        //double rup = ler_ref_r->GetBinLowEdge( ler_ref_r->GetNbinsX() ) + ler_ref_r->GetBinWidth(2);

    double min = 10000;
    int minbin = ler_ref_r->FindBin(rlow);

    for(int bin = ler_ref_r->FindBin(rlow); bin < ler_ref_r->FindBin(1); ++bin)
    {
        if(ler_ref_r->GetBinContent(bin) <= min)
        {
            min = ler_ref_r->GetBinContent(bin);
            minbin = bin;
        }
    }
    double rup = ler_ref_r->GetBinCenter(minbin);
    double rend = 1.5;
	TF1 *fit_ref = new TF1(funcname.c_str(),combined, rlow,rend,6);

	fit_ref->SetLineColorAlpha(cfref, falpha);
	fit_ref->SetLineWidth(linewidth);
    fit_ref->SetNpx(2000);

    funcname = "exp_fit_" + channel;
	fexp = new TF1(funcname.c_str(),exponential, 0, 150,2);

	funcname = "langaus_fit_" + channel;
	flangaus = new TF1(funcname.c_str(),langaufun, 0, 150,4);

    ler_ref_r->Fit(fexp, "OQ", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_ref->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(0, 0.05);
    flangaus->SetParameter(1, 0.8);
    // flangaus->SetParameter(2, ler_ref_r->Integral(ler_ref_r->FindBin(rup), ler_ref_r->FindBin(2.5)));
    flangaus->SetParameter(2, 40);
    flangaus->SetParameter(3, 0.25);

    //flangaus->FixParameter(1, 1);
    cout << "rup: " << rup<< endl;
    ler_ref_r->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_ref->SetParameter (j+2, flangaus->GetParameter(j) );

    ler_ref_r->Fit(fit_ref, "LQ", "", rlow,rend);
    cout << "ler-REF: " << fit_ref->GetMaximumX(rup, 5) << endl;

    // Fit NJ-ler
    funcname = channel +  "_nj_ler";

    TF1 *fit_non = new TF1(funcname.c_str(),combined, rlow,rend,6);

	fit_non->SetLineColorAlpha(cfnon, falpha);
	fit_non->SetLineWidth(linewidth);
    fit_non->SetNpx(2000);

    all_non->Fit(fexp, "OQ", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_non->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(0, 0.2);
    flangaus->SetParameter(1, 0.8);
    flangaus->SetParameter(2, 125.);
    flangaus->SetParameter(3, 0.22);

    all_non->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_non->SetParameter (j+2, flangaus->GetParameter(j) );

    all_non->Fit(fit_non, "LQ", "", rlow,rend);
    cout << "NJ-ler: " << fit_non->GetMaximumX(rup, 5) << endl;

    // Fit ler-ALL
    funcname = channel +  "_ler_all";

    TF1 *fit_ler = new TF1(funcname.c_str(),combined, rlow,rend,6);

    fit_ler->SetLineColorAlpha(cfler, falpha);
    fit_ler->SetLineWidth(linewidth);
    fit_ler->SetNpx(2000);

    all_ler->Fit(fexp, "QO", "", rlow, rup);

    for ( int j = 0; j < 2; ++j) fit_ler->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(0, 0.2);
    flangaus->SetParameter(1, 0.8);
    flangaus->SetParameter(2, 125);
    flangaus->SetParameter(3, 0.22);

    all_ler->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_ler->SetParameter (j+2, flangaus->GetParameter(j) );

    all_ler->Fit(fit_ler, "LQ", "", rlow,rend);
    cout << "ler-ALL: " << fit_ler->GetMaximumX(rup, 5) << endl;

    // Fit ler-ALL BUNCH
    funcname = channel +  "_ler_all_bunch";

    TF1 *fit_ler_bunch = new TF1(funcname.c_str(),combined, rlow,rend,6);

    fit_ler_bunch->SetLineColorAlpha(cfbunch, falpha);
    fit_ler_bunch->SetLineWidth(linewidth);
    fit_ler_bunch->SetNpx(2000);

    fexp->SetParameters(5, 8);
    fexp->SetParLimits(0,1,1e8);
    fexp->SetParLimits(1,1,1e8);
    all_ler_bunch->Fit(fexp, "OQ", "", rlow, 0.31);
    cout << "rlow: " << rlow << ", rup: "<< rup<< endl;
    for ( int j = 0; j < 2; ++j) fit_ler_bunch->SetParameter (j, fexp->GetParameter(j) );
    flangaus->SetParameter(0, 0.16);
    flangaus->SetParameter(1, 0.85);
    flangaus->SetParameter(2, 8.);
    flangaus->SetParameter(3, 0.1);

    all_ler_bunch->Fit(flangaus, "OQ", "", rup,2);

    for ( int j = 0; j < 4; ++j) fit_ler_bunch->SetParameter (j+2, flangaus->GetParameter(j) );

    all_ler_bunch->Fit(fit_ler_bunch, "LQ", "", rlow,rend);
    cout << "ler-ALL BUNCH: " << fit_ler_bunch->GetMaximumX(rup, 5) << endl;
    // -----   Axis   ---------------------
	TH1F* axis = new TH1F("axis", "axis", 100000, 0.14, 150);


    //TH1F* axis = (TH1F*) rfile->Get((channel+"_hit_energy_spectrum").c_str());
    //axis->SetDirectory(0);
    // axis->SetLineColorAlpha( c1, 1.);
    // axis->SetFillColorAlpha( c1, 1.);
    // axis->SetMarkerColorAlpha(c1, 1);
    // axis->SetMarkerSize(1.0);
    // // axis->SetLineWidth(linewidth);
	// axis->SetLineWidth(1.5);
    // axis->SetFillStyle(0);

	all_non->GetXaxis()->SetTitle("Hit Energy [MIP]");
	all_non->GetXaxis()->SetTitleOffset(1.2);
	all_non->GetYaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]");

	all_non->GetXaxis()->SetRangeUser(0.11, 150);
	all_non->GetYaxis()->SetRangeUser(1e-2, 5e3);

 	// stringstream ss;
    //  double binwidth = ler_ref_r->GetBinWidth(3);
    //  ss << fixed << setprecision(2) << "Entries [1/" << binwidth << " MIP #times s]";
    //  axis->GetYaxis()->SetTitle(ss.str().c_str());

	// -----   Text and legend ----

//	TLegend* leg =  new TLegend(0.63, 0.66, 0.82, 0.82);
	// TLegend* leg =  new TLegend(0.5, 0.64, 0.7, 0.85);
    TLegend* leg =  new TLegend(0.68, 0.64, 0.88, 0.85);
	    //leg->SetHeader("");
	//    leg->SetNColumns(2);
	leg->SetBorderSize(0);
	leg->SetFillColor(0);

	// leg->AddEntry(make_box)

	int legfontsize = leg->GetTextSize();
	int legfont     = leg->GetTextFont();
//
    leg->AddEntry(make_box(all_non, "EMPTY"), all_non_target.data.c_str(), "f");
    leg->AddEntry(make_box(all_ler, "EMPTY"), all_ler_target.data.c_str(), "f");
    leg->AddEntry(make_box(all_ler_bunch, "EMPTY"), (all_ler_target.data + "_{ib}").c_str(), "f");
    leg->AddEntry(make_box(ler_ref_r, "EMPTY"), ler_ref_target.data.c_str(), "f");


// 	// leg->AddEntry(set_line_width(err,0), "Stat. Unc.", "f");
// 	// leg->AddEntry(set_line_width_f(fit,3), "Exp. + Landau*Gaus Fit", "l");
//
// 	leg->AddEntry(make_box(all_non), "Non-Injection", "f");
//
// 	stringstream ss;
//     ss << fixed << setprecision(2) << "MPV_{NI.} = " << fit_non->GetMaximumX(rup, 5) << " MIP";
//
// 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
//
// 	leg->AddEntry(make_box(all_ler), "ler Injection", "f");
// 	ss.str("");
// 	ss << fixed << setprecision(2) << "MPV_{HI.} = " << fit_all->GetMaximumX(rup, 5) << " MIP";
// 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
//
// 	leg->AddEntry(make_box(ler_ref_r), "ler Ref. Inj.", "f");
// 	ss.str("");
// 	ss << fixed << setprecision(2) << "MPV_{HRI.} = " << fit_ref->GetMaximumX(rup, 5) << " MIP";
// 	leg->AddEntry((TObject*)0, ss.str().c_str(), "");
// //	leg->AddEntry((TObject*)0, "Injection Events", "");
//
	TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

	// ---- Drawing ----
    //can->cd();
    all_non->Draw("axis");


    //
	all_non->DrawCopy("same hist");
	fit_non->Draw("same");

//    axis->GetXaxis()->SetLimits(0.14, 150);
//    axis->GetXaxis()->SetRange(0.15, 150);
    // can->Modified();
    // can->Update();

   all_ler->Draw("same hist");
   fit_ler->Draw("same");
    // can->Update();
    //
    //
   all_ler_bunch->Draw("same hist");
   fit_ler_bunch->Draw("same");
    // can->Update();
    //
//    ler_ref->Draw("same hist");
	ler_ref_r->Draw("same hist");
	fit_ref->Draw("same");
    // can->Update();

    // fexp->Draw("same");
    // flangaus->Draw("same");
	// her_ref_r->DrawCopy("same hist");
	//
	// all_non->SetFillColorAlpha(kWhite, 0);
	// all_non->Draw("same hist");
	//
	// all_her->SetFillColorAlpha(kWhite, 0);
	// all_her->Draw("same hist");
	//
	// her_ref_r->SetFillColorAlpha(kWhite, 0);
	// her_ref_r->Draw("same hist");

	ttext->DrawLatex(0.43, 0.88, ("Hit Energy Spectrum: "+channel).c_str());
	leg->Draw();

	can->SaveAs((ler_ref_target.pathout+"/HitEnergy/"+ler_ref_target.fileout+"_hit_energy_"+channel+".pdf").c_str());
    can->SaveAs((ler_ref_target.pathout+"/HitEnergy/"+ler_ref_target.fileout+"_hit_energy_"+channel+".png").c_str());
    can->SaveAs((ler_ref_target.pathout+"/HitEnergy/"+ler_ref_target.fileout+"_hit_energy_"+channel+".eps").c_str());
    can->SaveAs((ler_ref_target.pathout+"/HitEnergy/"+ler_ref_target.fileout+"_hit_energy_"+channel+".jpg").c_str());
}


void hitenergy(string channel = "FWD1")
{
	double xlow = 0;
	double xup 	= 150;

	// ---- Do the canvas ---
	int width = round(vis_multi*textwidth*pt2pixel*1/2.);
    double ratio = 3./3.;

    string cantitle = "ler_injections_hit_energy_spectrum_" + channel;
    TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

	can->SetLogx();
    can->SetLogy();

    // -----   Colors and Style   ---------------------
    double linewidth = 2;
	double falpha = 1;

	int c_ler_all = tab20_green->GetNumber();
	int cf_ler_all = c_ler_all;

	int c_her_all = tab20_orange->GetNumber();
	int cf_her_all = c_her_all;

    int c_ler_all_ib = tab20_red->GetNumber();
    int cf_ler_all_ib = c_ler_all_ib;

    int c_her_all_ib = tab20_blue->GetNumber();
    int cf_her_all_ib = c_her_all_ib;

    int c_ni_non = kBlack;
    int cf_ni_non  = c_ni_non ;

    int c_her_non = tab20_lblue->GetNumber();
    int cf_her_non  = c_her_non ;

	// -----  HISTOS   ---------------------

	// LER-ALL

	Target ler_all_target = GetTarget("LER-ALL");
	TFile* ler_all_rfile = new TFile((ler_all_target.pathin+"/"+ler_all_target.filein).c_str(), "open");

	TH1F* ler_all = (TH1F*) ler_all_rfile->Get((channel+"_hit_energy_spectrum").c_str());
	ler_all->SetDirectory(0);

	if(ler_all_target.n != -1)
	{
		double scale = ler_all_target.n*0.8e-9*3*3;
		ler_all->Scale(1./scale);
	}

	ler_all->SetLineColor(c_ler_all);
	ler_all->SetLineWidth(linewidth);

    double largest_energy =0;

    for(int i = 1; i <= ler_all->GetNbinsX(); ++i){
        if(ler_all->GetBinContent(i) > 0.0000000001){
            if(ler_all->GetBinCenter(i)> largest_energy){
                largest_energy =ler_all->GetBinCenter(i);
            }
        }
    }
    cout << channel << " LER-ALL max: " << largest_energy << endl;

    // HER-ALL
    Target her_all_target = GetTarget("HER-ALL");
    TFile* her_all_rfile = new TFile((her_all_target.pathin+"/"+her_all_target.filein).c_str(), "open");

    TH1F* her_all = (TH1F*) her_all_rfile->Get((channel+"_hit_energy_spectrum").c_str());
    her_all->SetDirectory(0);

    if(her_all_target.n != -1)
    {
        double scale = her_all_target.n*0.8e-9*3*3;
        her_all->Scale(1./scale);
    }

    her_all->SetLineColor(c_her_all);
    her_all->SetLineWidth(linewidth);

    largest_energy =0;

    for(int i = 1; i <= her_all->GetNbinsX(); ++i){
        if(her_all->GetBinContent(i) > 0.0000000001){
            if(her_all->GetBinCenter(i)> largest_energy){
                largest_energy =her_all->GetBinCenter(i);
            }
        }
    }
    cout << channel << " HER-ALL max: " << largest_energy << endl;

    // NJ-ALL
    Target ni_non_target = GetTarget("NJ-ALL");
    TFile* ni_non_rfile = new TFile((ni_non_target.pathin+"/"+ni_non_target.filein).c_str(), "open");

    TH1F* ni_non = (TH1F*) ni_non_rfile->Get((channel+"_hit_energy_spectrum").c_str());
    ni_non->SetDirectory(0);

    if(ni_non_target.n != -1)
    {
        double scale = ni_non_target.n*0.8e-9*3*3;
        ni_non->Scale(1./scale);
    }

    largest_energy =0;

    for(int i = 1; i <= ni_non->GetNbinsX(); ++i){
        if(ni_non->GetBinContent(i) > 0.0000000001){
            if(ni_non->GetBinCenter(i)> largest_energy){
                largest_energy =ni_non->GetBinCenter(i);
            }
        }
    }
    cout << channel << " NI-ALL max: " << largest_energy << endl;

    ni_non->SetLineColor(c_ni_non);
    ni_non->SetLineWidth(linewidth);

    // LER-ALL IB
    Target ler_all_ib_target = GetTarget("LER-ALL");
    TFile* ler_all_ib_rfile = new TFile((ler_all_ib_target.pathin+"/"+ler_all_ib_target.filein).c_str(), "open");

    TH1F* ler_all_ib = (TH1F*) ler_all_ib_rfile->Get((channel+"_bunch_hit_energy_spectrum").c_str());
    ler_all_ib->SetDirectory(0);

    if(ler_all_ib_target.n != -1)
    {
        double scale = ler_all_ib_target.n*0.8e-9*3*3;
        ler_all_ib->Scale(1./scale);
    }

    ler_all_ib->SetLineColor(c_ler_all_ib);
    ler_all_ib->SetLineWidth(linewidth);

    // HER-ALL IB
    Target her_all_ib_target = GetTarget("HER-ALL");
    TFile* her_all_ib_rfile = new TFile((her_all_ib_target.pathin+"/"+her_all_ib_target.filein).c_str(), "open");

    TH1F* her_all_ib = (TH1F*) her_all_ib_rfile->Get((channel+"_bunch_hit_energy_spectrum_1").c_str());
    her_all_ib->SetDirectory(0);

    if(her_all_ib_target.n != -1)
    {
        double scale = her_all_ib_target.n*0.8e-9*3*3;
        her_all_ib->Scale(1./scale);
    }

    her_all_ib->SetLineColor(c_her_all_ib);
    her_all_ib->SetLineWidth(linewidth);
	// -----   Fits   ---------------------

	string funcname = channel +  "_ler_all";

    double rlow = ler_all->GetBinLowEdge(2);
    cout << channel << " rlow: " << rlow << endl;;

    double min = 10000;
    int minbin = ler_all->FindBin(rlow);

    for(int bin = ler_all->FindBin(rlow); bin < ler_all->FindBin(1); ++bin)
    {
        if(ler_all->GetBinContent(bin) <= min)
        {
            min = ler_all->GetBinContent(bin);
            minbin = bin;
        }
    }
    double rup = ler_all->GetBinCenter(minbin);

    double rstart = 0.25;
    double rend = 2;

	TF1 *fit_ler_all = new TF1(funcname.c_str(),langaufun, rstart,rend,4);

	fit_ler_all->SetLineColorAlpha(c_ler_all, falpha);
	fit_ler_all->SetLineWidth(linewidth);
    fit_ler_all->SetNpx(2000);

    // funcname = "exp_fit_" + channel;
	// fexp = new TF1(funcname.c_str(),exponential, 0, 150,2);

	// funcname = "langaus_fit_" + channel;
	// flangaus = new TF1(funcname.c_str(),langaufun, 0, 150,4);

    // ler_all->Fit(fexp, "OQ", "", rlow, rup);
    //
    // for ( int j = 0; j < 2; ++j) fit_ler_all->SetParameter (j, fexp->GetParameter(j) );
    fit_ler_all->SetParameter(0, 0.15);
    fit_ler_all->SetParameter(1, 0.7);
    // flangaus->SetParameter(2, ler_ref_r->Integral(ler_ref_r->FindBin(rup), ler_ref_r->FindBin(2.5)));
    fit_ler_all->SetParameter(2, 100);
    fit_ler_all->SetParameter(3, 0.11);

    //flangaus->FixParameter(1, 1);
    // cout << "rup: " << rup<< endl;
    // ler_all->Fit(flangaus, "OQ", "", rup,2);
    //
    // for ( int j = 0; j < 4; ++j) fit_ler_all->SetParameter (j+2, flangaus->GetParameter(j) );

    ler_all->Fit(fit_ler_all, "QL", "", rstart,rend);
    cout << "LER-ALL mode: " << fit_ler_all->GetMaximumX(0.5, 5) << endl;

    // Fit LER-ALL IB
    funcname = channel +  "_ler_all_ib";

    TF1 *fit_ler_all_ib = new TF1(funcname.c_str(),langaufun, rstart,rend,4);

	fit_ler_all_ib->SetLineColorAlpha(c_ler_all_ib, falpha);
	fit_ler_all_ib->SetLineWidth(linewidth);
    fit_ler_all_ib->SetNpx(2000);

    // all_non->Fit(fexp, "OQ", "", rlow, rup);
    //
    // for ( int j = 0; j < 2; ++j) fit_non->SetParameter (j, fexp->GetParameter(j) );
    fit_ler_all_ib->SetParameter(0, 0.18);
    fit_ler_all_ib->SetParameter(1, 0.7);
    // flangaus->SetParameter(2, ler_ref_r->Integral(ler_ref_r->FindBin(rup), ler_ref_r->FindBin(2.5)));
    fit_ler_all_ib->SetParameter(2, 24);
    fit_ler_all_ib->SetParameter(3, 0.09);
    // fit_ler_all_ib->SetParameter(0, 0.2);
    // fit_ler_all_ib->SetParameter(1, 0.8);
    // fit_ler_all_ib->SetParameter(2, 125.);
    // fit_ler_all_ib->SetParameter(3, 0.22);

    // all_non->Fit(flangaus, "OQ", "", rup,2);
    //
    // for ( int j = 0; j < 4; ++j) fit_non->SetParameter (j+2, flangaus->GetParameter(j) );
    //
    ler_all_ib->Fit(fit_ler_all_ib, "LQR", "", rstart,rend);
        cout << "LER-ALL_ib mode: " << fit_ler_all_ib->GetMaximumX(0.5, 5) << endl;
    //
    // // Fit HER-ALL
    funcname = channel +  "_her_all";

    TF1 *fit_her_all = new TF1(funcname.c_str(),langaufun, rstart,rend,4);

    fit_her_all->SetLineColorAlpha(cf_her_all, falpha);
    fit_her_all->SetLineWidth(linewidth);
    fit_her_all->SetNpx(2000);
    //
    // all_ler->Fit(fexp, "QO", "", rlow, rup);
    //
    // for ( int j = 0; j < 2; ++j) fit_ler->SetParameter (j, fexp->GetParameter(j) );
    fit_her_all->SetParameter(0, 0.05);
    fit_her_all->SetParameter(1, 0.8);
    // flangaus->SetParameter(2, ler_ref_r->Integral(ler_ref_r->FindBin(rup), ler_ref_r->FindBin(2.5)));
    fit_her_all->SetParameter(2, 40);
    fit_her_all->SetParameter(3, 0.25);

    // all_ler->Fit(flangaus, "OQ", "", rup,2);
    //
    // for ( int j = 0; j < 4; ++j) fit_ler->SetParameter (j+2, flangaus->GetParameter(j) );
    //
    her_all->Fit(fit_her_all, "LQ", "", rstart,rend);
    cout << "HER-ALL mode: " << fit_her_all->GetMaximumX(0.5, 5) << endl;

    // HER-ALL ib
    funcname = channel +  "_her_all_ib";

    TF1 *fit_her_all_ib = new TF1(funcname.c_str(),langaufun, rstart,rend,4);

    fit_her_all_ib->SetLineColorAlpha(cf_her_all_ib, falpha);
    fit_her_all_ib->SetLineWidth(linewidth);
    fit_her_all_ib->SetNpx(2000);
    //
    // all_ler->Fit(fexp, "QO", "", rlow, rup);
    //
    // for ( int j = 0; j < 2; ++j) fit_ler->SetParameter (j, fexp->GetParameter(j) );
    fit_her_all_ib->SetParameter(0, 0.05);
    fit_her_all_ib->SetParameter(1, 0.8);
    // flangaus->SetParameter(2, ler_ref_r->Integral(ler_ref_r->FindBin(rup), ler_ref_r->FindBin(2.5)));
    fit_her_all_ib->SetParameter(2, 40);
    fit_her_all_ib->SetParameter(3, 0.25);

    // all_ler->Fit(flangaus, "OQ", "", rup,2);
    //
    // for ( int j = 0; j < 4; ++j) fit_ler->SetParameter (j+2, flangaus->GetParameter(j) );
    //
    her_all_ib->Fit(fit_her_all_ib, "LQRO", "", rlow,rend);
    cout << "HER-ALL_ib mode: " << fit_her_all_ib->GetMaximumX(0.5, 5) << endl;

    // NI-ALL
    funcname = channel +  "_ni_non";

    TF1 *fit_ni_non = new TF1(funcname.c_str(),langaufun, rstart,rend,4);

    fit_ni_non->SetLineColorAlpha(cf_ni_non, falpha);
    fit_ni_non->SetLineWidth(linewidth);
    fit_ni_non->SetNpx(2000);
    //
    // all_ler->Fit(fexp, "QO", "", rlow, rup);
    //
    // for ( int j = 0; j < 2; ++j) fit_ler->SetParameter (j, fexp->GetParameter(j) );
    fit_ni_non->SetParameter(0, 0.05);
    fit_ni_non->SetParameter(1, 0.8);
    // flangaus->SetParameter(2, ler_ref_r->Integral(ler_ref_r->FindBin(rup), ler_ref_r->FindBin(2.5)));
    fit_ni_non->SetParameter(2, 40);
    fit_ni_non->SetParameter(3, 0.25);

    // all_ler->Fit(flangaus, "OQ", "", rup,2);
    //
    // for ( int j = 0; j < 4; ++j) fit_ler->SetParameter (j+2, flangaus->GetParameter(j) );
    //
    ni_non->Fit(fit_ni_non, "LQRO", "", rlow,rend);
    cout << "NI-ALL mode: " << fit_ni_non->GetMaximumX(0.5, 5) << endl;

    // -----   Axis   ---------------------

	ler_all->GetXaxis()->SetTitle("Hit Energy [MIP]");
	ler_all->GetXaxis()->SetTitleOffset(1.2);
	ler_all->GetYaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIP #upoint s #upoint cm^{2})]");

	// ler_all->GetXaxis()->SetRangeUser(0.11, 150);
	ler_all->GetYaxis()->SetRangeUser(0.5e-2, 2.5e3);

 	// stringstream ss;
    //  double binwidth = ler_ref_r->GetBinWidth(3);
    //  ss << fixed << setprecision(2) << "Entries [1/" << binwidth << " MIP #times s]";
    //  axis->GetYaxis()->SetTitle(ss.str().c_str());

	// -----   Text and legend ----

//	TLegend* leg =  new TLegend(0.63, 0.66, 0.82, 0.82);
	// TLegend* leg =  new TLegend(0.5, 0.64, 0.7, 0.85);
    TLegend* leg =  new TLegend(0.74, 0.6, 0.94, 0.85);
	    //leg->SetHeader("");
	//    leg->SetNColumns(2);
	leg->SetBorderSize(0);
	leg->SetFillColor(0);

	// leg->AddEntry(make_box)

	int legfontsize = leg->GetTextSize();
	int legfont     = leg->GetTextFont();
//
    leg->AddEntry(make_box(ni_non, "EMPTY"), ni_non_target.data.c_str(), "f");
    leg->AddEntry(make_box(ler_all, "EMPTY"), ler_all_target.data.c_str(), "f");
    leg->AddEntry(make_box(ler_all_ib, "EMPTY"), (ler_all_target.data + "_{ib}").c_str(), "f");
    leg->AddEntry(make_box(her_all, "EMPTY"), her_all_ib_target.data.c_str(), "f");
    leg->AddEntry(make_box(her_all_ib, "EMPTY"), (her_all_target.data + "_{ib}").c_str(), "f");

	TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());

	// ---- Drawing ----

    ler_all->Draw("axis");

	ler_all->DrawCopy("same hist");
	fit_ler_all->Draw("same");

    ler_all_ib->Draw("same hist");
    fit_ler_all_ib->Draw("same");

    her_all->Draw("same hist");
    fit_her_all->Draw("same");
//    // fit_ler->Draw("same");

    her_all_ib->Draw("same hist");
    fit_her_all_ib->Draw("same");


    ni_non->Draw("same hist");
    fit_ni_non->Draw("same");
//    all_ler_bunch->Draw("same hist");
//    fit_ler_bunch->Draw("same");
//     // can->Update();
//     //
// //    ler_ref->Draw("same hist");
// 	ler_ref_r->Draw("same hist");
// 	fit_ref->Draw("same");

	ttext->DrawLatex(0.58, 0.88, ("Hit Energy Spectrum: "+channel).c_str());
	leg->Draw();

	can->SaveAs((ler_all_target.pathout+"/HitEnergy/"+ler_all_target.fileout+"_hit_energy_"+channel+".pdf").c_str());
    can->SaveAs((ler_all_target.pathout+"/HitEnergy/"+ler_all_target.fileout+"_hit_energy_"+channel+".png").c_str());
    can->SaveAs((ler_all_target.pathout+"/HitEnergy/"+ler_all_target.fileout+"_hit_energy_"+channel+".eps").c_str());
    can->SaveAs((ler_all_target.pathout+"/HitEnergy/"+ler_all_target.fileout+"_hit_energy_"+channel+".jpg").c_str());
}
// TCanvas* hitmap(TFile* rfile,Target target, string channel = "FWD1")
// {
//     int width = round(vis_multi*textwidth*pt2pixel*1/2);
//     //int width = round(vis_multi*.25**pt2pixel*1/2);
//     double ratio = 3./3.;
//
//     string cantitle = target.typein+"_"+channel+ "_can_hit_map";
//     TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
//     can->SetFillColor(kWhite);
//     can->SetBorderMode(0);
//     can->SetBorderSize(2);
//     can->SetRightMargin(0.17);
//     can->SetTopMargin(0.025);
//     can->SetFrameBorderMode(0);
//
//     can->SetLeftMargin(0.12);
// //    can->SetLogy();
// //	if(channel != "FWD1") can->SetRightMargin(0.23);
//     // -----   Colors and Style   ---------------------
//     double linewidth = 2;
//
//     int c1 = kBlack;
//     // int c2 = claws_gray->GetNumber();
//     // int c3 = claws_black->GetNumber();
//     // int cfit = TColor::GetColor("#2ca02c");
//     // int cfit = kAzure+1;
//     int cfit = claws_red->GetNumber();
//     // // Katha 112
//
//
//
//     // -----   Axis   ---------------------
//
//     TH2F* hist = (TH2F*) rfile->Get((channel+"_hit_map").c_str());
//     hist->SetDirectory(0);
//
//     int nbinsx = hist->GetNbinsX();
//     double xlow = hist->GetXaxis()->GetBinLowEdge(0);
//     double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);
//
//     int nbinsy = hist->GetNbinsY();
//     double ylow = hist->GetYaxis()->GetBinLowEdge(0)*1e6;
//     double yup = (hist->GetYaxis()->GetBinLowEdge(nbinsy)+hist->GetYaxis()->GetBinWidth(nbinsy))*1e6;
//
//     hist->SetBins(nbinsx, xlow, xup, nbinsy, ylow, yup);
//
// 	if(target.n != -1)
// 	{
// 		double scale = target.n*0.8e-9;
// 		hist->Scale(1./scale);
// 	}
//     // -----  Colors   ---------------------
//
//      gStyle->SetPalette(kRainBow);
//    //  const Int_t Number = 3;
//    //  Double_t Red[Number]    = { 1.00, 0.00};
//    //  Double_t Green[Number]  = { 0.00, 1.00};
//    //  Double_t Blue[Number]   = { 1.00, 0.00};
//    //  Double_t Length[Number] = { 0.00, 0.50};
//    //  Int_t nb=50;
//    //  TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,10);
//  //  TColor::InvertPalette();
//
//      // -----  Axis   ---------------------
//
//      // X Axis
// //	 hist->GetXaxis()->SetRangeUser(xlow, 64.5);
// //     hist->GetXaxis()->SetRangeUser(xlow, 14.5);
// 	   //   hist->GetXaxis()->SetRangeUser(xlow, 4);
//
//        // Y Axis
//      hist->GetYaxis()->SetTitle("Time in Turn [#mus]");
//      hist->GetYaxis()->SetTitleOffset(1.05);
//       // Z Axis
//      stringstream ss;
//      double binwidth_x = hist->GetXaxis()->GetBinWidth(3);
//      double binwidth_y = hist->GetYaxis()->GetBinWidth(3)*1e3;
//
//      cout << "Hitmap, binwidth_x: " << binwidth_x<< ", binwidth_y: "<< binwidth_y<<endl;
//      ss << fixed << setprecision(2) << "Entries [1/" << binwidth_x*binwidth_y << " ns #times MIP #times s]";
//      hist->GetZaxis()->SetTitle(ss.str().c_str());
// //    hist->GetZaxis()->SetTitle("Penis");
//      hist->GetZaxis()->SetTitleOffset(1.2);
//      // axis->GetXaxis()->SetRangeUser(-0.1, 7.5);
// 	// if(channel != "FWD1") hist->GetZaxis()->SetTitleOffset(1.9);
//
// 	 hist->GetZaxis()->SetNdivisions(506);
//
//     // ---- Drawing ----
// //    axis->Draw();
//     hist->Draw("colz");
//     // hist->Draw("surf2Z");
//      // hist->Draw("lego2z");
//     // fit->Draw("same");
//
//     return can;
// }

int hitmap_inj_non(string channel = "FWD1")
{


	// Targets
	Target target = GetTarget("NJ-ALL");
	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

	TH2F* hist = (TH2F*) rfile->Get((channel+"_hit_map").c_str());
	hist->SetDirectory(0);

	int nbinsx = hist->GetNbinsX();
	double xlow = hist->GetXaxis()->GetBinLowEdge(0);
	double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

	int nbinsy = hist->GetNbinsY();
	double ylow = hist->GetYaxis()->GetBinLowEdge(0)*1e6;
	double yup = (hist->GetYaxis()->GetBinLowEdge(nbinsy)+hist->GetYaxis()->GetBinWidth(nbinsy))*1e6;

	hist->SetBins(nbinsx, xlow, xup, nbinsy, ylow, yup);

	if(target.n != -1)
	{
		double scale = target.n*0.8e-9*3*3;
		double trev = 10061.4e-9;
		scale = scale*target.n*0.8e-9/trev;
		hist->Scale(1./scale);
	}

	hist->RebinY(50);
    // hist->RebinY(2);
    // hist->RebinX(2);
//	if(channel != "FWD1") can->SetRightMargin(0.23);
    // -----   Colors and Style   ---------------------
    double linewidth = 2;

    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int cfit = claws_red->GetNumber();
    // // Katha 112


    // -----   Axis   ---------------------


    // -----  Colors   ---------------------

// 	const Int_t Number = 3;
// Double_t Red[Number]    = { 1.00, 0.00, 0.00};
// Double_t Green[Number]  = { 0.00, 1.00, 0.00};
// Double_t Blue[Number]   = { 1.00, 0.00, 1.00};
// Double_t Length[Number] = { 0.00, 0.50, 1.00 };
// Int_t nb=50;
// TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,nb);
// f2->SetContour(nb);

     //gStyle->SetPalette(kRainBow);
	// gStyle->SetPalette(kBird);
	// gStyle->SetPalette(kCool);
	// TColor::InvertPalette();
	 // const Int_t Number = 3;
     // Double_t Red[Number]    = { 31./255., 114./255., 214./255.};
     // Double_t Green[Number]  = { 119./255.,158./255., 39./255.};
     // Double_t Blue[Number]   = { 180./255., 206./255.,40./255.};
     // Double_t Length[Number] = {0.0,0.25, 1.0 };

	 // const Int_t Number = 3;
	 //  Double_t Red[Number]    = { 31./255., 31./255., 214./255.};
	 //  Double_t Green[Number]  = { 119./255.,119./255., 39./255.};
	 //  Double_t Blue[Number]   = { 180./255., 180./255.,40./255.};
	 //  Double_t Length[Number] = {0.0,0.25, 1.0 };

	 // const Int_t Number = 2;
	 //  Double_t Red[Number]    = { 31./255., 214./255.};
	 //  Double_t Green[Number]  = { 119./255., 39./255.};
	 //  Double_t Blue[Number]   = { 180./255.,40./255.};
	 //  Double_t Length[Number] = {0., 1.0 };
     // //
    // const Int_t Number = 3;
    // Double_t Red[Number]    = { 31./255., 44./255.,  214./255.};
    // Double_t Green[Number]  = { 119./255., 160./255. , 39./255.};
    // Double_t Blue[Number]   = { 180./255., 44./255., 40./255.};
    //   Double_t Length[Number] = {0.4,0.5, 0.6 };

    // const Int_t Number = 5;
    //
    //   //
    //   Double_t Red[Number]    = { 31./255., 31./255., 44./255., 214./255., 214./255.};
    //   Double_t Green[Number]  = { 119./255.,119./255., 160./255., 39./255., 39./255.};
    //   Double_t Blue[Number]   = { 180./255., 180./255., 44./255., 40./255., 40./255.};
    //
    //   Double_t Length[Number] = {0.0,0.15,0.5,0.85,1.0 };


//      const Int_t Number = 5;
        // Double_t Red[Number]    = { 31./255., 255./255., 44./255., 214./255., 148./255.};
        // Double_t Green[Number]  = { 119./255.,127./255., 160./255., 39./255., 103./255.};
        // Double_t Blue[Number]   = { 180./255., 14./255., 44./255., 40./255., 198./255.};
        //
        // Double_t Red[Number]    = { 31./255., 255./255., 44./255., 148./255., 214./255.};
        // Double_t Green[Number]  = { 119./255.,127./255., 160./255., 103./255., 39./255.};
        // Double_t Blue[Number]   = { 180./255., 14./255., 44./255., 198./255., 40./255.};
        //
        // Double_t Length[Number] = {0.,0.5,0.6,0.75,1.0 };

        // const Int_t Number = 5;
        // Double_t Red[Number]    = { 31./255., 31./255., 44./255., 214./255., 148./255.};
        // Double_t Green[Number]  = { 119./255.,119./255., 160./255., 39./255., 103./255.};
        // Double_t Blue[Number]   = { 180./255.,180./255., 44./255., 40./255., 198./255.};
        //
        // Double_t Length[Number] = {0.,0.35,0.5,0.75,1.0 };

        const Int_t Number = 4;
        Double_t Red[Number]    = { 31./255., 44./255., 214./255., 148./255.};
        Double_t Green[Number]  = { 119./255., 160./255., 39./255., 103./255.};
        Double_t Blue[Number]   = { 180./255., 44./255., 40./255., 198./255.};

        Double_t Length[Number] = {0.15,0.4,0.6,0.85 };


     // //
     Int_t nb=50;
     TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,nb);
     hist->SetContour(nb);

     // -----  Axis   ---------------------

     // X Axis
	 hist->GetXaxis()->SetRangeUser(0, 8.);
//     hist->GetXaxis()->SetRangeUser(xlow, 14.5);
	   //   hist->GetXaxis()->SetRangeUser(xlow, 4);
       hist->SetMinimum( 5e-5);
      // hist->SetMaximum(1e-2);
       // Y Axis
     hist->GetYaxis()->SetTitle("t_{turn} [#mus]");
     hist->GetYaxis()->SetTitleOffset(1.05);
      // Z Axis
     stringstream ss;
     double binwidth_x = hist->GetXaxis()->GetBinWidth(3);
     double binwidth_y = hist->GetYaxis()->GetBinWidth(3)*1e3;

     cout << "Hitmap, binwidth_x: " << binwidth_x<< ", binwidth_y: "<< binwidth_y<<endl;
     ss << fixed << setprecision(2) << "Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]";
     hist->GetZaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]");
//    hist->GetZaxis()->SetTitle("Penis");
     hist->GetZaxis()->SetTitleOffset(1.65);
     // axis->GetXaxis()->SetRangeUser(-0.1, 7.5);
	// if(channel != "FWD1") hist->GetZaxis()->SetTitleOffset(1.9);

	 hist->GetZaxis()->SetNdivisions(506);

	 int width = round(vis_multi*textwidth*pt2pixel*1/2);
	 //int width = round(vis_multi*.25**pt2pixel*1/2);
	 double ratio = 3./3.;


     int linestyle = 9;

     TLine *line = new TLine(0.84, 0,0.84,10.37);
  //	TArrow* line = new TArrow(fcombined->GetMaximumX(rup, 5.), 5,fcombined->GetMaximumX(rup, 5.),fcombined->GetMaximum(rup, 5.), 0.015, "-|>");
     line->SetLineColor(kBlack);
  // /	line->SetFillStyle(0);
     line->SetLineStyle(linestyle);
     line->SetLineWidth(linewidth);


     TLegend* leg =  new TLegend(0.64, 0.56, 0.84, 0.85);
        //leg->SetHeader("");
    //    leg->SetNColumns(2);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetTextSize(gStyle->GetLegendTextSize());
    // leg->AddEntry(make_box)

    int legfontsize = leg->GetTextSize();
    int legfont     = leg->GetTextFont();

    // leg->AddEntry(make_box(both, "EMPTY"), target.data.c_str(), "f");
    leg->AddEntry(hist, target.data.c_str(), "f");


    TLatex* ttext = new TLatex();
    ttext->SetNDC();
    ttext->SetTextFont(legfont+20);
    ttext->SetTextSize(gStyle->GetLegendTextSize());



     TLatex* atext = new TLatex();
     atext->SetNDC();
     atext->SetTextColor(kBlack);
     atext->SetTextFont(legfont+20);
     atext->SetTextSize(gStyle->GetLegendTextSize());


	 // ---- Canvas ----
	 string cantitle = target.typein+"_"+channel+ "_can_hit_map";
	 TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	 can->SetFillColor(kWhite);
	 can->SetBorderMode(0);
	 can->SetBorderSize(2);
	 can->SetRightMargin(0.21);
	 can->SetTopMargin(0.025);
	 can->SetFrameBorderMode(0);

	 can->SetLeftMargin(0.12);
     can->SetLogz();

    // ---- Drawing ----
    //hist->GetYaxis()->SetMaximum(13);
    //hist->Draw("CONT4Z");
	hist->Draw("COLZ");
    line->Draw();
//	hcont4->Draw("CONT4Z");
    // hist->Draw("surf2Z");
    // hist->Draw("lego2z");
    // fit->Draw("same");
    ttext->DrawLatex(0.44, 0.88, "Time Resolved Hit");
     ttext->DrawLatex(0.44, 0.83,  "Energy Spectrum:");
    ttext->DrawLatex(0.44, 0.78, ("NI-ALL, " +channel).c_str());

    atext->DrawLatex(0.2, 0.16, "MPV_{L*G}");
//    leg->Draw("same");

	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".pdf").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".png").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".eps").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".jpg").c_str());

    return 0;
}

int hitmap_her(string channel = "FWD1")
{


	// Targets
	Target target = GetTarget("HER-ALL");
	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

	TH2F* hist = (TH2F*) rfile->Get((channel+"_hit_map").c_str());
	hist->SetDirectory(0);

	int nbinsx = hist->GetNbinsX();
	double xlow = hist->GetXaxis()->GetBinLowEdge(0);
	double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

	int nbinsy = hist->GetNbinsY();
	double ylow = hist->GetYaxis()->GetBinLowEdge(0)*1e6;
	double yup = (hist->GetYaxis()->GetBinLowEdge(nbinsy)+hist->GetYaxis()->GetBinWidth(nbinsy))*1e6;

	hist->SetBins(nbinsx, xlow, xup, nbinsy, ylow, yup);

	if(target.n != -1)
	{
		double scale = target.n*0.8e-9*3*3;
		double trev = 10061.4e-9;
		scale = scale*target.n*0.8e-9/trev;
		hist->Scale(1./scale);
	}

	hist->RebinY(10);
    // hist->RebinY(2);
    // hist->RebinX(2);
//	if(channel != "FWD1") can->SetRightMargin(0.23);
    // -----   Colors and Style   ---------------------
    double linewidth = 2;

    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int cfit = claws_red->GetNumber();
    // // Katha 112


    // -----   Axis   ---------------------



        const Int_t Number = 4;
        Double_t Red[Number]    = { 31./255., 44./255., 214./255., 148./255.};
        Double_t Green[Number]  = { 119./255., 160./255., 39./255., 103./255.};
        Double_t Blue[Number]   = { 180./255., 44./255., 40./255., 198./255.};

        Double_t Length[Number] = {0.15,0.4,0.6,0.85 };


     // //
     Int_t nb=50;
     TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,nb);
     hist->SetContour(nb);

     // -----  Axis   ---------------------

     // X Axis
	 hist->GetXaxis()->SetRangeUser(0, 105.);
//     hist->GetXaxis()->SetRangeUser(xlow, 14.5);
	   //   hist->GetXaxis()->SetRangeUser(xlow, 4);
      hist->SetMinimum( 5e-5);
       hist->SetMaximum(1e-2);
       // Y Axis
     hist->GetYaxis()->SetTitle("t_{turn} [#mus]");
     hist->GetYaxis()->SetTitleOffset(1.2);
      // Z Axis
     stringstream ss;
     double binwidth_x = hist->GetXaxis()->GetBinWidth(3);
     double binwidth_y = hist->GetYaxis()->GetBinWidth(3)*1e3;

     cout << "Hitmap, binwidth_x: " << binwidth_x<< ", binwidth_y: "<< binwidth_y<<endl;
     ss << fixed << setprecision(2) << "Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]";
     hist->GetZaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIP #upoint s #upoint cm^{2})]");
//    hist->GetZaxis()->SetTitle("Penis");
     hist->GetZaxis()->SetTitleOffset(1.8);
     // axis->GetXaxis()->SetRangeUser(-0.1, 7.5);
	// if(channel != "FWD1") hist->GetZaxis()->SetTitleOffset(1.9);

	 hist->GetZaxis()->SetNdivisions(506);

	 int width = round(vis_multi*textwidth*pt2pixel*1/2);
	 //int width = round(vis_multi*.25**pt2pixel*1/2);
	 double ratio = 3./3.;


     int linestyle = 9;

     TLine *line = new TLine(0.84, 0,0.84,10.37);
  //	TArrow* line = new TArrow(fcombined->GetMaximumX(rup, 5.), 5,fcombined->GetMaximumX(rup, 5.),fcombined->GetMaximum(rup, 5.), 0.015, "-|>");
     line->SetLineColor(kBlack);
  // /	line->SetFillStyle(0);
     line->SetLineStyle(linestyle);
     line->SetLineWidth(linewidth);


     TLegend* leg =  new TLegend(0.63, 0.64, 0.83, 0.85);
    //leg->SetHeader("");
//    leg->SetNColumns(2);
leg->SetBorderSize(0);
leg->SetFillColor(0);

// leg->AddEntry(make_box)

int legfontsize = leg->GetTextSize();
int legfont     = leg->GetTextFont();

     TLatex* ttext = new TLatex();
 ttext->SetNDC();
 ttext->SetTextFont(legfont+20);
 ttext->SetTextSize(gStyle->GetLegendTextSize());

	 // ---- Canvas ----
	 string cantitle = target.typein+"_"+channel+ "_can_hit_map";
	 TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	 can->SetFillColor(kWhite);
	 can->SetBorderMode(0);
	 can->SetBorderSize(2);
	 can->SetRightMargin(0.175);
	 can->SetTopMargin(0.025);
	 can->SetFrameBorderMode(0);

	 can->SetLeftMargin(0.09);
     can->SetLogz();

    // ---- Drawing ----

	hist->Draw("COLZ");

    ttext->DrawLatex(0.155, 0.9, ("Time Resolved Hit Energy Spectrum: " + channel + ", HER-ALL").c_str());
    // ttext->DrawLatex(0.29, 0.83, "Spectrum:");
    // ttext->DrawLatex(0.29, 0.78, (channel+ ", HER-ALL").c_str());

	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".pdf").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".png").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".eps").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".jpg").c_str());

    return 0;
}

int hitmap_ler(string channel = "FWD1")
{

	// Targets
	Target target = GetTarget("LER-ALL");
	TFile* rfile = new TFile((target.pathin+"/"+target.filein).c_str(), "open");

	TH2F* hist = (TH2F*) rfile->Get((channel+"_hit_map").c_str());
	hist->SetDirectory(0);

	int nbinsx = hist->GetNbinsX();
	double xlow = hist->GetXaxis()->GetBinLowEdge(0);
	double xup = hist->GetXaxis()->GetBinLowEdge(nbinsx)+hist->GetXaxis()->GetBinWidth(nbinsx);

	int nbinsy = hist->GetNbinsY();
	double ylow = hist->GetYaxis()->GetBinLowEdge(0)*1e6;
	double yup = (hist->GetYaxis()->GetBinLowEdge(nbinsy)+hist->GetYaxis()->GetBinWidth(nbinsy))*1e6;

	hist->SetBins(nbinsx, xlow, xup, nbinsy, ylow, yup);

	if(target.n != -1)
	{
		double scale = target.n*0.8e-9*3*3;
		double trev = 10061.4e-9;
		scale = scale*target.n*0.8e-9/trev;
		hist->Scale(1./scale);
	}

	hist->RebinY(50);
    // hist->RebinY(2);
    // hist->RebinX(2);
//	if(channel != "FWD1") can->SetRightMargin(0.23);
    // -----   Colors and Style   ---------------------
    double linewidth = 2;

    int c1 = kBlack;
    // int c2 = claws_gray->GetNumber();
    // int c3 = claws_black->GetNumber();
    // int cfit = TColor::GetColor("#2ca02c");
    // int cfit = kAzure+1;
    int cfit = claws_red->GetNumber();
    // // Katha 112


    // -----   Axis   ---------------------


    // -----  Colors   ---------------------

// 	const Int_t Number = 3;
// Double_t Red[Number]    = { 1.00, 0.00, 0.00};
// Double_t Green[Number]  = { 0.00, 1.00, 0.00};
// Double_t Blue[Number]   = { 1.00, 0.00, 1.00};
// Double_t Length[Number] = { 0.00, 0.50, 1.00 };
// Int_t nb=50;
// TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,nb);
// f2->SetContour(nb);

     //gStyle->SetPalette(kRainBow);
	// gStyle->SetPalette(kBird);
	// gStyle->SetPalette(kCool);
	// TColor::InvertPalette();
	 // const Int_t Number = 3;
     // Double_t Red[Number]    = { 31./255., 114./255., 214./255.};
     // Double_t Green[Number]  = { 119./255.,158./255., 39./255.};
     // Double_t Blue[Number]   = { 180./255., 206./255.,40./255.};
     // Double_t Length[Number] = {0.0,0.25, 1.0 };

	 // const Int_t Number = 3;
	 //  Double_t Red[Number]    = { 31./255., 31./255., 214./255.};
	 //  Double_t Green[Number]  = { 119./255.,119./255., 39./255.};
	 //  Double_t Blue[Number]   = { 180./255., 180./255.,40./255.};
	 //  Double_t Length[Number] = {0.0,0.25, 1.0 };

	 // const Int_t Number = 2;
	 //  Double_t Red[Number]    = { 31./255., 214./255.};
	 //  Double_t Green[Number]  = { 119./255., 39./255.};
	 //  Double_t Blue[Number]   = { 180./255.,40./255.};
	 //  Double_t Length[Number] = {0., 1.0 };
     // //
    // const Int_t Number = 3;
    // Double_t Red[Number]    = { 31./255., 44./255.,  214./255.};
    // Double_t Green[Number]  = { 119./255., 160./255. , 39./255.};
    // Double_t Blue[Number]   = { 180./255., 44./255., 40./255.};
    //   Double_t Length[Number] = {0.4,0.5, 0.6 };

    // const Int_t Number = 5;
    //
    //   //
    //   Double_t Red[Number]    = { 31./255., 31./255., 44./255., 214./255., 214./255.};
    //   Double_t Green[Number]  = { 119./255.,119./255., 160./255., 39./255., 39./255.};
    //   Double_t Blue[Number]   = { 180./255., 180./255., 44./255., 40./255., 40./255.};
    //
    //   Double_t Length[Number] = {0.0,0.15,0.5,0.85,1.0 };


//      const Int_t Number = 5;
        // Double_t Red[Number]    = { 31./255., 255./255., 44./255., 214./255., 148./255.};
        // Double_t Green[Number]  = { 119./255.,127./255., 160./255., 39./255., 103./255.};
        // Double_t Blue[Number]   = { 180./255., 14./255., 44./255., 40./255., 198./255.};
        //
        // Double_t Red[Number]    = { 31./255., 255./255., 44./255., 148./255., 214./255.};
        // Double_t Green[Number]  = { 119./255.,127./255., 160./255., 103./255., 39./255.};
        // Double_t Blue[Number]   = { 180./255., 14./255., 44./255., 198./255., 40./255.};
        //
        // Double_t Length[Number] = {0.,0.5,0.6,0.75,1.0 };

        // const Int_t Number = 5;
        // Double_t Red[Number]    = { 31./255., 31./255., 44./255., 214./255., 148./255.};
        // Double_t Green[Number]  = { 119./255.,119./255., 160./255., 39./255., 103./255.};
        // Double_t Blue[Number]   = { 180./255.,180./255., 44./255., 40./255., 198./255.};
        //
        // Double_t Length[Number] = {0.,0.35,0.5,0.75,1.0 };

        const Int_t Number = 4;
        Double_t Red[Number]    = { 31./255., 44./255., 214./255., 148./255.};
        Double_t Green[Number]  = { 119./255., 160./255., 39./255., 103./255.};
        Double_t Blue[Number]   = { 180./255., 44./255., 40./255., 198./255.};

        Double_t Length[Number] = {0.15,0.4,0.6,0.85 };


     // //
     Int_t nb=50;
     TColor::CreateGradientColorTable(Number,Length,Red,Green,Blue,nb);
     hist->SetContour(nb);

     // -----  Axis   ---------------------

     // X Axis
	 hist->GetXaxis()->SetRangeUser(0, 105.);
//     hist->GetXaxis()->SetRangeUser(xlow, 14.5);
	   //   hist->GetXaxis()->SetRangeUser(xlow, 4);
       hist->SetMinimum( 5e-5);
      // hist->SetMaximum(1e-2);
       // Y Axis
     hist->GetYaxis()->SetTitle("t_{turn} [#mus]");
     hist->GetYaxis()->SetTitleOffset(1.05);
      // Z Axis
     stringstream ss;
     double binwidth_x = hist->GetXaxis()->GetBinWidth(3);
     double binwidth_y = hist->GetYaxis()->GetBinWidth(3)*1e3;

     cout << "Hitmap, binwidth_x: " << binwidth_x<< ", binwidth_y: "<< binwidth_y<<endl;
     ss << fixed << setprecision(2) << "Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]";
     hist->GetZaxis()->SetTitle("Entries/(Event #upoint Time #upoint Area) [1/(MIPscm^{2})]");
//    hist->GetZaxis()->SetTitle("Penis");
     hist->GetZaxis()->SetTitleOffset(1.65);
     // axis->GetXaxis()->SetRangeUser(-0.1, 7.5);
	// if(channel != "FWD1") hist->GetZaxis()->SetTitleOffset(1.9);

	 hist->GetZaxis()->SetNdivisions(506);

	 int width = round(vis_multi*textwidth*pt2pixel*1/2);
	 //int width = round(vis_multi*.25**pt2pixel*1/2);
	 double ratio = 3./3.;


     int linestyle = 9;

     TLine *line = new TLine(0.84, 0,0.84,10.37);
  //	TArrow* line = new TArrow(fcombined->GetMaximumX(rup, 5.), 5,fcombined->GetMaximumX(rup, 5.),fcombined->GetMaximum(rup, 5.), 0.015, "-|>");
     line->SetLineColor(kBlack);
  // /	line->SetFillStyle(0);
     line->SetLineStyle(linestyle);
     line->SetLineWidth(linewidth);

     TLegend* leg =  new TLegend(0.63, 0.64, 0.83, 0.85);
    //leg->SetHeader("");
//    leg->SetNColumns(2);
leg->SetBorderSize(0);
leg->SetFillColor(0);

// leg->AddEntry(make_box)

int legfontsize = leg->GetTextSize();
int legfont     = leg->GetTextFont();

     TLatex* ttext = new TLatex();
 ttext->SetNDC();
 ttext->SetTextFont(legfont+20);
 ttext->SetTextSize(gStyle->GetLegendTextSize());

	 // ---- Canvas ----
	 string cantitle = target.typein+"_"+channel+ "_can_hit_map";
	 TCanvas * can = new TCanvas(cantitle.c_str(),cantitle.c_str(), width, width/ratio);
	 can->SetFillColor(kWhite);
	 can->SetBorderMode(0);
	 can->SetBorderSize(2);
	 can->SetRightMargin(0.21);
	 can->SetTopMargin(0.025);
	 can->SetFrameBorderMode(0);

	 can->SetLeftMargin(0.12);
     can->SetLogz();

    // ---- Drawing ----

    //hist->Draw("CONT4Z");
	hist->Draw("COLZ");

    // ttext->DrawLatex(0.29, 0.88, ("Hit Map: "+channel+ ", LER-ALL").c_str());


        ttext->DrawLatex(0.29, 0.88, "Time Resolved Hit Energy");
        ttext->DrawLatex(0.29, 0.83, "Spectrum:");
        ttext->DrawLatex(0.29, 0.78, (channel+ ", LER-ALL").c_str());
//    line->Draw();
//	hcont4->Draw("CONT4Z");
    // hist->Draw("surf2Z");
    // hist->Draw("lego2z");
    // fit->Draw("same");

	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".pdf").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".png").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".eps").c_str());
	can->SaveAs((target.pathout+"/HitEnergy/"+target.fileout+"_hit_map_"+channel+".jpg").c_str());

    return 0;
}

void plt_hitenergy()
{
    SetPhDStyle();

	// Target target = GetTarget("NJ-ALL");

	// Open text file
	// ofstream txtfile;
	// txtfile.open ((target.pathout+"/HitEnergy/"+target.fileout+"_hit_energy.txt").c_str());
	// txtfile.close();

	string chs[] = {"FWD1", "FWD2", "FWD3"};
	// string chs[] = {"FWD1"};

	for(auto& ch: chs)
	{
	 	 // hitenergy_noinj(ch);
	 //	hitmap_inj_non(ch);
     //  hitenergy_her(ch);
      // hitenergy_ler(ch);
         // hitmap_her(ch);
        // hitmap_ler(ch);
    //    hitenergy_ler(ch);
      hitenergy(ch);
	}

}
