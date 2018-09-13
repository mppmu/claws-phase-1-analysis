//============================================================================
// Name        : event.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

// --- C++ includes ---
#include <iostream>
#include <string>
#include <ctype.h>
#include <cxxabi.h>
#include <algorithm>
#include <assert.h>
#include <algorithm>    // std::sort
// --- BOOST includes ---
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/replace.hpp>

// --- OpenMP includes ---
// #include <omp.h>
// --- ROOT includes ---

// --- GSL includes ---
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
// #include <gsl/gsl_errno.h>
// #include <gsl/gsl_rng.h>
// #include <gsl/gsl_randist.h>
// #include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

// --- Project includes ---
#include "event.hh"
#include "globalsettings.hh"

// ---


using namespace std;
using namespace boost;

//----------------------------------------------------------------------------------------------
// Definition of the Event base class.
//----------------------------------------------------------------------------------------------



Event::Event()
{

};

Event::Event(boost::filesystem::path file, boost::filesystem::path ini_file ) : path_(file.parent_path().parent_path()), file_(file), ini_file_(ini_file), state_(EVENTSTATE_INIT)
{
		boost::property_tree::ini_parser::read_ini(ini_file_.string(), pt_);
		pt_.put("General.State", state_);
};

Event::~Event()
{
		for(auto &channel: channels_) delete channel;
}

void Event::LoadFiles()
{
		this->LoadRaw();
}

void Event::LoadRaw()
{
		this->LoadHistograms(file_);
		// Only load the property tree from the ini file when it was not loaded yet
		if(pt_.empty()) boost::property_tree::ini_parser::read_ini(ini_file_.string(), pt_);

		//state_ = static_cast<EventState>( pt_.get<int>("General.State") );
		state_ = EVENTSTATE_RAW;
		pt_.put("General.State", state_);
}

void Event::LoadHistograms(boost::filesystem::path file)
{
		TFile *rfile = new TFile( file.string().c_str(), "open");

		if( rfile->IsZombie() )
		{
				std::cout << "Error openning file" << std::endl;
				exit(-1);
		}

		for (const auto &ch : channels_)
		{
				ch->LoadHistogram(rfile);
		}

		rfile->Close("R");
		delete rfile;
		rfile = NULL;
}

void Event::PrepareHistograms()
{
		for (auto channel : channels_)
		{
				channel->PrepareHistogram();
		}

		state_ = EVENTSTATE_PREP;
		pt_.put("General.State", state_);
}

void Event::SaveEvent(boost::filesystem::path dst, bool save_pd)
{
		/**
		 *  \todo Kill the path paramter and make it state dependet!
		 */

		std::string fname = dst.string() + "/";

		int status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		std::stringstream ss;
		ss << std::setw(3) << std::setfill('0') << nr_;
		fname += "_" + ss.str();
		fname += "_" + printEventState(state_);
		fname += ".root";

		TFile *rfile = new TFile(fname.c_str(), "RECREATE");

		for(auto channel : channels_)
		{
				channel->GetHistogram()->Write();
				if(save_pd) channel->GetHistogram("pedestal")->Write();
		}

		rfile->Close("R");
		delete rfile;

		boost::replace_last(fname, "root", "ini");
		boost::property_tree::write_ini(fname.c_str(), pt_);

		// boost::filesystem::path dest = folder/path_file_ini_.filename();
		// boost::filesystem::copy_file(path_file_ini_, dest, copy_option::overwrite_if_exists );
};

void Event::DeleteHistograms()
{
		for (auto channel : channels_)
		{
				channel->DeleteHistogram();
		}

		state_ = EVENTSTATE_INIT;
		pt_.put("General.State", state_);
};

void Event::FillPedestals()
{
		std::string pdnames[11] = {"PS_Status","PS_FitConstant","PS_FitMean","PS_FitMeanError","PS_FitSigma","PS_FitChi2","PS_FitNDF","PS_FitPVal","PS_HistMean","PS_HistError","PS_HistEntries"};

		// int nthreads   = GS->GetParameter<int>("General.nthreads");
		// bool parallelize = GS->GetParameter<bool>("General.parallelize");
		//
		// #pragma omp parallel for if(parallelize) num_threads(nthreads)
		// for( int i = 0; i < channels_.size(); ++i )
		// {
		//  // First fill and fit the pedestal
		//  channels_.at(i)->FillPedestal();
		// }
		for(auto &channel: channels_ )
		{
				// First fill and fit the pedestal
				channel->FillPedestal();
		}

		for(auto &channel: channels_ )
		{
				// Than add all the pd info from the
				// pd hist and the fit to the property
				// tree/ini file.
				std::string name = channel->GetName();
				double * pd = channel->GetPedestal();

				for(int i = 0; i< 11; i++)
				{
						pt_.put(pdnames[i] + "." + name, pd[i]);
				}

				if( channel->GetState() == CHANNELSTATE_FAILED or  channel->GetState() == CHANNELSTATE_PDFAILED )
				{
						state_ = EVENTSTATE_PDFAILED;
						pt_.put("General.State", state_);
				}
		}

		if( state_ == EVENTSTATE_PDFAILED )
		{
				return;
		}
		else
		{
				state_ = EVENTSTATE_PDFILLED;
				pt_.put("General.State", state_);
		}
};

void Event::SubtractPedestals(std::vector<double> pd)
{

		if( state_ == EVENTSTATE_PDFAILED ) return;


		if(pd.size() == channels_.size())
		{
				int nthreads   = GS->GetParameter<int>("General.nthreads");
				bool parallelize = GS->GetParameter<bool>("General.parallelize");

		#pragma omp parallel for if(parallelize) num_threads(nthreads) private(pd)
				for(int i = 0; i < pd.size(); i++ )
				{
						channels_.at(i)->SubtractPedestal(pd.at(i));
				}
				// for(auto &channel : channels_)
				// {
				//  channels_.at(i)->SubtractPedestal(pd.at(i));
				// }
		}
		else
		{
				int nthreads   = GS->GetParameter<int>("General.nthreads");
				bool parallelize = GS->GetParameter<bool>("General.parallelize");

		#pragma omp parallel for if(parallelize) num_threads(nthreads)
				for( int i = 0; i < channels_.size(); ++i )
				{
						channels_.at(i)->SubtractPedestal();
				}
				// for(auto &channel : channels_)
				// {
				//  channel->SubtractPedestal();
				// }
		}

		state_ = EVENTSTATE_PDSUBTRACTED;
		pt_.put("General.State", state_);
}

void Event::SetTime(double unixtime)
{
		unixtime_ = unixtime;
}


double Event::GetTime()
{
		return unixtime_;
}

long Event::GetNumber()
{
		return nr_;
}

std::vector<Channel*> Event::GetChannels()
{
		return channels_;
}

EventState Event::GetState()
{
		return state_;
}

filesystem::path Event::GetPath(string type)
{
		if(type == "path") return path_;
		else if(type == "file") return file_;
		else if(type == "ini_file") return ini_file_;
		else return boost::filesystem::path("");
};

//----------------------------------------------------------------------------------------------
// Definition of the CalibrationEvent class derived from Event.
//----------------------------------------------------------------------------------------------

CalibrationEvent::CalibrationEvent(boost::filesystem::path file, boost::filesystem::path ini_file, double unixtime ) : Event(file, ini_file), runnr_(-1)
{
		nr_       = std::atoi(file.filename().string().substr(14,3).c_str());
		runnr_    = std::atoi(file.filename().string().substr(4,6).c_str());

		unixtime_ = unixtime;

		pt_.put("Properties.UnixTime", unixtime);
		pt_.put("General.UnixTime", unixtime);

		for (auto name : GS->GetChannels("Calibration"))
		{
				std::string position = name.second.get_value<std::string>();
				if( position != "false")
				{
						if( isdigit(position[0]) && isalpha(position[1]) )
						{
								channels_.emplace_back( new CalibrationChannel(name.first, position) );
						}
				}
		}

};

CalibrationEvent::~CalibrationEvent() {
		// TODO Auto-generated destructor stub

};

void CalibrationEvent::LoadFiles(EventState state)
{
		switch (state)
		{
		case EVENTSTATE_RAW:
		{
				this->LoadRaw();
				break;
		}
		case EVENTSTATE_PDSUBTRACTED:
		{
				this->LoadSubtracted();
				break;
		}
		}

}

void CalibrationEvent::LoadSubtracted()
{
		std::string fname = (path_/boost::filesystem::path("Calibration")/boost::filesystem::path("PDS_Calibration")/boost::filesystem::path("Waveforms")).string() + "/";

		int status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		std::stringstream ss;
		ss << runnr_ << "-cal" << std::setw(3) << std::setfill('0') << nr_;
		fname += "_" + ss.str();
		fname += "_" + printEventState(EVENTSTATE_PDSUBTRACTED);
		fname += ".root";

		// If the PD failded the file will be saved with a pd_failed in its name
		if( boost::filesystem::exists(fname) )
		{
				this->LoadHistograms(boost::filesystem::path(fname));
				boost::replace_last(fname, "root","ini");
				boost::property_tree::ini_parser::read_ini(fname, pt_);

				state_ = static_cast<EventState>( pt_.get<int>("General.State") );
		}
		else
		{
				state_ = EVENTSTATE_FAILED;
		}

		// // If the PD failded the file will be saved with a pd_failed in its name
		// if( boost::filesystem::exists(fname) )
		// {
		//  this->LoadHistograms(boost::filesystem::path(fname));
		// }
		// else
		// {
		//  boost::replace_last(fname, printEventState(EVENTSTATE_PDSUBTRACTED),printEventState(EVENTSTATE_PDFAILED));
		//  if( boost::filesystem::exists(fname) )
		//  {
		//      this->LoadHistograms(boost::filesystem::path(fname));
		//  }
		//  else
		//  {
		//      return;
		//  }
		// }
		//
		// boost::replace_last(fname, "root","ini");
		// boost::property_tree::ini_parser::read_ini(fname, pt_);
		//
		// state_ = static_cast<EventState>( pt_.get<int>("General.State") );

}

void CalibrationEvent::SaveEvent(boost::filesystem::path dst, bool save_pd)
{
		/**
		 *  \todo Kill the path parameter and make it state dependet!
		 */

		std::string fname = dst.string() + "/";

		int status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		std::stringstream ss;

		ss << runnr_ << "-cal"<< std::setw(3) << std::setfill('0') << nr_;

		fname += "_" + ss.str();
		fname += "_" + printEventState(state_);
		fname += ".root";

		TFile *rfile = new TFile(fname.c_str(), "RECREATE");

		for(auto channel : channels_)
		{
				// string name = channel->GetHistogram()->GetName();
				// boost::replace_first(name, "-", "_");
				// channel->GetHistogram()->Write(name.c_str());
				// if(save_pd)
				// {
				//  string name = channel->GetHistogram()->GetName();
				//  boost::replace_first(name, "-", "_");
				//  channel->GetHistogram("pedestal")->Write(name.c_str());
				// }

				channel->GetHistogram()->Write();
				if(save_pd)
				{
						channel->GetHistogram("pedestal")->Write();
				}
		}

		rfile->Close("R");
		delete rfile;

		boost::replace_last(fname, "root", "ini");
		boost::property_tree::write_ini(fname.c_str(), pt_);

		// boost::filesystem::path dest = folder/path_file_ini_.filename();
		// boost::filesystem::copy_file(path_file_ini_, dest, copy_option::overwrite_if_exists );
};

void CalibrationEvent::PrepareHistograms()
{

		for (auto &channel : channels_)
		{
				channel->PrepareHistogram( claws::RangeToVoltage(claws::PS6000_50MV) );
		}

		state_ = EVENTSTATE_PREP;
		pt_.put("General.State", printEventState(state_));

}

//----------------------------------------------------------------------------------------------
// Definition of the PhysicsEvent class derived from Event.
//----------------------------------------------------------------------------------------------

