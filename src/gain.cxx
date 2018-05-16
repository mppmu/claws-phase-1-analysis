//============================================================================
// Name        : gain.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Jan 7, 2018
// Copyright   : GNU General Public License
// Description :
//============================================================================


// --- BOOST includes ---
// --- ROOT includes ---
#include "TF1.h"
#include <TFitResult.h>
// --- PROJECT includes ---
#include "channel.hh"
#include "gain.hh"
#include "globalsettings.hh"
// --- C++ includes ---
#include <math.h>

using namespace std;

//----------------------------------------------------------------------------------------------
// Definition of the GainChannel class
//----------------------------------------------------------------------------------------------

Double_t gainfunc(Double_t *x, Double_t *par)
{
    double number = par[0]*TMath::Exp(-0.5*pow((x[0] - par[1])/par[2], 2)) + par[3]*TMath::Exp(-0.5*pow((x[0] - par[4])/(par[2]*1.4142), 2));
    return number;
}

GainChannel::GainChannel(std::string name): name_(name), n_(0), gain_({-1}), avg_res_({-1})
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

    avg_ = NULL;
};

GainChannel::GainChannel(std::string name, TFile* rfile): name_(name), n_(0), gain_({-1}), avg_res_({-1})
{
    if(rfile->GetListOfKeys()->Contains(name.c_str()) )
    {
        hist_ = (TH1I*) rfile->Get(name_.c_str());
        hist_->SetDirectory(0);
    }
    else
    {
        hist_ = NULL;
    }

    if(rfile->GetListOfKeys()->Contains((name+"_gain_over_time").c_str()) )
    {
        gain_otime_ = (TGraph*) rfile->Get((name+"_gain_over_time").c_str());
    }
    else
    {
        gain_otime_ = NULL;
    }

    if(rfile->GetListOfKeys()->Contains((name+"_avg").c_str()) )
    {
        avg_ = (TH1F*) rfile->Get((name+"_avg").c_str());
        avg_->SetDirectory(0);
    }
    else
    {
        int nbins = GS->GetParameter<int>("Average1PE.waveform_size");
        double dt = GS->GetParameter<double>("Scope.delta_t");
        avg_ = new TH1F((name+"_avg").c_str(), (name+"_avg").c_str(), nbins, - dt/2, dt*(nbins-1)+dt/2);
        avg_->SetDirectory(0);
    }

}

GainChannel::~GainChannel()
{
    if(hist_ != NULL)       delete hist_;
    if(gain_otime_ != NULL) delete gain_otime_;
    if(avg_ != NULL)        delete avg_;
};

void GainChannel::AddGain(CalibrationChannel * channel, double t)
{
    double integral = channel->GetHistogram()->Integral("width");
    hist_->Fill( integral);
    gain_otime_->SetPoint(gain_otime_->GetN(), t, integral);
}

