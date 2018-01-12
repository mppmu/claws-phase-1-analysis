/*
 *  Channel.cxx
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

// --- C++ includes ---
// --- OpenMP includes ---
// #include <omp.h>
// --- BOOST includes ---
// #include <utility>
// #include <assert.h>
// #include <stdio.h>
// #include <math.h>
// #include <gsl/gsl_errno.h>
// #include <gsl/gsl_fft_real.h>
// #include <gsl/gsl_fft_halfcomplex.h>
//
// #include <gsl/gsl_rng.h>
// #include <gsl/gsl_randist.h>
// #include <gsl/gsl_complex.h>
// #include <gsl/gsl_complex_math.h>
// --- ROOT includes ---
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
// --- Project includes ---
#include "channel.hh"
#include "globalsettings.hh"

//----------------------------------------------------------------------------------------------
// Definition of the Channel class.
// TODO proper description
//----------------------------------------------------------------------------------------------

Channel::Channel(std::string name) : name_(name), state_(CHANNELSTATE_VALID), hist_(NULL), pdhist_(NULL), pd_({-1})
{
		//waveform_       = new vector<int8_t>();
		// waveform_           = new std::vector<float>();
    //
		// std::string title    = name_+"_pd";
		// // Calling GetNBitsScope which returns the number of actual bits of the scope. That is not equal to the Max or Min values of the
		// // integer values in the data.
		// n_bits_         = GS->GetNBitsScope();
		// pedestal_hist_      = new TH1I(title.c_str(), title.c_str(), n_bits_, GS->GetXLow(), GS->GetXUp());
		// pedestal_hist_->SetDirectory(0);        // Root is the most stupid BITCH!!!

};

Channel::~Channel(){
		// // TODO Auto-generated destructor stub
		if(hist_ != NULL) delete hist_;
		if(pdhist_ != NULL) delete pdhist_;
		// delete[] pd_;
		// delete waveform_;
		// if(hist_ != NULL)
		// {
		// 		delete hist_;
		// 		hist_ = NULL;
		// }
		// if(pedestal_hist_ != NULL)
		// {
		// 		delete pedestal_hist_;
		// 		pedestal_hist_ = NULL;
		// }

};


void Channel::LoadHistogram(TFile* file)
{
    // Prevent possible memory leak
		if(hist_ != NULL)
		{
				delete hist_;
				hist_ = NULL;
		}

		hist_= (TH1*)file->Get(name_.c_str());
		hist_->SetDirectory(0);



	//	n_sample_ = hist_->GetNbinsX();
		// //Last bin in physics wavefroms is filled with 0, takes care of that bug.
		// if( hist_->GetBinContent(hist_->GetNbinsX()) == 0 && !boost::algorithm::ends_with(name_,"-INT") ) n_sample_--;
};

void Channel::PrepHistogram()
{
		/* Make the signals go in the positiv direction
		 * and convert from [-32768, +32512] to
		 * [-128, +127]
		 */
    hist_->Scale(-1./256.);

	/**
	* Shift the bins and the x axis to resamble ns
	* \todo load binsize dynamically
	*/
	double dt = GS->GetParameter<double>("Scope.delta_t");

	if( abs(hist_->GetXaxis()->GetBinWidth(1) - dt) > 1e-10)
	{
		int nbins = hist_->GetNbinsX();
		hist_->SetBins(nbins,-dt/2, (nbins-1) *dt + dt/2);
	}

};

void Channel::DeleteHistogram()
{
		if(hist_ != NULL)
		{
				delete hist_;
				hist_ = NULL;
		}
};

void Channel::FillPedestal()
{
    if( pdhist_ != NULL )
    {
	      delete pdhist_;
	      pdhist_ = NULL;
    }

		std::string title = name_ + "_pd";
		pdhist_ = new TH1I(title.c_str(), title.c_str(), 256, -128.5, 127.5);
		pdhist_->SetDirectory(0);

		//for(int i =1; i<115; i++)
		// for(int i =1; (i<hist_->GetNbinsX()+1)/2. ;i++)
		// {
		// 	pdhist_->Fill( hist_->GetBinContent(i) );
		// }


		//bool fillflag   = true;

				// //float threshold = baseline_ + pd_delta_;
				// float threshold = 3;
				// float pd_gap = 20;
				// unsigned i = pd_gap_;
		int bins_over_threshold = GS->GetParameter<int>("PDS_Calibration.bins_over_threshold");
		double threshold = GS->GetParameter<double>("PDS_Calibration.threshold");
		int signal_length = GS->GetParameter<int>("PDS_Calibration.signal_length");

    	unsigned i=1;

		// while( i < hist_->GetNbinsX() - 2 * pd_gap_ +1 )
		// {
		while( i <= hist_->GetNbinsX() )
		{
		    double bin_contend  = hist_->GetBinContent(i);

				if( i <= hist_->GetNbinsX() - bins_over_threshold)
				{
				    if( bin_contend < threshold)
					  {
					      pdhist_->Fill( bin_contend );
					  }
						else
						{
								bool above_threshold = true;
								for (int j = 0; j < bins_over_threshold; j++)
								{
						 				if(hist_->GetBinContent(i+j) < threshold ) above_threshold = false;
								}

								if( above_threshold )
								{
									  i += signal_length;
								}
								else
								{
						    		pdhist_->Fill( bin_contend );
								}

						}
				}
				else
				{
						if( bin_contend < threshold)
						{
								pdhist_->Fill( bin_contend );
						}
				}
				i++;
    }



				// if( waveform_->at( i + 1 ) >= threshold &&
				//     waveform_->at( i + 2 ) >= threshold &&
				//     waveform_->at( i + 3 ) >= threshold )
				// {

				// if(  < threshold && fillflag == true)
				// {
				// 		pdhist_->Fill(waveform_->at(i - pd_gap_) );
				// }
				//
				// 		else if( waveform_->at( i ) >= threshold && fillflag == true)
				// 		{
				// 				if( waveform_->at( i + 1 ) >= threshold &&
				// 				    waveform_->at( i + 2 ) >= threshold &&
				// 				    waveform_->at( i + 3 ) >= threshold )
				// 				{
				// 						fillflag = false;
				// 				}
				// 				else
				// 				{
				// 						pedestal_hist_->Fill(waveform_->at(i - pd_gap_) );
				// 				}
				// 		}
				//
				// 		else if( waveform_->at( i ) < threshold &&
				// 		         fillflag == false &&
				// 		         // When jumping the tail of the signal (2*gap) needed to make
				// 		         // sure we are not jumping into a signal again.
				// 		         waveform_->at( i + 2 * pd_gap_ ) < threshold )
				// 		{
				// 				fillflag = true;
				// 				if( i < waveform_->size() - 2 * pd_gap_ )
				// 				{
				// 						i += 2 * pd_gap_;
				// 				}
				// 		}
				//
				//

				// if((pedestal_hist_->GetEntries()<waveform_->size()*0.01))
				// {
				//     std::string error = name_ + ":  (pedestal_hist_->GetEntries(): " + to_string(pedestal_hist_->GetEntries()) + ", waveform_->size(): "
				//                         + to_string(waveform_->size())+ ", i: " + to_string(i) + ", pd_gap_: " + to_string(pd_gap_)+ ", threshold: " + to_string(threshold);
				//     std::cout << error << std::endl;
				// }
				// std::cout << name_ << ": " << pedestal_hist_->GetEntries() << std::endl;

		/** In some cases two 1 pe waveforms are within a calibration waveform,
		*   leading to no value be able to pass the conditions to be filled into
		*   the pedestal histogram. So don't bother fitting an empty histogram!
		*/
		if( pdhist_->GetEntries() == 0 ) return;
		// if( pdhist_->GetEntries() == 0 )
		// {
		// 	for(int i = 1; i<7;i++ ) pd_[i] = -1;
		//
		// 	pd_[7]    = pdhist_->GetMean();
		// 	pd_[8]    = pdhist_->GetMeanError();
		// 	pd_[9]    = pdhist_->GetEntries();
		//
		// 	return;
		// }

		TF1* fit=new TF1("gaus","gaus",1,3, TF1::EAddToList::kNo);

		fit->SetParameter(0,50);
		fit->SetParameter(1,0);
		fit->SetParameter(2,1);

		TFitResultPtr result = pdhist_->Fit(fit, "QS","", -5, 5);

		pd_[0]    = int(result);

		if( int(result) == 0)
		{
			pd_[1]    = fit->GetParameter(0);
			pd_[2]    = fit->GetParameter(1);
			pd_[3]    = fit->GetParameter(2);
			pd_[4]    = fit->GetChisquare();
			pd_[5]    = fit->GetNDF();
			pd_[6]    = result->Prob();
		}
		else
		{
			state_ = CHANNELSTATE_PDFAILED;
		//	pd_[0]    = int(result);
		//	for(int i = 1; i<7;i++ ) pd_[i] = -1;
		}

		pd_[7]    = pdhist_->GetMean();
		pd_[8]    = pdhist_->GetMeanError();
		pd_[9]    = pdhist_->GetEntries();

		delete fit;
};

