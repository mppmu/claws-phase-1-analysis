#include <TH1D.h>
#include <TRandom1.h>

void playground()
{
    TRandom *r0 = new TRandom();

    int nbinsx = 200;
    double xlow = -0.5;
    double xup = nbinsx -0.5;

    TH1D* gaus1 = new TH1D("gaus1", "gaus1", nbinsx, xlow, xup);
    gaus1->SetLineColor(kBlue);

    TH1D* gaus2 = new TH1D("gaus2", "gaus2", nbinsx, xlow, xup);
    gaus2->SetLineColor(kGreen+2);

    TH1D* gaus3 = new TH1D("gaus3", "gaus3", nbinsx, xlow, xup);
    gaus3->SetLineColor(kOrange-3);

    double const1;
    double const2;

    double mu1 = 25;
    double mu2 = 35;

    double sigma1 = 10;
    double sigma2 = 15;

    int count = 100000;
    for (size_t i = 0; i < count; i++) {
        double n1 = r0->Gaus(mu1, sigma1);
        double n2 = r0->Gaus(mu2, sigma2);

        gaus1->Fill(n1);
        gaus2->Fill(n2);

        gaus3->Fill(n1+n2);
    }

    gaus3->Fit("gaus","L");

    gaus3->Draw();
    gaus1->Draw("same");
    gaus2->Draw("same");

}