PhysicsEvent::PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file) : Event(file, ini_file)
{
		/**
		 * TODO description of PhysicsEvent class.
		 */

		// fill_n(online_rate_, 6, -1);
		// fill_n(fast_rate_, 3, -1);
		// fill_n(rate_, 3, -1);

		// nr_str_ = file_root.filename().string().substr(6,9);
		// Get the path to the file from the online monitor
		int phase = GS->GetParameter<int>("General.Phase");

		if(phase == 1)
		{
				string tmp = file.filename().string();
				boost::replace_first(tmp, "Event-","");
				boost::replace_last(tmp, ".root", "");
				nr_     = stol( tmp );
		}
		else if(phase == 2)
		{
				string tmp = file.filename().string();
				boost::replace_first(tmp, "physics-","");
				boost::replace_last(tmp, ".root", "");
				tmp.erase(tmp.find("-"),1);
				nr_     = stol( tmp );
		}


		/** Now the string only contains a number. The first 6 digits
		 *  digits are the runnumber. Since for the muon runs 4 digit
		 *   and maybe even more event numbers are used, just take the
		 *   last 15 to be sure
		 */
		//nr_     = atoi( tmp.substr(6,15).c_str());

		// if(file.filename().string().size() == 20 )
		// {
		//  nr_     = atoi(file.filename().string().substr(6,9).c_str());
		// }
		// else if(file.filename().string().size() == 21)
		// {
		//  nr_     = atoi(file.filename().string().substr(6,10).c_str());
		// }
		// else
		// {
		//  assert(0);
		// }

		for (auto &name : GS->GetChannels("Physics"))
		{
				std::string position = name.second.get_value<std::string>();
				if( position != "false")
				{
						if( isdigit(position[0]) && isalpha(position[1]) )
						{
								channels_.emplace_back( new PhysicsChannel(name.first, position) );
						}
				}
		}
};

PhysicsEvent::PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file, boost::filesystem::path rate_file) : PhysicsEvent(file, ini_file)
{
		rate_file_ = rate_file;

		// online_rates_.resize(6, -1);
		// fast_rates_.resize(3,-1);
		// rates_.resize(3,-1);


		//
		//
		//
		//      for(unsigned i = 0; i < 4; i++)
		//      {
		//              if(!ends_with(GS->GetChannels(1).at(i), "4"))
		//              {
		//                      pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i]);
		//              }
		//      }
		//      for(unsigned i = 4; i < 8; i++)
		//      {
		//              if(!ends_with(GS->GetChannels(1).at(i), "4"))
		//              {
		//                      pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i-1]);
		//              }
		//      }

};


PhysicsEvent::~PhysicsEvent() {
		// TODO Auto-generated destructor stub
};

void PhysicsEvent::LoadFiles(EventState state)
{
		boost::filesystem::path tmppath = path_/boost::filesystem::path("Calibration");

		switch (state)
		{
		case EVENTSTATE_RAW:
		{
				// Don't do shit...
				// The raw files are loaded via the standard path given in the constructor
				break;
		}
		case EVENTSTATE_PDSUBTRACTED:
		{
				tmppath /= boost::filesystem::path("PDS_Physics");
				tmppath /= boost::filesystem::path("Waveforms");

				break;
		}

		case EVENTSTATE_OSCORRECTED:
		{
				tmppath /= boost::filesystem::path("OverShootCorrection");
				tmppath /= boost::filesystem::path("Waveforms");

				break;
		}

		case EVENTSTATE_TAGGED:
		{
				tmppath /= boost::filesystem::path("SignalTagging");
				tmppath /= boost::filesystem::path("Waveforms");

				break;
		}

		case EVENTSTATE_WFDECOMPOSED:
		{
				tmppath /= boost::filesystem::path("WaveformDecomposition");
				tmppath /= boost::filesystem::path("Waveforms");

				break;
		}
		case EVENTSTATE_WFRECONSTRUCTED:
		{
				tmppath /= boost::filesystem::path("WaveformReconstruction");
				tmppath /= boost::filesystem::path("Waveforms");

				break;
		}
		case EVENTSTATE_CALIBRATED:
		{
				tmppath = tmppath.parent_path()/boost::filesystem::path("Results");
				tmppath /= boost::filesystem::path("Waveforms");

				break;
		}

		}

		std::string fname = tmppath.string() + "/";

		int status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		std::stringstream ss;

		int ndigits = GS->GetParameter<int>("General.event_ndigits");

		ss << std::setw(ndigits) << std::setfill('0') << nr_;

//	ss << std::setw(3) << std::setfill('0') << nr_;
		fname += "_" + ss.str();
		fname += "_" + printEventState(state);
		fname += ".ini";


		switch (state)
		{
		case EVENTSTATE_RAW:
		{
				// Don't do shit...
				// The raw files are loaded via the standard path given in the constructor
				this->LoadRaw();

				vector<double> online_rates(6,-1);

				std::ifstream ratefile(rate_file_.string());

				if (ratefile)
				{
						double dummy;

						ratefile >> online_rates.at(0) >> online_rates.at(1) >> online_rates.at(2) >> dummy >> online_rates.at(3) >> online_rates.at(4) >> online_rates.at(5) >> dummy;
				}
				else
				{
						cout << "\033[1;31mOnlinerate missing:   \033[0m" << nr_ << endl;
				}

				ratefile.close();

				string chnames[6] = {"FWD1", "FWD2", "FWD3", "BWD1", "BWD2", "BWD3"};

				for(unsigned int j = 0; j < online_rates.size(); ++j)
				{
						pt_.put("OnlineRate." + chnames[j], online_rates.at(j) );
				}

				break;
		}

		case EVENTSTATE_PDSUBTRACTED:
		{
				if( boost::filesystem::exists(fname) )
				{
						boost::property_tree::ini_parser::read_ini(fname, pt_);

						boost::replace_last(fname, "ini","root");

						this->LoadHistograms(boost::filesystem::path(fname));

						state_ = static_cast<EventState>( pt_.get<int>("General.State") );
				}
				else
				{
						state_ = EVENTSTATE_FAILED;
				}
				// If the PD failded the file will be saved with a pd_failed in its name
				// boost::property_tree::ini_parser::read_ini(fname, pt_);
				//
				// if( static_cast<EventState>( pt_.get<int>("General.State") ) == EVENTSTATE_PDSUBTRACTED)
				// {
				//  boost::replace_last(fname, "ini","root");
				//  this->LoadHistograms(boost::filesystem::path(fname));
				// }

				// if( boost::filesystem::exists(fname) )
				// {
				//      this->LoadHistograms(boost::filesystem::path(fname));
				// }
				// else
				// {
				//  boost::replace_last(fname, printEventState(EVENTSTATE_PDSUBTRACTED),printEventState(EVENTSTATE_PDFAILED));
				//  if( boost::filesystem::exists(fname) )
				//  {
				//      this->LoadHistograms(boost::filesystem::path(fname));
				//  }
				// }

				// boost::replace_last(fname, "root","ini");

				break;
		}

		case EVENTSTATE_OSCORRECTED:
		{
				if( boost::filesystem::exists(fname) )
				{
						boost::property_tree::ini_parser::read_ini(fname, pt_);

						boost::replace_last(fname, "ini","root");

						this->LoadHistograms(boost::filesystem::path(fname));

						state_ = static_cast<EventState>( pt_.get<int>("General.State") );
				}
				else
				{
						state_ = EVENTSTATE_FAILED;
				}
				// this->LoadHistograms(boost::filesystem::path(fname));
				//
				// boost::replace_last(fname, "root","ini");
				//boost::property_tree::ini_parser::read_ini(fname, pt_);

				// boost::property_tree::ini_parser::read_ini(fname, pt_);
				//
				// if( static_cast<EventState>( pt_.get<int>("General.State") ) == EVENTSTATE_OSCORRECTED)
				// {
				//  boost::replace_last(fname, "ini","root");
				//  this->LoadHistograms(boost::filesystem::path(fname));
				// }

				break;
		}

		case EVENTSTATE_TAGGED:
		{
				if( boost::filesystem::exists(fname) )
				{
						boost::property_tree::ini_parser::read_ini(fname, pt_);

						boost::replace_last(fname, "ini","root");

						//this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco load"}));
						this->LoadHistograms(boost::filesystem::path(fname));

						state_ = static_cast<EventState>( pt_.get<int>("General.State") );
				}
				else
				{
						state_ = EVENTSTATE_FAILED;
				}

				// this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco load"}));
				//
				// boost::replace_last(fname, "root","ini");
				// boost::property_tree::ini_parser::read_ini(fname, pt_);

				// boost::property_tree::ini_parser::read_ini(fname, pt_);
				//
				// if( static_cast<EventState>( pt_.get<int>("General.State") ) == EVENTSTATE_TAGGED)
				// {
				//  boost::replace_last(fname, "ini","root");
				//  this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco load"}));
				// }

				break;
		}

		case EVENTSTATE_WFDECOMPOSED:
		{
				if( boost::filesystem::exists(fname) )
				{
						boost::property_tree::ini_parser::read_ini(fname, pt_);

						boost::replace_last(fname, "ini","root");

						this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco recreate", "pe"}));

						state_ = static_cast<EventState>( pt_.get<int>("General.State") );
				}
				else
				{
						state_ = EVENTSTATE_FAILED;
				}
				// this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco recreate", "pe"}));
				//
				// boost::replace_last(fname, "root","ini");
				// boost::property_tree::ini_parser::read_ini(fname, pt_);

				// boost::property_tree::ini_parser::read_ini(fname, pt_);
				//
				// if( static_cast<EventState>( pt_.get<int>("General.State") ) == EVENTSTATE_WFDECOMPOSED)
				// {
				//  boost::replace_last(fname, "ini","root");
				//  this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco recreate", "pe"}));
				// }

				break;
		}

		case EVENTSTATE_WFRECONSTRUCTED:
		{
				if( boost::filesystem::exists(fname) )
				{
						boost::property_tree::ini_parser::read_ini(fname, pt_);

						boost::replace_last(fname, "ini","root");

						this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe"}) );

						state_ = static_cast<EventState>( pt_.get<int>("General.State") );
				}
				else
				{
						state_ = EVENTSTATE_FAILED;
				}

				// this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe"}) );
				//
				// boost::replace_last(fname, "root","ini");
				// boost::property_tree::ini_parser::read_ini(fname, pt_);

				// boost::property_tree::ini_parser::read_ini(fname, pt_);
				//
				// if( static_cast<EventState>( pt_.get<int>("General.State") ) == EVENTSTATE_WFRECONSTRUCTED)
				// {
				//  boost::replace_last(fname, "ini","root");
				//  this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe"}) );
				// }

				break;
		}

		case EVENTSTATE_CALIBRATED:
		{
				if( boost::filesystem::exists(fname) )
				{
						boost::property_tree::ini_parser::read_ini(fname, pt_);

						boost::replace_last(fname, "ini","root");

						this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe", "mip"}) );

						state_ = static_cast<EventState>( pt_.get<int>("General.State") );
				}
				else
				{
						state_ = EVENTSTATE_FAILED;
				}
				// this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe", "mip"}) );
				//
				// boost::replace_last(fname, "root","ini");
				// boost::property_tree::ini_parser::read_ini(fname, pt_);

				// boost::property_tree::ini_parser::read_ini(fname, pt_);
				//
				// if( static_cast<EventState>( pt_.get<int>("General.State") ) == EVENTSTATE_CALIBRATED)
				// {
				//  boost::replace_last(fname, "ini","root");
				//  this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe", "mip"}) );
				// }

				break;
		}
		}

		//state_ = static_cast<EventState>( pt_.get<int>("General.State") );
};

void PhysicsEvent::LoadHistograms(boost::filesystem::path file, std::vector<std::string> types)
{
		TFile *rfile = new TFile( file.string().c_str(), "open");

		if( rfile->IsZombie() )
		{
				std::cout << "Error openning file" << std::endl;
				exit(-1);
		}

		for (const auto &ch : channels_)
		{
				PhysicsChannel* pch = dynamic_cast<PhysicsChannel*>( ch );
				pch->LoadHistogram(rfile, types);
		}

		rfile->Close("R");
		delete rfile;
		rfile = NULL;
}