void Channel::SubtractPedestal( double pd)
{
    if(pd != -1000)
	{
			pd_[2] = pd;
	}

	for( int bin = 1; bin <= hist_->GetNbinsX(); bin ++)
	{
			hist_->SetBinContent(bin, hist_->GetBinContent(bin) - pd_[2]);
	}
}


std::string Channel::GetName()
{
		return name_;
};

void Channel::SetName(std::string name)
{
		name_ = name;
};


TH1* Channel::GetHistogram(std::string type)
{
	if      (type == "waveform") return hist_;
	else if (type == "pedestal") return pdhist_;
	else												 exit(1);
};

double* Channel::GetPedestal()
{
	return pd_;
};

ChannelState Channel::GetState()
{
	return state_;
}

//----------------------------------------------------------------------------------------------
// Definition of the CalibrationChannel class derived from Channel.
// TODO proper description
//----------------------------------------------------------------------------------------------

CalibrationChannel::CalibrationChannel(std::string name) : Channel()
{
    if(boost::ends_with(name, "CAL")) this->SetName( name );
    else                      this->SetName( name+"-INT" );
};

CalibrationChannel::~CalibrationChannel() {
		// TODO Auto-generated destructor stub

};

void CalibrationChannel::LoadHistogram(TFile* file)
{
    // Prevent possible memory leak
		if(hist_ != NULL)
		{
				delete hist_;
				hist_ = NULL;
		}

  //  hist_ = (TH1D*)file->Get((name_.substr(0,4)+"-INT").c_str());
		//hist_ = (TH1D*)file->Get((name_.substr(0,4)+"-INT").c_str())->Clone();
		TH1D *tmp = (TH1D*)file->Get((name_.substr(0,4)+"-INT").c_str());

		hist_ = new TH1D( *tmp);

		delete tmp;
		//tmp->Copy(hist_);
    hist_->SetName(name_.c_str());
		hist_->SetTitle(name_.c_str());
    // hist_ = tmp->Clone(name_.c_str());
    hist_->SetDirectory(0);
		//n_sample_ = hist_->GetNbinsX();
    // delete tmp;
    // tmp = NULL;
		// //Last bin in physics wavefroms is filled with 0, takes care of that bug.
		// if( hist_->GetBinContent(hist_->GetNbinsX()) == 0 && !boost::algorithm::ends_with(name_,"-INT") ) n_sample_--;
};



//
// // void Channel::LoadPedestaet(name_.c_str());
// //     hist_->SetDirectory(0);
// //     n_sample_  = hist_->GetNbinsX();
// // };
//
// void Channel::LoadWaveform()
// {
// 		if(hist_ == NULL)
// 		{
// 				cout << "ERROR waveform histogram doesn't exists!" << endl;
// 				exit(-1);
// 		}
// 		// The vectors need to hold 10^6 or more elements, allocate enough memory in advance.
// 		waveform_->clear();
// 		waveform_->reserve(n_sample_+1);
//
// 		for(unsigned int i=0; i< n_sample_; i++)
// 		{
// 				waveform_->push_back( -hist_->GetBinContent(i+1)/n_bits_ + baseline_ - pedestal_);
// 		}
//
// };

// void Channel::DeleteWaveform()
// {
// 		std::vector<float>().swap(*waveform_);
// 		std::cout << "" << std::endl;
// };
//

//
// // void Channel::Subtract(double pedestal)
// //             }GetChannelsform_->at( i )
//
// // void Channel::Subtract()
// // {
// //     /*
// //         If no argument is given just use the mean value from the pedestal histogram, i.e. the event specific pedestal.
// //         This can be usefull when the pedestal over time is experiencing a shift or some pickup!
// //     */
// //     this->Subtract(pedestal_hist_->GetMean());
// // };
//
// // void Channel::Subtract(double pedestal, double pedestal_hist_sigma, bool backup)
// void Channel::SubtractPedestal(double pedestal, bool backup)
// {
// 		// TODO Pasing the sigma of the pedestal for the signal rejection in the wavefrom decomposition
// 		if( pedestal == 0 )
// 		{
// 				pedestal_ = pedestal_hist_->GetMean();
// 				for (unsigned int i = 0; i < waveform_->size(); i++)
// 				{
// 						waveform_->at(i) -= pedestal_;
// 				}
// 		}
// 		else if(pedestal != 0 && !backup)
// 		{
// 				for (unsigned int i = 0; i < waveform_->size(); i++)
// 				{
// 						waveform_->at(i) -= pedestal;
// 				}
// 				pedestal_ = pedestal;
// 		}
// 		else if( backup )
// 		{
// 				if(pedestal_hist_->GetEntries() == 0)
// 				{
// 						for (unsigned int i = 0; i < waveform_->size(); i++)
// 						{
// 								waveform_->at(i) -= pedestal;
// 						}
// 						pedestal_ -= pedestal;
// 				}
// 				else
// 				{
// 						TF1* fit=new TF1("penis","gaus",1,3, TF1::EAddToList::kNo);
// 						pedestal_hist_->Fit(fit, "Q");
// 						pedestal_ = fit->GetParameter(1);
// 						delete fit;
// 						for (unsigned int i = 0; i < waveform_->size(); i++)
// 						{
// 								waveform_->at(i) -= pedestal_;
// 						}
// 				}
// 		}
// };
//
// void Channel::SetPedestal(double pedestal, bool backup)
// {
// 		// TODO Pasing the sigma of the pedestal for the signal rejection in the wavefrom decomposition
// 		if( pedestal == 0 )
// 		{
// 				pedestal_= pedestal_hist_->GetMean();
// 		}
// 		else if(pedestal != 0 && !backup)
// 		{
// 				pedestal_ = pedestal;
// 		}
// 		else if( backup )
// 		{
// 				if(pedestal_hist_->GetEntries() == 0)
// 				{
// 						pedestal_ = pedestal;
// 				}
// 				else
// 				{
// 						pedestal_ = pedestal_hist_->GetMean();
// 				}
// 		}
// };
//
// void Channel::SetBaseline(float baseline)
// {
// 		baseline_ = baseline;
// };

