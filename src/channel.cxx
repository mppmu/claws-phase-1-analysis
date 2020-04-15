//============================================================================
// Name        : channel.cxx
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

// --- Project includes ---
#include "channel.hh"
#include "globalsettings.hh"

// --- C++ includes ---
#include <typeinfo>
#include <algorithm>
#include <math.h>
#include <cmath>

// --- OpenMP includes ---
// #include <omp.h>

// --- ROOT includes ---
#include <TF1.h>
#include <TH1F.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TMinuit.h>
#include <Math/IntegratorOptions.h>


//----------------------------------------------------------------------------------------------
// Definition of the Channel class.
// TODO proper description
//----------------------------------------------------------------------------------------------

Double_t osfunc(Double_t *x, Double_t *par)
{
		double number = par[0] * (GS->GetOverShootFunction()->Eval(x[0] - par[1]) - 53500.) + par[2];

		return number;
}


Channel::Channel(std::string name) : name_(name), state_(CHANNELSTATE_VALID), wf_(NULL), pdhist_(NULL), pd_(
{
		-1
}), scope_pos_("-1"), range_(-1)
{
};

Channel::~Channel()
{
		// // TODO Auto-generated destructor stub
		if (wf_ != NULL)
		{
				delete wf_;
		}
		if (pdhist_ != NULL)
		{
				delete pdhist_;
		}
}


void Channel::LoadHistogram(TFile *file)
{
		// Prevent possible memory leak
		if (wf_ != NULL)
		{
				delete wf_;
				wf_ = NULL;
		}

		// Returns nullpty if histo in file not of type TH1F.
		wf_ = dynamic_cast<TH1F *>(file->Get(name_.c_str()));

		if (wf_ == nullptr)
		{
				// This on the otherhand does some shit and it works.... I think it returns a TH1
				TH1F *tmp = static_cast<TH1F *>(file->Get(name_.c_str()));
				wf_ = new TH1F(*tmp);
		}

		wf_->SetDirectory(0);
}


void Channel::PrepareHistogram(double range)
{
		/**
		 *  Make the signals go in the positiv direction
		 *  and convert from [-32512, +32512] to
		 *  [-127, +127]
		 */
		wf_->Scale(-1. / 256.);

		/**
		 *  Now convert from [-127, +127] to [mV] and set
		 *  the error on the bins according to the specs
		 *  from picotech:
		 *  -127 = -range
		 *   127 = range
		 *   --> the value 128 is apparently not used, or
		 *       is an error bit
		 */
		if (range >= 0)
		{
				/**
				 *  The error on the voltage is 3% of the full range of the scope
				 *  + the finite bin width of the 8 bit scope.
				 */
				double range_error = range * GS->GetParameter<double>("Scope.range_error");
				double bin_error   = range / 127.;

				for (int i = 1; i <= wf_->GetNbinsX(); ++i)
				{
						double content = wf_->GetBinContent(i) / 127 * range;
						wf_->SetBinContent(i, content);

						// Assumption: the range error is a systematic deviation
						//             an anyway clibrated out in the pd subtraction

						wf_->SetBinError(i, bin_error);
				}

				range_ = range;
				wf_->GetYaxis()->SetTitle("Voltage [mV]");
		}

		/**
		 *   Shift the bins and the x axis to resamble ns.
		 */
		double dt = GS->GetParameter<double>("Scope.delta_t");

		if (fabs(wf_->GetXaxis()->GetBinWidth(1) - dt) > 1e-10)
		{
				int nbins = wf_->GetNbinsX();
				wf_->SetBins(nbins, -dt / 2, (nbins - 1) * dt + dt / 2);
				wf_->GetXaxis()->SetTitle("Time [ns]");
		}
}


void Channel::DeleteHistogram()
{
		if (wf_ != NULL)
		{
				delete wf_;
				wf_ = NULL;
		}

		if (pdhist_ != NULL)
		{
				delete pdhist_;
				pdhist_ = NULL;
		}
}


void Channel::SubtractPedestal(double pd)
{
		if (pd != -1000)
		{
				pd_[2] = pd;
		}

		for (int bin = 1; bin <= wf_->GetNbinsX(); bin++)
		{
				wf_->SetBinContent(bin, wf_->GetBinContent(bin) - pd_[2]);
		}
}


std::string Channel::GetName()
{
		return name_;
}


void Channel::SetName(std::string name)
{
		name_ = name;
}


TH1 *Channel::GetHistogram(std::string type)
{
		if (type == "waveform")
		{
				return wf_;
		}
		else if (type == "pedestal")
		{
				return pdhist_;
		}
		else
		{
				exit(1);
		}
}


double *Channel::GetPedestal()
{
		return pd_;
}


ChannelState Channel::GetState()
{
		return state_;
}


std::string Channel::GetScopePos()
{
		return scope_pos_;
}

//----------------------------------------------------------------------------------------------
// Definition of the CalibrationChannel class derived from Channel.
// TODO proper description
//----------------------------------------------------------------------------------------------

CalibrationChannel::CalibrationChannel(std::string name, std::string scope_pos) : Channel()
{
		if (boost::ends_with(name, "CAL"))
		{
				this->SetName(name);
		}
		else
		{
				this->SetName(name + "-INT");
		}

		scope_pos_ = scope_pos;
}


CalibrationChannel::~CalibrationChannel()
{
		// TODO Auto-generated destructor stub
}