double* GainChannel::FitGain()
{
    gain_[10]    = hist_->GetEntries();
    gain_[11]    = 0;

    TF1* g1=new TF1( (name_ + "_g1").c_str(),"gaus", hist_->GetBinCenter( hist_->GetMaximumBin())*0.75, hist_->GetBinCenter( hist_->GetMaximumBin())*1.25 );
    g1->SetParameter(0, hist_->GetMaximum());
    g1->SetParameter(1, hist_->GetBinCenter( hist_->GetMaximumBin()));
    g1->SetParameter(2, GS->GetParameter<double>("Gain.sigma"));

    TFitResultPtr result = hist_->Fit(g1,"RLQS+");

    if( int(result) != 0)
    {
        result = hist_->Fit(g1,"RLQS+","", hist_->GetBinCenter( hist_->GetMaximumBin())*0.5 , hist_->GetBinCenter( hist_->GetMaximumBin())*1.5 );
    }

    gain_[0]    = int(result);

    gain_[1]    = g1->GetParameter(0);
    gain_[2]    = g1->GetParameter(1);
    gain_[3]    = g1->GetParError(1);
    gain_[4]    = g1->GetParameter(2);

    // for(int i = 1; i<4; i++ ) gain_[i]    = g1->GetParameter( i-1 );

    gain_[9]     = g1->GetChisquare();
    gain_[10]    = g1->GetNDF();
    gain_[11]    = result->Prob();

    gain_[13]    = 1;
    gain_[14]    = g1->GetParameter(1);

   assert( int(result) == 0 );

   // double g1 fit
   if(result->Ndf() != 0 ) assert( result->Chi2()/result->Ndf() < GS->GetParameter<double>("Gain.chi2_bound") );
   else                    assert( result->Chi2()/0.001         < GS->GetParameter<double>("Gain.chi2_bound") );
    // if( int(result) != 0)
    // {
    //     assert(true)
    //     std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< name_ << ": \033[0m fit SINGLE GAUS due to status: " << int(result) << "\r" << std::endl;
    // 				ivec->gain =  ivec->gain_hist->GetMaximumBin();
    // }

//     //TF1* d_gaus=new TF1( (name_ + "_d_gaus").c_str(),"gaus(0)+gaus(3)",0,3*gaus->GetParameter(1) );
//     TF1* d_gaus=new TF1( (name_ + "_d_gaus").c_str(),gainfunc, 0, 3*gaus->GetParameter(1), 5);
//
// 	double mean_bias = GS->GetParameter<double>("Gain.mean_bias");
//
//     // A lot of magic number shit to get the double fit working;
// 	d_gaus->SetParameter(0,gaus->GetParameter(0));
//     d_gaus->SetParLimits(0,gaus->GetParameter(0)*0.75, gaus->GetParameter(0)*1.25);
//
// 	d_gaus->SetParameter(1,gaus->GetParameter(1));
//     d_gaus->SetParLimits(1,gaus->GetParameter(1)*0.5, gaus->GetParameter(1)*1.5);
//     //d_gaus->SetParLimits(1,gaus->GetParameter(1)*0.9, gaus->GetParameter(1)*1.1);
//
// 	d_gaus->SetParameter(2,gaus->GetParameter(2));
// 	d_gaus->SetParLimits(2,gaus->GetParameter(2)*0.5, gaus->GetParameter(2)*1.5);
//
// 	d_gaus->SetParameter(3,gaus->GetParameter(0)*0.1);
//
// 	d_gaus->SetParameter(4,(gaus->GetParameter(1)-mean_bias)*2 + mean_bias);
//     //d_gaus->SetParLimits(4,(gaus->GetParameter(1)- mean_bias)*1.5 + mean_bias, (gaus->GetParameter(1)-mean_bias)*2.25+mean_bias);
//     d_gaus->SetParLimits(4,gaus->GetParameter(1), gaus->GetParameter(1)*3);
//     //d_gaus->SetParLimits(1,gaus->GetParameter(1), gaus->GetParameter(1)*3);
// //	d_gaus->SetParameter(5,gaus->GetParameter(2));
//
// //    d_gaus->FixParameter(5,gaus->GetParameter(2)*1.414);
//
//
//
//
//
//
// 	//d_gaus->SetParLimits(5,gaus->GetParameter(2)*0.25, gaus->GetParameter(2)*2.5);
//
//     //result = hist_->Fit(d_gaus,"SLQ","",(gaus->GetParameter(1)-3*gaus->GetParameter(2)),((gaus->GetParameter(1)-mean_bias)*2+3*gaus->GetParameter(2)+mean_bias));
//
//     result = hist_->Fit(d_gaus,"SLQ","",(gaus->GetParameter(1)-10*gaus->GetParameter(2)),gaus->GetParameter(1)*2+10*gaus->GetParameter(2));
    // gain_[0]    = int(result);
    // for(int i = 1; i<7; i++ ) gain_[i]    = d_gaus->GetParameter( i-1 );
    //
    // gain_[7]    = d_gaus->GetChisquare();
    // gain_[8]    = d_gaus->GetNDF();
    // gain_[9]    = result->Prob();

    // instead do two gaussians
    double mean_bias = GS->GetParameter<double>("Gain.mean_bias");

    TF1* g2=new TF1( (name_ + "_g2").c_str(),"gaus", g1->GetParameter(1)*1.75, g1->GetParameter(1)*2.25);
    g2->SetParameter(0, g1->GetParameter(0)*0.1);
    g2->SetParameter(1, g1->GetParameter(1)*2 );
    g2->SetParameter(2, g1->GetParameter(2)*1.414);

    TFitResultPtr result2 = hist_->Fit(g2,"RLQS+" );

    gain_[0]    = int(result2);

    gain_[5]    = g2->GetParameter(0);
    gain_[6]    = g2->GetParameter(1);
    gain_[7]    = g2->GetParError(1);
    gain_[8]    = g2->GetParameter(2);


    gain_[13]   = 2;

    // Using the difference in the two peaks
    // if( int(result) == 0 && (result->Chi2()/result->Ndf() <= GS->GetParameter<double>("Gain.chi2_bound") ) )
	// {
    //     gain_[12]   = d_gaus->GetParameter(4) - d_gaus->GetParameter(1);
    // }
    // else
    // {
    //     std::cout << "\033[1;31mFit failing in Gain::FitGain() for ch "<< name_ << ": \033[0m fit DOUBLE GAUS due to status: " << int(result) << "\r" << std::endl;
    // 	gain_[12] = gaus->GetParameter(1);
    // }

    // Just using the first peak
    gain_[14] = g1->GetParameter(1);

    return gain_;
}

