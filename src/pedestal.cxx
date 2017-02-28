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

// Project includes
#include "globalsettings.hh"
#include "pedestal.hh"


Pedestal::Pedestal(int run_nr):run_nr_(run_nr)
{
    for(auto &ch : GS->GetChannels())
    {
        std::string title    = "Run-" + std::to_string(run_nr_) + "-" + ch + "_pd";
        h_[ch]               = new TH1I(title.c_str(), title.c_str(), GS->GetNBitsScope() , GS->GetXLow(), GS->GetXUp());
        g_[ch]               = new TGraphErrors();
        title                = "Run-" + std::to_string(run_nr_) + "-" + ch + "_pdg";
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
    std::map<std::string, float> tmp;

    for (auto &ch : h_)
    {
        if(type == 0)
        {
            tmp[ch.first]   = ch.second->GetMean();
        }
        else if(type == 1 && ch.first.length() == 4)
        {
            tmp[ch.first]   = ch.second->GetMean();
        }
        else if(type == 2 && ch.first.length() == 8)
        {
            tmp[ch.first]   = ch.second->GetMean();
        }

    }
    return tmp;
};

std::map<std::string, float> Pedestal::GetError()
{
    std::map<std::string, float> tmp;

    for (auto &ch : h_)
    {
        tmp[ch.first]   = ch.second->GetMeanError();
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