void CalibrationChannel::LoadHistogram(TFile *file)
{
		// Prevent possible memory leak
		if (wf_ != NULL)
		{
				delete wf_;
				wf_ = NULL;
		}

		// Returns nullpty if histo in file not of type TH1F.
		std::string name = name_.substr(0, 4) + "-INT";

		if (!file->GetListOfKeys()->Contains(name.c_str()))
		{
				boost::replace_first(name, "-", "_");
		}

		wf_ = dynamic_cast<TH1F *>(file->Get(name.c_str()));

		if (wf_ == nullptr)
		{
				// This on the otherhand does some shit and it works.... I think it returns a TH1
				TH1F *tmp = static_cast<TH1F *>(file->Get(name.c_str()));
				wf_ = new TH1F(*tmp);
		}

		wf_->SetName(name_.c_str());
		wf_->SetTitle(name_.c_str());

		wf_->SetDirectory(0);
}


void CalibrationChannel::FillPedestal()
{
		if (pdhist_ != NULL)
		{
				delete pdhist_;
				pdhist_ = NULL;
		}

		std::string title = name_ + "_pd";
		if (range_ >= 0)
		{
				pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5 * range_ / 127, 127.5 * range_ / 127);
		}
		else
		{
				pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5, 127.5);
		}
		pdhist_->SetDirectory(0);

		pdhist_->GetXaxis()->SetTitle("Pedestal [mV]");

		std::string vperbit = std::to_string(range_ / 127.);
		pdhist_->GetYaxis()->SetTitle(("Entries [1/" + vperbit + " mv]").c_str());

		for (int i = 1; i <= wf_->GetNbinsX(); ++i)
		{
				pdhist_->Fill(wf_->GetBinContent(i));
		}

		double tmp_mean = pdhist_->GetMean();

		pdhist_->Reset();

		int bins_over_threshold = GS->GetParameter<int>("PDS_Calibration.bins_over_threshold");
		double threshold           = tmp_mean + GS->GetParameter<double>("PDS_Calibration.threshold");
		int signal_length       = GS->GetParameter<int>("PDS_Calibration.signal_length");

		unsigned i = 1;

		while (i <= wf_->GetNbinsX())
		{
				double bin_contend = wf_->GetBinContent(i);

				if (i <= wf_->GetNbinsX() - bins_over_threshold)
				{
						if (bin_contend < threshold)
						{
								pdhist_->Fill(bin_contend);
						}
						else
						{
								bool above_threshold = true;

								for (int j = 0; j < bins_over_threshold; j++)
								{
										if (wf_->GetBinContent(i + j) < threshold)
										{
												above_threshold = false;
										}
								}

								if (above_threshold)
								{
										i += signal_length;
								}
								else
								{
										pdhist_->Fill(bin_contend);
								}
						}
				}
				else
				{
						if (bin_contend < threshold)
						{
								pdhist_->Fill(bin_contend);
						}
				}
				i++;
		}


		/** In some cases two 1 pe waveforms are within a calibration waveform,
		 *   leading to no value be able to pass the conditions to be filled into
		 *   the pedestal histogram. So don't bother fitting an empty histogram!
		 */
		if (pdhist_->GetEntries() == 0)
		{
				state_ = CHANNELSTATE_PDFAILED;
				return;
		}

		TF1 *fit = new TF1("gaus", "[0]*exp(-0.5*((x-[1])/[2])**2)", 1, 3, TF1::EAddToList::kNo);

		fit->SetParameter(0, 50);
		fit->SetParameter(1, 0);
		fit->SetParameter(2, 1);

		double max = pdhist_->GetBinCenter(pdhist_->GetMaximumBin());

		double low = max + GS->GetParameter<double>("PDS_Calibration.fitrange_low");
		double up  = max + GS->GetParameter<double>("PDS_Calibration.fitrange_up");

		TFitResultPtr result = pdhist_->Fit(fit, "QSL", "", low, up);

		if (result->Prob() < 0.05)
		{
				state_ = CHANNELSTATE_PDFAILED;
		}

		pd_[0] = int(result);

		if (int(result) == 0)
		{
				pd_[1] = fit->GetParameter(0);
				pd_[2] = fit->GetParameter(1);
				pd_[3] = fit->GetParError(1);
				pd_[4] = fit->GetParameter(2);
				pd_[5] = fit->GetChisquare();
				pd_[6] = fit->GetNDF();
				pd_[7] = result->Prob();
		}
		else
		{
				state_ = CHANNELSTATE_PDFAILED;
		}

		pd_[8]  = pdhist_->GetMean();
		pd_[9]  = pdhist_->GetMeanError();
		pd_[10] = pdhist_->GetEntries();


		delete fit;
}

//----------------------------------------------------------------------------------------------
// Definition of the PhysicsChannel class derived from Event.
// TODO prper description
//----------------------------------------------------------------------------------------------

PhysicsChannel::PhysicsChannel(std::string ch_name, std::string scope_pos) : Channel(ch_name), os_(
{
		-1
}), recowf_(nullptr), pewf_(nullptr), mipwf_(nullptr), mipwf_stat_(nullptr), mipwf_sys_(nullptr), rate_(Rate())
{
		scope_pos_ = scope_pos;
		rate_.name = ch_name;
};