void PhysicsEvent::SaveEvent(boost::filesystem::path dst)
{
		/**
		 *  \todo Kill the path parameter and make it state dependet!
		 */

		std::string fname = dst.string() + "/";

		int status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		std::stringstream ss;

		int ndigits = GS->GetParameter<int>("General.event_ndigits");

		ss << std::setw(ndigits) << std::setfill('0') << nr_;

		fname += "_" + ss.str();
		fname += "_" + printEventState(state_);
		fname += ".root";

		TFile *rfile = new TFile(fname.c_str(), "RECREATE");

		for(auto channel : channels_)
		{
				auto wf = channel->GetHistogram("waveform");
				if( wf ) wf->Write();

				auto pdhist = channel->GetHistogram("pedestal");
				if( pdhist ) pdhist->Write();

				auto reco = channel->GetHistogram("reco");
				if( reco ) reco->Write();

				auto pe = channel->GetHistogram("pe");
				if( pe ) pe->Write();

				auto mip = channel->GetHistogram("mip");
				if( mip ) mip->Write();

				auto mip_sys = channel->GetHistogram("mip_sys");
				if( mip_sys ) mip_sys->Write();

				auto mip_stat = channel->GetHistogram("mip_stat");
				if( mip_stat ) mip_stat->Write();
		}

		rfile->Close("R");
		delete rfile;

		boost::replace_last(fname, "root", "ini");
		boost::property_tree::write_ini(fname.c_str(), pt_);

		// boost::filesystem::path dest = folder/path_file_ini_.filename();
		// boost::filesystem::copy_file(path_file_ini_, dest, copy_option::overwrite_if_exists );
};

void PhysicsEvent::PrepareHistograms(boost::property_tree::ptree &settings)
{

		for (auto &channel : channels_)
		{
				std::string scope = std::string(1, channel->GetScopePos()[0]);
				std::string pos   = std::string(1, channel->GetScopePos()[1]);
				std::string sec   = "Scope-" + scope + "-Channel-Settings-" + pos;

				int irange = settings.get<int>(sec+".Range");
				double range     = claws::RangeToVoltage( (claws::enPS6000Range) irange );

				// The -1 is needed because earlier the signals are truned from downwards
				// to upwards.
				double offset = -1*settings.get<double>(sec+".AnalogOffset")*1000.;
				PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channel);
				tmp->PrepareHistogram( range, offset );
		}

		state_ = EVENTSTATE_PREP;
		pt_.put("General.State", state_);

}

std::vector<std::vector<OverShootResult> > PhysicsEvent::OverShootCorrection()
{
		std::string names[14] = {"_LStart", "_LStop", "_LResult", "_Start", "_Stop", "_Result", "_Par0", "_Par1", "_Par2", "_Chi2", "_Ndf", +"_PVal", "_Area1", "_Area2"};

		std::vector<std::vector<OverShootResult> > allresults;

		for (auto &channel : channels_)
		{
				PhysicsChannel *phc = dynamic_cast<PhysicsChannel*>(channel);
				std::vector<OverShootResult> results = phc->OverShootCorrection();

				for(auto &result : results)
				{
						std::string chname = phc->GetName();
						pt_.put("OSFIT_" +names[0] +"." + chname + "_" + std::to_string(result.n), result.lstart );
						pt_.put("OSFIT_" +names[1] +"." + chname + "_" + std::to_string(result.n), result.lstop );
						pt_.put("OSFIT_" +names[3] +"." + chname + "_" + std::to_string(result.n), result.start );
						pt_.put("OSFIT_" +names[2] +"." + chname + "_" + std::to_string(result.n), result.lresult );
						pt_.put("OSFIT_" +names[4] +"." + chname + "_" + std::to_string(result.n), result.stop );
						pt_.put("OSFIT_" +names[5] +"." + chname + "_" + std::to_string(result.n), result.result );
						pt_.put("OSFIT_" +names[6] +"." + chname + "_" + std::to_string(result.n), result.par0 );
						pt_.put("OSFIT_" +names[7] +"." + chname + "_" + std::to_string(result.n), result.par1 );
						pt_.put("OSFIT_" +names[8] +"." + chname + "_" + std::to_string(result.n), result.par2 );
						pt_.put("OSFIT_" +names[9] +"." + chname + "_" + std::to_string(result.n), result.chi2 );
						pt_.put("OSFIT_" +names[10] +"." + chname + "_" + std::to_string(result.n), result.ndf );
						pt_.put("OSFIT_" +names[11] +"." + chname + "_" + std::to_string(result.n), result.pval );
						pt_.put("OSFIT_" +names[12] +"." + chname + "_" + std::to_string(result.n), result.area1 );
						pt_.put("OSFIT_" +names[13] +"." + chname + "_" + std::to_string(result.n), result.area2 );
				}

				allresults.push_back(results);

				// If only one channel fails, the whole event will be dumped
				if( channel->GetState() == CHANNELSTATE_FAILED  )
				{
						state_ = EVENTSTATE_OSFAILED;
						pt_.put("General.State", state_);
				}
		}

		if( state_ != EVENTSTATE_OSFAILED )
		{
				state_ = EVENTSTATE_OSCORRECTED;
				pt_.put("General.State", state_);
		}

		return allresults;
}


void PhysicsEvent::FastRate( Gain* gain )
{

		for(int i = 0; i< channels_.size(); ++i)
		{
				GainChannel * gch = gain->GetChannel(channels_.at(i)->GetName());

				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channels_.at(i));

				double fast_rate = pch->FastRate(gch->GetAvg(), unixtime_);

				pt_.put("FastRate." + pch->GetName(), fast_rate);

				//fast_rates_.at(i) = rate;
		}

}

void PhysicsEvent::PrepareTagging()
{
		for(auto & channel : channels_)
		{
				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channel);
				pch->PrepareTagging();
		}
};

void PhysicsEvent::SignalTagging()
{
//	std::string names[12] = {"_LStart", "_LStop", "_LResult", "_Start", "_Stop", "_Result", "_Par0", "_Par1", "_Par2", "_Chi2", "_Ndf", +"_PVal"};
		for(auto & channel : channels_)
		{
				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channel);
				pch->SignalTagging();

		}

		state_ = EVENTSTATE_TAGGED;
		pt_.put("General.State", state_);
};

void PhysicsEvent::PrepareDecomposition()
{
		for(auto & channel : channels_)
		{
				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channel);
				pch->PrepareDecomposition();
		}
};

void PhysicsEvent::WaveformDecomposition(Gain* gain)
{
		/**
		 * [for description]
		 * @param  i [description]
		 * \todo Verify that allways FWD1 is matched to FWD1 etc
		 */

		int nthreads   = GS->GetParameter<int>("General.nthreads");
		bool parallelize = GS->GetParameter<bool>("General.parallelize");

		#pragma omp parallel for if(parallelize) num_threads(nthreads)  firstprivate(gain)
		for( int i = 0; i < channels_.size(); ++i)
		{
				GainChannel * gch = gain->GetChannel(channels_.at(i)->GetName());
				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channels_.at(i));
				pch->WaveformDecomposition2(gch->GetAvg());
		}
		// for(auto & channel : channels_)
		// {
		//  GainChannel * gch = gain->GetChannel(channel->GetName());
		//
		//  PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channel);
		//  pch->WaveformDecomposition(gch->GetAvg());
		// }

		state_ = EVENTSTATE_WFDECOMPOSED;
		pt_.put("General.State", state_);
};

void PhysicsEvent::WaveformReconstruction(Gain* gain)
{
		/**
		 * [for description]
		 * @param  i [description]
		 * \todo Verify that allways FWD1 is matched to FWD1 etc
		 */
		std::string names[5] = {"NBins", "BinError", "Chi2", "PVal", "Chi2/Ndf"};

		for(int i = 0; i < channels_.size(); ++i)
		{
				GainChannel * gch = gain->GetChannel(channels_.at(i)->GetName());

				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channels_.at(i));

				std::vector<double> result = pch->WaveformReconstruction(gch->GetAvg());



				std::string chname = pch->GetName();

				for(unsigned int j = 0; j < 4; ++j)
				{
						pt_.put("WFReco_" + names[j] +"." + chname, result[j] );
				}
				pt_.put("WFReco_" + names[4] +"." + chname, result[2]/result[0] );

				reco_.emplace_back(result);
		}

		state_ = EVENTSTATE_WFRECONSTRUCTED;
		pt_.put("General.State", state_);
};

void PhysicsEvent::PrepareRetrieval()
{
		for(auto & channel : channels_)
		{
				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channel);
				pch->PrepareRetrieval();
		}
};

void PhysicsEvent::MipTimeRetrieval()
{
		/**
		 * [for description]
		 * @param  i [description]
		 * \todo Verify that allways FWD1 is matched to FWD1 etc
		 */

		for(int i = 0; i < channels_.size(); ++i)
		{

				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channels_.at(i));

				pch->MipTimeRetrieval(unixtime_);

				std::string chname = pch->GetName();

				Rate rate   = pch->GetRate();

				//rates_.at(i) = rate;

				pt_.put("Rate." + chname, rate.rate );
				pt_.put("RateSysErr." + chname, rate.staterr );
				pt_.put("RateStatErr." + chname, rate.syserr );
				pt_.put("RateErr." + chname, rate.err );

		}

		state_ = EVENTSTATE_CALIBRATED;
		pt_.put("General.State", state_);
};

bool PhysicsEvent::CheckInjection()
{
		// return true if injection, false else
		//	vector<int> maxbins;
		vector<double> maxima;

		for(auto& ch : channels_)
		{
				PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(ch);
				maxima.push_back(pch->GetHistogram("mip")->GetMaximum());
				//		maxbins.push_back(pch->mipwf_->GetMaximumBin());
		}

		// sort(maxbins.begin(), maxbins.end());
		sort(maxima.begin(), maxima.end());

		if(maxima.front()>7.)
		{
				cout<< "CheckInjection for evt " << nr_ << "found Injection despite INJECTION Flag." << endl;
				for(auto &max: maxima) cout<< "Max: " << max<< endl;
				return true;
		}

		else return false;
};

std::vector<std::vector<double> > PhysicsEvent::GetReconstruction()
{
		return reco_;
}



// vector<double> PhysicsEvent::GetOnlineRates()
// {
//  return online_rates_;
// }

// vector< vector<double> > PhysicsEvent::GetFastRates()
// {
//  // vector<vector<double>> rates;
//  // pt_.get
//  return fast_rates_;
// }

vector<Rate > PhysicsEvent::GetRates()
{
		// vector<double> init(3,-1);
		// vector<vector<double> > rates(6,init);
		//
		//
		//
		// boost::property_tree::ptree childpt = pt_.get_child("OnlineRate");
		//
		// int i = 0;
		//
		// for(auto iter = childpt.begin(); iter != childpt.end(); iter++)
		// {
		//      rates.at(i).at(0) = stod(iter->second.data());
		//      ++i;
		// }
		//
		// childpt = pt_.get_child("FastRate");
		//
		// i = 0;
		//
		// for(auto iter = childpt.begin(); iter != childpt.end(); iter++)
		// {
		//      rates.at(i).at(1) = stod(iter->second.data());
		//      ++i;
		// }
		//
		// childpt = pt_.get_child("Rate");
		//
		// i = 0;
		// for(auto iter = childpt.begin(); iter != childpt.end(); iter++)
		// {
		//      rates.at(i).at(2) = stod(iter->second.data());
		//      ++i;
		// }

		vector<Rate> rates;

		// boost::property_tree::ptree childpt = pt_.get_child("OnlineRate");
		// pt_.get<string>("SuperKEKBData.HERStatus");

		for(auto & ch : channels_)
		{
				PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(ch);
				Rate rate = tmp->GetRate();
				try
				{
						rate.online = pt_.get<double>("OnlineRate."+ch->GetName());
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						rate.online = -1;
				}

				try
				{
						rate.fast = pt_.get<double>("FastRate."+ch->GetName());
				}
				catch(const property_tree::ptree_bad_path &e)
				{
						rate.fast = -1;
				}


				rates.push_back(rate);
		}

		Rate bwd1;
		bwd1.name = "BWD1";
		try
		{
				bwd1.online = pt_.get<double>("OnlineRate.BWD1");
		}
		catch(const property_tree::ptree_bad_path &e)
		{
				bwd1.online = -1;
		}
		rates.push_back(bwd1);

		Rate bwd2;
		bwd2.name = "BWD2";
		try
		{
				bwd2.online = pt_.get<double>("OnlineRate.BWD2");
		}
		catch(const property_tree::ptree_bad_path &e)
		{
				bwd2.online = -1;
		}
		rates.push_back(bwd2);

		Rate bwd3;
		bwd3.name = "BWD3";

		try
		{
				bwd3.online = pt_.get<double>("OnlineRate.BWD3");
		}
		catch(const property_tree::ptree_bad_path &e)
		{
				bwd3.online = -1;
		}
		rates.push_back(bwd3);

		return rates;
};

