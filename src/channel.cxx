/*
 *  Channel.cxx
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

 // OpenMP
 #include <omp.h>

#include <assert.h>

#include "channel.hh"
#include "globalsettings.hh"

//----------------------------------------------------------------------------------------------
// Definition of the Channel class derived from Event.
// TODO prper description
//----------------------------------------------------------------------------------------------

Channel::Channel(string ch_name): name_(ch_name)
{
    //waveform_       = new vector<int8_t>();
    waveform_       = new vector<float>();
    string title    = name_+"_pd";
    // Calling GetNBitsScope which returns the number of actual bits of the scope. That is not equal to the Max or Min values of the
    // integer values in the data.
    n_bits_         = GS->GetNBitsScope();
    pedestal_      = new TH1I(title.c_str(), title.c_str(), n_bits_ , GS->GetXLow(), GS->GetXUp());
    pedestal_->SetDirectory(0);            // Root is the most stupid BITCH!!!
};

Channel::~Channel() {
	// TODO Auto-generated destructor stub
};

void Channel::LoadHistogram(TFile* file)
{
    hist_= (TH1I*)file->Get(name_.c_str());
    hist_->SetDirectory(0);
    n_sample_  = hist_->GetNbinsX();
};

void Channel::LoadWaveform()
{
    if(hist_ == NULL)
    {
        cout << "ERROR waveform histogram doesn't exists!" << endl;
        exit(-1);
    }
    // The vectors need to hold 10^6 or more elements, allocate enough memory in advance.
    waveform_->clear();
    waveform_->reserve(n_sample_+1);

    for(unsigned int i=0; i< n_sample_; i++)
    {
        waveform_->push_back(int8_t(hist_->GetBinContent(i)/n_bits_));
    }

};

void Channel::DeleteHistogram()
{
    delete hist_;
    hist_ = NULL;
};

void Channel::LoadPedestal()
{

    if( waveform_->size() > pd_gap_ )
    {

        bool fillflag   = true;

        float threshold = baseline_ - pd_delta_;

        unsigned i = pd_gap_;

        while( i < waveform_->size() - 2 * pd_gap_ )
        {


            if( waveform_->at( i ) > threshold && fillflag == true)
            {
                pedestal_->Fill(waveform_->at(i - pd_gap_) );
            }

            else if( waveform_->at( i ) <= threshold && fillflag == true)
            {
                if( waveform_->at( i + 1 ) <= threshold &&
                    waveform_->at( i + 2 ) <= threshold &&
                    waveform_->at( i + 3 ) <= threshold )
                {
                    fillflag = false;
                }
                else
                {
                    pedestal_->Fill(waveform_->at(i - pd_gap_) );
                }
            }

            else if( waveform_->at( i ) > threshold &&
                     fillflag == false &&
                     // When jumping the tail of the signal (2*gap) needed to make
                     // sure we are not jumping into a signal again.
                     waveform_->at( i + 2 * pd_gap_ ) > threshold )
            {
                fillflag = true;
                if( i < waveform_->size() - 2 * pd_gap_ )
                {
                    i += 2 * pd_gap_;
                }
            }

            i++;
        }

    if((pedestal_->GetEntries()<waveform_->size()*0.01)) std::cout<< name_<< ":  (pedestal_->GetEntries(): " <<pedestal_->GetEntries()<< ", waveform_->size(): "<<waveform_->size() << std::endl;

    }
    else
    {
        exit(1);
    }
    pd_mean_    =   pedestal_->GetMean();
    pd_error_   =   pedestal_->GetMeanError();
};

void Channel::Subtract()
{
    /*
        If no argument is given just use the mean value from the pedestal histogram, i.e. the event specific pedestal.
        This can be usefull when the pedestal over time is experiencing a shift or some pickup!
    */
    this->Subtract(pedestal_->GetMean());
};

void Channel::Subtract(double pedestal)
{
    for (unsigned int i = 0; i < waveform_->size(); i++)
    {
        waveform_->at(i) = waveform_->at(i)-pedestal;
    }
};

void Channel::SetBaseline(float baseline)
{
    baseline_ = baseline;
};

string Channel::GetName()
{
    return name_;
};

vector<float>* Channel::GetWaveform()
// vector<int8_t>* Channel::GetWaveform()
{
    return waveform_;
};

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

};


TH1I* Channel::GetPedestal()
{
    return pedestal_;
};

double Channel::GetIntegral()
{
    return integral_;
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

void PhysicsChannel::CalculateIntegral()
{
    /*
        TODO Implement
    */
};

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
};

void IntChannel::CalculateIntegral()
{
    integral_ = 0;
    for(unsigned i=107; i < waveform_->size(); i++)
    {
        integral_ += waveform_->at(i);

    }
};