// vector<float>* Channel::GetWaveform()
// // vector<int8_t>* Channel::GetWaveform()
// {
// 		return waveform_;
// };
//// TH1I* Channel::GetPedestal()
// {
// 		return pedestal_hist_;
// };
// TH1F* Channel::GetWaveformHist()
// {
// //    std::cout<< "Channel::GetWaveformHist(), name: "<< name_ << std::endl;
// 		if( waveform_->size() != 0 )
// 		{
// 				string title = name_+"_wf";
//
// 				TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5, waveform_->size()+0.5);
//
// 				for(unsigned int i = 0; i < waveform_->size(); i++)
// 				{
// 						hist_wf->SetBinContent(i+1, waveform_->at(i));
// 				}
//
// 				return hist_wf;
// 		}
// 		else
// 		{
// 				return NULL;
// 		}
//
// };
//
// void Channel::CreateHistogram()
// {
// 		if( waveform_->size() != 0 )
// 		{
// 				this->DeleteHistogram();
//
// 				std::string title = name_+"_wf";
// 				hist_ = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5, waveform_->size()+0.5);
//
// 				for(unsigned int i = 0; i < waveform_->size(); i++)
// 				{
// 						hist_->SetBinContent(i+1, waveform_->at(i));
// 				}
// 		}
// }

//