property_tree::ptree PhysicsEvent::GetPT()
{

		// for(auto p : pt_)
		// {
		//      cout << p.first << endl;
		// }
		//
		// auto test = pt_.get<double>("Rate.FWD1");
		//  cout << test << endl;
		//
		return pt_;
};

// filesystem::path PhysicsEvent::GetPath()
// {
//      return path_;
// };

//----------------------------------------------------------------------------------------------
// Definition of the AnlysisEvent class.
//----------------------------------------------------------------------------------------------

AnalysisEvent::AnalysisEvent(string suffix) : n_(0), norm_(true), suffix_(suffix), first_run_(5000000), last_run_(-1)
{

		for (auto &name : GS->GetChannels("Physics"))
		{
				std::string position = name.second.get_value<std::string>();
				if( position != "false")
				{
						if( isdigit(position[0]) && isalpha(position[1]) )
						{

								double dt = GS->GetParameter<double>("Scope.delta_t");

								TH1F* tmphist = new TH1F(name.first.c_str(), name.first.c_str(), 1, -dt/2., dt/2.);
								tmphist->SetDirectory(0);
								tmphist->GetXaxis()->SetTitle("Time [ns]");
								tmphist->GetYaxis()->SetTitle("Particles/Event [1/0.8 ns]");

								AnalysisChannel* ch = new AnalysisChannel();
								ch->name = name.first.c_str();
								ch->wf = tmphist;

								string title = name.first + "_stat";
								ch->wf_stat = new TH1F(title.c_str(), title.c_str(), 1, -dt/2., dt/2.);
								ch->wf_stat->SetDirectory(0);
								ch->wf_stat->GetXaxis()->SetTitle("Time [ns]");
								ch->wf_stat->GetYaxis()->SetTitle("Particles/Event [1/0.8 ns]");

								title = name.first + "_sys";
								ch->wf_sys = new TH1F(title.c_str(), title.c_str(), 1, -dt/2., dt/2.);
								ch->wf_sys->SetDirectory(0);
								ch->wf_sys->GetXaxis()->SetTitle("Time [ns]");
								ch->wf_sys->GetYaxis()->SetTitle("Particles/Event [1/0.8 ns]");


								double npe = 0;

								try
								{
										npe = GS->GetParameter<double>("PEToMIP." + ch->name +"val2");
								}
								catch( const property_tree::ptree_bad_path &e )
								{
										npe = GS->GetParameter<double>("PEToMIP." + ch->name +"val");
								}

								int nmip = 100;

								int nbinsx   = (int)round(nmip*npe);

								double xlow  = +1./(2*npe);
								double xup   = nmip +1./(2*npe);

								title = ch->name + "_hit_map";

								int nbinsy   = 13000;
								//int multi    = 10;
								ch->hit_map = new TH2F(title.c_str(), title.c_str(), nbinsx, xlow, xup, nbinsy/10., -dt/2., nbinsy*dt -dt/2.);
								ch->hit_map->SetDirectory(0);
								ch->hit_map->GetXaxis()->SetTitle("Hit Energy [MIP]");
								ch->hit_map->GetYaxis()->SetTitle("Time in Turn [ns]");

								title = ch->name + "_time_in_turn";

								ch->time_in_turn = new TH2F(title.c_str(), title.c_str(), 30000, -dt/2., 3e6*dt -dt/2., nbinsy/100, -dt/2., nbinsy*dt -dt/2.);
								ch->time_in_turn->SetDirectory(0);
								ch->time_in_turn->GetXaxis()->SetTitle("Time [ns]");
								ch->time_in_turn->GetYaxis()->SetTitle("Time in Turn [ns]");

								title = ch->name + "_rate_in_turn";

								ch->rate_in_turn = new TH1F(title.c_str(), title.c_str(), nbinsy, -dt/2., nbinsy*dt -dt/2.);
								ch->rate_in_turn->SetDirectory(0);
								ch->rate_in_turn->GetXaxis()->SetTitle("Time in Turn [ns]");
								ch->rate_in_turn->GetYaxis()->SetTitle("Rate [MIP/0.8 ns x Length/10 us]");

								title = ch->name + "_hit_energy_spectrum";

								ch->hit_energy = new TH1F( title.c_str(), title.c_str(), nbinsx, xlow, xup );
								ch->hit_energy->SetDirectory(0);
								ch->hit_energy->SetXTitle("Hit Energy [MIP]");

								ch->hit_energy->SetYTitle("Entries per event [1/equivalent of one p.e. in MIP]");

								ch->hit_energy_sync = (TH1F*) ch->hit_energy->Clone((title+"_sync").c_str());
								ch->hit_energy_sync->SetDirectory(0);
								ch->hit_energy_mip = (TH1F*) ch->hit_energy->Clone((title+"_mip").c_str());
								ch->hit_energy_mip->SetDirectory(0);

								channels_.emplace_back( ch);
						}
				}
		}
};

AnalysisEvent::AnalysisEvent(PhysicsEvent* ph_evt, string suffix) : n_(0), norm_(true), suffix_(suffix), first_run_(5000000), last_run_(-1)
{
		pt_ = ph_evt->GetPT();
		// auto test = ph_evt->GetPT().get<double>("Rate.FWD1");
		// cout << test << endl;

		int phnr =  stoi(ph_evt->GetPath().filename().string().substr(4));
		n_ = stoi(ph_evt->GetPath("file").filename().string().substr(6,9));
		first_run_ = phnr;
		last_run_ = phnr;
};

AnalysisEvent::~AnalysisEvent()
{
		for(auto &ch: channels_)
		{
				delete ch;
				ch = nullptr;
		}
};

void AnalysisEvent::AddEvent(PhysicsEvent* ph_evt)
{
		// in nanoseconds
		double t_rev = GS->GetParameter<double>("SuperKEKB.rev_time");

		// in seconds
		double dt = GS->GetParameter<double>("Scope.delta_t");

		for(int i = 0; i < channels_.size(); ++i)
		{
				TH1F* ph_hist = dynamic_cast<TH1F*>(ph_evt->GetChannels().at(i)->GetHistogram("mip"));
				TH1F* ph_hist_stat = dynamic_cast<TH1F*>(ph_evt->GetChannels().at(i)->GetHistogram("mip_stat"));
				TH1F* ph_hist_sys = dynamic_cast<TH1F*>(ph_evt->GetChannels().at(i)->GetHistogram("mip_sys"));

				if(channels_.at(i)->wf->GetNbinsX() < ph_hist->GetNbinsX() )
				{
						if(channels_.at(i)->wf->GetNbinsX() == 1)
						{
								string entry = "MinWFLength."+channels_.at(i)->name;
								pt_.put(entry,ph_hist->GetNbinsX());
						}
						else
						{
								cout << "WARNING the number of bins of the waveform you are trying to add is BIGGER than the previous ones!!!" << endl;
								cout << "AnalysisEventWF: " << channels_.at(i)->wf->GetNbinsX() << ", PhysicsEvent: "<< ph_hist->GetNbinsX() <<", Nr: "<< ph_evt->GetNumber()<< endl;

								string entry = "MinWFLength."+channels_.at(i)->name;
								if(pt_.get<int>(entry) > channels_.at(i)->wf->GetNbinsX())
								{
										pt_.put(entry, channels_.at(i)->wf->GetNbinsX());
								}
						}

						double low1 = channels_.at(i)->wf->GetBinLowEdge(0);
						double low2 = ph_hist->GetBinLowEdge(0);

						if( fabs(channels_.at(i)->wf->GetBinLowEdge(0) - ph_hist->GetBinLowEdge(0)) > 1e-12)
						{
								assert(false);
						}

						int nbins = ph_hist->GetNbinsX();
						double low = channels_.at(i)->wf->GetBinLowEdge(1);
						double up = ph_hist->GetBinLowEdge(nbins)+ ph_hist->GetBinWidth(nbins);

						channels_.at(i)->wf->SetBins(nbins, low, up);
						channels_.at(i)->wf_stat->SetBins(nbins, low, up);
						channels_.at(i)->wf_sys->SetBins(nbins, low, up);

						// int 2d_nbinsx = channels_.at(i)->hit_map->GetNbinsX();
						// int 2d_xlow = channels_.at(i)->hit_map->GetBinLowEdge(1);
						// int 2d_xup = channels_.at(i)->hit_map->GetBinLowEdge(2d_nbinsx) + channels_.at(i)->hit_map->GetBinWidth(2d_nbinsx);
						// channels_.at(i)->SetBins(nbins, low, up, )
				}
				else if(channels_.at(i)->wf->GetNbinsX() > ph_hist->GetNbinsX() )
				{
						cout << "WARNING the number of bins of the waveform you are trying to add is SMALLER than the previous ones!!!" << endl;
						cout << "AnalysisEventWF: " << channels_.at(i)->wf->GetNbinsX() << ", PhysicsEvent: "<< ph_hist->GetNbinsX() <<", Nr: "<<ph_evt->GetNumber()<< endl;

						string entry = "MinWFLength."+channels_.at(i)->name;
						if(pt_.get<int>(entry) > ph_hist->GetNbinsX())
						{
								pt_.put(entry, ph_hist->GetNbinsX());
						}
				}

				int bin_first_signal = 0;

				for(int bin = 1; bin <= ph_hist->GetNbinsX(); ++bin )
				{
						if(ph_hist->GetBinContent(bin) > 0 )
						{
								bin_first_signal = bin;
								break;
						}
				}

				--bin_first_signal;

				// else if(channels_.at(i)->wf->GetNbinsX() < ph_hist->GetNbinsX() )
				// {
				//
				// }

				for(int j = 1; j<= ph_hist->GetNbinsX() - bin_first_signal; ++j)
				{

						channels_.at(i)->wf->SetBinContent(j, channels_.at(i)->wf->GetBinContent(j) + ph_hist->GetBinContent(j  +bin_first_signal) );

						channels_.at(i)->wf_stat->SetBinContent(j, channels_.at(i)->wf_stat->GetBinContent(j) + ph_hist->GetBinContent(j+bin_first_signal) );
						channels_.at(i)->wf_stat->SetBinError(j, channels_.at(i)->wf_stat->GetBinError(j) + ph_hist_stat->GetBinError(j+bin_first_signal)*ph_hist_stat->GetBinError(j+bin_first_signal) );

						channels_.at(i)->wf_sys->SetBinContent(j, channels_.at(i)->wf_sys->GetBinContent(j) + ph_hist->GetBinContent(j+bin_first_signal) );
						channels_.at(i)->wf_sys->SetBinError(j, channels_.at(i)->wf_sys->GetBinError(j) + ph_hist_sys->GetBinError(j+bin_first_signal) );

						// // in bins
						// int bin_in_turn = round(fmod(j,t_rev/0.8));
						//
						// channels_.at(i)->rate_in_turn->SetBinContent(bin_in_turn, channels_.at(i)->rate_in_turn->GetBinContent(bin_in_turn)+ph_hist_stat->GetBinContent(j));
						//
						// if(ph_hist_stat->GetBinContent(j) > 0)
						// {
						//      channels_.at(i)->hit_energy->Fill(ph_hist_stat->GetBinContent(j));
						//
						//      // in seconds
						//      double t_in_turn = fmod((j-1)*dt,t_rev*1e-9);
						//
						//      channels_.at(i)->hit_map->Fill(ph_hist_stat->GetBinContent(j), t_in_turn);
						//
						//      channels_.at(i)->time_in_turn->Fill((j-1)*dt, t_in_turn, ph_hist_stat->GetBinContent(j));
						//
						//      //channels_.at(i)->hit_map->Fill(5, 1e-6);
						// }

				}

				int min_nbins = ph_hist->GetNbinsX();

				string entry = "MinWFLength."+channels_.at(i)->name;
				if(pt_.get<int>(entry) != min_nbins)
				{
						min_nbins = pt_.get<int>(entry);
				}

				for(int j = 1; j<= min_nbins; ++j)
				{
						// in bins
						int bin_in_turn = round(fmod(j,t_rev/0.8));

						channels_.at(i)->rate_in_turn->SetBinContent(bin_in_turn, channels_.at(i)->rate_in_turn->GetBinContent(bin_in_turn)+ph_hist_stat->GetBinContent(j));

						if(ph_hist_stat->GetBinContent(j) > 0)
						{
								channels_.at(i)->hit_energy->Fill(ph_hist_stat->GetBinContent(j));

								// in seconds
								double t_in_turn = fmod((j-1)*dt,t_rev*1e-9);

								channels_.at(i)->hit_map->Fill(ph_hist_stat->GetBinContent(j), t_in_turn);

								channels_.at(i)->time_in_turn->Fill((j-1)*dt, t_in_turn, ph_hist_stat->GetBinContent(j));

								//channels_.at(i)->hit_map->Fill(5, 1e-6);
						}

				}



		}

		n_++;
		norm_ = false;

		int phnr =  stoi(ph_evt->GetPath().filename().string().substr(4));

		if(phnr < first_run_) first_run_ = phnr;
		else if (phnr > last_run_) last_run_ = phnr;

};

