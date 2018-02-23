/*
 *  Channel.cxx
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

// --- C++ includes ---
#include <typeinfo>
#include <algorithm>
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
#include <TH1F.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TMinuit.h>
// --- Project includes ---
#include "channel.hh"
#include "globalsettings.hh"


using namespace std;

//----------------------------------------------------------------------------------------------
// Definition of the Channel class.
// TODO proper description
//----------------------------------------------------------------------------------------------

Double_t osfunc(Double_t *x, Double_t *par)
{
    double number = par[0]*(GS->GetOverShootFunction()->Eval(x[0] - par[1]) - 53500.) + par[2];
    return number;
}


Channel::Channel( string name) : name_(name), state_(CHANNELSTATE_VALID), wf_(NULL), pdhist_(NULL), pd_({-1}), scope_pos_("-1"), range_(-1)
{

};

Channel::~Channel(){
		// // TODO Auto-generated destructor stub
		if(wf_ != NULL) delete wf_;
		if(pdhist_ != NULL) delete pdhist_;
};


void Channel::LoadHistogram(TFile* file)
{
    // Prevent possible memory leak
    if(wf_ != NULL)
	{
		delete wf_;
		wf_ = NULL;
	}

	// Returns nullpty if histo in file not of type TH1F.
	wf_ = dynamic_cast<TH1F*>(file->Get( name_.c_str() ) );

	if( wf_ == nullptr )
	{
		// This on the otherhand does some shit and it works.... I think it returns a TH1
		TH1F *tmp = static_cast<TH1F*>(file->Get( name_.c_str() ) );
		wf_ = new TH1F( *tmp);
	}

	wf_->SetDirectory(0);

};

void Channel::PrepHistogram( double range )
{
	/**
	 *  Make the signals go in the positiv direction
	 *  and convert from [-32512, +32512] to
	 *  [-127, +127]
	 */
    wf_->Scale(-1./256.);

	/**
	 *  Now convert from [-127, +127] to [mV] and set
	 *  the error on the bins according to the specs
	 *  from picotech:
	 *  -127 = -range
	 *   127 = range
	 *   --> the value 128 is apparently not used, or
	 *       is an error bit
	 */
	if( range >= 0)
	{
		/**
		 *  The error on the voltage is 3% of the full range of the scope
		 *  + the finite bin width of the 8 bit scope.
		 */
		double range_error = range*GS->GetParameter<double>("Scope.range_error");
		double bin_error = range/127.;

		for( int i = 1; i <= wf_->GetNbinsX(); ++i)
		{
			double content = wf_->GetBinContent(i)/127 * range;
			wf_->SetBinContent(i, content);
			wf_->SetBinError(i, range_error + bin_error );
		}

		range_ = range;
		wf_->GetYaxis()->SetTitle("Voltage [mV]");
	}

	/**
	*   Shift the bins and the x axis to resamble ns.
	*/
	double dt = GS->GetParameter<double>("Scope.delta_t");

	if( fabs(wf_->GetXaxis()->GetBinWidth(1) - dt) > 1e-10)
	{
		int nbins = wf_->GetNbinsX();
		wf_->SetBins(nbins,-dt/2, (nbins-1) *dt + dt/2);
		wf_->GetXaxis()->SetTitle("Time [ns]");
	}

};

void Channel::DeleteHistogram()
{
		if(wf_ != NULL)
		{
				delete wf_;
				wf_ = NULL;
		}

		if(pdhist_ != NULL)
		{
				delete pdhist_;
				pdhist_ = NULL;
		}
};

