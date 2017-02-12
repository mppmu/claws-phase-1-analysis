/*
 *  Channel.cxx
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

 // OpenMP
// #include <omp.h>

#include <utility>
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
    waveform_           = new std::vector<float>();

    std::string title    = name_+"_pd";
    // Calling GetNBitsScope which returns the number of actual bits of the scope. That is not equal to the Max or Min values of the
    // integer values in the data.
    n_bits_         = GS->GetNBitsScope();
    pedestal_      = new TH1I(title.c_str(), title.c_str(), n_bits_ , GS->GetXLow(), GS->GetXUp());
    pedestal_->SetDirectory(0);            // Root is the most stupid BITCH!!!

};

Channel::~Channel(){
	// TODO Auto-generated destructor stub
    delete waveform_;
    if(hist_ != NULL) delete hist_;
    delete pedestal_;
};

void Channel::LoadHistogram(TFile* file)
{

    if(hist_ != NULL)
    {
      delete hist_;
      hist_ = NULL;
    }

  //  double test = ((TH1I*)file->Get(name_.c_str()))->GetNbinsX();

    // TODO WARNING!!!! Root version 6.08.00 and 6.08.02 are failing here.
    //      The number of bins in the histogram from file is put to 1!!!!

    hist_= (TH1I*)file->Get(name_.c_str());

    hist_->SetDirectory(0);
    n_sample_  = hist_->GetNbinsX();

    //Last bin in physics wavefroms is filled with 0, takes care of that bug.
    if(hist_->GetBinContent(hist_->GetNbinsX()) == 0) n_sample_--;
};

// void Channel::LoadPedestaet(name_.c_str());
//     hist_->SetDirectory(0);
//     n_sample_  = hist_->GetNbinsX();
// };

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
        waveform_->push_back(-int8_t(hist_->GetBinContent(i+1)/n_bits_));
    }

};

void Channel::DeleteHistogram()
{
    if(hist_ != NULL)
    {
      delete hist_;
      hist_ = NULL;
    }
//    hist_ = NULL;
};

void Channel::LoadPedestal()
{

    if( waveform_->size() > pd_gap_ )
    {

        bool fillflag   = true;

        float threshold = baseline_ + pd_delta_;

        unsigned i = pd_gap_;

        while( i < waveform_->size() - 2 * pd_gap_ )
        {


            if( waveform_->at( i ) < threshold && fillflag == true)
            {
                pedestal_->Fill(waveform_->at(i - pd_gap_) );
            }

            else if( waveform_->at( i ) >= threshold && fillflag == true)
            {
                if( waveform_->at( i + 1 ) >= threshold &&
                    waveform_->at( i + 2 ) >= threshold &&
                    waveform_->at( i + 3 ) >= threshold )
                {
                    fillflag = false;
                }
                else
                {
                    pedestal_->Fill(waveform_->at(i - pd_gap_) );
                }
            }

            else if( waveform_->at( i ) < threshold &&
                     fillflag == false &&
                     // When jumping the tail of the signal (2*gap) needed to make
                     // sure we are not jumping into a signal again.
                     waveform_->at( i + 2 * pd_gap_ ) < threshold )
            {
                fillflag = true;
                if( i < waveform_->size() - 2 * pd_gap_ )
                {
                    i += 2 * pd_gap_;
                }
            }


            i++;
        }

        // if((pedestal_->GetEntries()<waveform_->size()*0.01))
        // {
        //     std::string error = name_ + ":  (pedestal_->GetEntries(): " + to_string(pedestal_->GetEntries()) + ", waveform_->size(): "
        //                         + to_string(waveform_->size())+ ", i: " + to_string(i) + ", pd_gap_: " + to_string(pd_gap_)+ ", threshold: " + to_string(threshold);
        //     std::cout << error << std::endl;
        // }
        // std::cout << name_ << ": " << pedestal_->GetEntries() << std::endl;
    }

    else
    {
        exit(1);
    }

    pd_mean_    =   pedestal_->GetMean();
    pd_error_   =   pedestal_->GetMeanError();
};

// void Channel::Subtract(double pedestal)
//             }GetChannelsform_->at( i )

// void Channel::Subtract()
// {
//     /*
//         If no argument is given just use the mean value from the pedestal histogram, i.e. the event specific pedestal.
//         This can be usefull when the pedestal over time is experiencing a shift or some pickup!
//     */
//     this->Subtract(pedestal_->GetMean());
// };