void AnalysisEvent::Normalize()
{
		for(auto &ch: channels_)
		{
				ch->wf->Scale(1./n_, "nosw2");
				ch->wf_stat->Scale(1./n_, "nosw2");
				ch->wf_sys->Scale(1./n_, "nosw2");

				double error_limits[6] = {1e5,0,1e5,0,1e5,0};

				for(int i = 1; i < ch->wf->GetNbinsX(); ++i)
				{
						// Stat error
						double stat_err = ch->wf_stat->GetBinError(i);
						stat_err = sqrt(stat_err)/n_;
						ch->wf_stat->SetBinError(i, stat_err );

						if(stat_err < error_limits[0] && !(fabs(error_limits[0])<1e-10) ) error_limits[0] = stat_err;
						if(stat_err > error_limits[1]) error_limits[1] = stat_err;

						// Sys error
						double sys_err = ch->wf_sys->GetBinError(i);
						sys_err = sys_err/n_;
						ch->wf_sys->SetBinError(i, sys_err );

						if(sys_err < error_limits[2] && !(fabs(error_limits[2])<1e-10) ) error_limits[2] = sys_err;
						if(sys_err > error_limits[3]) error_limits[3] = sys_err;

						// Total error
						double err = sqrt(pow(stat_err,2)+pow(sys_err,2));
						ch->wf->SetBinError(i, err );

						if(err < error_limits[4] && !(fabs(error_limits[4])<1e-10) ) error_limits[4] = err;
						if(err > error_limits[5]) error_limits[5] = err;
				}

				ch->hit_map->Scale(1./n_, "nosw2");
				ch->time_in_turn->Scale(1./n_, "nosw2");
				ch->rate_in_turn->Scale(1./n_, "nosw2");

				pt_.put(ch->name +".NEvents", n_);

				pt_.put(ch->name+".StatErrMin", error_limits[0]);
				pt_.put(ch->name+".StatErrMax", error_limits[1]);
				pt_.put(ch->name+".SysErrMin", error_limits[2]);
				pt_.put(ch->name+".SysErrMax", error_limits[3]);
				pt_.put(ch->name+".ErrMin", error_limits[4]);
				pt_.put(ch->name+".ErrMax", error_limits[5]);
		}

		for(auto &ch: channels_)
		{
				ch->hit_energy->Scale(1./n_);
				// ch->hit_energy_sync->Scale(1./n_);
				// ch->hit_energy_sync->Scale(1./n_);

				string name = ch->name + "_exp";
				TF1* expo = new TF1(name.c_str(),"exp([const] -[slope]*x)",-0.01,1.5);
				ch->hit_energy->Fit(expo,"S+");

				for(int i = 1; i<ch->hit_energy->GetNbinsX(); i++)
				{
						double x = ch->hit_energy->GetBinCenter(i);
						double yf = expo->Eval(x);
						double yh = ch->hit_energy->GetBinContent(i);
						ch->hit_energy_sync->SetBinContent(i,yf );
						ch->hit_energy_mip->SetBinContent(i,yh-yf );
				}
				delete expo;
		}

		norm_ = true;

};

void AnalysisEvent::RunPeak()
{

		for(int i = 0; i < channels_.size(); ++i)
		{
				if(channels_.at(i)->peak != nullptr )
				{
						delete channels_.at(i)->peak;
						channels_.at(i)->peak = nullptr;
				}

				string entry = "MinWFLength."+channels_.at(i)->name;
				int length = pt_.get<int>(entry);

				double scale = 1e-9;
				string title = channels_.at(i)->name + "_peak";
				//	int nbins = channels_.at(i)->wf->GetNbinsX();
				int nbins = length;
				double xlow = channels_.at(i)->wf->GetBinLowEdge(1)/scale;
				// double xup = (channels_.at(i)->wf->GetBinLowEdge(channels_.at(i)->wf->GetNbinsX()) + channels_.at(i)->wf->GetBinWidth(channels_.at(i)->wf->GetNbinsX()))/scale;
				double xup = (channels_.at(i)->wf->GetBinLowEdge(nbins) + channels_.at(i)->wf->GetBinWidth(nbins))/scale;

				channels_.at(i)->peak = new TH1F( title.c_str(), title.c_str(),nbins, xlow, xup);
				channels_.at(i)->peak->SetDirectory(0);
				channels_.at(i)->peak->SetXTitle("Peak to Peak Distance [ns]");
				channels_.at(i)->peak->SetYTitle("amp_{1} #times amp_{2} [MIPs^{2} / 0.8 ns]");

		}

		int nthreads   = GS->GetParameter<int>("General.nthreads");
		bool parallelize = GS->GetParameter<bool>("General.parallelize");

		#pragma omp parallel for if(parallelize) num_threads(nthreads)
		for(int i = 0; i < channels_.size(); ++i)
		{
				// if(channels_.at(i)->peak != nullptr )
				// {
				//      delete channels_.at(i)->peak;
				//      channels_.at(i)->peak = nullptr;
				// }
				//
				double scale = 1e-9;
				// string title = channels_.at(i)->name + "_peak";
				// int nbins = channels_.at(i)->wf->GetNbinsX();
				// double xlow = channels_.at(i)->wf->GetBinLowEdge(1)/scale;
				// double xup = (channels_.at(i)->wf->GetBinLowEdge(channels_.at(i)->wf->GetNbinsX()) + channels_.at(i)->wf->GetBinWidth(channels_.at(i)->wf->GetNbinsX()))/scale;
				//
				// channels_.at(i)->peak = new TH1F( title.c_str(), title.c_str(),nbins, xlow, xup);
				// channels_.at(i)->peak->SetDirectory(0);
				// channels_.at(i)->peak->SetXTitle("Peak to Peak Distance [ns]");
				// channels_.at(i)->peak->SetYTitle("amp_{1} #times amp_{2} [MIPs^{2} / 0.8 ns]");

				// if( channels_.at(i)->peak->GetNbinsX() < channels_.at(i)->wf->GetNbinsX() )
				// {
				//      channels_.at(i)->peak->SetBins(channels_.at(i)->wf->GetNbinsX(), channels_.at(i)->wf->GetBinLowEdge(1), channels_.at(i)->wf->GetBinLowEdge(channels_.at(i)->wf->GetNbinsX())+channels_.at(i)->wf->GetBinWidth(channels_.at(i)->wf->GetNbinsX()) );
				// }

				//	double amp = 0;

				// Iterate over bins j
				string entry = "MinWFLength."+channels_.at(i)->name;
				int length = pt_.get<int>(entry);

				//	for (int j = 1; j < channels_.at(i)->wf->GetNbinsX()+1; j++)
				for (int j = 1; j <= length; j++)
				{
						if ( channels_.at(i)->wf->GetBinContent(j) > 0)
						{
								//Iterate over following bins k
								for (int k = j+1; k <= length; k++)
								{
										if (channels_.at(i)->wf->GetBinContent(j) > 0)
										{
												channels_.at(i)->peak->Fill( (channels_.at(i)->wf->GetBinCenter(k) - channels_.at(i)->wf->GetBinCenter(j))/scale, ( channels_.at(i)->wf->GetBinContent(j) * channels_.at(i)->wf->GetBinContent(k) ) );
										}

								}
						}
				}
		}
}