void Channel::SubtractPedestal( double pd)
{
    if(pd != -1000)
	{
			pd_[2] = pd;
	}

	for( int bin = 1; bin <= wf_->GetNbinsX(); bin ++)
	{
			wf_->SetBinContent(bin, wf_->GetBinContent(bin) - pd_[2]);
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
	if      (type == "waveform") return wf_;
	else if (type == "pedestal") return pdhist_;
	else						 exit(1);
};

double* Channel::GetPedestal()
{
	return pd_;
};

ChannelState Channel::GetState()
{
	return state_;
}

std::string Channel::GetScopePos()
{
	return scope_pos_;
};


//----------------------------------------------------------------------------------------------
// Definition of the CalibrationChannel class derived from Channel.
// TODO proper description
//----------------------------------------------------------------------------------------------

CalibrationChannel::CalibrationChannel(std::string name, std::string scope_pos) : Channel()
{
    if(boost::ends_with(name, "CAL")) this->SetName( name );
    else                      this->SetName( name+"-INT" );

	scope_pos_ = scope_pos;
};

CalibrationChannel::~CalibrationChannel() {
		// TODO Auto-generated destructor stub

};

void CalibrationChannel::LoadHistogram(TFile* file)
{
    // Prevent possible memory leak
		if(wf_ != NULL)
		{
				delete wf_;
				wf_ = NULL;
		}

		// Returns nullpty if histo in file not of type TH1F.
		wf_ = dynamic_cast<TH1F*>(file->Get( (name_.substr(0,4)+"-INT").c_str() ) );

		if( wf_ == nullptr )
		{
			// This on the otherhand does some shit and it works.... I think it returns a TH1
			TH1F *tmp = static_cast<TH1F*>(file->Get( (name_.substr(0,4)+"-INT").c_str() ) );
			wf_ = new TH1F( *tmp);
		}

    wf_->SetName(name_.c_str());
	wf_->SetTitle(name_.c_str());

    wf_->SetDirectory(0);

};

void CalibrationChannel::FillPedestal()
{
    if( pdhist_ != NULL )
    {
	      delete pdhist_;
	      pdhist_ = NULL;
    }

		std::string title = name_ + "_pd";
		if(range_ >= 0) pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5*range_/127, 127.5 * range_/127);
		else 			pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5, 127.5);
		pdhist_->SetDirectory(0);

		//for(int i =1; i<115; i++)
		// for(int i =1; (i<hist_->GetNbinsX()+1)/2. ;i++)
		// {
		// 	pdhist_->Fill( hist_->GetBinContent(i) );
		// }


		//bool fillflag   = true;PD

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
		while( i <= wf_->GetNbinsX() )
		{
		    double bin_contend  = wf_->GetBinContent(i);

				if( i <= wf_->GetNbinsX() - bins_over_threshold)
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
						 				if(wf_->GetBinContent(i+j) < threshold ) above_threshold = false;
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
		if( pdhist_->GetEntries() == 0 )
		{
			state_ = CHANNELSTATE_PDFAILED;
			return;
		}
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

		double low = GS->GetParameter<double>("PDS_Calibration.fitrange_low");
		double up = GS->GetParameter<double>("PDS_Calibration.fitrange_up");

		TFitResultPtr result = pdhist_->Fit(fit, "QSL","", low, up);

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


//----------------------------------------------------------------------------------------------
// Definition of the PhysicsChannel class derived from Event.
// TODO prper description
//----------------------------------------------------------------------------------------------

PhysicsChannel::PhysicsChannel(std::string ch_name, std::string scope_pos) : Channel(ch_name), os_({-1}), mipwf_(nullptr), recowf_(nullptr)
{
	scope_pos_ = scope_pos;
};

PhysicsChannel::~PhysicsChannel() {
	// TODO Auto-generated destructor stub
    if( mipwf_ != nullptr )
    {
        delete mipwf_;
        mipwf_ = nullptr;
    }
    if( recowf_ != nullptr )
    {
        delete recowf_;
        recowf_ = nullptr;
    }
};

void PhysicsChannel::LoadHistogram(TFile* rfile, vector<string> types)
{
    if ( std::find(types.begin(), types.end(), "wf") != types.end() )
    {
        // Get the normal waveform
        Channel::LoadHistogram( rfile );
    }

    if ( std::find(types.begin(), types.end(), "mip") != types.end() )
    {
        if(rfile->GetListOfKeys()->Contains((name_+"_mip").c_str()) )
        {
            mipwf_ = (TH1F*) rfile->Get((name_+"_mip").c_str());
            mipwf_->SetDirectory(0);
        }
        else
        {
            cout << "WARNING! item: mip not in rfile!" << endl;
        }
    }

    if ( std::find(types.begin(), types.end(), "reco load") != types.end() )
    {
        if(rfile->GetListOfKeys()->Contains((name_+"_reco").c_str()) )
        {
            recowf_ = (TH1F*) rfile->Get((name_+"_reco").c_str());
            recowf_->SetDirectory(0);
        }
        else
        {
            cout << "WARNING! item: reco not in rfile!" << endl;
        }
    }

    if ( std::find(types.begin(), types.end(), "reco recreate") != types.end() )
    {
        if(rfile->GetListOfKeys()->Contains((name_+"_reco").c_str()) )
        {
            std::string title = name_+"_reco";
            int nbins         = wf_->GetNbinsX();
            double lowedge    = wf_->GetBinLowEdge(1);
            double highedge   = wf_->GetBinLowEdge(nbins) + wf_->GetBinWidth(nbins);

            recowf_ = new TH1F(title.c_str(), title.c_str(), nbins, lowedge, highedge);
            recowf_->SetDirectory(0);

            double bin_error = wf_->GetBinError(1);
            for(unsigned int i = 1; i<=recowf_->GetNbinsX(); ++i)
            {
                recowf_->SetBinError(i, bin_error);
            }
        }
        else
        {
            cout << "WARNING! item: reco not in rfile!" << endl;
        }
    }
};

void PhysicsChannel::PrepHistogram( double range, double offset)
{
	// First convert Y to mV and X to ns
	this->Channel::PrepHistogram( range );

	for( int i = 0; i <= wf_->GetNbinsX(); ++i)
	{
		wf_->SetBinContent(i, wf_->GetBinContent(i) - offset);
	}

	// Somehow the last bin has an unphysical entry, set it to 0
	if( wf_->GetBinContent(wf_->GetNbinsX()) != 0) wf_->SetBinContent(wf_->GetNbinsX(), 0);
};

void PhysicsChannel::FillPedestal()
{
    if( pdhist_ != NULL )
    {
	      delete pdhist_;
	      pdhist_ = NULL;
    }

	std::string title = name_ + "_pd";
	if(range_ >= 0) pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5*range_/127, 127.5 * range_/127);
	else 			pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5, 127.5);

	pdhist_->SetDirectory(0);
	pdhist_->GetXaxis()->SetTitle("Voltag [mV]");
	pdhist_->GetYaxis()->SetTitle("Eintries");


	// for(int i =1; i <= hist_->GetNbinsX() ; ++i)
	// {
	// 	pdhist_->Fill( hist_->GetBinContent(i) );
	// }

	int bins_over_threshold 	= GS->GetParameter<int>("PDS_Physics.bins_over_threshold");
	double threshold_low 		= GS->GetParameter<double>("PDS_Physics.threshold_low");
	double threshold_high 		= GS->GetParameter<double>("PDS_Physics.threshold_high");
	int signal_length 			= GS->GetParameter<int>("PDS_Physics.signal_length");
	int overshoot_length 		= GS->GetParameter<int>("PDS_Physics.overshoot_length");

	unsigned i=1;

	while( i <= wf_->GetNbinsX() )
	{
	    double bin_contend  = wf_->GetBinContent(i);

		if( i <= wf_->GetNbinsX() - bins_over_threshold)
		{
		    if( bin_contend > threshold_low && bin_contend < threshold_high )
		  	{
			      pdhist_->Fill( bin_contend );
	    	}
			else if( bin_contend >= threshold_high )
			{
				bool above_threshold = true;
				for (int j = 0; j < bins_over_threshold; j++)
				{
	 				if(wf_->GetBinContent(i+j) < threshold_high ) above_threshold = false;
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
			else if( bin_contend <= threshold_low )
			{
				bool below_threshold = true;
				for (int j = 0; j < bins_over_threshold; j++)
				{
					if( wf_->GetBinContent(i+j) > threshold_low ) below_threshold = false;
				}

				if( below_threshold )
				{
					i += overshoot_length;
				}
				else
				{
					pdhist_->Fill( bin_contend );
				}
			}

		}
		i++;
    }

		/** In some cases two 1 pe waveforms are within a calibration waveform,
		*   leading to no value be able to pass the conditions to be filled into
		*   the pedestal histogram. So don't bother fitting an empty histogram!
		*/
		if( pdhist_->GetEntries() == 0 )
		{
			state_ = CHANNELSTATE_PDFAILED;
			return;
		}

		TF1* fit=new TF1("gaus","gaus",1,3, TF1::EAddToList::kNo);

		fit->SetParameter(0, wf_->GetNbinsX() );
		fit->SetParameter(1,0);
		//fit->SetParameter(2,1);

		double low = GS->GetParameter<double>("PDS_Physics.fitrange_low");
		double up  = GS->GetParameter<double>("PDS_Physics.fitrange_up");

		TFitResultPtr result = pdhist_->Fit(fit, "QSL","", low, up);

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
		}

		pd_[7]    = pdhist_->GetMean();
		pd_[8]    = pdhist_->GetMeanError();
		pd_[9]    = pdhist_->GetEntries();

		delete fit;
};

