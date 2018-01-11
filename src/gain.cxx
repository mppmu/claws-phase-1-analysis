

//============================================================================
// Name        : gain.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Jan 7, 2018
// Copyright   : GNU General Public License
// Description :
//============================================================================

// --- C++ includes ---
// --- BOOST includes ---
// --- ROOT includes ---
#include "TF1.h"
#include <TFitResult.h>
// --- PROJECT includes ---
#include "channel.hh"
#include "gain.hh"
#include "globalsettings.hh"

//----------------------------------------------------------------------------------------------
// Definition of the GainChannel class
//----------------------------------------------------------------------------------------------

GainChannel::GainChannel(std::string name): name_(name), n_(0)
{
    hist_ = new TH1I(    name.c_str(),
                                        name.c_str(),
                                        GS->GetParameter<int>("Gain.nbinsx"),
                                        GS->GetParameter<double>("Gain.xlow"),
                                        GS->GetParameter<double>("Gain.xup")
                    );
    gain_otime_ = new TGraph();
    gain_otime_->SetName( (name +"_gain_over_time").c_str() );
    gain_otime_->SetMarkerStyle(23);
    gain_otime_->SetMarkerColor(kRed);
    gain_otime_->SetMarkerSize(1);
    gain_otime_->GetXaxis()->SetTitle("Time [s]");
    gain_otime_->GetYaxis()->SetTitle("Gain [au]");

};

GainChannel::GainChannel(std::string name, TFile* rfile): name_(name), n_(0)
{
    if(rfile->GetListOfKeys()->Contains(name.c_str()) )
    {
        hist_ = (TH1I*) rfile->Get(name_.c_str());
        hist_->SetDirectory(0);
    }
    if(rfile->GetListOfKeys()->Contains((name+"_gain_over_time").c_str()) )
    {
        gain_otime_ = (TGraph*) rfile->Get((name+"_gain_over_time").c_str());
    //    gain_otime_->SetDirectory(0);
    }
    if(rfile->GetListOfKeys()->Contains((name+"_avg").c_str()) )
    {
        avg_ = (TH1D*) rfile->Get((name+"_avg").c_str());
        avg_->SetDirectory(0);
    }
}

GainChannel::~GainChannel()
{
    delete hist_;
    delete gain_otime_;
    //delete avg_;
};

void GainChannel::AddChannel(CalibrationChannel * channel, double t)
{
    double integral = channel->GetHistogram()->Integral("width");
    hist_->Fill( integral);
    gain_otime_->SetPoint(gain_otime_->GetN(), t, integral);
    n_++;
}