void GainChannel::AddWaveform( CalibrationChannel * channel)
{
    double integral = channel->GetHistogram()->Integral("width");

    double low      = GS->GetParameter<double>("Average1PE.gain_low");
    double up       = GS->GetParameter<double>("Average1PE.gain_high");

    if( integral >= low*gain_[12] && integral <= up*gain_[12])
    {
        TH1F * hist_2add = dynamic_cast<TH1F*>(channel->GetHistogram());
        for(int i = 1 ;i< hist_2add->GetNbinsX()+1; ++i) avg_->SetBinContent(i, avg_->GetBinContent(i) + hist_2add->GetBinContent(i) );
        //avg_->Sumw2();
        n_++;
    }
}

double* GainChannel::FitAvg()
{
    avg_res_[0] = n_;


    int waveform_size     = GS->GetParameter<int>("Average1PE.waveform_size");
    int waveform_recorded = GS->GetParameter<int>("Average1PE.waveform_recorded");

    // Extending the calibration waveform is only neccessary if the recording window was to small.
    if( avg_->GetNbinsX() >  waveform_size )
    {
        return avg_res_;
    }

    TF1* expo=new TF1( ( name_ + "_exp" ).c_str(),"[0]*exp(-[1]*(x-[2]))+[3]", 0, avg_->GetNbinsX());

    expo->SetParameter(0, GS->GetParameter<double>("Average1PE.par_0") );
    expo->SetParameter(1, GS->GetParameter<double>("Average1PE.par_1") );
    expo->SetParameter(2, GS->GetParameter<double>("Average1PE.par_2") );
    expo->FixParameter(3, GS->GetParameter<double>("Average1PE.par_3") );

    double dt = GS->GetParameter<double>("Scope.delta_t");

    double start_fit = ( avg_->GetMaximumBin() + GS->GetParameter<int>("Average1PE.start_shift") )*dt;
    double stop_fit  = waveform_recorded * dt;

    avg_res_[1] = start_fit;
    avg_res_[2] = stop_fit;

    TFitResultPtr result = avg_->Fit(expo, "SQ", "", start_fit, stop_fit );

    avg_res_[3] = int(result);

    for(int i = 4; i < 8 ; i++ ) avg_res_[i] = expo->GetParameter( i-4 );

    avg_res_[8]    = result->Chi2();
    avg_res_[9]    = result->Ndf();
    avg_res_[10]   = result->Prob();
    avg_res_[11]   = expo->GetX(0.005);


    if( GS->GetParameter<bool>("Average1PE.extend"))
    {
        int stop_extend = avg_->FindBin( avg_res_[11] );

        if( stop_extend > avg_->GetNbinsX() ) stop_extend = avg_->GetNbinsX();

        for(int i =  waveform_recorded +1; i <= waveform_size; i++)
        {
            double val = expo->Eval( avg_->GetBinCenter(i) );
            avg_->SetBinContent(i, val);
        }
    }


    avg_res_[12]   = avg_->Integral("width");

    return avg_res_;
}

void GainChannel::Normalize()
{
    if(n_ != 0 )
    {
        avg_->Scale( 1./n_);

        double range = claws::RangeToVoltage(claws::PS6000_50MV);
        double bin_error = range/127.;

        for( int i = 1; i <= avg_->GetNbinsX(); ++i)
        {
            avg_->SetBinError(i, bin_error/sqrt(n_) );
        }
    }
}

// Setter and getter methods
std::string GainChannel::GetName()
{
    return name_;
}

TH1I* GainChannel::GetHistogram()
{
    return hist_;
}

TGraph* GainChannel::GetGraph()
{
    return gain_otime_;
}

TH1F* GainChannel::GetAvg()
{
    return avg_;
}

double* GainChannel::GetGain()
{
    return gain_;
}
void GainChannel::SetGain(double *gain)
{
    for( int i = 0; i<13; ++i) gain_[i] = gain[i];
}

double* GainChannel::GetAvgResults()
{
    return avg_res_;
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
        case GAINSTATE_EXTENDED:
            this->LoadChannels(state);
            state_ = GAINSTATE_EXTENDED;
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
        std::string position = name.second.get_value<std::string>();
        if( position != "false")
        {
            if( isdigit(position[0]) && isalpha(position[1]) )
            {
                channels_.emplace_back(new GainChannel(name.first) );
            }
        }
    }
}

