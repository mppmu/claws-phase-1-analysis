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


void extract_correction_factor()
{
    TFile* rfile = new TFile("./run_900122_systematics_ph1v4_19.7.root");

    TH1F* fwd1 = (TH1F*) rfile->Get("FWD1_mip_per_event");
    TH1F* fwd2 = (TH1F*) rfile->Get("FWD2_mip_per_event");
    TH1F* fwd3 = (TH1F*) rfile->Get("FWD3_mip_per_event");
    TH1F* fwd4 = (TH1F*) rfile->Get("FWD4_mip_per_event");

    TH1F* tres = (TH1F*) rfile->Get("time_resolution");
    tres->GetXaxis()->SetRangeUser(-20e-9,20e-9);

    TF1 *langaus1 = fwd1->GetFunction("FWD1_mip_per_eventlangaus");
    TF1 *langaus2 = fwd2->GetFunction("FWD2_mip_per_eventlangaus");
    TF1 *langaus3 = fwd3->GetFunction("FWD3_mip_per_eventlangaus");
    TF1 *langaus4 = fwd4->GetFunction("FWD4_mip_per_eventlangaus");

    // Double_t pllo[4], plhi[4];
    //
    // pllo[0]=0.0; pllo[1]=0.0;   pllo[2]=0; pllo[3]=0;
    // plhi[0]=1.0; plhi[1]=2.0; plhi[2]=1000000.0; plhi[3]=2.0;
    //
    // double rlow1 = fwd1->GetBinLowEdge(5);
    // double rup1 = fwd1->GetMean()*4.0;
    //
    // double rlow2 = fwd2->GetBinLowEdge(5);
    // double rup2 = fwd2->GetMean()*4.0;
    //
    // double rlow3 = fwd3->GetBinLowEdge(5);
    // double rup3 = fwd3->GetMean()*4.0;
    //
    // double rlow4 = fwd4->GetBinLowEdge(5);
    // double rup4 = fwd4->GetMean()*4.0;
    // // Shit basically stolen from the root example:
    // // https://root.cern.ch/root/html/tutorials/fit/langaus.C.html
    // TF1 *langaus1 = new TF1("langaus",langaufun, rlow1, rup1,4);
    // TF1 *langaus2 = new TF1("langaus",langaufun, rlow2, rup2,4);
    // TF1 *langaus3 = new TF1("langaus",langaufun, rlow3, rup3,4);
    // TF1 *langaus4 = new TF1("langaus",langaufun, rlow4, rup4,4);
    //
    // double par1 = fwd1->GetBinCenter(fwd1->GetMaximumBin());
    // double par2 = fwd1->GetBinCenter(fwd2->GetMaximumBin());
    // double par3 = fwd1->GetBinCenter(fwd3->GetMaximumBin());
    // double par4 = fwd1->GetBinCenter(fwd4->GetMaximumBin());
    //
    // langaus1->SetParameters(0.1, par1, 500., 0.1);
    // langaus1->SetParNames("Width","MP","Area","GSigma");
    //
    // langaus2->SetParameters(0.1, par2, 500., 0.1);
    // langaus2->SetParNames("Width","MP","Area","GSigma");
    //
    // langaus3->SetParameters(0.1, par3, 500., 0.1);
    // langaus3->SetParNames("Width","MP","Area","GSigma");
    //
    // langaus4->SetParameters(0.1, par4, 500., 0.1);
    // langaus4->SetParNames("Width","MP","Area","GSigma");
    //
    // for ( int j = 0; j < 4; ++j)
    // {
    //         langaus1->SetParLimits(j, pllo[j], plhi[j]);
    //         langaus2->SetParLimits(j, pllo[j], plhi[j]);
    //         langaus3->SetParLimits(j, pllo[j], plhi[j]);
    //         langaus4->SetParLimits(j, pllo[j], plhi[j]);
    // }
    //
    //
    // fwd1->Fit(langaus1, "QRSL");
    // fwd2->Fit(langaus2, "QRSL");
    // fwd3->Fit(langaus3, "QRSL");
    // fwd4->Fit(langaus4, "QRSL");

    cout << "Max 1: " << langaus1->GetMaximumX() << endl;
    cout << "Max 2: " << langaus2->GetMaximumX() << endl;
    cout << "Max 3: " << langaus3->GetMaximumX() << endl;
    cout << "Max 4: " << langaus4->GetMaximumX() << endl;

    cout << "Corr 1: " << 1-langaus1->GetMaximumX() << endl;
    cout << "Corr 2: " << 1-langaus2->GetMaximumX() << endl;
    cout << "Corr 3: " << 1-langaus3->GetMaximumX() << endl;
    cout << "Corr 4: " << 1-langaus4->GetMaximumX() << endl;

    TF1* gaus = tres->GetFunction("gaus");

    cout << "TRes: " << gaus->GetParameter(2) << endl;
    cout << "TRes shift: " << gaus->GetParameter(1) << endl;

    double mean = 1-langaus1->GetMaximumX() + 1-langaus2->GetMaximumX() + 1-langaus3->GetMaximumX() + 1-langaus4->GetMaximumX();
    mean /= 4.;
    double sigma = pow(1-langaus1->GetMaximumX()-mean,2) + pow(1-langaus2->GetMaximumX()-mean,2) + pow(1-langaus3->GetMaximumX()-mean,2) + pow(1-langaus4->GetMaximumX()-mean,2);
    sigma /= 3;
    sigma = sqrt(sigma);
    cout << "Mean: " << mean << endl;
    cout << "Sigma: " << sigma << endl;

    TCanvas * can = new TCanvas("can", "can", 1200, 1200);
    can->SetFillColor(kWhite);
    can->SetBorderMode(0);
    can->SetBorderSize(2);
    can->SetFrameBorderMode(0);

    can->Divide(3,2);
//    double
    //TPad* pad1 = new TPad("pad1", "The pad 80% of the height", 0., 0.5, 0.5, 1);
//    pad1->SetLeftMargin( leftmargin );
//    pad1->SetBottomMargin( 0.13 );
    //pad1->SetLeftMargin( leftmargin );

//    pad1->Draw();

    // TPad* pad2 = new TPad("pad2", "The pad 80% of the height", 0.5, 0.5, 1, 1);
    // pad2->Draw();
    //
    // TPad* pad3 = new TPad("pad3", "The pad 80% of the height", 0., 0., 0.5, 0.5);
    // pad3->Draw();
    //
    // TPad* pad4 = new TPad("pad4", "The pad 80% of the height", 0.5, 0, 1, 0.5);
    // pad4->Draw();

//    pad1->cd();
    can->cd(1);
    fwd1->Draw();

//    pad2->cd();
    can->cd(2);
    fwd2->Draw();

    TPad* pad = (TPad*) can->cd(3);
    pad->SetLogy();
    tres->Draw();

    can->cd(4);
//    pad3->cd();
    fwd3->Draw();

    can->cd(5);
//    pad4->cd();
    fwd4->Draw();

}