void AnalysisEvent::RunFFT()
{
		//  int nthreads   = GS->GetParameter<int>("General.nthreads");
		//  bool parallelize = GS->GetParameter<bool>("General.parallelize");
		//
		// #pragma omp parallel for if(parallelize) num_threads(nthreads)

		double dt = 0.8e-9;

		//channels_.at(i)->n = (int)(length/10);
		//      else channels_.at(i)->n = (int)(length/10) - 1;
		//
		//      channels_.at(i)->n = channels_.at(i)->n +1;
		//
		//      long n = channels_.at(i)->n;
		//

		for(int i = 0; i < channels_.size(); ++i)
		{
				string entry = "MinWFLength."+channels_.at(i)->name;
				int length = pt_.get<int>(entry);
				double binwidth = 1./(2*dt)*2./length;

				if( length % 2 != 0 ) --length;

				if(channels_.at(i)->fft_real_h != nullptr)
				{
						delete channels_.at(i)->fft_real_h;
						channels_.at(i)->fft_real_h = nullptr;
				}

				string title = channels_.at(i)->name + "_fft_real_h";
				channels_.at(i)->fft_real_h = new TH1F( title.c_str(), title.c_str(), length/2. +1,  -binwidth/2., 1./(2*dt) +binwidth/2.);
				channels_.at(i)->fft_real_h->SetDirectory(0);
				channels_.at(i)->fft_real_h->SetXTitle("Frequency [Hz]");
				channels_.at(i)->fft_real_h->SetYTitle("Gute Frage...");

				if(channels_.at(i)->fft_img_h != nullptr)
				{
						delete channels_.at(i)->fft_img_h;
						channels_.at(i)->fft_img_h = nullptr;
				}

				title = channels_.at(i)->name + "_fft_img_h";
				channels_.at(i)->fft_img_h = new TH1F( title.c_str(), title.c_str(), length/2. +1,  -binwidth/2., 1./(2*dt) +binwidth/2. );
				channels_.at(i)->fft_img_h->SetDirectory(0);
				channels_.at(i)->fft_img_h->SetXTitle("Frequency [Hz]");
				channels_.at(i)->fft_img_h->SetYTitle("Gute Frage...");

				if(channels_.at(i)->fft_mag_h != nullptr)
				{
						delete channels_.at(i)->fft_mag_h;
						channels_.at(i)->fft_mag_h = nullptr;
				}

				title = channels_.at(i)->name + "_fft_mag_h";
				channels_.at(i)->fft_mag_h = new TH1F( title.c_str(), title.c_str(), length/2. +1,  -binwidth/2., 1./(2*dt) +binwidth/2. );
				channels_.at(i)->fft_mag_h->SetDirectory(0);
				channels_.at(i)->fft_mag_h->SetXTitle("Magnitude [Hz]");
				channels_.at(i)->fft_mag_h->SetYTitle("Gute Frage...");

				if(channels_.at(i)->fft_phase_h != nullptr)
				{
						delete channels_.at(i)->fft_phase_h;
						channels_.at(i)->fft_phase_h = nullptr;
				}

				title = channels_.at(i)->name + "_fft_phase_h";
				channels_.at(i)->fft_phase_h = new TH1F( title.c_str(), title.c_str(), length/2. +1,  -binwidth/2., 1./(2*dt) +binwidth/2. );
				channels_.at(i)->fft_phase_h->SetDirectory(0);
				channels_.at(i)->fft_phase_h->SetXTitle("Phaseshift [probably degree]");
				channels_.at(i)->fft_phase_h->SetYTitle("Gute Frage...");

				channels_.at(i)->n = length;
		}

		int nthreads   = GS->GetParameter<int>("General.nthreads");
		bool parallelize = GS->GetParameter<bool>("General.parallelize");

		#pragma omp parallel for if(parallelize) num_threads(nthreads)
		for(int i = 0; i < channels_.size(); ++i)
		{
				long n = channels_.at(i)->n;

				gsl_fft_real_wavetable *        real;
				gsl_fft_real_workspace *        work;

				double * data = new double[n];
				double * cpacked = new double[2*n];

				for (int j = 0; j < n; j++)
				{
						data[j] = channels_.at(i)->wf->GetBinContent(j+1);
				}

				work = gsl_fft_real_workspace_alloc ( n );
				real = gsl_fft_real_wavetable_alloc ( n );

				gsl_fft_real_transform(data, 1, n, real, work);

				gsl_fft_real_wavetable_free(real);

				gsl_fft_halfcomplex_unpack( data, cpacked, 1, n);


				for (int j = 0; j < n/2+1; j++)
				{
						channels_.at(i)->fft_real_h->SetBinContent(j+1,cpacked[2*j]);
				}

				for (int j = 0; j < n/2+1; j++)
				{
						channels_.at(i)->fft_img_h->SetBinContent(j+1,cpacked[2*j+1]);
				}

				delete work;
				delete data;
				delete cpacked;

				for (int j = 1; j <= channels_.at(i)->fft_real_h->GetNbinsX(); j++)
				{
						gsl_complex z = gsl_complex_rect(channels_.at(i)->fft_real_h->GetBinContent(j),channels_.at(i)->fft_img_h->GetBinContent(j));
						channels_.at(i)->fft_mag_h->SetBinContent(j,gsl_complex_abs(z));
						channels_.at(i)->fft_phase_h->SetBinContent(j,gsl_complex_arg(z));
				}
		}


		// for(int i = 0; i < channels_.size(); ++i)
		// {
		//      // Create the histogram holding the real fft spectrum
		//
		//      double timestep = 0.8e-9;
		//
		//      if(channels_.at(i)->fft_real_h != nullptr)
		//      {
		//              delete channels_.at(i)->fft_real_h;
		//              channels_.at(i)->fft_real_h = nullptr;
		//      }
		//
		//      string title = channels_.at(i)->name + "_fft_real_h";
		//      channels_.at(i)->fft_real_h = new TH1F( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
		//      channels_.at(i)->fft_real_h->SetDirectory(0);
		//      channels_.at(i)->fft_real_h->SetXTitle("Frequency [Hz]");
		//      channels_.at(i)->fft_real_h->SetYTitle("Gute Frage...");
		//      // Create the histogram holding the imaginary fft spectrum
		//      if(channels_.at(i)->fft_img_h != nullptr)
		//      {
		//              delete channels_.at(i)->fft_img_h;
		//              channels_.at(i)->fft_img_h = nullptr;
		//      }
		//
		//      title = channels_.at(i)->name + "_fft_img_h";
		//      channels_.at(i)->fft_img_h = new TH1F( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
		//      channels_.at(i)->fft_img_h->SetDirectory(0);
		//      channels_.at(i)->fft_img_h->SetXTitle("Frequency [Hz]");
		//      channels_.at(i)->fft_img_h->SetYTitle("Gute Frage...");
		//
		//      if(channels_.at(i)->fft_mag_h != nullptr)
		//      {
		//              delete channels_.at(i)->fft_mag_h;
		//              channels_.at(i)->fft_mag_h = nullptr;
		//      }
		//
		//      title = channels_.at(i)->name + "_fft_mag_h";
		//      channels_.at(i)->fft_mag_h = new TH1F( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
		//      channels_.at(i)->fft_mag_h->SetDirectory(0);
		//      channels_.at(i)->fft_mag_h->SetXTitle("Magnitude [Hz]");
		//      channels_.at(i)->fft_mag_h->SetYTitle("Gute Frage...");
		//
		//      // Create the histogram holding the phaseshift fft spectrum
		//      if(channels_.at(i)->fft_phase_h != nullptr)
		//      {
		//              delete channels_.at(i)->fft_phase_h;
		//              channels_.at(i)->fft_phase_h = nullptr;
		//      }
		//
		//      title = channels_.at(i)->name + "_fft_phase_h";
		//      channels_.at(i)->fft_phase_h = new TH1F( title.c_str(), title.c_str(), 2, -1/(2*timestep)/2, 1/(2*timestep)+ 1/(2*timestep)/2 );
		//      channels_.at(i)->fft_phase_h->SetDirectory(0);
		//      channels_.at(i)->fft_phase_h->SetXTitle("Phaseshift [probably degree]");
		//      channels_.at(i)->fft_phase_h->SetYTitle("Gute Frage...");
		//
		//      //	long n = 0;
		//
		//      // Iterate over bins j
		//      string entry = "MinWFLength."+channels_.at(i)->name;
		//      int length = pt_.get<int>(entry);
		//
		//      if( length % 2 == 0 ) channels_.at(i)->n = (int)(length/10);
		//      else channels_.at(i)->n = (int)(length/10) - 1;
		//
		//      channels_.at(i)->n = channels_.at(i)->n +1;
		//
		//      long n = channels_.at(i)->n;
		//
		//      if( channels_.at(i)->fft_real_h->GetNbinsX() < n/2+1 )
		//      {
		//              double xmin = channels_.at(i)->fft_real_h->GetBinCenter(1);
		//              double xmax = channels_.at(i)->fft_real_h->GetBinCenter(channels_.at(i)->fft_real_h->GetNbinsX());
		//              double range = xmax-xmin;
		//              channels_.at(i)->fft_real_h->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
		//      }
		//
		//      if( channels_.at(i)->fft_img_h->GetNbinsX() < n/2+1 )
		//      {
		//              double xmin = channels_.at(i)->fft_img_h->GetBinCenter(1);
		//              double xmax = channels_.at(i)->fft_img_h->GetBinCenter(channels_.at(i)->fft_img_h->GetNbinsX());
		//              double range = xmax-xmin;
		//              channels_.at(i)->fft_img_h->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
		//      }
		//
		//      if( channels_.at(i)->fft_mag_h->GetNbinsX() < n/2+1 )
		//      {
		//              double xmin = channels_.at(i)->fft_mag_h->GetBinCenter(1);
		//              double xmax = channels_.at(i)->fft_mag_h->GetBinCenter(channels_.at(i)->fft_mag_h->GetNbinsX());
		//              double range = xmax-xmin;
		//              channels_.at(i)->fft_mag_h->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
		//      }
		//
		//      if( channels_.at(i)->fft_phase_h->GetNbinsX() < n/2+1 )
		//      {
		//              double xmin = channels_.at(i)->fft_phase_h->GetBinCenter(1);
		//              double xmax = channels_.at(i)->fft_phase_h->GetBinCenter(channels_.at(i)->fft_phase_h->GetNbinsX());
		//              double range = xmax-xmin;
		//              channels_.at(i)->fft_phase_h->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );
		//      }
		// }
		//
		// int nthreads   = GS->GetParameter<int>("General.nthreads");
		// bool parallelize = GS->GetParameter<bool>("General.parallelize");
		// //
		// #pragma omp parallel for if(parallelize) num_threads(nthreads)
		// for(int i = 0; i < channels_.size(); ++i)
		// {
		//      long n = channels_.at(i)->n;
		//
		//      gsl_fft_real_wavetable *        real;
		//      gsl_fft_real_workspace *        work;
		//
		//      double * data = new double[n];
		//      double * cpacked = new double[2*n];
		//
		//      for (int j = 0; j < n; j++)
		//      {
		//              data[j] = channels_.at(i)->wf->GetBinContent(j+1);
		//      }
		//
		//      work = gsl_fft_real_workspace_alloc ( n );
		//      real = gsl_fft_real_wavetable_alloc ( n );
		//
		//      gsl_fft_real_transform(data, 1, n, real, work);
		//
		//      gsl_fft_real_wavetable_free(real);
		//
		//      gsl_fft_halfcomplex_unpack( data, cpacked, 1, n);
		//
		//
		//      for (int j = 0; j < n/2+1; j++)
		//      {
		//              channels_.at(i)->fft_real_h->SetBinContent(j+1,cpacked[2*j]);
		//      }
		//
		//      for (int j = 0; j < n/2+1; j++)
		//      {
		//              channels_.at(i)->fft_img_h->SetBinContent(j+1,cpacked[2*j+1]);
		//      }
		//
		//      delete work;
		//      delete data;
		//      delete cpacked;
		//
		//      for (int j = 1; j < channels_.at(i)->fft_real_h->GetNbinsX() + 1; j++)
		//      {
		//              gsl_complex z = gsl_complex_rect(channels_.at(i)->fft_real_h->GetBinContent(j),channels_.at(i)->fft_img_h->GetBinContent(j));
		//              channels_.at(i)->fft_mag_h->SetBinContent(j,gsl_complex_abs(z));
		//              channels_.at(i)->fft_phase_h->SetBinContent(j,gsl_complex_arg(z));
		//      }
		//
		//      // switch axis from Hx to ns
		//      //channels_.at(i)->fft_phase_h->SetBins( n/2+1, xmin - range/(2*(n/2+1) - 2), xmax + range/(2*(n/2+1) - 2) );

		//}
}

void AnalysisEvent::HitEnergySpectrum()
{
		int nthreads   = GS->GetParameter<int>("General.nthreads");
		bool parallelize = GS->GetParameter<bool>("General.parallelize");

		for(int i = 0; i < channels_.size(); ++i)
		{
				// Create the histogram holding the real fft spectrum

				double timestep = 0.8e-9;

				if(channels_.at(i)->hit_energy != nullptr)
				{
						delete channels_.at(i)->hit_energy;
						channels_.at(i)->hit_energy = nullptr;
				}

				string title = channels_.at(i)->name + "_hit_energy_spectrum";

				double npe = 0;

				try
				{
						npe = GS->GetParameter<double>("PEToMIP." + channels_.at(i)->name +"val2");
				}
				catch( const property_tree::ptree_bad_path &e )
				{
						npe = GS->GetParameter<double>("PEToMIP." + channels_.at(i)->name +"val");
				}

				int nmip = 5;

				int nbinsx   = (int)round(nmip*npe);

				double xlow  = +1./(2*npe);
				double xup   = nmip +1./(2*npe);

				channels_.at(i)->hit_energy = new TH1F( title.c_str(), title.c_str(), nbinsx, xlow, xup );
				channels_.at(i)->hit_energy->SetXTitle("Hit Energy [MIP]");

				channels_.at(i)->hit_energy->SetYTitle("Entries [1/xxx]");

				for(int j = 1; j < channels_.at(i)->wf->GetNbinsX(); j++)
				{
						if(channels_.at(i)->wf->GetBinContent(j) > 0)
						{

								channels_.at(i)->hit_energy->Fill(channels_.at(i)->wf->GetBinContent(j)*n_);
						}
				}
		}
};