//
// double Channel::GetIntegral()
// {
// 		return integral_;
// }
//
// void Channel::PrintType()
// {
// 		cout << "I'm a Channel " << endl;
// }
//
//
// //----------------------------------------------------------------------------------------------
// // Definition of the PhysicsChannel class derived from Event.
// // TODO prper description
// //----------------------------------------------------------------------------------------------
//
// PhysicsChannel::PhysicsChannel(std::string ch_name) : Channel(ch_name)
// {
// 		waveform_workon_    =   new std::vector<float>();
// 		waveform_photon_    =   new std::vector<std::uint8_t>();
//
// 		clean_wf_           =   new std::vector<float>();
// 		wh_wf_              =   new std::vector<float>();
// 		mip_wf_             =   new std::vector<float>();
// };
//
// PhysicsChannel::~PhysicsChannel() {
// 		// TODO Auto-generated destructor stub
// };
//
// void PhysicsChannel::PrintType()
// {
// 		cout << "I'm a PhysicsChannel " << endl;
// }
//
// // void PhysicsChannel::LoadHistogram(TFile* file)
// // {
// //     /* Calls the base class LoadHistogram mehtod, therefore, function is identicall to base class
// //        method. Last bin in physics wavefrom is filled with 0, mehtod takes care of that bug.
// //     */
// //     this->Channel::LoadHistogram(file);
// //     n_sample_ --;
// // };
//
// void PhysicsChannel::CalculateIntegral()
// {
// 		/*
// 		    TODO Implement
// 		 */
// };
//
// void PhysicsChannel::SetUpWaveforms()
// {
// 		waveform_workon_->clear();
// 		waveform_photon_->clear();
//
// 		waveform_workon_->reserve(waveform_->size());
// 		waveform_photon_->reserve(waveform_->size());
//
// 		std::pair<double, double> test = GS->GetPEtoMIP(name_);
//
// 		for(unsigned i = 0; i < waveform_->size(); i++)
// 		{
// 				// (*waveform_workon_)[i] = (*waveform_)[i];
// 				// (*waveform_photon_)[i] = 0;
// 				//    waveform_workon_->push_back(waveform_->at(i));
// //        std::cout << "i: " << i  << std::endl;
// 				//    waveform_workon_->push_back(waveform_->at(i));
// 				waveform_photon_->push_back(0);
// 		}
//
// 		double threshold = GS->GetParameter<double>("PhysicsChannel.SignalFlagThreshold");
// 		bool fillflag = false;
// 		unsigned counter = 0;
//
// 		for(unsigned i = 0; i < waveform_->size() -4; i++)
// 		{
// 				if(    waveform_->at(i)   > threshold
// 				       && waveform_->at(i+1) > threshold
// 				       && waveform_->at(i+2) > threshold
// 				       && waveform_->at(i+3) > threshold
// 				       && waveform_->at(i+4) > threshold)
// 				{
// 						fillflag = true;
// 						waveform_workon_->push_back(waveform_->at(i));
// 				}
// 				else if( fillflag )
// 				{
// 						fillflag = false;
// 						counter = 100;
// 						if(waveform_->at(i) >= 0) waveform_workon_->push_back(waveform_->at(i));
// 						else waveform_workon_->push_back(0);
// 				}
// 				else if( !fillflag && counter > 0)
// 				{
// 						if(waveform_->at(i) >= 0) waveform_workon_->push_back(waveform_->at(i));
// 						else waveform_workon_->push_back(0);
//
// 						counter--;
//
// 				}
// 				else
// 				{
// 						waveform_workon_->push_back(0);
// 				}
// 		}
// 		if(fillflag || (counter > 2))
// 		{
// 				if(waveform_->at(waveform_->size()-3) >= 0) waveform_workon_->push_back(waveform_->at(waveform_->size()-3));
// 				else waveform_workon_->push_back(0);
// 				if(waveform_->at(waveform_->size()-2) >= 0) waveform_workon_->push_back(waveform_->at(waveform_->size()-2));
// 				else waveform_workon_->push_back(0);
// 				if(waveform_->at(waveform_->size()-1) >= 0) waveform_workon_->push_back(waveform_->at(waveform_->size()-1));
// 				else waveform_workon_->push_back(0);
//
// 		}
// 		else if(fillflag || (counter == 2))
// 		{
// 				if(waveform_->at(waveform_->size()-3) >= 0) waveform_workon_->push_back(waveform_->at(waveform_->size()-2));
// 				else waveform_workon_->push_back(0);
// 				if(waveform_->at(waveform_->size()-2) >= 0) waveform_workon_->push_back(waveform_->at(waveform_->size()-1));
// 				else waveform_workon_->push_back(0);
//
// 				waveform_workon_->push_back(0);
// 		}
// 		else if(counter == 1)
// 		{
// 				if(waveform_->at(waveform_->size()-3) >= 0) waveform_workon_->push_back(waveform_->at(waveform_->size()-2));
// 				else waveform_workon_->push_back(0);
//
// 				waveform_workon_->push_back(0);
// 				waveform_workon_->push_back(0);
// 		}
// 		else
// 		{
// 				waveform_workon_->push_back(0);
// 				waveform_workon_->push_back(0);
// 				waveform_workon_->push_back(0);
// 		}
//
//
//
// };
//
// void PhysicsChannel::SetUpWaveformsV2()
// {
// 		this->InitCleanWF();
// 		this->InitWorkhorseWF();
// 		this->InitMipWF();
//
// 		this->BuildCleanWF();
// 		this->BuildWorkhorseWF();
// }
//
// void PhysicsChannel::InitCleanWF()
// {
// 		clean_wf_->clear();
// 		clean_wf_->resize(waveform_->size(), 0);
// };
//
// void PhysicsChannel::InitWorkhorseWF()
// {
// 		wh_wf_->clear();
// 		wh_wf_->resize(waveform_->size(), 0);
// };
//
// void PhysicsChannel::InitMipWF()
// {
// 		/** \todo not calling a clear, but just setting every entry to 0 by std::fill would be more efficient. However, this will most likely
// 		 *      online get called on an empty vecotr anyway.
// 		 */
// 		mip_wf_->clear();
// 		mip_wf_->resize(waveform_->size(), 0);
// };
//
// void PhysicsChannel::BuildCleanWF()
// {
// 		/**
// 		 * [PhysicsChannel::RunFFT descript]
// 		 * \todo Increase the tail length
// 		 * \todo acoount for the last bins affected by: waveform_->size() - bins_over_threshold
// 		 */
// 		double threshold            = GS->GetParameter<double>("PhysicsChannel.SignalFlagThreshold");
// 		int bins_over_threshold  = int(GS->GetParameter<double>("PhysicsChannel.BinsOverThreshold"));
// 		int tail_length          = int(GS->GetParameter<double>("PhysicsChannel.TailLength"));
// 		int offset               = int(GS->GetParameter<double>("PhysicsChannel.Offset"));
// 		int fillflag             = 0;
//
// 		for(unsigned i = 0; i < waveform_->size() - bins_over_threshold - offset; i++)
// 		{
// 				// First check if bin_i and it's subsequent bins are over the signal threshold.
// 				bool over_threshold = false;
// 				if( waveform_->at(i + offset) >= threshold )
// 				{
// 						over_threshold = true;
// 						for(unsigned j = i + 1 + offset; j < i + bins_over_threshold + offset; j++ )
// 						{
// 								if( waveform_->at(j) < threshold )
// 								{
// 										over_threshold = false;
// 								}
// 						}
// 				}
//
// 				// Second: If yes we are still in a signal and the tail should be long.
// 				if(over_threshold)
// 				{
// 						fillflag = tail_length;
// 				}
//
//
// 				//Third: If we are in a signal or tail, fill, else set to 0;
// 				if( fillflag > 0 && waveform_->at(i) >= 0 )
// 				{
// 						clean_wf_->at(i) = waveform_->at(i);
// 						fillflag--;
// 				}
// 				else
// 				{
// 						clean_wf_->at(i) = 0;
// 				}
//
// 		}
//
//
// }
//
// void PhysicsChannel::BuildWorkhorseWF()
// {
// 		for(unsigned i = 0; i < waveform_->size(); i++)
// 		{
// 				wh_wf_->at(i) = clean_wf_->at(i);
// 		}
// };
//
// double PhysicsChannel::DecomposeV2(std::vector<float>* avg_wf)
// {
// 		/**
// 		 * [adsasd]
// 		 * @param avg_wf [Full average 1 pe waveform that will be subtracted.]
// 		 */
// 		this->Subtract1PE(avg_wf);
// 		this->ReconstructV2(avg_wf);
// 		this->CalculateChi2V2();
//
// 		return this->GetChi2();
// };
//
// void PhysicsChannel::Subtract1PE(std::vector<float>* avg_wf)
// {
// 		/** [Test description of Subtract1PE]
// 		 *
// 		 *  @param avg_wf [Full average 1 pe waveform that will be subtracted.]
// 		 */
// 		double avg_max        = *std::max_element(avg_wf->begin(),avg_wf->end());
// 		int avg_peak       =  std::distance(avg_wf->begin(), std::max_element(avg_wf->begin(),avg_wf->end()));
//
// 		double stop_decompose =  GS->GetParameter<double>("PhysicsChannel.stop_decompose");
// 		double int_ratio      =  GS->GetParameter<double>("PhysicsChannel.int_ratio");
//
// 		while((*std::max_element(wh_wf_->begin(), wh_wf_->end())) > stop_decompose*avg_max/int_ratio)
// 		{
// 				int max = std::distance(wh_wf_->begin(), std::max_element(wh_wf_->begin(), wh_wf_->end()));
//
// 				int sub_start = max - avg_peak;
// 				int sub_stop  = max + (avg_wf->size() - avg_peak);
//
// 				// If max is close to the begining, begining of avg_wf would be be-
// 				// fore the start of wh_wf_.
// 				if( sub_start < 0 )
// 				{
// 						sub_start = 0;
// 				}
//
// 				// If max is close to the end of wh_wf_, avg_wf would excide it.
// 				if( sub_stop > wh_wf_->size() )
// 				{
// 						sub_stop = wh_wf_->size();
// 				}
//
//
// 				for(int i = sub_start; i < sub_stop; i++)
// 				{
// //            std::cout << "Subtracting: " << avg_waveform->at(i - sub_start )/20. << " at: " << i << ",  in avg at: "<< (i - sub_start ) << std::endl;
// 						// if( (wh_wf_->at(i) - avg_wf->at(i - sub_start )/20.) >= 0 ) wh_wf_->at(i) -= avg_wf->at(i - sub_start )/20.;
// 						// else                                                        wh_wf_->at(i) = 0;
// 						if( (wh_wf_->at(i)  - avg_wf->at(i - ( max - avg_peak ) )/int_ratio) >= 0)
// 						{
// 								wh_wf_->at(i) -= avg_wf->at(i - ( max - avg_peak ) )/int_ratio;
// 						}
// 						else
// 						{
// 								wh_wf_->at(i) = 0;
// 						}
// 				}
//
// 				mip_wf_->at(max)++;
// 		}
//
// 		// Get the total number of photons in the waveform for the rate.
// 		for(auto &ivec: (*mip_wf_))
// 		{
// 				nr_ph_ += ivec;
// 		}
//
// };
//
// void PhysicsChannel::ReconstructV2(std::vector<float>* avg_waveform)
// {
// 		/**
// 		 * \todo Validate
// 		 * \todo Line 621 make avg_waveform height 20 dynamic;
// 		 */
//
// 		for(unsigned i = 0; i < wh_wf_->size(); i++)
// 		{
// 				wh_wf_->at(i) = 0;
// 		}
//
// 		int avg_peak = std::distance(avg_waveform->begin(), std::max_element(avg_waveform->begin(),avg_waveform->end()));
//
// 		double int_ratio      =  GS->GetParameter<double>("PhysicsChannel.int_ratio");
//
// 		for(unsigned ph_pos = 0; ph_pos < mip_wf_->size(); ph_pos++ )
// 		{
// 				if( mip_wf_->at( ph_pos ) != 0 )
// 				{
// 						// Add the photon:
// 						int add_start = ph_pos - avg_peak;
// 						if( add_start < 0 ) add_start = 0;
//
// 						int add_stop  = ph_pos + ( avg_waveform->size() - avg_peak );
// 						if(add_stop > wh_wf_->size() ) add_stop = wh_wf_->size();
//
// 						for(int i = add_start; i < add_stop; i++)
// 						{
// 								wh_wf_->at(i) += avg_waveform->at(i - (ph_pos - avg_peak) ) * mip_wf_->at(ph_pos)/int_ratio;
// 						}
//
// 				}
// 		}
// };
//
// void PhysicsChannel::CalculateChi2V2()
// {
// 		/** [The ensure, that the reconstructed waveform and the cleaned original one
// 		 *  agree, calculate a Chi2 test statistic. ]
// 		 * \todo Validate
// 		 * \todo implement useful definition of Chi2 and it's sigma. Maybe with Poisson/Binomial of number of detected photons.
// 		 *
// 		 */
//
// 		double sigma = GS->GetParameter<double>("PhysicsChannel.chi2_sigma");
// 		int n = 0;
//
// 		chi2_ = 0;
//
// 		for(unsigned i = 0; i < wh_wf_->size(); i++)
// 		{
//
// 				if(wh_wf_->at(i) != 0) n++;
// 				chi2_ += ( clean_wf_->at(i) - wh_wf_->at(i) ) * ( clean_wf_->at(i) - wh_wf_->at(i) ) / ( sigma * sigma );
// 		}
//
// //   chi2_ /= wh_wf_->size();
// 		if(nr_ph_ > 0)
// 		{
// 				chi2_ /= nr_ph_ * wh_wf_->size();
// 		}
// 		else
// 		{
// 				chi2_ /= -1 * wh_wf_->size();
// 		}
// 		// std::cout << "Channel: " <<  name_<< " Chi2 " << chi2_ << std::endl;
// };
//
// void PhysicsChannel::RunFFT()
// {
// 		gsl_fft_real_wavetable *        real;
// 		gsl_fft_halfcomplex_wavetable * hc;
// 		gsl_fft_real_workspace *        work;
//
// //    clean_wf_.size()
// 		work = gsl_fft_real_workspace_alloc (clean_wf_->size());
// 		real = gsl_fft_real_wavetable_alloc (clean_wf_->size());
// }
//
// void PhysicsChannel::FastRate(std::vector<float>* avg_waveform, double pe_to_mip)
// {
// 		fast_rate_ = 0;
// 		double one_pe_int = 0;
// 		for(auto & ivec: (*avg_waveform))
// 		{
// 				one_pe_int += ivec;
// 		}
//
// 		/**  Scaling factor because of different voltag ranges;
// 		 *    \todo Replace hardcoded 20. by dynamic factor from ini file.
// 		 *    \todo Adapt to new naming in v2: clean_wf_, mip_wf_
// 		 */
// 		one_pe_int /= 20.;
//
// 		double integral = 0;
// 		for(auto & ivec: (*clean_wf_))
// 		{
// 				integral += ivec;
// 		}
//
// 		fast_rate_  = integral/one_pe_int;
// 		fast_rate_ /= pe_to_mip;
// 		fast_rate_ /= ( n_sample_ * GS->GetParameter<double>("PhysicsChannel.BinWidth") );
// }
//
// void PhysicsChannel::Rate(double pe_to_mip)
// {
// 		rate_ = 0;
//
// 		/**  Scaling factor because of different voltag ranges;
// 		 *    \todo Replace hardcoded 20. by dynamic factor from ini file.
// 		 *    \todo Adapt to new naming in v2: clean_wf_, mip_wf_
// 		 */
//
// 		// Get the total number of photons in the waveform for the rate.
// 		for(auto &ivec: (*mip_wf_))
// 		{
// 				ivec /= pe_to_mip;
// 		}
//
// 		rate_ = nr_ph_/pe_to_mip;
// 		rate_ /= ( n_sample_ * GS->GetParameter<double>("PhysicsChannel.BinWidth") );
// }
//
// void PhysicsChannel::Decompose(std::vector<float>* avg_waveform)
// //void PhysicsChannel::Decompose()
// {
// 		/*
// 		    TODO Implement
// 		 */
// 		// std::vector<float>* tmp_avg_waveform = new std::vector<float>();
// 		// tmp_avg_waveform->reserve(avg_waveform->size());
// 		// for(unsigned i = 0; i < avg_waveform->size(); i++)
// 		// {
// 		//     (*tmp_avg_waveform)[i] = (*avg_waveform)[i]/20.;
// 		// }
// //    std::cout << "Subtracting of channel: " << name_ << " started " << std::endl;
// //    std::cout<< name_<<" avg size: " << avg_waveform->size() << std::endl;
//
// 		int avg_peak = std::distance(avg_waveform->begin(), std::max_element(avg_waveform->begin(),avg_waveform->end()));
// //    std::cout<< name_<<" avg peak: " << avg_peak << std::endl;
//
// //    std::cout<< name_<<" waveform_workon_->size(): " << waveform_workon_->size() << ", waveform_workon_->capacity(): " << waveform_workon_->capacity() << std::endl;
// //    std::cout<< name_<<" waveform_photon_->size(): " << waveform_photon_->size() << ", waveform_photon_->capacity(): " << waveform_photon_->capacity()<< std::endl;
//
// 		// waveform_workon_->reserve(waveform_->size()+1);
// 		// waveform_photon_->reserve(waveform_->size()+1);
//
//
// //    std::cout << name_<< std::endl;
// 		// for(unsigned i = 0; i < waveform_->size(); i++)
// 		// {
// 		//     // (*waveform_workon_)[i] = (*waveform_)[i];
// 		//     // (*waveform_photon_)[i] = 0;
// 		//     waveform_workon_->push_back(waveform_->at(i));
// 		//     waveform_photon_->push_back(0);
// 		// }
// //    if(name_ == "BWD4") std::cout<< "WF: "<< waveform_->size()<< " WF_workon: "<< waveform_workon_->size() << " WF_photon: " << waveform_photon_->size() <<std::endl;
// 		//  std::cout<< "Max is: " << *std::max_element(waveform_workon_->begin(),waveform_workon_->end()) << std::endl;
// 		//  std::cout<< "Distance is: " << std::distance(waveform_->begin(), std::max_element(waveform_->begin(),waveform_->end())) << std::endl;
//
//
// 		// try only to call max_element once per iteration, save cpu time!!!
// //    std::cout << "while loop: " << name_<< " starting " << std::endl;
//
// 		while((*std::max_element(waveform_workon_->begin(),waveform_workon_->end())) > 1.65)
// 		{
// 				int max = std::distance(waveform_workon_->begin(), std::max_element(waveform_workon_->begin(),waveform_workon_->end()));
//
// 				int sub_start = max - avg_peak;
// 				int sub_stop  = max + (avg_waveform->size() - avg_peak);
//
// 				if( sub_stop > waveform_workon_->size() ) sub_stop = waveform_workon_->size();
//
// 				for(int i = sub_start; i < sub_stop; i++)
// 				{
// //            std::cout << "Subtracting: " << avg_waveform->at(i - sub_start )/20. << " at: " << i << ",  in avg at: "<< (i - sub_start ) << std::endl;
// 						waveform_workon_->at(i) -= avg_waveform->at(i - sub_start )/20.;
// 				}
// 				waveform_photon_->at(max)++;
//
// 		}
// //    std::cout << "Subtracting of channel: " << name_ << " done!" << std::endl;
// };
//
// void PhysicsChannel::Reconstruct(std::vector<float>* avg_waveform)
// {
// 		// std::fill(waveform_workon_->begin(), waveform_workon_->end(), 0);
// 		// std::cout << "Reconstruction of channel: " << name_ << " done!" << std::endl;
//
// 		for(unsigned i = 0; i < waveform_workon_->size(); i++)
// 		{
// 				waveform_workon_->at(i) = 0;
// 		}
// 		std::cout << "avg_waveform: " << avg_waveform->size() << std::endl;
// 		int avg_peak = std::distance(avg_waveform->begin(), std::max_element(avg_waveform->begin(),avg_waveform->end()));
//
// 		for(unsigned i = 0; i < waveform_photon_->size(); i++)
// 		{
// 				if( waveform_photon_->at(i) != 0 )
// 				{
// 						int add_start = i - avg_peak;
// 						int add_stop  = i + (avg_waveform->size() - avg_peak);
//
// 						if( add_stop > waveform_workon_->size() ) add_stop = waveform_workon_->size();
//
// 						for(int j = add_start; j < add_stop; j++)
// 						{
// 								//            std::cout << "Subtracting: " << avg_waveform->at(i - sub_start )/20. << " at: " << i << ",  in avg at: "<< (i - sub_start ) << std::endl;
// 								waveform_workon_->at(j) += avg_waveform->at(j - add_start ) * waveform_photon_->at(i)/20.;
// 						}
//
// 				}
// 		}
// };
//
// void PhysicsChannel::CalculateChi2()
// {
// 		double sigma = 0.1;
// 		double chi2 = 0;
//
// 		for(unsigned i = 0; i < waveform_->size(); i++)
// 		{
// 				// chi2 += ( (*waveform_)[i] - (*waveform_workon_)[i] ) * ( (*waveform_)[i] - (*waveform_workon_)[i] ) / ( sigma * sigma );
// 				chi2 += ( waveform_->at(i) - waveform_workon_->at(i) ) * ( waveform_->at(i) - waveform_workon_->at(i) ) / ( sigma * sigma );
// 		}
// 		// for(unsigned i = 0 ; i < waveform_->size() ; i++)
// 		// {
// 		//     //chi2 += ( (*waveform_)[i] - (*waveform_photon_)[i] ) * ( (*waveform_)[i] - (*waveform_photon_)[i] ) / ( sigma * sigma );
// 		//     chi2 += ( waveform_->at(i) - waveform_photon_->at(i) ) * ( waveform_->at(i) - waveform_photon_->at(i) ) / ( sigma * sigma );
// 		// }
// 		chi2 /= waveform_->size();
// //    std::cout << "Channel: " <<  name_<< " Chi2 " << chi2 << std::endl;
// }
//
// TH1F* PhysicsChannel::GetWaveformHist()
// {
// 		// std::cout<< "PhysicsChannel::GetWaveformHist(), name: "<< name_ << std::endl;
// 		// std::cout<< "Size: "<< waveform_workon_->size() << std::endl;
// 		// std::cout<< "Element: "<< waveform_workon_->at(5000) << std::endl;
// 		// if( waveform_->size() != 0 )
// 		// {
// 		//     string title = name_+"_wf";
// 		//
// 		//     TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5 , waveform_->size()+0.5);
// 		//
// 		//     for(unsigned int i = 0; i < waveform_->size(); i++)
// 		//     {
// 		//         hist_wf->SetBinContent(i+1, waveform_->at(i));
// 		//     }
// 		//
// 		//     return hist_wf;
// 		// }
// 		if( waveform_workon_->size() != 0 )
// 		{
// 				string title = name_+"_wf_workon";
//
// 				TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_workon_->size(), 0.5, waveform_workon_->size()+0.5);
//
// 				for(unsigned int i = 0; i < waveform_workon_->size(); i++)
// 				{
// 						hist_wf->SetBinContent(i+1, waveform_workon_->at(i));
// 				}
//
// 				return hist_wf;
// 		}
// 		// if( waveform_photon_->size() != 0 )
// 		// {
// 		//     string title = name_+"_wf_photon";
// 		//
// 		//     TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_photon_->size(), 0.5 , waveform_photon_->size()+0.5);
// 		//
// 		//     for(unsigned int i = 0; i < waveform_photon_->size(); i++)
// 		//     {
// 		//         hist_wf->SetBinContent(i+1, waveform_photon_->at(i));
// 		//     }
// 		//
// 		//     return hist_wf;
// 		// }
//
// 		else if( waveform_->size() != 0 )
// 		{
// 				string title = name_+"_wf";
//
// 				TH1F* hist_wf = new TH1F( title.c_str(), title.c_str(), waveform_->size(), 0.5, waveform_->size()+0.5);
//
// 				for(unsigned int i = 0; i < waveform_->size(); i++)
// 				{
// 						hist_wf->SetBinContent(i+1, waveform_->at(i));
// 				}
//
// 				return hist_wf;
// 		}
// 		else
// 		{
// 				return NULL;
// 		}
//
// };
//
// void PhysicsChannel::CreateHistogram(std::string type)
// {
// 		//TODO If this method gets called for BWD4 or FWD4 clean_wf_size() == 0 and nothing happens. This is wanted for now.
// 		if( type == "raw" )
// 		{
// 				this->Channel::CreateHistogram();
// 		}
// 		else if( type == "clean" )
// 		{
// 				if( clean_wf_->size() != 0 )
// 				{
// 						this->DeleteHistogram();
//
// 						//std::string title = name_+"_clean_wf";
// 						std::string title = name_;
// 						hist_ = new TH1F( title.c_str(), title.c_str(), clean_wf_->size(), 0.5, clean_wf_->size()+0.5);
//
// 						for(unsigned int i = 0; i <clean_wf_->size(); i++)
// 						{
// 								hist_->SetBinContent(i+1, clean_wf_->at(i));
// 						}
// 				}
// 		}
// 		else if( type == "workhorse")
// 		{
// 				if( wh_wf_->size() != 0 )
// 				{
// 						this->DeleteHistogram();
//
// 						//std::string title = name_+"_wh_wf";
// 						std::string title = name_;
// 						hist_ = new TH1F( title.c_str(), title.c_str(), wh_wf_->size(), 0.5, wh_wf_->size()+0.5);
//
// 						for(unsigned int i = 0; i < wh_wf_->size(); i++)
// 						{
// 								hist_->SetBinContent(i+1, wh_wf_->at(i));
// 						}
// 				}
// 		}
// 		else if( type == "mip" )
// 		{
// 				if( mip_wf_->size() != 0 )
// 				{
// 						this->DeleteHistogram();
//
// 						//std::string title = name_+"_mip_wf";
// 						std::string title = name_;
// 						hist_ = new TH1F( title.c_str(), title.c_str(), mip_wf_->size(), 0.5, mip_wf_->size()+0.5);
//
// 						for(unsigned int i = 0; i < mip_wf_->size(); i++)
// 						{
// 								hist_->SetBinContent( i+1, mip_wf_->at(i));
// 						}
// 				}
// 		}
// 		else
// 		{
// 				this->DeleteHistogram();
// 				//TODO TBD
// 		}
// }
//
// TH1* PhysicsChannel::GetHistogram(std::string type)
// {
// 		this->CreateHistogram(type);
// 		return hist_;
// };
//
// double PhysicsChannel::GetRate(int type)
// {
// 		if(type == 1) return fast_rate_;
// 		else if( type == 2 ) return rate_;
// 		else return 0;
// };
//
// double PhysicsChannel::GetChi2()
// {
// 		return chi2_;
// };
//
// void PhysicsChannel::DeleteWaveform()
// {
// 		std::vector<float>().swap(*waveform_);
// 		std::vector<float>().swap(*waveform_workon_);
// 		std::vector<std::uint8_t>().swap(*waveform_photon_);
//
// 		std::vector<float>().swap(*clean_wf_);
// 		std::vector<float>().swap(*wh_wf_);
// 		std::vector<float>().swap(*mip_wf_);
// };