void GainChannel::FitGain()
{
    TF1* gaus=new TF1( (name_ + "_gaus").c_str(),"gaus", GS->GetParameter<double>("Gain.xlow"), GS->GetParameter<double>("Gain.xup"));
    gaus->SetParameter(0, hist_->GetMaximum());
    gaus->SetParameter(1, hist_->GetBinCenter( hist_->GetMaximumBin()));
    gaus->SetParameter(2, GS->GetParameter<double>("Gain.sigma"));

    TFitResultPtr result = hist_->Fit(gaus,"QS","",0 , hist_->GetBinCenter( hist_->GetMaximumBin())*1.5 );

    if( int(result) != 0)
    {
        result = hist_->Fit(gaus,"QS","", hist_->GetBinCenter( hist_->GetMaximumBin())*0.75 , hist_->GetBinCenter( hist_->GetMaximumBin())*1.25 );
    }

   assert( int(result) == 0 );

   assert( result->Chi2()/result->Ndf() < GS->GetParameter<double>("Gain.chi2_bound") );

    // if( int(result) != 0)
    // {
    //     assert(true)
    //     std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< name_ << ": \033[0m fit SINGLE GAUS due to status: " << int(result) << "\r" << std::endl;
    // 				ivec->gain =  ivec->gain_hist->GetMaximumBin();
    // }

    TF1* d_gaus=new TF1( (name_ + "_d_gaus").c_str(),"gaus(220)+gaus(420)",0,3*gaus->GetParameter(1) );

	double mean_bias = GS->GetParameter<double>("Gain.mean_bias");

    // A lot of magic number shit to get the double fit working;
	d_gaus->SetParameter(0,gaus->GetParameter(0));
	d_gaus->SetParameter(1,gaus->GetParameter(1));
	d_gaus->SetParameter(2,gaus->GetParameter(2));

	d_gaus->SetParameter(3,gaus->GetParameter(0)*0.1);
	d_gaus->SetParameter(4,(gaus->GetParameter(1)-mean_bias)*2 + mean_bias);
	d_gaus->SetParameter(5,gaus->GetParameter(2));

	d_gaus->SetParLimits(0,gaus->GetParameter(0)*0.75, gaus->GetParameter(0)*1.25);
	d_gaus->SetParLimits(1,gaus->GetParameter(1)*0.9, gaus->GetParameter(1)*1.1);
	d_gaus->SetParLimits(2,gaus->GetParameter(2)*0.75, gaus->GetParameter(2)*1.25);

	d_gaus->SetParLimits(4,(gaus->GetParameter(1)- mean_bias)*1.75 + mean_bias, (gaus->GetParameter(1)-mean_bias)*2.25+mean_bias);
	d_gaus->SetParLimits(5,gaus->GetParameter(2)*0.75, gaus->GetParameter(2)*1.25);

    result = hist_->Fit(d_gaus,"SLQ","",(gaus->GetParameter(1)-3*gaus->GetParameter(2)),((gaus->GetParameter(1)-mean_bias)*2+3*gaus->GetParameter(2)+mean_bias));

    if( int(result) == 0 && (result->Chi2()/result->Ndf() <= GS->GetParameter<double>("Gain.chi2_bound") ) )
	{
        gain_ = d_gaus->GetParameter(4) - d_gaus->GetParameter(1);
    }
    else
    {
        std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< name_ << ": \033[0m fit DOUBLE GAUS due to status: " << int(result) << "\r" << std::endl;
    	gain_ = gaus->GetParameter(1);
    }

}

void GainChannel::Normalize()
{

}

TH1I* GainChannel::GetHistogram()
{
    return hist_;
}

TGraph* GainChannel::GetGraph()
{
    return gain_otime_;
}

TH1D* GainChannel::GetAvg()
{
    return avg_;
}

//----------------------------------------------------------------------------------------------
// Definition of the Gain class used to determin the gain of the Calibration waveforms
//----------------------------------------------------------------------------------------------

Gain::Gain(boost::filesystem::path path, GainState state): path_(path)
{
    nr_     = atoi(path_.filename().string().substr(4,20).c_str());

    switch(state)
    {
        case GAINSTATE_FITTED:
            this->LoadChannels(state);
            state_ = GAINSTATE_FITTED;
            break;
        default:
            this->CreateChannels();
            state_ = GAINSTATE_INIT;
            break;
    }
};

Gain::~Gain()
{
		for(auto & channel : channels_)
		{
				delete channel;
		}
};

void Gain::CreateChannels()
{
    for (auto &name : GS->GetChannels("Calibration"))
    {
        if( name.second.get_value<std::string>() == "true")
        {
            channels_.emplace_back(new GainChannel(name.first) );
        }
    }
}