void AnalysisEvent::SaveEvent(boost::filesystem::path dst, string prefix)
{

		filesystem::path output = dst;

		// if(first_run_ == last_run_) output /=  "Run-" + to_string(first_run_);
		// else output /= "Run-" + to_string(first_run_) + "-" + to_string(last_run_);
		//
		// if( !boost::filesystem::is_directory(output) )
		// {
		//      boost::filesystem::create_directory(output);
		// }

		output /=prefix+suffix_;

		if( !boost::filesystem::is_directory(output) )
		{
				boost::filesystem::create_directory(output);
		}

		std::string fname = output.string() + "/";

		int status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		if(first_run_ == last_run_) fname +=  "_run-" + to_string(first_run_);
		else fname += "_run-" + to_string(first_run_) + "_" + to_string(last_run_);

		fname += "_" + prefix+suffix_;
		// std::stringstream ss;
		//
		// int ndigits = GS->GetParameter<int>("General.event_ndigits");
		//
		// ss << std::setw(ndigits) << std::setfill('0') << nr_;

		// fname += "_" + ss.str();
		// fname += "_" + printEventState(state_);

		fname += ".root";

		TFile *rfile = new TFile(fname.c_str(), "RECREATE");

		for(auto &channel : channels_)
		{
				if(channel->wf) channel->wf->Write();
				if(channel->wf_stat) channel->wf_stat->Write();
				if(channel->wf_sys) channel->wf_sys->Write();
				if(channel->peak) channel->peak->Write();
				if(channel->fft_real_h) channel->fft_real_h->Write();
				if(channel->fft_img_h) channel->fft_img_h->Write();
				if(channel->fft_mag_h) channel->fft_mag_h->Write();
				if(channel->fft_phase_h) channel->fft_phase_h->Write();
				if(channel->hit_map) channel->hit_map->Write();
				if(channel->hit_energy) channel->hit_energy->Write();
				if(channel->hit_energy_sync) channel->hit_energy_sync->Write();
				if(channel->hit_energy_mip) channel->hit_energy_mip->Write();
				if(channel->time_in_turn) channel->time_in_turn->Write();
				if(channel->rate_in_turn) channel->rate_in_turn->Write();
		}

		rfile->Close("R");
		delete rfile;

		boost::replace_last(fname, "root", "ini");
		boost::property_tree::write_ini(fname.c_str(), pt_);

};

int AnalysisEvent::GetRunNr(string type)
{
		if(type == "first") return first_run_;
		else if(type == "last") return last_run_;
		else if(type == "nevent") return n_;
		else return -1;
};

// void PhysicsEvent::LoadIniFile(){
//
//
//      property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);
//
//      unixtime_       = pt_.get<double>("Properties.UnixTime");
//      lerbg_          = pt_.get<int>("SuperKEKBData.LERBg");
//      herbg_          = pt_.get<int>("SuperKEKBData.HERBg");
//      kekb_status_    = pt_.get<string>("SuperKEKBData.SuperKEKBStatus");
//      ler_status_     = pt_.get<string>("SuperKEKBData.LERSTatus");
//      her_status_     = pt_.get<string>("SuperKEKBData.HERStatus");
//
//      if (lerbg_ || herbg_ ) injection_ = true;
//      else injection_ = false;
//      if(kekb_status_ == "Vacuum Scrubbing"
//         && ler_status_ == "Vacuum Scrubbing"
//         && her_status_ == "Vacuum Scrubbing") scrubbing_ = 3;
//      else if(ler_status_ == "Vacuum Scrubbing") scrubbing_ = 1;
//      else if(her_status_ == "Vacuum Scrubbing") scrubbing_ = 2;
//      else scrubbing_ = 0;
//
//      //TODO load the rest that is written in the .ini file.
// };

//
// void Event::SubtractPedestal(std::map<std::string, float> ped, bool backup)
// {
//      /* Subtracts the pedestal from the individual waveforms. Input options:
//          1) If no parameters are specified, the pedestal from the indi -
//             vidual events will be used for the subtraction. Usually the
//             average run pedestal should be used, unstable pedestal over
//             time like with th pickup in intermediate events in phase one
//             this might lead to gains.
//          2) If the first input parameter is given (i.e. not an empty map
//             like default), it will be used for pedestal subtraction instead
//             of the event specific one.
//          3) If input pedestal is specified and backup == true, the individual
//             event pedestal is used, only if this equal to zero the run-wide
//             pedestal is used.
//       */
//      if( ped.empty() && !backup )
//      {
//              for(auto &ch : channels_)
//              {
//                      ch.second->SubtractPedestal();
//              }
//      }
//      else if(!ped.empty() && !backup)
//      {
//              for(auto &ch : channels_)
//              {
//                      ch.second->SubtractPedestal( ped[ch.first] );
//              }
//      }
//      else if(!ped.empty() && backup)
//      {
//              for(auto &ch : channels_)
//              {
//                      ch.second->SubtractPedestal( ped[ch.first], backup);
//              }
//      }
// }
//
// void Event::SetPedestal(std::map<std::string, float> ped, bool backup)
// {
//      /* Subtracts the pedestal from the individual waveforms. Input options:
//          1) If no parameters are specified, the pedestal from the indi -
//             vidual events will be used for the subtraction. Usually the
//             average run pedestal should be used, unstable pedestal over
//             time like with th pickup in intermediate events in phase one
//             this might lead to gains.
//          2) If the first input parameter is given (i.e. not an empty map
//             like default), it will be used for pedestal subtraction instead
//             of the event specific one.
//          3) If input pedestal is specified and backup == true, the individual
//             event pedestal is used, only if this equal to zero the run-wide
//             pedestal is used.
//       */
//      if( ped.empty() && !backup )
//      {
//              for(auto &ch : channels_)
//              {
//                      ch.second->SetPedestal();
//              }
//      }
//      else if(!ped.empty() && !backup)
//      {
//              for(auto &ch : channels_)
//              {
//                      ch.second->SetPedestal( ped[ch.first] );
//              }
//      }
//      else if(!ped.empty() && backup)
//      {
//              for(auto &ch : channels_)
//              {
//                      ch.second->SetPedestal( ped[ch.first], backup);
//              }
//      }
// }
//
// void Event::SetBaseline(map<std::string, float> baseline)
// {
//      for(auto & itr : channels_)
//      {
//              itr.second->SetBaseline( baseline[itr.first] );
//      }
// };


// void Event::CalculateIntegral()
// {
//      for(auto & itr : channels_)
//      {
//              itr.second->CalculateIntegral();
//      }
//
// };
//
// std::map<std::string, double> Event::GetIntegral()
// {
//      std::map<std::string, double>    rtn;
//
//      for(auto & itr : channels_)
//      {
//              rtn[itr.first]  =  itr.second->GetIntegral();
//      }
//
//      return rtn;
// };
//
// void Event::CreateHistograms()
// {
//      for(auto & itr : channels_)
//      {
//              itr.second->CreateHistogram();
//      }
// };
//
// std::map<std::string, TH1*> Event::GetHistograms()
// {
//      std::map<std::string, TH1*>    rtn;
//
//      for(auto & itr : channels_)
//      {
//              rtn[itr.first]  =  itr.second->GetHistogram();
//      }
//
//      return rtn;
// }
//
//
//
// std::map<std::string, std::vector<float>*> Event::GetWaveforms()
// {
//      std::map<std::string, std::vector<float>*>  rtn;
//      for(auto& mvec : channels_)
//      {
//              rtn[mvec.first] = mvec.second->GetWaveform();
//      }
//      return rtn;
// };
//


//
// void PhysicsEvent::LoadOnlineRate(){
//
//      std::ifstream ratefile(path_online_rate_.string());
//
//      if (ratefile)
//      {
//              double dummy;
//              ratefile >> online_rate_[0] >> online_rate_[1] >> online_rate_[2] >> dummy >> online_rate_[3] >> online_rate_[4] >> online_rate_[5] >> dummy;
//      }
//      else
//      {
//              cout << "\033[1;31mOnlinerate missing:   \033[0m" << nr_ << endl;
//              for(int i=0; i<6; i++)
//              {
//                      online_rate_[i] = 0;
//              }
//      }
//
//      ratefile.close();
//
//
//
//      for(unsigned i = 0; i < 4; i++)
//      {
//              if(!ends_with(GS->GetChannels(1).at(i), "4"))
//              {
//                      pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i]);
//              }
//      }
//      for(unsigned i = 4; i < 8; i++)
//      {
//              if(!ends_with(GS->GetChannels(1).at(i), "4"))
//              {
//                      pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i-1]);
//              }
//      }
//
//
// };
//
//
// void PhysicsEvent::SetUpWaveforms()
// {
//      /**
//       * [PhysicsEvent::SetUpWaveforms2 description]
//       */
//      for(auto& mvec : channels_)
//      {
//              if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
//              {
//                      PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
//                      tmp->SetUpWaveforms();
//              }
//              // std::string tmp_name = mvec.first;
//              // replace_last(tmp_name, "-INT", "");
//              //
//              // tmp->Decompose(mvec.second);
//              //        tmp->Decompose();
//      }
// };
//
// void PhysicsEvent::SetUpWaveformsV2()
// {
//      //TODO Validation
//      //    #pragma omp parallel for num_threads(5) firstprivate(avg_waveforms)
//      //
//      for(auto& mvec : channels_)
//      {
//              if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
//              {
//                      PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
//                      tmp->SetUpWaveformsV2();
//              }
//      }
// };
//
// void PhysicsEvent::DeleteWaveforms()
// {
//      for (const auto &mvec : channels_)
//      {
//              if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
//              {
//                      PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
//                      tmp->DeleteWaveform();
//              }
//      }
// };
//
// void PhysicsEvent::FastRate(std::map<std::string, std::vector<float>*> avg_waveforms, std::map<std::string, double> pe_to_mips)
// {
//      /**
//       * \todo Validation
//       */
//      for(auto& mvec : avg_waveforms)
//      {
//              std::string tmp_name = mvec.first;
//              replace_last(tmp_name, "-INT", "");
//              PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
//              tmp->FastRate(mvec.second, pe_to_mips[tmp_name]);
//              //        tmp->Decompose();
//              // double rate = tmp->GetRate();
//
//              if(tmp_name == "FWD1")
//              {
//                      fast_rate_[0] = tmp->GetRate();
//                      pt_.put("FastRate." + tmp_name, fast_rate_[0]);
//              }
//              else if(tmp_name == "FWD2")
//              {
//                      fast_rate_[1] = tmp->GetRate();
//                      pt_.put("FastRate." + tmp_name, fast_rate_[1]);
//              }
//              else if(tmp_name == "FWD3")
//              {
//                      fast_rate_[2] = tmp->GetRate();
//                      pt_.put("FastRate." + tmp_name, fast_rate_[2]);
//              }
//      }
// };
//
// void PhysicsEvent::Rate(std::map<std::string, double> pe_to_mips)
// {
//      /**
//       * \todo Validation
//       */
//      for(auto& mvec : pe_to_mips)
//      {
//              std::string tmp_name = mvec.first;
//              PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
//              tmp->Rate(pe_to_mips[tmp_name]);
//
//              if(tmp_name == "FWD1")
//              {
//                      rate_[0] = tmp->GetRate(2);
//                      pt_.put("Rate." + tmp_name, rate_[0]);
//              }
//              else if(tmp_name == "FWD2")
//              {
//                      rate_[1] = tmp->GetRate(2);
//                      pt_.put("Rate." + tmp_name, rate_[1]);
//              }
//              else if(tmp_name == "FWD3")
//              {
//                      rate_[2] = tmp->GetRate(2);
//                      pt_.put("Rate." + tmp_name, rate_[2]);
//              }
//      }
// };

//
//
// void PhysicsEvent::Reconstruct(std::map<std::string, std::vector<float>*> avg_waveforms)
// {
//      //TODO Implentation    // channels_["BWD1-INT"] = new IntChannel("BWD1");
//      for(auto& mvec : avg_waveforms)
//      {
//
//              // channels_["BWD2-INT"] = new IntChannel("BWD2");
//              // channels_["BWD3-INT"] = new IntChannel("BWD3");
//              std::string tmp_name = mvec.first;
//              replace_last(tmp_name, "-INT", "");
//              PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
//              tmp->Reconstruct(mvec.second);
//      }
// };
//
// void PhysicsEvent::CalculateChi2()
// {
//      //TODO Implentation
//      for(auto& mmap : channels_)
//      {
//              // std::string tmp_name = mvec.first;
//              // replace_last(tmp_name, "-INT", "");
//              if( !ends_with(mmap.first, "4"))
//              {
//                      PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mmap.second);
//                      tmp->CalculateChi2();
//              }
//      }
// };
//