std::vector<OverShootResult> PhysicsChannel::OverShootCorrection()
{
	// int bins_over_threshold 	= GS->GetParameter<int>("PDS_Physics.bins_over_threshold");
	// double threshold_low 		= GS->GetParameter<double>("PDS_Physics.threshold_low");
	// double threshold_high 		= GS->GetParameter<double>("PDS_Physics.threshold_high");
	// int signal_length 			= GS->GetParameter<int>("PDS_Physics.signal_length");
	// int overshoot_length 		= GS->GetParameter<int>("PDS_Physics.overshoot_length");

	double dt 					= GS->GetParameter<double>("Scope.delta_t");
	double threshold 			= GS->GetParameter<double>("OverShootCorrection.threshold");
	int line_after_threshold 	= GS->GetParameter<int>("OverShootCorrection.line_after_threshold");
	int line_length 			= GS->GetParameter<int>("OverShootCorrection.line_length");

	double line_par0 			= GS->GetParameter<double>("OverShootCorrection.line_par0");
	double line_par2 			= GS->GetParameter<double>("OverShootCorrection.line_par2");

    double length				= GS->GetParameter<double>("OverShootCorrection.length");

	// int os_after_start 			= GS->GetParameter<double>("OverShootCorrection.os_after_start");
	// double gconst				= GS->GetParameter<double>("OverShootCorrection.gconst");
	// double gmean				= GS->GetParameter<double>("OverShootCorrection.gmean");
	// double gsigma				= GS->GetParameter<double>("OverShootCorrection.gsigma");
	// double exdecay				= GS->GetParameter<double>("OverShootCorrection.exdecay");
	// double border				= GS->GetParameter<double>("OverShootCorrection.border");
    std::vector<OverShootResult> results;
    int nfits = 0;
	for( int i = 1; i <= wf_->GetNbinsX(); ++i )
	{
		if(wf_->GetBinContent(i) > threshold)
		{

            OverShootResult result;
            result.n = nfits;
            ++nfits;
			result.lstart = wf_->GetBinCenter(i) + line_after_threshold*dt;
			result.lstop = wf_->GetBinCenter(i) + (line_after_threshold + line_length)*dt;


			TF1 *fit_line = new TF1("fit_line","[0]*(x-[1])+[2]", 0, 1, TF1::EAddToList::kNo);
			fit_line->SetParameters(line_par0, result.lstart + 100*dt, line_par2);

            // fresult stands for fit result
			TFitResultPtr fresult = 	wf_->Fit(fit_line, "QS+","", result.lstart, result.lstop);

            result.lresult = int(fresult);
			result.start = fit_line->GetX( 0. );
            result.stop  = result.start + length;


            TF1 *osfit = new TF1("osfit", osfunc, 0., 1., 3);

            osfit->SetParameter(0, 0.001);
            osfit->SetParLimits(0, 0, 100);

            osfit->SetParameter(1, result.start);
            osfit->SetParLimits(1, result.start - 50e-9, result.start + 50e-9);

            osfit->FixParameter(2, 0);

            fresult = 	wf_->Fit(osfit, "QS+","", result.start, result.stop);

            result.result = int(fresult);

            if( fresult->IsValid() )
            {
                result.par0 = osfit->GetParameter(0);
                result.par1 = osfit->GetParameter(1);
                result.par2 = osfit->GetParameter(2);
                result.chi2 = osfit->GetChisquare();
                result.ndf  = osfit->GetNDF();
                result.pval = fresult->Prob();

                for(int j = wf_->GetXaxis()->FindBin(result.start); j < wf_->GetXaxis()->FindBin(result.stop); ++j )
                {
             		double content  = wf_->GetBinContent(j);
             		double subtract = osfit->Eval(wf_->GetBinCenter(j));
             		wf_->SetBinContent(j, content - subtract);
                }
            }

            results.push_back(result);

			delete fit_line;
			delete osfit;

			i += line_after_threshold + line_length;
		}
	}

    return results;
}