void Gain::LoadChannels(GainState state)
{
    boost::filesystem::path src = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("GainDetermination");
    std::string fname = (src/("run_"+std::to_string(nr_)+"_" + printGainState(state)+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root")).string();

    TFile *rfile = new TFile(fname.c_str(), "OPEN");

    for (auto &name : GS->GetChannels("Calibration"))
    {
        if( name.second.get_value<std::string>() == "true")
        {
            channels_.emplace_back(new GainChannel(name.first, rfile) );
        }
    }

    rfile->Close();
}

void Gain::AddEvent(CalibrationEvent* evt)
{
    std::vector<Channel*> channels = evt->GetChannels();
    double t = evt->GetTime();

    for(unsigned i =0; i < channels_.size() ; i++)
    {
        CalibrationChannel* channel = dynamic_cast<CalibrationChannel*>(channels.at(i));
        if( evt->GetParameter<int>("PS_Status." + channel->GetName()) == 0 )
        {
            channels_.at(i)->AddChannel(channel, t);
            // double integral = channels.at(i)->GetHistogram()->Integral("width");
            // channels_.at(i)->Fill( integral);
            // gain_otime_.at(i)->SetPoint(gain_otime_.at(i)->GetN(), t, integral);
        }
    }
    state_ = GAINSTATE_LOADED;
};

void Gain::Normalize()
{
    for(const auto& channel : channels_)
    {
        channel->Normalize();
    }
    state_ = GAINSTATE_NORMALIZED;
};

void Gain::FitGain()
{
    for(const auto& channel : channels_)
    {
        channel->FitGain();
    }

    state_ = GAINSTATE_FITTED;
};

void Gain::SaveGain(boost::filesystem::path dst)
{
    std::string fname = (dst/("run_"+std::to_string(nr_)+"_" + printGainState(state_)+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root")).string();
 	TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(const auto& channel : channels_)
    {
        channel->GetHistogram()->Write();
        channel->GetGraph()->Write();
    //    channel->GetAvg()->Write();
    }

	rfile->Close();
	delete rfile;
};





// Gain::Gain(int int_nr) : int_nr_(int_nr)
// {
// 		for(auto & ivec : GS->GetChannels(2))
// 		{
// 				std::string title    = "run_" + std::to_string(int_nr_) + "_" + ivec + "_gain";
// 				std::replace(title.begin(), title.end(), '-','_');
// 				channels_.push_back(
// 						new GainChannel(
// 								ivec,
// 								// histogram for gain extraction
// 								new TH1I(title.c_str(),
// 								         title.c_str(),
// 								         GS->GetParameter<int>("Gain.nbinsx"),
// 								         GS->GetParameter<int>("Gain.xlow"),
// 								         GS->GetParameter<int>("Gain.xup") ),
// 								0,
// 								// Vector holding avg 1 pe waveform
// 								new std::vector<float>(GS->GetParameter<int>("Average1PE.vector_size"),0)
// 								)
// 						);
// 		}
//
// 		// for(auto & v : channel_list_)
// 		// {
// 		//     \todo make the vector initialization proper.
// 		//     std::string title    = "run_" + std::to_string(run_nr_) + "_" + v + "_gain";
// 		//     channels_.push_back(new TH1I(title.c_str(), title.c_str(),140,-210 ,2610));
// 		//     gain_.push_back(0);
// 		// }
//
// };
//
// void Gain::AddValue(std::map<std::string, double> values)
// {
// 		for(auto & ivec : channels_)
// 		{
// 				ivec->gain_hist->Fill(values[ivec->name]);
// 		}
// 		// for(unsigned i =0; i<channels_.size();i++)
// 		// {
// 		//     channels_.at(i).hist->Fill(-values[channel_list_.at(i)]);
// 		// }
// };
//
// // void Gain::AddValue(std::vector<double> values)
// // {
// //         if(values.size() != channels_.size())
// //         {
// //             std::cout << "\033[32;1mGain::AddValue: number of values and channels does not agree! \033[0m done!     "<< std::endl;
// //         }
// //
// //         for(unsigned i =0; i<channels_.size();i++)
// //         {
// //             channels_.at(i)->gain_hist->Fill(-values.at(i));
// //         }
// //         // for(unsigned i =0; i<channels_.size();i++)
// //         // {
// //         //     channels_.at(i)->Fill(-values.at(i));
// //         // }
// // };
//


//
// void Gain::AddIntWf(std::map<std::string, std::vector<float>*> wfs, std::map<std::string, double>integral)
// {
// 		for(unsigned i=0; i < GS->GetChannels(2).size(); i++ )
// 		{
// 				std::string ch_name = GS->GetChannels(2).at(i);
// 				if(    integral[ch_name] >= channels_.at(i)->gain*(1 - GS->GetParameter<double>("Average1PE.allowed_gain"))
// 				       && integral[ch_name] <= channels_.at(i)->gain*(1 + GS->GetParameter<double>("Average1PE.allowed_gain"))
// 				       )
// 				{
// 						std::transform(channels_.at(i)->avg_wf->begin(), channels_.at(i)->avg_wf->end(),wfs[ch_name]->begin(), channels_.at(i)->avg_wf->begin(), std::plus<float>());
// 				}
// 		}
// };
//
// void Gain::AddIntWfs(std::vector<std::vector<IntChannel*> > int_channels)
// {
// 		for(unsigned i = 0; i < channels_.size(); i++)
// 		{
// 				int counter = 0;
// 				for(auto &ivec : int_channels.at(i))
// 				{
// 						if(    ivec->GetIntegral() >= channels_.at(i)->gain*( 1 - GS->GetParameter<double>("Average1PE.allowed_gain") )
// 						       && ivec->GetIntegral() <= channels_.at(i)->gain*( 1 + GS->GetParameter<double>("Average1PE.allowed_gain") )
// 						       )
// 						{
// 								std::vector<float>* wf= ivec->GetWaveform();
// 								std::transform(channels_.at(i)->avg_wf->begin(), channels_.at(i)->avg_wf->end(),wf->begin(), channels_.at(i)->avg_wf->begin(), std::plus<float>());
// 								counter++;
// 						}
// 				}
// 				this->NormalizeWaveform(i, counter);
// 		}
// };
//// void Gain::FitGain()
// {
//
// 		for(auto & ivec : channels_)
// 		{
//
// 				TF1* gaus=new TF1( (ivec->name + "_gaus").c_str(),"gaus",-200, 2600);
//
// 				gaus->SetParameter(0, ivec->gain_hist->GetMaximum());
// 				gaus->SetParameter(1, ivec->gain_hist->GetBinCenter(ivec->gain_hist->GetMaximumBin()));
// 				gaus->SetParameter(2, GS->GetParameter<double>("Gain.sigma"));
//
// 				TFitResultPtr result = ivec->gain_hist->Fit(gaus,"QS","",0,ivec->gain_hist->GetBinCenter(ivec->gain_hist->GetMaximumBin())*1.5);
//
// 				if( int(result) == 0)
// 				{
// 						if( (result->Chi2()/result->Ndf() > GS->GetParameter<double>("Gain.chi2_bound")) )
// 						{
// 								std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< ivec->name << ": \033[0m fit SINGLE GAUS due to Chi2: " << result->Chi2() << ", ndf: "<< result->Ndf()
// 								          << ", status: " << int(result) << "\033[0m"<< "\r" << std::endl;
// 								//    exit(1);
// 								ivec->gain = ivec->gain_hist->GetMaximumBin();
// 						}
// 						else
// 						{
//
// 								TF1* d_gaus=new TF1( (ivec->name + "_d_gaus").c_str(),"gaus(220)+gaus(420)",0,3*gaus->GetParameter(1) );
//
// 								double mean_bias = 25;
//
// 								d_gaus->SetParameter(0,gaus->GetParameter(0));
// 								d_gaus->SetParameter(1,gaus->GetParameter(1));
// 								d_gaus->SetParameter(2,gaus->GetParameter(2));
//
// 								d_gaus->SetParameter(3,gaus->GetParameter(0)*0.1);
// 								d_gaus->SetParameter(4,(gaus->GetParameter(1)-mean_bias)*2 + mean_bias);
// 								d_gaus->SetParameter(5,gaus->GetParameter(2));
//
// 								d_gaus->SetParLimits(0,gaus->GetParameter(0)*0.75, gaus->GetParameter(0)*1.25);
// 								d_gaus->SetParLimits(1,gaus->GetParameter(1)*0.9, gaus->GetParameter(1)*1.1);
// 								d_gaus->SetParLimits(2,gaus->GetParameter(2)*0.75, gaus->GetParameter(2)*1.25);
//
// 								d_gaus->SetParLimits(4,(gaus->GetParameter(1)- mean_bias)*1.75 + mean_bias, (gaus->GetParameter(1)-mean_bias)*2.25+mean_bias);
// 								d_gaus->SetParLimits(5,gaus->GetParameter(2)*0.75, gaus->GetParameter(2)*1.25);
//
// 								result = ivec->gain_hist->Fit(d_gaus,"SLQ","",(gaus->GetParameter(1)-3*gaus->GetParameter(2)),((gaus->GetParameter(1)-mean_bias)*2+3*gaus->GetParameter(2)+mean_bias));
//
// 								if( int(result) == 0 )
// 								{
// 										if( (result->Chi2()/result->Ndf() > GS->GetParameter<double>("Gain.chi2_bound") ) )
// 										{
// 												// If fit fails tell the world and than use the result from the single gaussian fit.
// 												std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< ivec->name << ": \033[0m fit DOUBLE GAUS due to Chi2: " << result->Chi2() << ", ndf: "<< result->Ndf()
// 												          << ", status: " << int(result) << "\033[0m"<< "\r" << std::endl;
// 												ivec->gain = gaus->GetParameter(1);
// 												//    exit(1);
// 										}
// 										else
// 										{
// 												ivec->gain = d_gaus->GetParameter(4) - d_gaus->GetParameter(1);
// 										}
// 								}
// 								else
// 								{
// 										std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< ivec->name << ": \033[0m fit DOUBLE GAUS due to status: " << int(result) << "\r" << std::endl;
// 										ivec->gain = gaus->GetParameter(1);
// 								}
// 						}
// 				}
// 				else
// 				{
// 						std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< ivec->name << ": \033[0m fit SINGLE GAUS due to status: " << int(result) << "\r" << std::endl;
// 						ivec->gain =  ivec->gain_hist->GetMaximumBin();
// 				}
//
// 		}
//
// };
//
// void Gain::NormalizeWaveform(int ch, double norm)
// {
// 		for(unsigned i = 0; i<channels_.at(ch)->avg_wf->size(); i++)
// 		{
// 				channels_.at(ch)->avg_wf->at(i) /= norm;
// 		}
// 		channels_.at(ch)->norm = norm;
// };
//
// void Gain::FitAvg()
// {
// 		for(auto & ivec : channels_)
// 		{
// 				if(ivec->avg_wf->size() < 400)
// 				{
// 						std::string name = ivec->name + "_exponential";
// 						std::replace(name.begin(), name.end(), '-','_');
// 						TF1* expo=new TF1( name.c_str(),"[0]*exp([1]*(x-[2]))+[3]", 1, ivec->avg_hist->GetNbinsX());
// 						expo->SetParameter(0, 37.9);
// 						expo->SetParameter(1,-0.033);
// 						expo->SetParameter(2,75.5);
// 						expo->SetParameter(3,0);
// 						// expo->SetParameter(3,-0.11);
// 						ivec->avg_hist->Fit(expo, "Q", "", ivec->avg_hist->GetMaximumBin()+10, ivec->avg_wf->size());
// 						ivec->end = round( expo->GetX(0.005) );
// 						// std::cout<< "ivec->end: " << ivec->end << std::endl;
// 						for(signed i = ivec->avg_wf->size()+1; i < ivec->end + 1; i++)
// 						{
// 								ivec->avg_hist->SetBinContent(i, expo->Eval(i));
// 						}
// 				}
// 				else
// 				{
// 						ivec->end = ivec->avg_wf->size();
// 				}
// 		}
// }
//
// void Gain::SaveAvg(boost::filesystem::path path_run)
// {
// 		if(!boost::filesystem::is_directory(path_run/boost::filesystem::path("Calibration")) )
// 		{
// 				boost::filesystem::create_directory(path_run/boost::filesystem::path("Calibration"));
// 		}
//
// 		std::string fname = path_run.string()+"/Calibration/run_"+std::to_string(int_nr_)+"_avg1pe" +"_v"+ std::to_string(GS->GetParameter<int>("General.CalibrationVersion"))+".root";
// 		TFile *rfile = new TFile(fname.c_str(), "RECREATE");
//
// 		TGraphErrors* total_avg1pe = new TGraphErrors();
// 		total_avg1pe->SetName("total_avg1pe");
// 		total_avg1pe->SetTitle("Average 1 PE integral for all channles");
// 		total_avg1pe->SetMarkerColor(kRed);
// 		total_avg1pe->SetMarkerStyle(34);
// 		total_avg1pe->SetMarkerSize(1.8);
// 		total_avg1pe->SetMinimum(0);
// 		total_avg1pe->SetMaximum(1e-6);
// 		total_avg1pe->GetXaxis()->SetTitle("channel FWD1-3 BWD1-3");
// 		total_avg1pe->GetYaxis()->SetTitle("Integral of average 1 pe waveform [IntCountsX0.8e-9ns]");
//
// 		TGraph* total_norm = new TGraph();
// 		total_norm->SetName("norm");
// 		total_norm->SetTitle("Number of intermediate waveforms used for avg.");
// 		total_norm->SetMarkerColor(kRed);
// 		total_norm->SetMarkerStyle(34);
// 		total_norm->SetMarkerSize(1.8);
// 		total_norm->SetMinimum(0);
// 		total_norm->SetMaximum(2000);
//
// 		int i=0;
// 		for(auto & ivec : channels_)
// 		{
// 				// std::string title = "run_" + std::to_string(run_nr_) + "_" + ivec->name + "_avg1pe";
// 				//
// 				// TH1F* tmp = new TH1F(title.c_str(), title.c_str(),ivec->avg_wf->size() ,-0.5 ,ivec->avg_wf->size()+0.5 );
// 				// for(unsigned i=0; i < ivec->avg_wf->size(); i++)
// 				// {
// 				//     tmp->SetBinContent(i+1, ivec->avg_wf->at(i));
// 				// }
// 				ivec->avg_hist->Write();
// 				total_avg1pe->SetPoint(i, i+1, ivec->avg_hist->Integral()*GS->GetParameter<double>("General.BinWidth"));
// 				total_avg1pe->SetPointError(i, 0, ivec->avg_hist->Integral()*GS->GetParameter<double>("General.BinWidth")/sqrt(ivec->norm) );
// 				total_norm->SetPoint(i, i+1, ivec->norm);
// 				i++;
// 				// delete tmp;
// 				// tmp = NULL;
// 		}
//
// 		total_avg1pe->Write();
// 		total_norm->Write();
//
// 		rfile->Close();
// 		delete rfile;
// };
//
// void Gain::WfToHist()
// {
// 		for(auto & ivec : channels_)
// 		{
// 				delete ivec->avg_hist;
//
// 				std::string title = "run_" + std::to_string(int_nr_) + "_" + ivec->name + "_avg1pe";
// 				std::replace(title.begin(), title.end(), '-','_');
//
// 				// If avg 1 pe waveforms are smaller than 400 values they need to be fitted and extended!
// 				if(ivec->avg_wf->size() < 400)
// 				{
// 						ivec->avg_hist = new TH1F(title.c_str(), title.c_str(), 400, 0.5, 400.5 );
// 				}
// 				else
// 				{
// 						ivec->avg_hist = new TH1F(title.c_str(), title.c_str(), ivec->avg_wf->size(), 0.5, ivec->avg_wf->size() +0.5 );
// 				}
//
// 				for(unsigned i=0; i < ivec->avg_wf->size(); i++)
// 				{
// 						ivec->avg_hist->SetBinContent(i+1, ivec->avg_wf->at(i));
// 				}
// 		}
// };
//
// void Gain::HistToWf()
// {
// 		for(auto & ivec : channels_)
// 		{
// 				/**
// 				 * [Gain::GetGain description]
// 				 * @param  channel [description]
// 				 * @return         [description]
// 				 * \ todo id would be faster to set the vector completely to zero and than only fill the entries bigger than 0
// 				 *  instead of pushbacking.
// 				 *  \ todo check if setting everything ecept the waveform to zero works
// 				 */
// 				ivec->avg_wf->clear();
// 				// std::cout<< "1 Size: " << ivec->avg_wf->size() << ", Capacity: " << ivec->avg_wf->capacity()<< std::endl;
// 				//if(ivec->end-129 > ivec->avg_wf->capacity()) ivec->avg_wf->reserve(ivec->end-129);
// 				// std::cout<< "2 Size: " << ivec->avg_wf->size() << ", Capacity: " << ivec->avg_wf->capacity()<< std::endl;
//
// 				// for( unsigned i = 0; i < ivec->avg_hist->GetBinCenter(ivec->gain_hist->GetMaximumBin()) - 10; i++)
// 				// {
// 				//     ivec->avg_wf->push_back(0);
// 				// }
//
// //        for( unsigned i = ivec->avg_hist->GetBinCenter(ivec->gain_hist->GetMaximumBin()) - 10; i < ivec->avg_hist->GetNbinsX(); i++)
// 				for( unsigned i = 0; i < ivec->avg_hist->GetNbinsX(); i++)
// 				{
// 						// if(ivec->avg_hist->GetBinContent( i + 1 ) >= 0 )
// 						// {
// 						ivec->avg_wf->push_back(ivec->avg_hist->GetBinContent( i + 1 ));
// 						// }
// 						// else
// 						// {
// 						//     ivec->avg_wf->push_back(0);
// 						// }
// 				}
// 		}
// };
//
// double Gain::GetGain(std::string channel)
// {
// 		std::vector<GainChannel*>::iterator ivec = std::find_if(channels_.begin(), channels_.end(), boost::bind(&GainChannel::name, _1) == channel);
// 		// std::vector<std::string>::iterator ivec = std::find(channel_list_.begin(), channel_list_.end(), channel);
// 		// if(ivec != channels_.end()) return ain_[std::distance(channels_.begin(),ivec)];
// 		if(ivec != channels_.end()) return (*ivec)->gain;
// 		else return -1;
// };
//
// std::map<std::string, double> Gain::GetGain()
// {
// 		std::map<std::string, double> rtn;
// 		for(auto & ivec : channels_)
// 		{
// 				rtn[ivec->name] = ivec->gain;
// 		}
//
// 		// for(unsigned i = 0; i < channel_list_.size(); i++)
// 		// {
// 		//     rtn[channel_list_.at(i)] = gain_.at(i);
// 		// }
// 		return rtn;
// };
//
// std::vector<float>* Gain::GetWaveform(std::string channel)
// {
// 		std::vector<GainChannel*>::iterator ivec = std::find_if(channels_.begin(), channels_.end(), boost::bind(&GainChannel::name, _1) == channel);
// 		return (*ivec)->avg_wf;
// };
//
// std::map<std::string, std::vector<float>*> Gain::GetWaveform()
// {
// 		std::map<std::string, std::vector<float>*> rtn;
// 		for(auto & ivec : channels_)
// 		{
// 				rtn[ivec->name] = ivec->avg_wf;
// 		}
// 		return rtn;
// };
//
//
//