//
//
//
//
// void PhysicsEvent::CreateHistograms(std::string type)
// {
//      for(auto & imap : channels_)
//      {
//              PhysicsChannel* p_ch = dynamic_cast<PhysicsChannel*>(imap.second);
//              p_ch->CreateHistogram(type);
//      }
// };
//
// std::map<std::string, TH1*> PhysicsEvent::GetHistograms(std::string type)
// {
//      std::map<std::string, TH1*>    rtn;
//
//      for(auto & imap : channels_)
//      {
//              PhysicsChannel* p_ch = dynamic_cast<PhysicsChannel*>(imap.second);
//              rtn[imap.first]  =  p_ch->GetHistogram(type);
//      }
//      return rtn;
// }
//
// double* PhysicsEvent::GetRate(int type){
//      if( type == 0 )
//      {
//              return online_rate_;
//      }
//      else if( type == 1 )
//      {
//              return fast_rate_;
//      }
//      else if( type == 2 )
//      {
//              return rate_;
//      }
//      else
//      {
//              return NULL;
//      }
// }
//

//
// int PhysicsEvent::GetScrubbing()  const
// {
//      return scrubbing_;
// }
//
// int PhysicsEvent::GetLerBg()       const
// {
//      return lerbg_;
// }
//
// int PhysicsEvent::GetHerBg()       const
// {
//      return herbg_;
// }
//
//
// // template<typename T>;
// // T PhysicsEvent::GetPV(std::string pv)
// // {
// //     return pt_.get<T>( "SuperKEKBData." + pv );
// // };
//
// // std::map<std::string, double> PhysicsEvent::GetIntegral()
// // {
// //     /*
// //     TODO implementation
// //     */
// //
// //     std::map<std::string, double>    rtn;
// //     return rtn;
// // }
//
// //----------------------------------------------------------------------------------------------
// // Definition of the IntEvent class derived from Event.
// //----------------------------------------------------------------------------------------------

//
// void IntEvent::LoadIniFile()
// {
//      property_tree::ptree pt_;
//      property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);
//
//      for(int i=1; i<5; i++)
//      {
//              try
//              {
//                      mean_online_[i-1] = pt_.get<double>("FWD."+std::to_string(i)+"-Mean");
//              }
//              catch(...)
//              {
//                      mean_online_[i-1] = 0;
//              }
//              try
//              {
//                      accepted_online_[i-1] = pt_.get<double>("FWD."+std::to_string(i)+"-Accepted");
//              }
//              catch(...)
//              {
//                      accepted_online_[i-1] = 0;
//              }
//      }
//      for(int i=1; i<5; i++)
//      {
//              try
//              {
//                      mean_online_[i+4-1] = pt_.get<double>("BWD."+std::to_string(i)+"-Mean");
//              }
//              catch(...)
//              {
//                      mean_online_[i+4-1] =0;
//              }
//              try
//              {
//                      accepted_online_[i+4-1] = pt_.get<double>("BWD."+std::to_string(i)+"-Accepted");
//              }
//              catch(...)
//              {
//                      accepted_online_[i+4-1] = 0;
//              }
//      }
// };
//
// // IntChannel* IntEvent::GetChannel(std::string name)
// // {
// //     return channels_[name];
// // }
//

//
//
// AnalysisEvent::AnalysisEvent()
// {
//      channels_["FWD1"] = new AnalysisChannel("FWD1");
//      channels_["FWD2"] = new AnalysisChannel("FWD2");
//      channels_["FWD3"] = new AnalysisChannel("FWD3");
//      channels_["FWD4"] = new AnalysisChannel("FWD4");
//
//      channels_["BWD1"] = new AnalysisChannel("BWD1");
//      channels_["BWD2"] = new AnalysisChannel("BWD2");
//      channels_["BWD3"] = new AnalysisChannel("BWD3");
//      channels_["BWD4"] = new AnalysisChannel("BWD4");
// };
//
//
// AnalysisEvent::AnalysisEvent( const boost::filesystem::path &file_root, const boost::filesystem::path &file_ini) : Event(file_root,file_ini)
// {
//      nr_str_ = file_root.filename().string().substr(6,9);
//      nr_     = atoi(nr_str_.c_str());
//
//      channels_["FWD1"] = new AnalysisChannel("FWD1");
//      channels_["FWD2"] = new AnalysisChannel("FWD2");
//      channels_["FWD3"] = new AnalysisChannel("FWD3");
//      channels_["FWD4"] = new AnalysisChannel("FWD4");
//
//      channels_["BWD1"] = new AnalysisChannel("BWD1");
//      channels_["BWD2"] = new AnalysisChannel("BWD2");
//      channels_["BWD3"] = new AnalysisChannel("BWD3");
//      channels_["BWD4"] = new AnalysisChannel("BWD4");
//
// };
//
// AnalysisEvent::~AnalysisEvent() {
// };
//
// void AnalysisEvent::LoadIniFile()
// {//
// Rate& AnalysisEvent::GetRates()
// {
//      return rates;
// }
//      property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);
//      unixtime_       = pt_.get<double>("Properties.UnixTime");
//
//      rates.rate_online[0] = pt_.get<double>("OnlineRate.FWD1");
//      rates.rate_online[1] = pt_.get<double>("OnlineRate.FWD2");
//      rates.rate_online[2] = pt_.get<double>("OnlineRate.FWD3");
//      rates.rate_online[3] = pt_.get<double>("OnlineRate.BWD1");
//      rates.rate_online[4] = pt_.get<double>("OnlineRate.BWD2");
//      rates.rate_online[5] = pt_.get<double>("OnlineRate.BWD3");
//
//      rates.rate_fast[0] = pt_.get<double>("FastRate.FWD1");
//      rates.rate_fast[1] = pt_.get<double>("FastRate.FWD2");
//      rates.rate_fast[2] = pt_.get<double>("FastRate.FWD3");
//
//      rates.decomposition[0] = pt_.get<double>("DecompositionResults.FWD1");
//      rates.decomposition[1] = pt_.get<double>("DecompositionResults.FWD2");
//      rates.decomposition[2] = pt_.get<double>("DecompositionResults.FWD3");
//
//      rates.rate[0] = pt_.get<double>("Rate.FWD1");
//      rates.rate[1] = pt_.get<double>("Rate.FWD2");
//      rates.rate[2] = pt_.get<double>("Rate.FWD3");
//
// };
//
// std::tuple<double, double> AnalysisEvent::GetCurrent()
// {
//      return std::make_tuple( pt_.get<double>("SuperKEKBData.LERCurrent"),
//                              pt_.get<double>("SuperKEKBData.HERCurrent")   );
// };
//
// void AnalysisEvent::AddEvent(AnalysisEvent* evt)
// {
//      std::vector<std::string> vch = GS->GetChannels(1);
//
// //    #pragma omp parallel for num_threads(8)
//      for(unsigned i = 0; i < vch.size(); i++)
//      {
//              TH1* hist_local = dynamic_cast<AnalysisChannel*>(evt->GetChannel(vch.at(i)))->GetHistogram();
//              TH1* hist_global = dynamic_cast<AnalysisChannel*>(channels_[vch.at(i)])->GetHistogram();
//
//              if( hist_global->GetNbinsX() < hist_local->GetNbinsX() )
//              {
//                      hist_global->SetBins(hist_local->GetNbinsX(), hist_local->GetBinLowEdge(1), hist_local->GetBinLowEdge(hist_local->GetNbinsX())+hist_local->GetBinWidth(hist_local->GetNbinsX()));
//              }
//              hist_global->Add(hist_local);
//      }
//
//      n_evts_++;
// };
//
// void AnalysisEvent::Normalize()
// {
//      for(auto & imap : channels_)
//      {
//              AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(imap.second);
//              tmp->Normalize(1./n_evts_);
//      }
// };
//
// void AnalysisEvent::SetErrors(double err)
// {
//      for(auto & imap : channels_)
//      {
//              AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(imap.second);
//              tmp->SetErrors(err);
//      }
// }
//
// void AnalysisEvent::RunPeak()
// {
//      std::cout << "\033[33;1mAnalysisEvent::RunPeak:\033[0m running" << "\r" << std::flush;
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//      std::vector<std::string> ch_names;
//
//      for(auto & imap : channels_)
//      {
//              if(!ends_with(imap.first, "4") ) ch_names.push_back(imap.first);
//      }
//
//   #pragma omp parallel for
//      for(int i = 0; i< ch_names.size(); i++)
//      {
//              AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(channels_[ch_names.at(i)]);
//              tmp->RunPeak();
//      }
//      // for(auto & imap : channels_)
//      // {
//      //     AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(imap.second);
//      //     tmp->RunPeak();
//      // }
//
//      std::cout << "\033[32;1mAnalysisEvent::RunPeak:\033[0m done!   " << "\r" << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// };
//
// void AnalysisEvent::RunFFT()
// {
//      std::cout << "\033[33;1mAnalysisEvent::RunFFT:\033[0m running" << "\r" << std::flush;
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//
//      std::vector<std::string> ch_names;
//
//      for(auto & imap : channels_)
//      {
//              if(!ends_with(imap.first, "4") ) ch_names.push_back(imap.first);
//      }
//
// //    #pragma omp parallel for
//      for(int i = 0; i< ch_names.size(); i++)
//      {
//              AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(channels_[ch_names.at(i)]);
//              tmp->RunFFT();
//      }
//
//      std::cout << "\033[32;1mAnalysisEvent::RunFFT:\033[0m done!   " << "\r" << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
//
// };
//
// std::tuple<bool, double, bool, double> AnalysisEvent::GetInjection()
// {
//      return std::make_tuple( pt_.get<bool>("SuperKEKBData.LERBg"),
//                              pt_.get<double>("SuperKEKBData.LERInj"),
//                              pt_.get<bool>("SuperKEKBData.HERBg"),
//                              pt_.get<double>("SuperKEKBData.HERInj")  );
// }
//
//
// std::map<std::string, TH1*> AnalysisEvent::GetHistograms()
// {
//      std::map<std::string, TH1*>    rtn;
//
//      for(auto & itr : channels_)
//      {
//              AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(itr.second);
//              /**
//               *  \todo When here not parameter is givin, somehow the type parameter does not go to default waveform but peak...
//               */
//              rtn[itr.first]  =  tmp->GetHistogram("waveform");
//      }
//
//      for(auto & itr : channels_)
//      {
//              if(boost::algorithm::ends_with(itr.first,"4")) continue;
//
//              AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(itr.second);
//              rtn[itr.first + "_peak"]        =  tmp->GetHistogram("peak");
//              rtn[itr.first + "_fft_real"]    =  tmp->GetHistogram("fft_real");
//              rtn[itr.first + "_fft_img"]     =  tmp->GetHistogram("fft_img");
//              rtn[itr.first + "_fft_mag"]     =  tmp->GetHistogram("fft_mag");
//              rtn[itr.first + "_fft_phase"]   =  tmp->GetHistogram("fft_phase");
//      }
//
//      // for(auto & itr : channels_)
//      // {
//      //     if(boost::algorithm::ends_with(itr.first,"4")) continue;
//      //
//      //     AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(itr.second);
//      //     rtn[itr.first + "_fft_real"]  =  tmp->GetHistogram("fft_real");
//      // }
//
//      return rtn;
// }
//
// Rate& AnalysisEvent::GetRates()
// {
//      return rates;
// }
//
// void AnalysisEvent::SaveEvent(boost::filesystem::path folder)
// {
//      if(!boost::filesystem::is_directory( folder.parent_path() ))
//      {
//              boost::filesystem::create_directory( folder.parent_path() );
//      }
//
// //    std::string fname = folder.string()+"/event_"+std::to_string(nr_)+"_" + type +"_v"+ std::to_string(GS->GetCaliPar<int>("General.CalibrationVersion")) + ".root";
//      TFile *rfile = new TFile((folder.string()+".root").c_str(), "RECREATE");
//
//      for(auto imap : this->GetHistograms())
//      {
//              imap.second->Write();
//      }
//
//      rfile->Close();
//      delete rfile;
// };