PhysicsChannel::~PhysicsChannel()
{
		// TODO Auto-generated destructor stub
		if (recowf_ != nullptr)
		{
				delete recowf_;
				recowf_ = nullptr;
		}
		if (pewf_ != nullptr)
		{
				delete pewf_;
				pewf_ = nullptr;
		}
		if (mipwf_ != nullptr)
		{
				delete mipwf_;
				mipwf_ = nullptr;
		}
		if (mipwf_stat_ != nullptr)
		{
				delete mipwf_stat_;
				mipwf_stat_ = nullptr;
		}
		if (mipwf_sys_ != nullptr)
		{
				delete mipwf_sys_;
				mipwf_sys_ = nullptr;
		}
}


void PhysicsChannel::LoadHistogram(TFile *rfile, std::vector<std::string> types)
{
		for (auto& type: types)
		{
				if (type == "wf")
				{
						// Get the normal waveform
						this->Channel::LoadHistogram(rfile);
				}

				else if (type == "reco load")
				{
						recowf_ = (TH1F *)rfile->Get((name_ + "_reco").c_str());
						recowf_->SetDirectory(0);
				}


				else if (type == "reco recreate")
				{
						std::string title    = name_ + "_reco";
						int nbins    = wf_->GetNbinsX();
						double lowedge  = wf_->GetBinLowEdge(1);
						double highedge = wf_->GetBinLowEdge(nbins) + wf_->GetBinWidth(nbins);

						recowf_ = new TH1F(title.c_str(), title.c_str(), nbins, lowedge, highedge);
						recowf_->SetDirectory(0);

						double bin_error = wf_->GetBinError(1);
						for (unsigned int i = 1; i <= recowf_->GetNbinsX(); ++i)
						{
								recowf_->SetBinError(i, bin_error);
						}
				}
				else if (type == "pe")
				{
						pewf_ = (TH1I *)rfile->Get((name_ + "_pe").c_str());
						pewf_->SetDirectory(0);
				}
				else if (type == "mip")
				{
						mipwf_ = (TH1F *)rfile->Get((name_ + "_mip").c_str());
						mipwf_->SetDirectory(0);

						mipwf_stat_ = (TH1F *)rfile->Get((name_ + "_mip_stat").c_str());
						mipwf_stat_->SetDirectory(0);

						mipwf_sys_ = (TH1F *)rfile->Get((name_ + "_mip_sys").c_str());
						mipwf_sys_->SetDirectory(0);
				}
				else
				{
						std::cout << "WARNING! item not in rfile!" << std::endl;
				}
		}
}


void PhysicsChannel::DeleteHistogram()
{
		this->Channel::DeleteHistogram();

		if (recowf_ != NULL)
		{
				delete recowf_;
				recowf_ = NULL;
		}

		if (pewf_ != NULL)
		{
				delete pewf_;
				pewf_ = NULL;
		}

		if (mipwf_ != NULL)
		{
				delete mipwf_;
				mipwf_ = NULL;
		}

		if (mipwf_stat_ != NULL)
		{
				delete mipwf_stat_;
				mipwf_stat_ = NULL;
		}

		if (mipwf_sys_ != NULL)
		{
				delete mipwf_sys_;
				mipwf_sys_ = NULL;
		}
}


void PhysicsChannel::PrepareHistogram(double range, double offset)
{
		// First convert Y to mV and X to ns
		this->Channel::PrepareHistogram(range);

		for (int i = 0; i <= wf_->GetNbinsX(); ++i)
		{
				wf_->SetBinContent(i, wf_->GetBinContent(i) - offset);
		}

		// Somehow the last bin has an unphysical entry, set it to 0
		if (wf_->GetBinContent(wf_->GetNbinsX()) != 0)
		{
				wf_->SetBinContent(wf_->GetNbinsX(), 0);
		}
}