void Gain::LoadChannels(GainState state)
{
    boost::filesystem::path src = path_/boost::filesystem::path("Calibration");

    if(state == GAINSTATE_FITTED)
    {
        src = src/boost::filesystem::path("GainDetermination");
    }
    else if( state == GAINSTATE_EXTENDED)
    {
        src = src/boost::filesystem::path("Average1PE");
    }

    std::string fname = (src/("run_"+std::to_string(nr_)+"_" + printGainState(state)+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root")).string();

    TFile *rfile = new TFile(fname.c_str(), "OPEN");

    for (auto &name : GS->GetChannels("Calibration"))
    {
        std::string position = name.second.get_value<std::string>();
        if( position != "false")
        {
            if( isdigit(position[0]) && isalpha(position[1]) )
            {
                channels_.emplace_back(new GainChannel(name.first, rfile) );
            }
        }
    }

    rfile->Close();

    boost::replace_last(fname, "root", "ini");
    if(pt_.empty()) boost::property_tree::ini_parser::read_ini(fname, pt_);


    std::string gain_names[13] = {"Gain_FitStatus","Gain_FitConstant1","Gain_FitMean1","Gain_FitSigma1","Gain_FitConstant2","Gain_FitMean2","Gain_FitSigma2","Gain_FitChi2","Gain_FitNDF","Gain_FitPVal","Gain_Entries","Gain_FitStage","Gain_Gain"};

    for(const auto& channel : channels_)
    {
        double gain[13];

        for(int i = 0; i < 13; i++)
        {
            gain[i] = pt_.get<double>( gain_names[i] + "." + channel->GetName() );
        }

        channel->SetGain(gain);
    }
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
            if(state_ == GAINSTATE_INIT || state_ == GAINSTATE_LOADED )
            {
                channels_.at(i)->AddGain(channel, t);
                state_ = GAINSTATE_LOADED;
            }
            else if(state_ == GAINSTATE_FITTED || state_ == GAINSTATE_AVGED)
            {
                channels_.at(i)->AddWaveform(channel);
                state_ == GAINSTATE_AVGED;
            }

        }
    }

};

void Gain::FitGain()
{
    std::string gain_names[15] = {"Gain_FitStatus","Gain_FitConstant1","Gain_FitMean1","Gain_FitMean1Error","Gain_FitSigma1","Gain_FitConstant2","Gain_FitMean2","Gain_FitMean2Error","Gain_FitSigma2","Gain_FitChi2","Gain_FitNDF","Gain_FitPVal","Gain_Entries","Gain_FitStage","Gain_Gain"};

    //property tree create

    for(const auto& channel : channels_)
    {
        double *gain = channel->FitGain();

        for(int i = 0; i < 15; i++)
        {
            pt_.put( gain_names[i] + "." + channel->GetName(), gain[i] );
        }
    }

    state_ = GAINSTATE_FITTED;
};

void Gain::Normalize()
{
    for(const auto& channel : channels_)
    {
        channel->Normalize();
    }
    state_ = GAINSTATE_NORMALIZED;
};

void Gain::FitAvg()
{
//    std::string gain_names[13] = {"Gain_Status","Gain_FitConstant1","Gain_FitMean1","Gain_FitSigma1","Gain_FitConstant2","Gain_FitMean2","Gain_FitSigma2","Gain_FitChi2","Gain_FitNDF","Gain_FitPVal","Gain_Entries","Gain_FitStage","Gain_Gain"};
    std::string avg_res_names[13] = {"Avg_N", "Avg_FitStart", "Avg_FitStop", "Avg_FitStatus", "Avg_FitPar0", "Avg_FitPar1", "Avg_FitPar2", "Avg_FitPar3", "Avg_FitChi2", "Avg_FitNDF", "Avg_PVal", "Avg_XDecay", "Avg_Integral"};
    //property tree create

    for(const auto& channel : channels_)
    {
        double *avg_res = channel->FitAvg();

        for(int i = 0; i < 13; i++)
        {
            pt_.put( avg_res_names[i] + "." + channel->GetName(), avg_res[i] );
        }
    }

    state_ = GAINSTATE_EXTENDED;
};

void Gain::SaveGain(boost::filesystem::path dst)
{
    std::string fname = (dst/("run_"+std::to_string(nr_)+"_" + printGainState(state_)+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root")).string();
 	TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(const auto& channel : channels_)
    {
        channel->GetHistogram()->Write();
        channel->GetGraph()->Write();
        if( channel->GetAvg() != NULL ) channel->GetAvg()->Write();
    }

	rfile->Close();
	delete rfile;

    pt_.put("General.State", state_);
    boost::replace_last(fname, "root", "ini");
    boost::property_tree::write_ini(fname.c_str(), pt_);
};


GainChannel* Gain::GetChannel(std::string ch)
{
    GainChannel * rtn = nullptr;

    for(auto channel : channels_)
    {
        if(channel->GetName() == ch) rtn = channel;
    }

    return rtn;
}


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