// void Channel::Subtract(double pedestal, double pedestal_sigma, bool backup)
void Channel::Subtract(double pedestal, bool backup)
{
    // TODO Pasing the sigma of the pedestal for the signal rejection in the wavefrom decomposition
    if( pedestal == 0 )
    {
        for (unsigned int i = 0; i < waveform_->size(); i++)
        {
            waveform_->at(i) -= pedestal_->GetMean();
        }
    }
    else if(pedestal != 0 && !backup)
    {
        for (unsigned int i = 0; i < waveform_->size(); i++)
        {
            waveform_->at(i) -= pedestal;
        }
    }
    else if( backup )
    {
        if(pedestal_->GetEntries() == 0)
        {
            for (unsigned int i = 0; i < waveform_->size(); i++)
            {
                waveform_->at(i) -= pedestal;
            }
        }
        else
        {
            for (unsigned int i = 0; i < waveform_->size(); i++)
            {
                waveform_->at(i) -= pedestal_->GetMean();
            }
        }
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
//    std::cout<< "Channel::GetWaveformHist(), name: "<< name_ << std::endl;
    if( waveform_->size() != 0 )
    {
        string title = name_+"_wf";

        TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5 , waveform_->size()+0.5);

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

PhysicsChannel::PhysicsChannel(std::string ch_name): Channel(ch_name)
{
    waveform_workon_   = new std::vector<float>();
    waveform_photon_    = new std::vector<std::uint8_t>();
};

PhysicsChannel::~PhysicsChannel() {
	// TODO Auto-generated destructor stub
};

void PhysicsChannel::PrintType()
{
    cout << "I'm a PhysicsChannel " << endl;
}

// void PhysicsChannel::LoadHistogram(TFile* file)
// {
//     /* Calls the base class LoadHistogram mehtod, therefore, function is identicall to base class
//        method. Last bin in physics wavefrom is filled with 0, mehtod takes care of that bug.
//     */
//     this->Channel::LoadHistogram(file);
//     n_sample_ --;
// };

void PhysicsChannel::CalculateIntegral()
{
    /*
        TODO Implement
    */
};

void PhysicsChannel::SetUpWaveforms()
{
    waveform_workon_->clear();
    waveform_photon_->clear();

    waveform_workon_->reserve(waveform_->size());
    waveform_photon_->reserve(waveform_->size());

    std::pair<double, double> test = GS->GetPEtoMIP(name_);

    for(unsigned i = 0; i < waveform_->size(); i++)
    {
        // (*waveform_workon_)[i] = (*waveform_)[i];
        // (*waveform_photon_)[i] = 0;
    //    waveform_workon_->push_back(waveform_->at(i));
//        std::cout << "i: " << i  << std::endl;
        waveform_workon_->push_back(waveform_->at(i));
        waveform_photon_->push_back(0);
    }
    // double threshold = 0.5
    // bool fillflag = false;
    // for(unsigned i = 0; i < waveform_->size() -2; i++)
    // {
    //     if(    waveform_->at(i)   >= 1.0
    //         && waveform_->at(i+1) >= 1.0
    //         && waveform_->at(i+2) >= 1.0 )
    //     {
    //         fillflag = true;
    //         waveform_workon_->push_back(waveform_->at(i));
    //     }
    //     else
    //     {
    //         waveform_workon_->push_back(0);
    //     }
    // }


};

void PhysicsChannel::Decompose(std::vector<float>* avg_waveform)
//void PhysicsChannel::Decompose()
{
    /*
        TODO Implement
    */
    // std::vector<float>* tmp_avg_waveform = new std::vector<float>();
    // tmp_avg_waveform->reserve(avg_waveform->size());
    // for(unsigned i = 0; i < avg_waveform->size(); i++)
    // {
    //     (*tmp_avg_waveform)[i] = (*avg_waveform)[i]/20.;
    // }
//    std::cout << "Subtracting of channel: " << name_ << " started " << std::endl;
//    std::cout<< name_<<" avg size: " << avg_waveform->size() << std::endl;

    int avg_peak = std::distance(avg_waveform->begin(), std::max_element(avg_waveform->begin(),avg_waveform->end()));
//    std::cout<< name_<<" avg peak: " << avg_peak << std::endl;

//    std::cout<< name_<<" waveform_workon_->size(): " << waveform_workon_->size() << ", waveform_workon_->capacity(): " << waveform_workon_->capacity() << std::endl;
//    std::cout<< name_<<" waveform_photon_->size(): " << waveform_photon_->size() << ", waveform_photon_->capacity(): " << waveform_photon_->capacity()<< std::endl;

    // waveform_workon_->reserve(waveform_->size()+1);
    // waveform_photon_->reserve(waveform_->size()+1);


//    std::cout << name_<< std::endl;
    // for(unsigned i = 0; i < waveform_->size(); i++)
    // {
    //     // (*waveform_workon_)[i] = (*waveform_)[i];
    //     // (*waveform_photon_)[i] = 0;
    //     waveform_workon_->push_back(waveform_->at(i));
    //     waveform_photon_->push_back(0);
    // }
//    if(name_ == "BWD4") std::cout<< "WF: "<< waveform_->size()<< " WF_workon: "<< waveform_workon_->size() << " WF_photon: " << waveform_photon_->size() <<std::endl;
    //  std::cout<< "Max is: " << *std::max_element(waveform_workon_->begin(),waveform_workon_->end()) << std::endl;
    //  std::cout<< "Distance is: " << std::distance(waveform_->begin(), std::max_element(waveform_->begin(),waveform_->end())) << std::endl;


    // try only to call max_element once per iteration, save cpu time!!!
//    std::cout << "while loop: " << name_<< " starting " << std::endl;

    while((*std::max_element(waveform_workon_->begin(),waveform_workon_->end())) > 1.65)
    {
        int max = std::distance(waveform_workon_->begin(), std::max_element(waveform_workon_->begin(),waveform_workon_->end()));

        int sub_start = max - avg_peak;
        int sub_stop  = max + (avg_waveform->size() - avg_peak);

        if( sub_stop > waveform_workon_->size() ) sub_stop = waveform_workon_->size();

        for(int i = sub_start ; i < sub_stop ; i++)
        {
//            std::cout << "Subtracting: " << avg_waveform->at(i - sub_start )/20. << " at: " << i << ",  in avg at: "<< (i - sub_start ) << std::endl;
            waveform_workon_->at(i) -= avg_waveform->at(i - sub_start )/20.;
        }
        waveform_photon_->at(max) ++;

    }
//    std::cout << "Subtracting of channel: " << name_ << " done!" << std::endl;
};

void PhysicsChannel::Reconstruct(std::vector<float>* avg_waveform)
{
    // std::fill(waveform_workon_->begin(), waveform_workon_->end(), 0);
    // std::cout << "Reconstruction of channel: " << name_ << " done!" << std::endl;

    for(unsigned i = 0; i < waveform_workon_->size(); i++)
    {
        waveform_workon_->at(i) = 0;
    }
    std::cout << "avg_waveform: " << avg_waveform->size() << std::endl;
    int avg_peak = std::distance(avg_waveform->begin(), std::max_element(avg_waveform->begin(),avg_waveform->end()));

    for(unsigned i = 0; i < waveform_photon_->size(); i++)
    {
        if( waveform_photon_->at(i) != 0 )
        {
            int add_start = i - avg_peak;
            int add_stop  = i + (avg_waveform->size() - avg_peak);

            if( add_stop > waveform_workon_->size() ) add_stop = waveform_workon_->size();

            for(int j = add_start ; j < add_stop ; j++)
            {
    //            std::cout << "Subtracting: " << avg_waveform->at(i - sub_start )/20. << " at: " << i << ",  in avg at: "<< (i - sub_start ) << std::endl;
                waveform_workon_->at(j) += avg_waveform->at(j - add_start ) * waveform_photon_->at(i)/20.;
            }

        }
    }
};

void PhysicsChannel::CalculateChi2()
{
    double sigma = 0.1;
    double chi2 = 0;

    for(unsigned i = 0 ; i < waveform_->size() ; i++)
    {
        // chi2 += ( (*waveform_)[i] - (*waveform_workon_)[i] ) * ( (*waveform_)[i] - (*waveform_workon_)[i] ) / ( sigma * sigma );
        chi2 += ( waveform_->at(i) - waveform_workon_->at(i) ) * ( waveform_->at(i) - waveform_workon_->at(i) ) / ( sigma * sigma );
    }
    // for(unsigned i = 0 ; i < waveform_->size() ; i++)
    // {
    //     //chi2 += ( (*waveform_)[i] - (*waveform_photon_)[i] ) * ( (*waveform_)[i] - (*waveform_photon_)[i] ) / ( sigma * sigma );
    //     chi2 += ( waveform_->at(i) - waveform_photon_->at(i) ) * ( waveform_->at(i) - waveform_photon_->at(i) ) / ( sigma * sigma );
    // }
    chi2 /= waveform_->size();
    std::cout << "Channel: " <<  name_<< " Chi2 " << chi2 << std::endl;
}

TH1F* PhysicsChannel::GetWaveformHist()
{
    // std::cout<< "PhysicsChannel::GetWaveformHist(), name: "<< name_ << std::endl;
    // std::cout<< "Size: "<< waveform_workon_->size() << std::endl;
    // std::cout<< "Element: "<< waveform_workon_->at(5000) << std::endl;
    // if( waveform_->size() != 0 )
    // {
    //     string title = name_+"_wf";
    //
    //     TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5 , waveform_->size()+0.5);
    //
    //     for(unsigned int i = 0; i < waveform_->size(); i++)
    //     {
    //         hist_wf->SetBinContent(i+1, waveform_->at(i));
    //     }
    //
    //     return hist_wf;
    // }
    if( waveform_workon_->size() != 0 )
    {
        string title = name_+"_wf_workon";

        TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_workon_->size(), 0.5 , waveform_workon_->size()+0.5);

        for(unsigned int i = 0; i < waveform_workon_->size(); i++)
        {
            hist_wf->SetBinContent(i+1, waveform_workon_->at(i));
        }

        return hist_wf;
    }
    // if( waveform_photon_->size() != 0 )
    // {
    //     string title = name_+"_wf_photon";
    //
    //     TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_photon_->size(), 0.5 , waveform_photon_->size()+0.5);
    //
    //     for(unsigned int i = 0; i < waveform_photon_->size(); i++)
    //     {
    //         hist_wf->SetBinContent(i+1, waveform_photon_->at(i));
    //     }
    //
    //     return hist_wf;
    // }

    else if( waveform_->size() != 0 )
    {
        string title = name_+"_wf";

        TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5 , waveform_->size()+0.5);

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
    // integral_ *= -1;
};