void PhysicsChannel::FillPedestal()
{
		if (pdhist_ != NULL)
		{
				delete pdhist_;
				pdhist_ = NULL;
		}

		std::string title = name_ + "_pd";
		if (range_ >= 0)
		{
				pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5 * range_ / 127, 127.5 * range_ / 127);
		}
		else
		{
				pdhist_ = new TH1I(title.c_str(), title.c_str(), 255, -127.5, 127.5);
		}

		pdhist_->SetDirectory(0);
		pdhist_->GetXaxis()->SetTitle("Voltag [mV]");
		pdhist_->GetYaxis()->SetTitle("Eintries");

		int bins_over_threshold = GS->GetParameter<int>("PDS_Physics.bins_over_threshold");
		double threshold_low       = GS->GetParameter<double>("PDS_Physics.threshold_low");
		double threshold_high      = GS->GetParameter<double>("PDS_Physics.threshold_high");
		int signal_length       = GS->GetParameter<int>("PDS_Physics.signal_length");
		int overshoot_length    = GS->GetParameter<int>("PDS_Physics.overshoot_length");

		unsigned i = 1;

		while (i <= wf_->GetNbinsX())
		{
				double bin_content = wf_->GetBinContent(i);

				if (i <= wf_->GetNbinsX() - bins_over_threshold)
				{
						if ((bin_content > threshold_low) && (bin_content < threshold_high))
						{
								pdhist_->Fill(bin_content);
						}
						else if (bin_content >= threshold_high)
						{
								bool above_threshold = true;
								for (int j = 0; j < bins_over_threshold; j++)
								{
										if (wf_->GetBinContent(i + j) < threshold_high)
										{
												above_threshold = false;
										}
								}

								if (above_threshold)
								{
										i += signal_length;
								}
								else
								{
										pdhist_->Fill(bin_content);
								}
						}
						else if (bin_content <= threshold_low)
						{
								bool below_threshold = true;
								for (int j = 0; j < bins_over_threshold; j++)
								{
										if (wf_->GetBinContent(i + j) > threshold_low)
										{
												below_threshold = false;
										}
								}

								if (below_threshold)
								{
										i += overshoot_length;
								}
								else
								{
										pdhist_->Fill(bin_content);
								}
						}
				}
				i++;
		}

		/** In some cases two 1 pe waveforms are within a calibration waveform,
		 *   leading to no value be able to pass the conditions to be filled into
		 *   the pedestal histogram. So don't bother fitting an empty histogram!
		 */
		if (pdhist_->GetEntries() == 0)
		{
				state_ = CHANNELSTATE_FAILED;
				//	delete fit;
				return;
		}

		double max = pdhist_->GetBinCenter(pdhist_->GetMaximumBin());

		double low = max + GS->GetParameter<double>("PDS_Physics.fitrange_low");
		double up  = max + GS->GetParameter<double>("PDS_Physics.fitrange_up");

		TF1 *fit = new TF1("gaus", "gaus", low, up, TF1::EAddToList::kNo);

		fit->SetParameter(0, wf_->GetNbinsX());
		fit->SetParameter(1, 0);
		fit->SetParameter(2, 3.5);

		TFitResultPtr result = pdhist_->Fit(fit, "QSL", "", low, up);

		pd_[0] = int(result);

		if (int(result) == 0)
		{
				pd_[1] = fit->GetParameter(0);
				pd_[2] = fit->GetParameter(1);
				pd_[3] = fit->GetParError(1);
				pd_[4] = fit->GetParameter(2);
				pd_[5] = fit->GetChisquare();
				pd_[6] = fit->GetNDF();
				pd_[7] = result->Prob();
		}
		else
		{
				state_ = CHANNELSTATE_FAILED;
		}

		if ((pd_[1] < 0) || (pd_[4] < 0))
		{
				pd_[0] = -1;
				state_ = CHANNELSTATE_FAILED;
		}

		pd_[8]  = pdhist_->GetMean();
		pd_[9]  = pdhist_->GetMeanError();
		pd_[10] = pdhist_->GetEntries();

		delete fit;
}


std::vector<OverShootResult> PhysicsChannel::OverShootCorrection()
{
		ROOT::Math::IntegratorOneDimOptions::SetDefaultIntegrator("Gauss");
		ROOT::Math::IntegratorOneDimOptions::SetDefaultWKSize(10000);

		double dt                   = GS->GetParameter<double>("Scope.delta_t");
		double threshold            = GS->GetParameter<double>("OverShootCorrection.threshold");
		int line_after_threshold = GS->GetParameter<int>("OverShootCorrection.line_after_threshold");
		int line_length          = GS->GetParameter<int>("OverShootCorrection.line_length");

		double line_par0 = GS->GetParameter<double>("OverShootCorrection.line_par0");
		double line_par2 = GS->GetParameter<double>("OverShootCorrection.line_par2");

		double length = GS->GetParameter<double>("OverShootCorrection.length");

		std::vector<OverShootResult> results;
		int nfits = 0;
		for (int i = 1; i <= wf_->GetNbinsX(); ++i)
		{
				if (wf_->GetBinContent(i) > threshold)
				{
						OverShootResult result;
						result.n = nfits;
						++nfits;
						result.lstart = wf_->GetBinCenter(i) + line_after_threshold * dt;
						result.lstop  = wf_->GetBinCenter(i) + (line_after_threshold + line_length) * dt;


						TF1 *fit_line = new TF1("fit_line", "[0]*(x-[1])+[2]", 0, 1, TF1::EAddToList::kNo);
						fit_line->SetParameters(line_par0, result.lstart + 100 * dt, line_par2);

						// fresult stands for fit result
						TFitResultPtr fresult = wf_->Fit(fit_line, "QS+", "", result.lstart, result.lstop);

						result.lresult = int(fresult);
						result.start   = fit_line->GetX(0.);
						result.stop    = result.start + length;


						TF1 *osfit = new TF1("osfit", osfunc, 0., 1., 3, 1, TF1::EAddToList::kNo);

						osfit->SetParameter(0, 0.001);
						osfit->SetParLimits(0, 0, 1);

						osfit->SetParameter(1, result.start);
						osfit->SetParLimits(1, result.start - 50e-9, result.start + 50e-9);

						osfit->SetParameter(2, 0);
						osfit->SetParLimits(2, -10, 10);


						fresult = wf_->Fit(osfit, "QS+", "", result.start, result.stop);

						result.result = int(fresult);

						if (fresult->IsValid())
						{
								result.par0 = osfit->GetParameter(0);
								result.par1 = osfit->GetParameter(1);
								result.par2 = osfit->GetParameter(2);
								result.chi2 = osfit->GetChisquare();
								result.ndf  = osfit->GetNDF();
								result.pval = fresult->Prob();

								for (int j = wf_->GetXaxis()->FindBin(result.start); j < wf_->GetXaxis()->FindBin(result.stop); ++j)
								{
										double content  = wf_->GetBinContent(j);
										double subtract = osfit->Eval(wf_->GetBinCenter(j));
										wf_->SetBinContent(j, content - subtract);
								}
						}
						else
						{
								state_ = CHANNELSTATE_FAILED;
						}

						result.area1 = wf_->Integral(i - 3, wf_->GetXaxis()->FindBin(result.start), "width");
						result.area2 = osfit->Integral(result.start, result.stop);

						results.push_back(result);

						delete fit_line;
						delete osfit;

						i += line_after_threshold + line_length;
				}
		}

		return results;
}