//
// void IntChannel::PrintType()
// {
// 		cout << "I'm a Intermediate Channel!" << endl;
// };
//
// void IntChannel::CalculateIntegral()
// {
// 		integral_ = 0;
// 		for(unsigned i=107; i < waveform_->size(); i++)
// 		{
// 				integral_ += waveform_->at(i);
//
// 		}
// 		// integral_ *= -1;
// };
//
//
//
// //----------------------------------------------------------------------------------------------
// // Definition of the PhysicsChannel class derived from Event.
// // TODO prper description
// //----------------------------------------------------------------------------------------------
//
// AnalysisChannel::AnalysisChannel(std::string ch_name) : Channel( ch_name )
// {
// 		delete pedestal_hist_;
// 		pedestal_hist_ = NULL;
//
//
// };
//
// AnalysisChannel::~AnalysisChannel()
// {
// 		// TODO Write a proper destructor...
// 		if(peak_h_ == NULL)
// 		{
// 				delete peak_h_;
// 				peak_h_ = NULL;
// 		}
// 		if(fft_real_h_ == NULL)
// 		{
// 				delete fft_real_h_;
// 				fft_real_h_ = NULL;
// 		}
// 		if(fft_img_h_ == NULL)
// 		{
// 				delete fft_img_h_;
// 				fft_img_h_ = NULL;
// 		}
// 		if(fft_mag_h_ == NULL)
// 		{
// 				delete fft_mag_h_;
// 				fft_mag_h_ = NULL;
// 		}
// 		if(fft_phase_h_ == NULL)
// 		{
// 				delete fft_phase_h_;
// 				fft_phase_h_ = NULL;
// 		}
//
// };
//
// void AnalysisChannel::LoadHistogram(TFile* file)
// {
// 		if(hist_ != NULL)
// 		{
// 				delete hist_;
// 				hist_ = NULL;
// 		}
//
// 		// TODO WARNING!!!! Root version 6.08.00 and 6.08.02 are failing here.
// 		//      The number of bins in the histogram from file is put to 1!!!!
// 		if( boost::algorithm::ends_with(name_,"4") )
// 		{
// 				hist_= (TH1F*) file->Get( (name_+"_wf").c_str());
// 		}
// 		else
// 		{
// 				hist_= (TH1F*) file->Get(name_.c_str());
// 		}
//
// 		hist_->SetDirectory(0);
//
// 		n_sample_ = hist_->GetNbinsX();
//
// 		/**
// 		 * Shift the bins and the x axis to resamble ns
// 		 * \todo load binsize dynamically
// 		 */
// 		hist_->SetBins(hist_->GetNbinsX(),0.5*0.8, (hist_->GetNbinsX()+0.5) *0.8);
//
// 		//Last bin in physics wavefroms is filled with 0, takes care of that bug.
// 		//    if( hist_->GetBinContent(hist_->GetNbinsX()) == 0 && !boost::algorithm::ends_with(name_,"-INT") ) n_sample_--;
// };
//
// void AnalysisChannel::CreateHistogram()
// {
// 		// Create the histogram holding the waveform
// 		if(hist_ != NULL)
// 		{
// 				delete hist_;
// 				hist_ = NULL;
// 		}
//
// 		std::string title = name_;
// 		hist_ = new TH1F( title.c_str(), title.c_str(), 1, 0.5, 1+0.5);
// 		hist_->SetXTitle("Time [ns]");
// 		hist_->SetYTitle("Particle rate [MIPs/0.8 ns]");
//
//
// 		// Create the histogram the peak spectrum from Frank
// 		if(!boost::algorithm::ends_with(name_, "4") )
// 		{
// 				if(peak_h_ != NULL)
// 				{
// 						delete peak_h_;
// 						peak_h_ = NULL;
// 				}
//
// 				title = name_ + "_peak";
// 				peak_h_ = new TH1D( title.c_str(), title.c_str(), 1, 0.5, 1+0.5);
// 				peak_h_->SetXTitle("Peak to Peak Distance [ns]");
// 				peak_h_->SetYTitle("amp_{1} #times amp_{2} [MIPs^{2} / 0.8 ns]");
// 		}
//
// 		double timestep = 0.8e-9;
//
// 		// Create the histogram holding the real fft spectrum
// 		if(fft_real_h_ != NULL)
// 		{
// 				delete fft_real_h_;
// 				fft_real_h_ = NULL;
// 		}
//
// 		title = name_ + "_fft_real";
// 		fft_real_h_ = new TH1D( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
// 		fft_real_h_->SetXTitle("Frequency [Hz]");
// 		fft_real_h_->SetYTitle("Gute Frage...");
// 		// Create the histogram holding the imaginary fft spectrum
// 		if(fft_img_h_ != NULL)
// 		{
// 				delete fft_img_h_;
// 				fft_img_h_ = NULL;
// 		}
//
// 		title = name_ + "_fft_img";
// 		fft_img_h_ = new TH1D( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
// 		fft_img_h_->SetXTitle("Frequency [Hz]");
// 		fft_img_h_->SetYTitle("Gute Frage...");
//
// 		if(fft_mag_h_ != NULL)
// 		{
// 				delete fft_mag_h_;
// 				fft_mag_h_ = NULL;
// 		}
//
// 		title = name_ + "_fft_mag";
// 		fft_mag_h_ = new TH1D( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
// 		fft_mag_h_->SetXTitle("Magnitude [Hz]");
// 		fft_mag_h_->SetYTitle("Gute Frage...");
//
// 		// Create the histogram holding the phaseshift fft spectrum
// 		if(fft_phase_h_ != NULL)
// 		{
// 				delete fft_phase_h_;
// 				fft_phase_h_ = NULL;
// 		}
//
// 		title = name_ + "_fft_phase";
// 		fft_phase_h_ = new TH1D( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
// 		fft_phase_h_->SetXTitle("Phaseshift [probably degree]");
// 		fft_phase_h_->SetYTitle("Gute Frage...");
// }
//
// void AnalysisChannel::Normalize(double n)
// {
// 		hist_->Scale(n);
// }
//
// void AnalysisChannel::SetErrors(double err)
// {
// 		for(int i = 1; i<hist_->GetNbinsX()+1; i++)
// 		{
// 				hist_->SetBinError(i, err);
// 		}
// }
//
// void AnalysisChannel::RunPeak()
// {
// 		if(boost::algorithm::ends_with(name_, "4") ) return;
//
// 		if( peak_h_->GetNbinsX() < hist_->GetNbinsX() )
// 		{
// 				peak_h_->SetBins(hist_->GetNbinsX(), hist_->GetBinLowEdge(1), hist_->GetBinLowEdge(hist_->GetNbinsX())+1.);
// 		}
//
// 		double amp = 0;
// 		for (int i = 1; i < hist_->GetNbinsX()+1; i++)
// 				if ( hist_->GetBinContent(i) > 0) {
//
// 						for (int j = i+1; j < hist_->GetNbinsX()+1; j++) {
// 								if (hist_->GetBinContent(j) > 0)
// 								{
// 										peak_h_->Fill( hist_->GetBinCenter(j) - hist_->GetBinCenter(i), ( hist_->GetBinContent(i) * hist_->GetBinContent(j) ) );
// 								}
//
// 						}
// 				}
//
// }
//
// void AnalysisChannel::RunFFT()
// {
// 		/**
// 		 * \todo implement
// 		 */
//
// 		if(boost::algorithm::ends_with(name_, "4") ) return;
//
// 		long n = 0;
//
// 		if( hist_->GetNbinsX() % 2 == 0 ) n = (int)(hist_->GetNbinsX()/10);
// 		else n = (int)(hist_->GetNbinsX()/10) - 1;
// 		n++;
//
// 		if( fft_real_h_->GetNbinsX() < n/2+1 )
// 		{
// 				double xmin = fft_real_h_->GetBinCenter(1);
// 				double xmax = fft_real_h_->GetBinCenter(fft_real_h_->GetNbinsX());
// 				double range = xmax-xmin;
// 				fft_real_h_->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
// 		}
//
// 		if( fft_img_h_->GetNbinsX() < n/2+1 )
// 		{
// 				double xmin = fft_img_h_->GetBinCenter(1);
// 				double xmax = fft_img_h_->GetBinCenter(fft_img_h_->GetNbinsX());
// 				double range = xmax-xmin;
// 				fft_img_h_->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
// 		}
//
// 		if( fft_mag_h_->GetNbinsX() < n/2+1 )
// 		{
// 				double xmin = fft_mag_h_->GetBinCenter(1);
// 				double xmax = fft_mag_h_->GetBinCenter(fft_mag_h_->GetNbinsX());
// 				double range = xmax-xmin;
// 				fft_mag_h_->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
// 		}
//
// 		if( fft_phase_h_->GetNbinsX() < n/2+1 )
// 		{
// 				double xmin = fft_phase_h_->GetBinCenter(1);
// 				double xmax = fft_phase_h_->GetBinCenter(fft_phase_h_->GetNbinsX());
// 				double range = xmax-xmin;
// 				fft_phase_h_->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
// 		}
//
// 		gsl_fft_real_wavetable *        real;
// 		gsl_fft_real_workspace *        work;
//
// 		double * data = new double[n];
// 		double * cpacked = new double[2*n];
//
// 		for (int i = 0; i < n; i++)
// 		{
// 				data[i] = hist_->GetBinContent(i+1);
// 		}
//
// 		work = gsl_fft_real_workspace_alloc ( n );
// 		real = gsl_fft_real_wavetable_alloc ( n );
//
// 		gsl_fft_real_transform(data, 1, n, real, work);
//
// 		gsl_fft_real_wavetable_free(real);
//
// 		gsl_fft_halfcomplex_unpack( data, cpacked, 1,n );
//
//
// 		for (int i = 0; i < n/2+1; i++)
// 		{
// 				fft_real_h_->SetBinContent(i+1,cpacked[2*i]);
// 		}
//
// 		for (int i = 0; i < n/2+1; i++)
// 		{
// 				fft_img_h_->SetBinContent(i+1,cpacked[2*i+1]);
// 		}
//
// 		delete work;
// 		delete data;
// 		delete cpacked;
//
// 		double wall1 = claws::get_wall_time();
// 		double cpu1  = claws::get_cpu_time();
//
// 		for (int i = 1; i < fft_real_h_->GetNbinsX() + 1; i++)
// 		{
// 				gsl_complex z = gsl_complex_rect(fft_real_h_->GetBinContent(i),fft_img_h_->GetBinContent(i));
// 				fft_mag_h_->SetBinContent(i,gsl_complex_abs(z));
// 				fft_phase_h_->SetBinContent(i,gsl_complex_arg(z));
// 		}
//
// }
//
//
// void AnalysisChannel::CalculateIntegral()
// {
// 		/**
// 		 * \todo implement
// 		 */
// }
//
// TH1* AnalysisChannel::GetHistogram(std::string type)
// {
// 		if(type == "waveform")
// 		{
// 				return hist_;
// 		}
// 		else if(type == "peak")
// 		{
// 				return peak_h_;
// 		}
// 		else if(type == "fft_real")
// 		{
// 				return fft_real_h_;
// 		}
// 		else if(type == "fft_img")
// 		{
// 				return fft_img_h_;
// 		}
// 		else if(type == "fft_mag")
// 		{
// 				return fft_mag_h_;
// 		}
// 		else if(type == "fft_phase")
// 		{
// 				return fft_phase_h_;
// 		}
// 		else
// 		{
// 				return NULL;
// 		}
// };
//
// void AnalysisChannel::PrintType()
// {
// 		cout << "I'm an Analysis Channel!" << endl;
// };