double * PhysicsChannel::GetOS()
{
	return os_;
}

TH1* PhysicsChannel::GetHistogram(std::string type)
{
	if      (type == "waveform") return wf_;
	else if (type == "pedestal") return pdhist_;
    else if (type == "mip") return mipwf_;
    else if (type == "reco") return recowf_;
	else						 exit(1);
};

void PhysicsChannel::PrepareTagging()
{
    if( recowf_ != nullptr )
    {
        delete recowf_;
        recowf_ = nullptr;
    }

    std::string title = name_ + "_reco";

    recowf_ = (TH1F*) wf_->Clone(title.c_str());
    recowf_->SetTitle( title.c_str() );
    recowf_->SetDirectory(0);
};

void PhysicsChannel::SignalTagging()
{

};

void PhysicsChannel::PrepareDecomposition()
{
    if( mipwf_ != nullptr )
    {
        delete mipwf_;
        mipwf_ = nullptr;
    }

    title             = name_ + "_mip";
    int nbins         = wf_->GetNbinsX();
    double lowedge    = wf_->GetBinLowEdge(1);
    double highedge   = wf_->GetBinLowEdge(nbins) + wf_->GetBinWidth(nbins);

    mipwf_ = new TH1F(title.c_str(), title.c_str(), nbins, lowedge, highedge);
    mipwf_->SetDirectory(0);
};