void PhysicsChannel::PrepareTagging()
{
		if (recowf_ != nullptr)
		{
				delete recowf_;
				recowf_ = nullptr;
		}

		std::string title = name_ + "_reco";

		recowf_ = (TH1F *)wf_->Clone(title.c_str());
		recowf_->SetTitle(title.c_str());
		recowf_->SetDirectory(0);
}


void PhysicsChannel::SignalTagging()
{
		int bins_over_threshold = GS->GetParameter<int>("SignalTagging.bins_over_threshold");
		double threshold           = GS->GetParameter<double>("SignalTagging.threshold");
		int signal_length       = GS->GetParameter<int>("SignalTagging.signal_length");
		int pre_threshold       = GS->GetParameter<int>("SignalTagging.pre_threshold");

		unsigned i = 1 + pre_threshold;

		while (i <= wf_->GetNbinsX())
		{
				double bin_content = wf_->GetBinContent(i);

				if (i <= wf_->GetNbinsX() - bins_over_threshold)
				{
						if (bin_content >= threshold)
						{

								double binavg = 0;
								for (int j = 0; j < bins_over_threshold; j++)
								{
										binavg += wf_->GetBinContent(i + j);
								}

								binavg /= bins_over_threshold;

								if (binavg >= threshold)
								{
										i += signal_length + pre_threshold;
								}
								else
								{
										wf_->SetBinContent(i - pre_threshold, 0);
								}
						}
						else
						{
								wf_->SetBinContent(i - pre_threshold, 0);
						}
				}
				else
				{
						wf_->SetBinContent(i - pre_threshold, 0);
				}

				i++;
		}

}

double PhysicsChannel::FastRate(TH1F *avg, double unixtime)
{
		double pe_per_mip = -1.;

		if (GS->GetParameter<std::string>("PEToMIP." + name_) != "false")
		{
				if (GS->GetParameter<double>("PEToMIP." + name_) > unixtime)
				{
						pe_per_mip = GS->GetParameter<double>("PEToMIP." + name_ + "val2");
				}
				else
				{
						pe_per_mip = GS->GetParameter<double>("PEToMIP." + name_ + "val");
				}
		}
		else
		{
				pe_per_mip = GS->GetParameter<double>("PEToMIP." + name_ + "val");
		}

		double dt = GS->GetParameter<double>("Scope.delta_t");

		double fastrate = wf_->Integral() / avg->Integral();
		fastrate /= pe_per_mip;
		fastrate /= (dt * wf_->GetNbinsX());

		rate_.fast = fastrate;

		return fastrate;
}


void PhysicsChannel::PrepareDecomposition()
{
		if (pewf_ != nullptr)
		{
				delete pewf_;
				pewf_ = nullptr;
		}

		std::string title    = name_ + "_pe";
		int nbins    = wf_->GetNbinsX();
		double lowedge  = wf_->GetBinLowEdge(1);
		double highedge = wf_->GetBinLowEdge(nbins) + wf_->GetBinWidth(nbins);

		pewf_ = new TH1I(title.c_str(), title.c_str(), nbins, lowedge, highedge);
		pewf_->SetDirectory(0);
}


