/*
 *  Channel.cxx
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

#include "Channel.hh"

//----------------------------------------------------------------------------------------------
// Definition of the Channel class derived from Event.
// TODO prper description
//----------------------------------------------------------------------------------------------

Channel::Channel(string ch_name): name_(ch_name)
{
    //waveform_       = new vector<int8_t>();
    waveform_       = new vector<float>();
    string title    = name_+"_pd";
    pedestal_      = new TH1I(title.c_str(), title.c_str(), GS->GetNBitsScope() , GS->GetXLow(), GS->GetXUp());
    pedestal_->SetDirectory(0);            // Root is the most stupid BITCH!!!
};

Channel::~Channel() {
	// TODO Auto-generated destructor stub
};

void Channel::LoadWaveform(TFile* file)
{
    TH1I* hist = (TH1I*)file->Get(name_.c_str());
    n_sample_  = hist->GetNbinsX();

    // Clean up set size of vector
    waveform_->clear();
    waveform_->reserve(n_sample_+1);

    for(unsigned int i=0; i< n_sample_; i++)
    {
        waveform_->push_back(int8_t(hist->GetBinContent(i)/GS->GetNBitsScope()));
    }
    delete hist;
    hist = NULL;

    //TODO decide if I'm staying with 16 bit values or switching to 8 bit values
    // waveform_->Scale(1./256.);
}

void Channel::LoadPedestal()
{

    if(waveform_->size() != 0)
    {

        for (unsigned int i = 0; i < waveform_->size(); i++)
        {
            pedestal_->Fill(waveform_->at(i));
        }

    }
    else
    {
        exit(1);
    }

}

void Channel::Subtract(double sb)
{
    for (unsigned int i = 0; i < waveform_->size(); i++)
    {
        waveform_->at(i) = waveform_->at(i)-sb;

    }
}

string Channel::GetName()
{
    return name_;
}

vector<float>* Channel::GetWaveform()
// vector<int8_t>* Channel::GetWaveform()
{
    return waveform_;
}

TH1F* Channel::GetWaveformHist()
{
    if( waveform_->size() != 0 )
    {
        string title = name_+"_wf";

        TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0 , waveform_->size()*0.8);

        for(unsigned int i = 0; i < waveform_->size(); i++)
        {
            hist_wf->SetBinContent(i+1, waveform_->at(i));
        }

        return hist_wf;
    }
    else
    {
        return NULL;
    }

}


TH1I* Channel::GetPedestal()
{
    return pedestal_;
}



//----------------------------------------------------------------------------------------------
// Definition of the PhysicsChannel class derived from Event.
// TODO prper description
//----------------------------------------------------------------------------------------------

PhysicsChannel::PhysicsChannel(string ch_name): Channel(ch_name)
{

};

PhysicsChannel::~PhysicsChannel() {
	// TODO Auto-generated destructor stub
};

void PhysicsChannel::PrintType()
{
    cout << "I'm a PhysicsChannel " << endl;
}
//----------------------------------------------------------------------------------------------
// Definition of the PhysicsChannel class derived from Event.
// TODO prper description
//----------------------------------------------------------------------------------------------

IntChannel::IntChannel(string ch_name): Channel( ch_name + "-INT" )
{

};

IntChannel::~IntChannel() {
	// TODO Auto-generated destructor stub
};

void IntChannel::PrintType()
{
    cout << "I'm a Intermediate Channel!" << endl;
}