void PhysicsChannel::WaveformDecomposition(TH1F* avg)
{
    double  threshold    =  GS->GetParameter<double>("WaveformDecomposition.threshold");
    int     search_range =  GS->GetParameter<double>("WaveformDecomposition.search_range");

    int     nbins        = recowf_->GetNbinsX();

    int     avg_nbins    = avg->GetNbinsX();
    double  avg_max      = avg->GetMaximum();
    int     avg_maxbin   = avg->GetMaximumBin();

    // make sure the edges in the histogram are handled properly:
    //double b_low = recowf_->GetBinCenter(avg_maxbin);
    double b_low = recowf_->GetBinCenter(search_range);
    // double b_up  = recowf_->GetBinCenter( nbins - ( avg->GetNbinsX() - avg_maxbin ) );
    double b_up  = recowf_->GetBinCenter( nbins - search_range );
    recowf_->GetXaxis()->SetRangeUser(b_low, b_up);


    int maxbin = recowf_->GetMaximumBin();

    while( recowf_->GetBinContent(maxbin) > threshold )
    {
        /**
        * \todo Validate
        * \todo find out why the hell I need to shift the avg by -1
        */

        for( unsigned int i = 2 ; i <= avg_nbins; ++i)
        {
            double avg_bincont = avg->GetBinContent( i - 1 );
            double bincont     = recowf_->GetBinContent( i + maxbin - avg_maxbin );

            recowf_->SetBinContent( i + maxbin - avg_maxbin, bincont - avg_bincont);
        }

        // does ++GetBinContent(maxbin)
        mipwf_->AddBinContent(maxbin);

        // // // Because it is very time consuming to search the full waveform with each iteration
        // // // look only in the vincity of the last maximum first.
        if( recowf_->GetBinContent(maxbin) > threshold*2 )
        {
            double tmp_max = 0;
            int tmp_max_bin = maxbin - search_range;

            for(int j = maxbin - search_range; j< maxbin + search_range; ++j)
            {
                if(recowf_->GetBinContent(j) > tmp_max)
                {
                    tmp_max = recowf_->GetBinContent(j);
                    tmp_max_bin = j;
                }
            }

            // Make sure we are not in the flank of a large signal at the edge of the search region
            if( tmp_max_bin > (maxbin - int(search_range*0.9)) && tmp_max_bin < (maxbin + int(0.9*search_range)) )
            {
                maxbin = tmp_max_bin;
            }
            else
            {
                maxbin = recowf_->GetMaximumBin();
            }
        }
        else
        {
            maxbin = recowf_->GetMaximumBin();
        }
        //maxbin = recowf_->GetMaximumBin();
    }
};

std::vector<double> PhysicsChannel::WaveformReconstruction(TH1F* avg)
{
 		/**
 		 * \todo Validate
 		 * \todo Line 621 make avg_waveform height 20 dynamic;
 		 */

    int reco_range =  GS->GetParameter<double>("WaveformReconstruction.reco_range");

    int nbins          = recowf_->GetNbinsX();

    int avg_nbins      = avg->GetNbinsX();
    int avg_maxbin     = avg->GetMaximumBin();

    for(unsigned int i = 1 + reco_range; i <= mipwf_->GetNbinsX() - reco_range; ++i)
    {
        if(mipwf_->GetBinContent(i) > 0)
        {
            int n_ph    = mipwf_->GetBinContent(i);
            int ph_bin  = i;

            for(int j = 0; j < n_ph; ++j)
            {
                for(unsigned int k = 2 ; k <= avg_nbins; ++k)
                {
                    double avg_bincont = avg->GetBinContent( k - 1 );
                    double bincont     = recowf_->GetBinContent(k + ph_bin - avg_maxbin);

                    recowf_->SetBinContent(k + ph_bin - avg_maxbin, bincont + avg_bincont);
                }
            }
        }
    }

    double bin_error = wf_->GetBinError(1);

    std::vector<double> res(4);

    res[0] = nbins;
    res[1] = bin_error;

    double chi2 = 0;

    for(unsigned int i =1; i<= wf_->GetNbinsX(); ++i)
    {
        double org = wf_->GetBinContent(i);
        double reco = recowf_->GetBinContent(i);

        chi2 += (org-reco)*(org-reco)/(bin_error*bin_error);
    }

    res[2] = chi2;
    res[3] = TMath::Prob(chi2, nbins);

    return res;
};


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