void PhysicsChannel::WaveformDecomposition(TH1F *avg)
{

		int search_range = GS->GetParameter<double>("WaveformDecomposition.search_range");
		int search_edge  = GS->GetParameter<double>("WaveformDecomposition.search_edge");
		int fwhm         = GS->GetParameter<double>("WaveformDecomposition.fwhm");
		int stop_region  = GS->GetParameter<double>("WaveformDecomposition.stop_region");
		int stop_methode = GS->GetParameter<int>("WaveformDecomposition.stop_methode");

		double avg_max    = avg->GetMaximum();
		int avg_maxbin = avg->GetMaximumBin();

		double threshold = avg_max * GS->GetParameter<double>("WaveformDecomposition.threshold");

		auto check = [stop_region, stop_methode](TH1F *wf, int maxbin, double threshold, int fwhm) -> bool
					 {
							 // check threshold
							 bool threshold_true = wf->GetBinContent(maxbin) > threshold;

							 bool fwhm_true = false;

							 if (stop_methode == 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // Methode 1
							 {
									 int bin1 = maxbin;
									 while (wf->GetBinContent(bin1) > wf->GetBinContent(maxbin) / 2)
									 {
											 --bin1;
									 }
									 int bin2 = maxbin;
									 while (wf->GetBinContent(bin2) > wf->GetBinContent(maxbin) / 2)
									 {
											 ++bin2;
									 }

									 fwhm_true = (bin2 - bin1) >= fwhm;
							 }
							 else if (stop_methode == 2)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // Methode 2
							 {
									 double avgtmp = 0;
									 for (int bin = maxbin - int(fwhm / 2) + 1; bin <= maxbin + int(fwhm / 2) - 1; ++bin)
									 {
											 avgtmp += wf->GetBinContent(bin);
									 }

									 avgtmp   /= fwhm - 1;
									 fwhm_true = avgtmp >= wf->GetBinContent(maxbin) * 0.5;
							 }

							 // check neighborhood > 0
							 double hood = 0;

							 for (int bin = maxbin - stop_region; bin <= maxbin + stop_region; ++bin)
							 {
									 hood += wf->GetBinContent(bin);
							 }

							 bool hood_true = hood >= 0;

							 return threshold_true && fwhm_true && hood_true;
					 };

		auto subtract = [](TH1F *wf, int maxbin, TH1F *avg, int avg_maxbin) -> void
						{
								for (int bin = 1; bin <= avg->GetNbinsX(); ++bin)
								{
										double avg_bin_cont = avg->GetBinContent(bin);
										double bin_cont     = wf->GetBinContent(bin + maxbin - avg_maxbin);
										wf->SetBinContent(bin + maxbin - avg_maxbin, bin_cont - avg_bin_cont);
								}
						};

		auto get_max_bin = [](TH1F *wf, int first, int last, std::vector<int> *reject) -> int
						   {
								   double max       = 0;
								   double maxbintmp = first;

								   for (int bin = first; bin <= last; ++bin)
								   {
										   // If bin is in rejection list, skip it.
										   if (std::find(reject->begin(), reject->end(), bin) != reject->end())
										   {
												   continue;
										   }

										   if (wf->GetBinContent(bin) > max)
										   {
												   maxbintmp = bin;
												   max       = wf->GetBinContent(bin);
										   }
								   }

								   return maxbintmp;
						   };

		std::vector<int> *reject = new std::vector<int>;
		reject->reserve(10000);

		int maxbin = get_max_bin(recowf_, 1, recowf_->GetNbinsX(), reject);

		while (recowf_->GetBinContent(maxbin) >= threshold)
		{
				// maybe here put check(... 4x threshold)
				if (recowf_->GetBinContent(maxbin) >= 25.)
				{
						// maybe here put check(... 4x threshold)
						while (recowf_->GetBinContent(maxbin) >= 25.)
						{
								subtract(recowf_, maxbin, avg, avg_maxbin);
								pewf_->Fill(recowf_->GetBinCenter(maxbin));

								// Seach for new max in the hood of the previous and check if
								// it is located on the borders of the search range.
								int first = maxbin - search_range;
								int last  = maxbin + search_range;

								maxbin = get_max_bin(recowf_, first, last, reject);

								if ((maxbin <= first + search_edge) || (maxbin >= last - search_edge))
								{
										break;
								}
						}
				}
				else if (check(recowf_, maxbin, threshold, fwhm))
				{
						subtract(recowf_, maxbin, avg, avg_maxbin);
						pewf_->Fill(recowf_->GetBinCenter(maxbin));
				}
				else
				{
						reject->push_back(maxbin);
				}

				maxbin = get_max_bin(recowf_, 1, recowf_->GetNbinsX(), reject);
		}

		delete reject;
}

void PhysicsChannel::SubtractWaveform(int start, TH1F *avg)
{
		for (int bin = 1; bin <= avg->GetNbinsX(); ++bin)
		{
				double avg_bin_cont = avg->GetBinContent(bin);
				double bin_cont     = recowf_->GetBinContent(bin + start);
				recowf_->SetBinContent(bin + start, bin_cont - avg_bin_cont);
		}
}


int PhysicsChannel::GetMaximumBin(double threshold, int methode, double fwhm, int stop_region, int first, int last)
{
		if (last == -1)
		{
				last = recowf_->GetNbinsX();
		}

		int maxbin = 1;
		double max    = 0;

		for (int bin = first; bin <= last; ++bin)
		{
				if (recowf_->GetBinContent(bin) > max)
				{
						if (recowf_->GetBinContent(bin) > threshold)
						{
								if (this->FWHM(bin, methode, fwhm))
								{
										if (this->Hood(bin, stop_region))
										{
												maxbin = bin;
												max    = recowf_->GetBinContent(bin);
										}
								}
						}
				}
		}

		return maxbin;
}


bool PhysicsChannel::FWHM(int bin, int methode, double fwhm)
{
		if (methode == 1)
		{
				int pre = bin;
				while (recowf_->GetBinContent(pre) > recowf_->GetBinContent(bin) / 2)
				{
						--pre;
				}
				int post = bin;
				while (recowf_->GetBinContent(post) > recowf_->GetBinContent(bin) / 2)
				{
						++post;
				}

				return (post - pre) >= fwhm;
		}
		else if (methode == 2)
		{
				double avg = 0;
				for (int i = bin - int(fwhm / 2) + 1; i <= bin + int(fwhm / 2) - 1; ++i)
				{
						avg += recowf_->GetBinContent(i);
				}

				avg /= fwhm - 1;

				return avg >= recowf_->GetBinContent(bin) * 0.5;
		}
}


bool PhysicsChannel::Hood(int bin, int stop_region)
{
		// check neighborhood > 0
		double hood = 0;

		for (int i = bin - stop_region; i <= bin + stop_region; ++i)
		{
				hood += recowf_->GetBinContent(i);
		}

		return hood >= 0;
}


void PhysicsChannel::WaveformDecomposition2(TH1F *avg)
{
		int search_range = GS->GetParameter<double>("WaveformDecomposition.search_range");
		int search_edge  = GS->GetParameter<double>("WaveformDecomposition.search_edge");
		int fwhm         = GS->GetParameter<double>("WaveformDecomposition.fwhm");
		int stop_region  = GS->GetParameter<double>("WaveformDecomposition.stop_region");
		int stop_methode = GS->GetParameter<int>("WaveformDecomposition.stop_methode");

		double avg_max    = avg->GetMaximum();
		int avg_maxbin = avg->GetMaximumBin();

		double threshold = avg_max * GS->GetParameter<double>("WaveformDecomposition.threshold");


		int maxbin = this->GetMaximumBin(threshold, stop_methode, fwhm, stop_region);

		while (recowf_->GetBinContent(maxbin) > threshold)
		{
				if (recowf_->GetBinContent(maxbin) > 25.)
				{
						while (recowf_->GetBinContent(maxbin) > 25.)
						{
								this->SubtractWaveform(maxbin - avg_maxbin, avg);
								pewf_->Fill(recowf_->GetBinCenter(maxbin));

								// Seach for new max in the hood of the previous and check if
								// it is located on the borders of the search range.
								int first = maxbin - search_range;
								int last  = maxbin + search_range;

								maxbin = this->GetMaximumBin(25., stop_methode, fwhm, stop_region, first, last);

								if ((maxbin <= first + search_edge) || (maxbin >= last - search_edge))
								{
										break;
								}
						}
				}
				else
				{
						this->SubtractWaveform(maxbin - avg_maxbin, avg);
						pewf_->Fill(recowf_->GetBinCenter(maxbin));
				}

				maxbin = this->GetMaximumBin(threshold, stop_methode, fwhm, stop_region);
		}

}


std::vector<double> PhysicsChannel::WaveformReconstruction(TH1F *avg)
{
		/**
		 * \todo Validate
		 * \todo Line 621 make avg_waveform height 20 dynamic;
		 */

		int reco_range = GS->GetParameter<double>("WaveformReconstruction.reco_range");

		int avg_nbins  = avg->GetNbinsX();
		int avg_maxbin = avg->GetMaximumBin();

		for (unsigned int i = 1 + reco_range; i <= pewf_->GetNbinsX() - reco_range; ++i)
		{
				if (pewf_->GetBinContent(i) > 0)
				{
						int n_ph   = pewf_->GetBinContent(i);
						int ph_bin = i;

						for (int j = 0; j < n_ph; ++j)
						{
								for (unsigned int k = 2; k <= avg_nbins; ++k)
								{
										double avg_bincont = avg->GetBinContent(k - 1);
										double bincont     = recowf_->GetBinContent(k + ph_bin - avg_maxbin);

										recowf_->SetBinContent(k + ph_bin - avg_maxbin, bincont + avg_bincont);
								}
						}
				}
		}

		double bin_error = wf_->GetBinError(1);


		double chi2  = 0;
		int nbins = 0;

		for (unsigned int i = 1; i <= wf_->GetNbinsX(); ++i)
		{
				if (fabs(wf_->GetBinContent(i)) > 1e-5)
				{
						double org  = wf_->GetBinContent(i);
						double reco = recowf_->GetBinContent(i);

						chi2 += (org - reco) * (org - reco) / (bin_error * bin_error);
						++nbins;
				}
		}

		std::vector<double> res(4);

		res[0] = nbins;
		res[1] = bin_error;

		res[2] = chi2;
		res[3] = TMath::Prob(chi2, nbins);

		return res;
}


void PhysicsChannel::PrepareRetrieval()
{
		if (mipwf_ != nullptr)
		{
				delete mipwf_;
				mipwf_ = nullptr;
		}

		std::string title  = name_ + "_mip";
		int nbinsx = pewf_->GetNbinsX();
		double xlow   = pewf_->GetBinLowEdge(1);
		double xup    = pewf_->GetBinLowEdge(nbinsx) + pewf_->GetBinWidth(nbinsx);

		mipwf_ = new TH1F(title.c_str(), title.c_str(), nbinsx, xlow, xup);
		mipwf_->SetDirectory(0);

		if (mipwf_sys_ != nullptr)
		{
				delete mipwf_sys_;
				mipwf_sys_ = nullptr;
		}

		mipwf_sys_ = (TH1F *)mipwf_->Clone((name_ + "_mip_sys").c_str());

		if (mipwf_stat_ != nullptr)
		{
				delete mipwf_stat_;
				mipwf_stat_ = nullptr;
		}

		mipwf_stat_ = (TH1F *)mipwf_->Clone((name_ + "_mip_stat").c_str());
}


void PhysicsChannel::MipTimeRetrieval(double unixtime)
{
		int window_length     = GS->GetParameter<int>("MipTimeRetrieval.window_length");
		int window_threshold  = GS->GetParameter<int>("MipTimeRetrieval.window_threshold");
		int npe_hit_time      = GS->GetParameter<int>("MipTimeRetrieval.pe_hit_time");
		std::string timing_type       = GS->GetParameter<std::string>("MipTimeRetrieval.timing_type");
		double constant_fraction = GS->GetParameter<double>("MipTimeRetrieval.constant_fraction");
		int jump = GS->GetParameter<double>("MipTimeRetrieval.jump");

		double sigma_correction = GS->GetParameter<double>("MipTimeRetrieval.sigma_correction");

		double pe_per_mip        = -1.;
		double sigma_pe_per_mip  = -1.;
		double correction_factor = -1;

		if (GS->GetParameter<std::string>("PEToMIP." + name_) != "false")
		{
				if (GS->GetParameter<double>("PEToMIP." + name_) > unixtime)
				{
						pe_per_mip        = GS->GetParameter<double>("PEToMIP." + name_ + "val2");
						sigma_pe_per_mip  = GS->GetParameter<double>("PEToMIP." + name_ + "err2");
						correction_factor = GS->GetParameter<double>("PEToMIP." + name_ + "corr2");
				}
				else
				{
						pe_per_mip        = GS->GetParameter<double>("PEToMIP." + name_ + "val");
						sigma_pe_per_mip  = GS->GetParameter<double>("PEToMIP." + name_ + "err");
						correction_factor = GS->GetParameter<double>("PEToMIP." + name_ + "corr");
				}
		}
		else
		{
				pe_per_mip        = GS->GetParameter<double>("PEToMIP." + name_ + "val");
				sigma_pe_per_mip  = GS->GetParameter<double>("PEToMIP." + name_ + "err");
				correction_factor = GS->GetParameter<double>("PEToMIP." + name_ + "corr");
		}

		rate_.rate    = 0;
		rate_.staterr = 0;
		rate_.syserr  = 0;
		rate_.err     = 0;

		for (int i = 1; i <= pewf_->GetNbinsX(); ++i)
		{
				if (pewf_->GetBinContent(i) > 0)
				{
						// First check how many pes are in the
						// integration window
						int npe = 0;
						for (int j = i; j < i + window_length; ++j)
						{
								npe += pewf_->GetBinContent(j);
						}

						if (npe >= window_threshold)
						{
								int j       = i - 1;
								int time_pe = 0;

								if (timing_type == "pe hit time")
								{
										while (time_pe < npe_hit_time && (j - i) <= window_length)
										{
												++j;
												time_pe += pewf_->GetBinContent(j);
										}

								}
								else if (timing_type == "constant fraction")
								{
										int thres_pe = (int)round(constant_fraction * npe);

										if (thres_pe == 0)
										{
												thres_pe = 1;
										}

										while (time_pe < thres_pe && (j - i) <= window_length)
										{
												++j;
												time_pe += pewf_->GetBinContent(j);
										}

								}
								else if (timing_type == "cfj")
								{
										int thres_pe = (int)round(constant_fraction * npe);

										if (thres_pe == 0)
										{
												thres_pe = 1;
										}


								}

								double alpha = double(npe) / pe_per_mip + correction_factor;


								mipwf_->SetBinContent(j, alpha);
								mipwf_stat_->SetBinContent(j, alpha);
								mipwf_sys_->SetBinContent(j, alpha);

								double staterr = sqrt(npe) / pe_per_mip;

								double syserr = npe * sigma_pe_per_mip / pow(pe_per_mip, 2) + sigma_correction;

								double err = sqrt(pow(staterr, 2) + pow(syserr, 2));

								mipwf_->SetBinError(j, err);
								mipwf_stat_->SetBinError(j, staterr);
								mipwf_sys_->SetBinError(j, syserr);

								int up_gamma_mip = GS->GetParameter<double>("PEToMIP.Up_gamma_mip");

								if (up_gamma_mip == 0)
								{
										rate_.rate += alpha;

										rate_.staterr += pow(staterr, 2);
										rate_.syserr  += syserr;
								}
								else if (up_gamma_mip == 1)
								{
										double gamma_mip = GS->GetParameter<double>("PEToMIP." + name_ + "_gamma_mip");
										if (alpha <= gamma_mip)
										{
												rate_.rate += alpha;

												rate_.staterr += pow(staterr, 2);
												rate_.syserr  += syserr;
										}
								}
								else if (up_gamma_mip == 2)
								{
										double gamma_mip = GS->GetParameter<double>("PEToMIP." + name_ + "_gamma_mip");
										if (alpha > gamma_mip)
										{
												rate_.rate += alpha;

												rate_.staterr += pow(staterr, 2);
												rate_.syserr  += syserr;
										}
								}

								i += window_length - 1;


						}
				}
		}

		double dt = GS->GetParameter<double>("Scope.delta_t");
		double t  = dt * mipwf_->GetNbinsX();

		rate_.rate /= t;

		rate_.staterr = sqrt(rate_.staterr) / t;
		rate_.syserr  = rate_.syserr / t;

		rate_.err = sqrt(pow(rate_.staterr, 2) + pow(rate_.syserr, 2));

}


double *PhysicsChannel::GetOS()
{
		return os_;
}


TH1 *PhysicsChannel::GetHistogram(std::string type)
{
		if (type == "waveform")
		{
				return wf_;
		}
		else if (type == "pedestal")
		{
				return pdhist_;
		}
		else if (type == "reco")
		{
				return recowf_;
		}
		else if (type == "pe")
		{
				return pewf_;
		}
		else if (type == "mip")
		{
				return mipwf_;
		}
		else if (type == "mip_sys")
		{
				return mipwf_sys_;
		}
		else if (type == "mip_stat")
		{
				return mipwf_stat_;
		}
		else
		{
				exit(1);
		}
}

Rate PhysicsChannel::GetRate()
{
		return rate_;
}

// DONE!!!
