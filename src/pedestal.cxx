/*
 *  Channel.cxx
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

// Std includes:
#include <vector>
#include <map>

// Root includes:
#include <TFile.h>
#include <TH1I.h>
#include <TGraphErrors.h>
#include <TF1.h>
// Project includes
#include "globalsettings.hh"
#include "pedestal.hh"


Pedestal::Pedestal(int run_nr, int int_nr) : run_nr_(run_nr), int_nr_(int_nr)
{
		for(auto &ch : GS->GetChannels(1))
		{
				std::string title    = "Run-" + std::to_string(run_nr_) + "-" + ch + "_pd";
				h_[ch]               = new TH1I(title.c_str(), title.c_str(), GS->GetNBitsScope(), GS->GetXLow(), GS->GetXUp());
				g_[ch]               = new TGraphErrors();
				title                = "Run-" + std::to_string(run_nr_) + "-" + ch + "_pdg";
				g_[ch]->SetName(title.c_str());
				// This should be eventually removed by my own global root style for the TBrowser
				g_[ch]->SetMarkerStyle(2);
				g_[ch]->SetMarkerColor(kRed);
				g_[ch]->SetMarkerSize(2);
		}

		for(auto &ch : GS->GetChannels(2))
		{
				std::string title    = "Run-" + std::to_string(int_nr) + "-" + ch + "_pd";
				h_[ch]               = new TH1I(title.c_str(), title.c_str(), GS->GetNBitsScope(), GS->GetXLow(), GS->GetXUp());
				g_[ch]               = new TGraphErrors();
				title                = "Run-" + std::to_string(int_nr) + "-" + ch + "_pdg";
				g_[ch]->SetName(title.c_str());
				// This should be eventually removed by my own global root style for the TBrowser
				g_[ch]->SetMarkerStyle(2);
				g_[ch]->SetMarkerColor(kRed);
				g_[ch]->SetMarkerSize(2);
		}

		for(auto &ch : GS->GetChannels(1))
		{
				// Fucking SetRangeUser does not work for TGraphs because the Min/Max is recalculated before drawing.
				g_[ch]->SetMaximum(5);
				g_[ch]->SetMinimum(-5);
		}
		for(auto &ch : GS->GetChannels(2))
		{
				g_[ch]->SetMaximum(5);
				g_[ch]->SetMinimum(-5);
		}

};

void Pedestal::AddEvent(std::map<std::string, TH1I*> event)
{
		/**
		 * [Pedestal::SavePedestal description]
		 * @param file [description]
		 * \todo check if ch.secon->GetMeanError is the right thing. According to the histogram it's supposed to be around 0.44
		 *        but the error on the points seems to be to small.
		 */
		for (auto &ch : event)
		{
				h_[ch.first]->Add(ch.second);
				int nr = g_[ch.first]->GetN();
				g_[ch.first]->SetPoint(nr,nr,ch.second->GetMean());
				g_[ch.first]->SetPointError(nr,0,ch.second->GetMeanError());
		}
};

void Pedestal::SavePedestal(TFile* file)
{
		// Methode writes the pedestal histograms and graphs to a root file coming from outside.
		for (auto &ch : h_)
		{
				ch.second->Write();
		}
		for (auto &ch : g_)
		{
				ch.second->Write();
		}
};

std::map<std::string, float> Pedestal::GetPedestal(int type)
{
		/**
		 * [Pedestal::GetPedestal: ]
		 * @param type [type=0 means normal and intermediate, 1 normal only, 2 intermediate only]
		 * \todo Changed the calculation from simply extracting the mean to a gaussian fit. VALIDATE!
		 */
		std::map<std::string, float> tmp;

		for (auto &ch : h_)
		{
				if(type == 0)
				{
						// TF1* gaus=new TF1( ,"gaus",-200, 2600);
						//TF1* fit=new TF1("penis","[p0]*TMath::Exp(-((x-[p1])/[p2]*(x-[p1])/[p2])",1,3, TF1::EAddToList::kNo);
						TF1* fit=new TF1("penis","gaus",1,3, TF1::EAddToList::kNo);
						ch.second->Fit(fit, "Q");
						//TF1* fit = ch.second->GetFunction("gaus");
						tmp[ch.first]   = fit->GetParameter(1);
						delete fit;

				}
				else if(type == 1 && ch.first.length() == 4)
				{
						//TF1* fit=new TF1("penis","[p0]*TMath::Exp(-((x-[p1])/[p2]*(x-[p1])/[p2])",1,3, TF1::EAddToList::kNo);
						TF1* fit=new TF1("penis","gaus",1,3, TF1::EAddToList::kNo);
						ch.second->Fit(fit, "Q");
						//TF1* fit = ch.second->GetFunction("gaus");
						tmp[ch.first]   = fit->GetParameter(1);
						delete fit;
						//tmp[ch.first]   = ch.second->GetMean();
				}
				else if(type == 2 && ch.first.length() == 8)
				{
						//TF1* fit=new TF1("penis","[p0]*TMath::Exp(-((x-[p1])/[p2]*(x-[p1])/[p2])",1,3, TF1::EAddToList::kNo);
						TF1* fit=new TF1("penis","gaus",1,3, TF1::EAddToList::kNo);
						ch.second->Fit(fit, "Q");
						//TF1* fit = ch.second->GetFunction("gaus");
						tmp[ch.first]   = fit->GetParameter(1);
						delete fit;
						//	tmp[ch.first]   = ch.second->GetMean();
				}

		}
		return tmp;
};

std::map<std::string, float> Pedestal::GetError()
{
		std::map<std::string, float> tmp;

		for (auto &ch : h_)
		{
				//tmp[ch.first]   = ch.second->GetMeanError();
				//TF1* fit=new TF1("penis","[p0]*TMath::Exp(-((x-[p1])/[p2]*(x-[p1])/[p2])",1,3, TF1::EAddToList::kNo);
				TF1* fit=new TF1("penis","gaus",1,3, TF1::EAddToList::kNo);
				ch.second->Fit(fit, "Q");
				//TF1* fit = ch.second->GetFunction("gaus");
				tmp[ch.first]   = fit->GetParameter(2);
				delete fit;
		}

		return tmp;
};

std::map<std::string, TH1I*> Pedestal::GetHistograms()
{
		return h_;
};

std::map<std::string, TGraphErrors*> Pedestal::GetGraphs()
{
		return g_;
};

Pedestal::~Pedestal() {
		// TODO Auto-generated destructor stub
		std::cout << "Deleting Pedestal Object!" << std::endl;

		for (auto &ch : h_)
		{
				delete ch.second;
		}
		for (auto &ch : g_)
		{
				delete ch.second;
		}
};
