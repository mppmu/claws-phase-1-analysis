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
// --- BOOST includes ---
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/replace.hpp>

// --- OpenMP includes ---
// #include <omp.h>
// --- ROOT includes ---

// --- Project includes ---
#include "event.hh"
#include "globalsettings.hh"


using namespace std;
using namespace boost;

//----------------------------------------------------------------------------------------------
// Definition of the Event base class.
//----------------------------------------------------------------------------------------------



Event::Event()
{

};

Event::Event(boost::filesystem::path file, boost::filesystem::path ini_file ): path_(file.parent_path().parent_path()), file_(file), ini_file_(ini_file), state_(EVENTSTATE_INIT)
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

    	int     status;
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
	std::string pdnames[10] = {"PS_Status","PS_FitConstant","PS_FitMean","PS_FitSigma","PS_FitChi2","PS_FitNDF","PS_FitPVal","PS_HistMean","PS_HistError","PS_HistEntries"};

	// int nthreads   = GS->GetParameter<int>("General.nthreads");
	// bool parallelize = GS->GetParameter<bool>("General.parallelize");
	//
	// #pragma omp parallel for if(parallelize) num_threads(nthreads)
	// for( int i = 0; i < channels_.size(); ++i )
	// {
	// 	// First fill and fit the pedestal
	// 	channels_.at(i)->FillPedestal();
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

		for(int i = 0; i< 10; i++)
		{
			pt_.put(pdnames[i] + "." + name, pd[i]);
		}

		if( channel->GetState() == CHANNELSTATE_FAILED  )
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
	    for(int i = 0 ; i < pd.size(); i++ )
		{
			channels_.at(i)->SubtractPedestal(pd.at(i));
		}
		// for(auto &channel : channels_)
		// {
		// 	channels_.at(i)->SubtractPedestal(pd.at(i));
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
		// 	channel->SubtractPedestal();
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

//----------------------------------------------------------------------------------------------
// Definition of the CalibrationEvent class derived from Event.
//----------------------------------------------------------------------------------------------

CalibrationEvent::CalibrationEvent(boost::filesystem::path file, boost::filesystem::path ini_file, double unixtime ) : Event(file, ini_file), runnr_(-1)
{
		nr_       = std::atoi(file.filename().string().substr(14,3).c_str());
		runnr_ 	  = std::atoi(file.filename().string().substr(4,6).c_str());

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

	int     status;
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
	// 	this->LoadHistograms(boost::filesystem::path(fname));
	// }
	// else
	// {
	// 	boost::replace_last(fname, printEventState(EVENTSTATE_PDSUBTRACTED),printEventState(EVENTSTATE_PDFAILED));
	// 	if( boost::filesystem::exists(fname) )
	// 	{
	// 		this->LoadHistograms(boost::filesystem::path(fname));
	// 	}
	// 	else
	// 	{
	// 		return;
	// 	}
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

    	int     status;
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
			// 	string name = channel->GetHistogram()->GetName();
			// 	boost::replace_first(name, "-", "_");
			// 	channel->GetHistogram("pedestal")->Write(name.c_str());
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
	 *TODO description of PhysicsEvent class.
	 */

	// fill_n(online_rate_, 6, -1);
	// fill_n(fast_rate_, 3, -1);
	// fill_n(rate_, 3, -1);

	// nr_str_ = file_root.filename().string().substr(6,9);
	// Get the path to the file from the online monitor

	string tmp = file.filename().string();
  	boost::replace_first(tmp, "Event-","");
  	boost::replace_last(tmp, ".root", "");

	/** Now the string only contains a number. The first 6 digits
	* 	digits are the runnumber. Since for the muon runs 4 digit
	*   and maybe even more event numbers are used, just take the
	*   last 15 to be sure
	*/
	//nr_     = atoi( tmp.substr(6,15).c_str());
	nr_     = stol( tmp );
	// if(file.filename().string().size() == 20 )
	// {
	// 	nr_     = atoi(file.filename().string().substr(6,9).c_str());
	// }
	// else if(file.filename().string().size() == 21)
	// {
	// 	nr_     = atoi(file.filename().string().substr(6,10).c_str());
	// }
	// else
	// {
	// 	assert(0);
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
		// 		for(unsigned i = 0; i < 4; i++)
		// 		{
		// 				if(!ends_with(GS->GetChannels(1).at(i), "4"))
		// 				{
		// 						pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i]);
		// 				}
		// 		}
		// 		for(unsigned i = 4; i < 8; i++)
		// 		{
		// 				if(!ends_with(GS->GetChannels(1).at(i), "4"))
		// 				{
		// 						pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i-1]);
		// 				}
		// 		}

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

	int     status;
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
			// 	boost::replace_last(fname, "ini","root");
			// 	this->LoadHistograms(boost::filesystem::path(fname));
			// }

			// if( boost::filesystem::exists(fname) )
			// {
			//  	this->LoadHistograms(boost::filesystem::path(fname));
			// }
			// else
			// {
			// 	boost::replace_last(fname, printEventState(EVENTSTATE_PDSUBTRACTED),printEventState(EVENTSTATE_PDFAILED));
			// 	if( boost::filesystem::exists(fname) )
			// 	{
			// 		this->LoadHistograms(boost::filesystem::path(fname));
			// 	}
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
			// 	boost::replace_last(fname, "ini","root");
			// 	this->LoadHistograms(boost::filesystem::path(fname));
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
			// 	boost::replace_last(fname, "ini","root");
			// 	this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco load"}));
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
			// 	boost::replace_last(fname, "ini","root");
			// 	this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"wf", "reco recreate", "pe"}));
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
			// 	boost::replace_last(fname, "ini","root");
			// 	this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe"}) );
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
			// 	boost::replace_last(fname, "ini","root");
			// 	this->LoadHistograms(boost::filesystem::path(fname), vector<string>({"pe", "mip"}) );
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

    	int     status;
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
		std::string sec   = "Scope-" + scope + "-Channel-Settings-" + pos ;

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

std::vector<std::vector<OverShootResult>> PhysicsEvent::OverShootCorrection()
{
	std::string names[14] = {"_LStart", "_LStop", "_LResult", "_Start", "_Stop", "_Result", "_Par0", "_Par1", "_Par2", "_Chi2", "_Ndf", +"_PVal", "_Area1", "_Area2"};

	std::vector<std::vector<OverShootResult>> allresults;

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

		 // int nthreads   = GS->GetParameter<int>("General.nthreads");
		 // bool parallelize = GS->GetParameter<bool>("General.parallelize");
		 //
		 // #pragma omp parallel for if(parallelize) num_threads(nthreads)

		for(auto & channel : channels_)
		{
			GainChannel * gch = gain->GetChannel(channel->GetName());

			PhysicsChannel *pch = dynamic_cast<PhysicsChannel*>(channel);
			pch->WaveformDecomposition(gch->GetAvg());
		}

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

		std::string    chname = pch->GetName();

		double rate   = pch->GetRate();

		//rates_.at(i) = rate;

		pt_.put("Rate." + chname, rate );

	}

	state_ = EVENTSTATE_CALIBRATED;
	pt_.put("General.State", state_);
};

std::vector<std::vector<double>> PhysicsEvent::GetReconstruction()
{
	return reco_;
}



// vector<double> PhysicsEvent::GetOnlineRates()
// {
// 	return online_rates_;
// }

// vector< vector<double> > PhysicsEvent::GetFastRates()
// {
// 	// vector<vector<double>> rates;
// 	// pt_.get
// 	return fast_rates_;
// }

vector<vector<double>> PhysicsEvent::GetRates()
{
	vector<double> init(3,-1);
	vector<vector<double>> rates(6,init);

	boost::property_tree::ptree childpt = pt_.get_child("OnlineRate");

	int i = 0;

	for(auto iter = childpt.begin(); iter != childpt.end(); iter++)
	{
		rates.at(i).at(0) = stod(iter->second.data());
		++i;
	}

	childpt = pt_.get_child("FastRate");

	i = 0;

	for(auto iter = childpt.begin(); iter != childpt.end(); iter++)
	{
		rates.at(i).at(1) = stod(iter->second.data());
		++i;
	}

	childpt = pt_.get_child("Rate");

	i = 0;
	for(auto iter = childpt.begin(); iter != childpt.end(); iter++)
	{
		rates.at(i).at(2) = stod(iter->second.data());
		++i;
	}

	return rates;
}
// void PhysicsEvent::LoadIniFile(){
//
//
// 		property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);
//
// 		unixtime_       = pt_.get<double>("Properties.UnixTime");
// 		lerbg_          = pt_.get<int>("SuperKEKBData.LERBg");
// 		herbg_          = pt_.get<int>("SuperKEKBData.HERBg");
// 		kekb_status_    = pt_.get<string>("SuperKEKBData.SuperKEKBStatus");
// 		ler_status_     = pt_.get<string>("SuperKEKBData.LERSTatus");
// 		her_status_     = pt_.get<string>("SuperKEKBData.HERStatus");
//
// 		if (lerbg_ || herbg_ ) injection_ = true;
// 		else injection_ = false;
// 		if(kekb_status_ == "Vacuum Scrubbing"
// 		   && ler_status_ == "Vacuum Scrubbing"
// 		   && her_status_ == "Vacuum Scrubbing") scrubbing_ = 3;
// 		else if(ler_status_ == "Vacuum Scrubbing") scrubbing_ = 1;
// 		else if(her_status_ == "Vacuum Scrubbing") scrubbing_ = 2;
// 		else scrubbing_ = 0;
//
// 		//TODO load the rest that is written in the .ini file.
// };

//
// void Event::SubtractPedestal(std::map<std::string, float> ped, bool backup)
// {
// 		/* Subtracts the pedestal from the individual waveforms. Input options:
// 		    1) If no parameters are specified, the pedestal from the indi -
// 		       vidual events will be used for the subtraction. Usually the
// 		       average run pedestal should be used, unstable pedestal over
// 		       time like with th pickup in intermediate events in phase one
// 		       this might lead to gains.
// 		    2) If the first input parameter is given (i.e. not an empty map
// 		       like default), it will be used for pedestal subtraction instead
// 		       of the event specific one.
// 		    3) If input pedestal is specified and backup == true, the individual
// 		       event pedestal is used, only if this equal to zero the run-wide
// 		       pedestal is used.
// 		 */
// 		if( ped.empty() && !backup )
// 		{
// 				for(auto &ch : channels_)
// 				{
// 						ch.second->SubtractPedestal();
// 				}
// 		}
// 		else if(!ped.empty() && !backup)
// 		{
// 				for(auto &ch : channels_)
// 				{
// 						ch.second->SubtractPedestal( ped[ch.first] );
// 				}
// 		}
// 		else if(!ped.empty() && backup)
// 		{
// 				for(auto &ch : channels_)
// 				{
// 						ch.second->SubtractPedestal( ped[ch.first], backup);
// 				}
// 		}
// }
//
// void Event::SetPedestal(std::map<std::string, float> ped, bool backup)
// {
// 		/* Subtracts the pedestal from the individual waveforms. Input options:
// 		    1) If no parameters are specified, the pedestal from the indi -
// 		       vidual events will be used for the subtraction. Usually the
// 		       average run pedestal should be used, unstable pedestal over
// 		       time like with th pickup in intermediate events in phase one
// 		       this might lead to gains.
// 		    2) If the first input parameter is given (i.e. not an empty map
// 		       like default), it will be used for pedestal subtraction instead
// 		       of the event specific one.
// 		    3) If input pedestal is specified and backup == true, the individual
// 		       event pedestal is used, only if this equal to zero the run-wide
// 		       pedestal is used.
// 		 */
// 		if( ped.empty() && !backup )
// 		{
// 				for(auto &ch : channels_)
// 				{
// 						ch.second->SetPedestal();
// 				}
// 		}
// 		else if(!ped.empty() && !backup)
// 		{
// 				for(auto &ch : channels_)
// 				{
// 						ch.second->SetPedestal( ped[ch.first] );
// 				}
// 		}
// 		else if(!ped.empty() && backup)
// 		{
// 				for(auto &ch : channels_)
// 				{
// 						ch.second->SetPedestal( ped[ch.first], backup);
// 				}
// 		}
// }
//
// void Event::SetBaseline(map<std::string, float> baseline)
// {
// 		for(auto & itr : channels_)
// 		{
// 				itr.second->SetBaseline( baseline[itr.first] );
// 		}
// };


// void Event::CalculateIntegral()
// {
// 		for(auto & itr : channels_)
// 		{
// 				itr.second->CalculateIntegral();
// 		}
//
// };
//
// std::map<std::string, double> Event::GetIntegral()
// {
// 		std::map<std::string, double>    rtn;
//
// 		for(auto & itr : channels_)
// 		{
// 				rtn[itr.first]  =  itr.second->GetIntegral();
// 		}
//
// 		return rtn;
// };
//
// void Event::CreateHistograms()
// {
// 		for(auto & itr : channels_)
// 		{
// 				itr.second->CreateHistogram();
// 		}
// };
//
// std::map<std::string, TH1*> Event::GetHistograms()
// {
// 		std::map<std::string, TH1*>    rtn;
//
// 		for(auto & itr : channels_)
// 		{
// 				rtn[itr.first]  =  itr.second->GetHistogram();
// 		}
//
// 		return rtn;
// }
//
//
//
// std::map<std::string, std::vector<float>*> Event::GetWaveforms()
// {
// 		std::map<std::string, std::vector<float>*>  rtn;
// 		for(auto& mvec : channels_)
// 		{
// 				rtn[mvec.first] = mvec.second->GetWaveform();
// 		}
// 		return rtn;
// };
//


//
// void PhysicsEvent::LoadOnlineRate(){
//
// 		std::ifstream ratefile(path_online_rate_.string());
//
// 		if (ratefile)
// 		{
// 				double dummy;
// 				ratefile >> online_rate_[0] >> online_rate_[1] >> online_rate_[2] >> dummy >> online_rate_[3] >> online_rate_[4] >> online_rate_[5] >> dummy;
// 		}
// 		else
// 		{
// 				cout << "\033[1;31mOnlinerate missing:   \033[0m" << nr_ << endl;
// 				for(int i=0; i<6; i++)
// 				{
// 						online_rate_[i] = 0;
// 				}
// 		}
//
// 		ratefile.close();
//
//
//
// 		for(unsigned i = 0; i < 4; i++)
// 		{
// 				if(!ends_with(GS->GetChannels(1).at(i), "4"))
// 				{
// 						pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i]);
// 				}
// 		}
// 		for(unsigned i = 4; i < 8; i++)
// 		{
// 				if(!ends_with(GS->GetChannels(1).at(i), "4"))
// 				{
// 						pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i-1]);
// 				}
// 		}
//
//
// };
//
//
// void PhysicsEvent::SetUpWaveforms()
// {
// 		/**
// 		 * [PhysicsEvent::SetUpWaveforms2 description]
// 		 */
// 		for(auto& mvec : channels_)
// 		{
// 				if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
// 				{
// 						PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
// 						tmp->SetUpWaveforms();
// 				}
// 				// std::string tmp_name = mvec.first;
// 				// replace_last(tmp_name, "-INT", "");
// 				//
// 				// tmp->Decompose(mvec.second);
// 				//        tmp->Decompose();
// 		}
// };
//
// void PhysicsEvent::SetUpWaveformsV2()
// {
// 		//TODO Validation
// 		//    #pragma omp parallel for num_threads(5) firstprivate(avg_waveforms)
// 		//
// 		for(auto& mvec : channels_)
// 		{
// 				if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
// 				{
// 						PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
// 						tmp->SetUpWaveformsV2();
// 				}
// 		}
// };
//
// void PhysicsEvent::DeleteWaveforms()
// {
// 		for (const auto &mvec : channels_)
// 		{
// 				if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
// 				{
// 						PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
// 						tmp->DeleteWaveform();
// 				}
// 		}
// };
//
// void PhysicsEvent::FastRate(std::map<std::string, std::vector<float>*> avg_waveforms, std::map<std::string, double> pe_to_mips)
// {
// 		/**
// 		 * \todo Validation
// 		 */
// 		for(auto& mvec : avg_waveforms)
// 		{
// 				std::string tmp_name = mvec.first;
// 				replace_last(tmp_name, "-INT", "");
// 				PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
// 				tmp->FastRate(mvec.second, pe_to_mips[tmp_name]);
// 				//        tmp->Decompose();
// 				// double rate = tmp->GetRate();
//
// 				if(tmp_name == "FWD1")
// 				{
// 						fast_rate_[0] = tmp->GetRate();
// 						pt_.put("FastRate." + tmp_name, fast_rate_[0]);
// 				}
// 				else if(tmp_name == "FWD2")
// 				{
// 						fast_rate_[1] = tmp->GetRate();
// 						pt_.put("FastRate." + tmp_name, fast_rate_[1]);
// 				}
// 				else if(tmp_name == "FWD3")
// 				{
// 						fast_rate_[2] = tmp->GetRate();
// 						pt_.put("FastRate." + tmp_name, fast_rate_[2]);
// 				}
// 		}
// };
//
// void PhysicsEvent::Rate(std::map<std::string, double> pe_to_mips)
// {
// 		/**
// 		 * \todo Validation
// 		 */
// 		for(auto& mvec : pe_to_mips)
// 		{
// 				std::string tmp_name = mvec.first;
// 				PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
// 				tmp->Rate(pe_to_mips[tmp_name]);
//
// 				if(tmp_name == "FWD1")
// 				{
// 						rate_[0] = tmp->GetRate(2);
// 						pt_.put("Rate." + tmp_name, rate_[0]);
// 				}
// 				else if(tmp_name == "FWD2")
// 				{
// 						rate_[1] = tmp->GetRate(2);
// 						pt_.put("Rate." + tmp_name, rate_[1]);
// 				}
// 				else if(tmp_name == "FWD3")
// 				{
// 						rate_[2] = tmp->GetRate(2);
// 						pt_.put("Rate." + tmp_name, rate_[2]);
// 				}
// 		}
// };

//
//
// void PhysicsEvent::Reconstruct(std::map<std::string, std::vector<float>*> avg_waveforms)
// {
// 		//TODO Implentation    // channels_["BWD1-INT"] = new IntChannel("BWD1");
// 		for(auto& mvec : avg_waveforms)
// 		{
//
// 				// channels_["BWD2-INT"] = new IntChannel("BWD2");
// 				// channels_["BWD3-INT"] = new IntChannel("BWD3");
// 				std::string tmp_name = mvec.first;
// 				replace_last(tmp_name, "-INT", "");
// 				PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
// 				tmp->Reconstruct(mvec.second);
// 		}
// };
//
// void PhysicsEvent::CalculateChi2()
// {
// 		//TODO Implentation
// 		for(auto& mmap : channels_)
// 		{
// 				// std::string tmp_name = mvec.first;
// 				// replace_last(tmp_name, "-INT", "");
// 				if( !ends_with(mmap.first, "4"))
// 				{
// 						PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mmap.second);
// 						tmp->CalculateChi2();
// 				}
// 		}
// };
//

//
//
//
//
// void PhysicsEvent::CreateHistograms(std::string type)
// {
// 		for(auto & imap : channels_)
// 		{
// 				PhysicsChannel* p_ch = dynamic_cast<PhysicsChannel*>(imap.second);
// 				p_ch->CreateHistogram(type);
// 		}
// };
//
// std::map<std::string, TH1*> PhysicsEvent::GetHistograms(std::string type)
// {
// 		std::map<std::string, TH1*>    rtn;
//
// 		for(auto & imap : channels_)
// 		{
// 				PhysicsChannel* p_ch = dynamic_cast<PhysicsChannel*>(imap.second);
// 				rtn[imap.first]  =  p_ch->GetHistogram(type);
// 		}
// 		return rtn;
// }
//
// double* PhysicsEvent::GetRate(int type){
// 		if( type == 0 )
// 		{
// 				return online_rate_;
// 		}
// 		else if( type == 1 )
// 		{
// 				return fast_rate_;
// 		}
// 		else if( type == 2 )
// 		{
// 				return rate_;
// 		}
// 		else
// 		{
// 				return NULL;
// 		}
// }
//
// bool PhysicsEvent::GetInjection()  const
// {
// 		return injection_;
// }
//
// int PhysicsEvent::GetScrubbing()  const
// {
// 		return scrubbing_;
// }
//
// int PhysicsEvent::GetLerBg()       const
// {
// 		return lerbg_;
// }
//
// int PhysicsEvent::GetHerBg()       const
// {
// 		return herbg_;
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
// 		property_tree::ptree pt_;
// 		property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);
//
// 		for(int i=1; i<5; i++)
// 		{
// 				try
// 				{
// 						mean_online_[i-1] = pt_.get<double>("FWD."+std::to_string(i)+"-Mean");
// 				}
// 				catch(...)
// 				{
// 						mean_online_[i-1] = 0;
// 				}
// 				try
// 				{
// 						accepted_online_[i-1] = pt_.get<double>("FWD."+std::to_string(i)+"-Accepted");
// 				}
// 				catch(...)
// 				{
// 						accepted_online_[i-1] = 0;
// 				}
// 		}
// 		for(int i=1; i<5; i++)
// 		{
// 				try
// 				{
// 						mean_online_[i+4-1] = pt_.get<double>("BWD."+std::to_string(i)+"-Mean");
// 				}
// 				catch(...)
// 				{
// 						mean_online_[i+4-1] =0;
// 				}
// 				try
// 				{
// 						accepted_online_[i+4-1] = pt_.get<double>("BWD."+std::to_string(i)+"-Accepted");
// 				}
// 				catch(...)
// 				{
// 						accepted_online_[i+4-1] = 0;
// 				}
// 		}
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
// 		channels_["FWD1"] = new AnalysisChannel("FWD1");
// 		channels_["FWD2"] = new AnalysisChannel("FWD2");
// 		channels_["FWD3"] = new AnalysisChannel("FWD3");
// 		channels_["FWD4"] = new AnalysisChannel("FWD4");
//
// 		channels_["BWD1"] = new AnalysisChannel("BWD1");
// 		channels_["BWD2"] = new AnalysisChannel("BWD2");
// 		channels_["BWD3"] = new AnalysisChannel("BWD3");
// 		channels_["BWD4"] = new AnalysisChannel("BWD4");
// };
//
//
// AnalysisEvent::AnalysisEvent( const boost::filesystem::path &file_root, const boost::filesystem::path &file_ini) : Event(file_root,file_ini)
// {
// 		nr_str_ = file_root.filename().string().substr(6,9);
// 		nr_     = atoi(nr_str_.c_str());
//
// 		channels_["FWD1"] = new AnalysisChannel("FWD1");
// 		channels_["FWD2"] = new AnalysisChannel("FWD2");
// 		channels_["FWD3"] = new AnalysisChannel("FWD3");
// 		channels_["FWD4"] = new AnalysisChannel("FWD4");
//
// 		channels_["BWD1"] = new AnalysisChannel("BWD1");
// 		channels_["BWD2"] = new AnalysisChannel("BWD2");
// 		channels_["BWD3"] = new AnalysisChannel("BWD3");
// 		channels_["BWD4"] = new AnalysisChannel("BWD4");
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
// 		return rates;
// }
// 		property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);
// 		unixtime_       = pt_.get<double>("Properties.UnixTime");
//
// 		rates.rate_online[0] = pt_.get<double>("OnlineRate.FWD1");
// 		rates.rate_online[1] = pt_.get<double>("OnlineRate.FWD2");
// 		rates.rate_online[2] = pt_.get<double>("OnlineRate.FWD3");
// 		rates.rate_online[3] = pt_.get<double>("OnlineRate.BWD1");
// 		rates.rate_online[4] = pt_.get<double>("OnlineRate.BWD2");
// 		rates.rate_online[5] = pt_.get<double>("OnlineRate.BWD3");
//
// 		rates.rate_fast[0] = pt_.get<double>("FastRate.FWD1");
// 		rates.rate_fast[1] = pt_.get<double>("FastRate.FWD2");
// 		rates.rate_fast[2] = pt_.get<double>("FastRate.FWD3");
//
// 		rates.decomposition[0] = pt_.get<double>("DecompositionResults.FWD1");
// 		rates.decomposition[1] = pt_.get<double>("DecompositionResults.FWD2");
// 		rates.decomposition[2] = pt_.get<double>("DecompositionResults.FWD3");
//
// 		rates.rate[0] = pt_.get<double>("Rate.FWD1");
// 		rates.rate[1] = pt_.get<double>("Rate.FWD2");
// 		rates.rate[2] = pt_.get<double>("Rate.FWD3");
//
// };
//
// std::tuple<double, double> AnalysisEvent::GetCurrent()
// {
// 		return std::make_tuple( pt_.get<double>("SuperKEKBData.LERCurrent"),
// 		                        pt_.get<double>("SuperKEKBData.HERCurrent")   );
// };
//
// void AnalysisEvent::AddEvent(AnalysisEvent* evt)
// {
// 		std::vector<std::string> vch = GS->GetChannels(1);
//
// //    #pragma omp parallel for num_threads(8)
// 		for(unsigned i = 0; i < vch.size(); i++)
// 		{
// 				TH1* hist_local = dynamic_cast<AnalysisChannel*>(evt->GetChannel(vch.at(i)))->GetHistogram();
// 				TH1* hist_global = dynamic_cast<AnalysisChannel*>(channels_[vch.at(i)])->GetHistogram();
//
// 				if( hist_global->GetNbinsX() < hist_local->GetNbinsX() )
// 				{
// 						hist_global->SetBins(hist_local->GetNbinsX(), hist_local->GetBinLowEdge(1), hist_local->GetBinLowEdge(hist_local->GetNbinsX())+hist_local->GetBinWidth(hist_local->GetNbinsX()));
// 				}
// 				hist_global->Add(hist_local);
// 		}
//
// 		n_evts_++;
// };
//
// void AnalysisEvent::Normalize()
// {
// 		for(auto & imap : channels_)
// 		{
// 				AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(imap.second);
// 				tmp->Normalize(1./n_evts_);
// 		}
// };
//
// void AnalysisEvent::SetErrors(double err)
// {
// 		for(auto & imap : channels_)
// 		{
// 				AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(imap.second);
// 				tmp->SetErrors(err);
// 		}
// }
//
// void AnalysisEvent::RunPeak()
// {
// 		std::cout << "\033[33;1mAnalysisEvent::RunPeak:\033[0m running" << "\r" << std::flush;
// 		double wall0 = claws::get_wall_time();
// 		double cpu0  = claws::get_cpu_time();
//
// 		std::vector<std::string> ch_names;
//
// 		for(auto & imap : channels_)
// 		{
// 				if(!ends_with(imap.first, "4") ) ch_names.push_back(imap.first);
// 		}
//
//   #pragma omp parallel for
// 		for(int i = 0; i< ch_names.size(); i++)
// 		{
// 				AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(channels_[ch_names.at(i)]);
// 				tmp->RunPeak();
// 		}
// 		// for(auto & imap : channels_)
// 		// {
// 		//     AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(imap.second);
// 		//     tmp->RunPeak();
// 		// }
//
// 		std::cout << "\033[32;1mAnalysisEvent::RunPeak:\033[0m done!   " << "\r" << std::endl;
//
// 		double wall1 = claws::get_wall_time();
// 		double cpu1  = claws::get_cpu_time();
//
// 		cout << "Wall Time = " << wall1 - wall0 << endl;
// 		cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// };
//
// void AnalysisEvent::RunFFT()
// {
// 		std::cout << "\033[33;1mAnalysisEvent::RunFFT:\033[0m running" << "\r" << std::flush;
// 		double wall0 = claws::get_wall_time();
// 		double cpu0  = claws::get_cpu_time();
//
//
// 		std::vector<std::string> ch_names;
//
// 		for(auto & imap : channels_)
// 		{
// 				if(!ends_with(imap.first, "4") ) ch_names.push_back(imap.first);
// 		}
//
// //    #pragma omp parallel for
// 		for(int i = 0; i< ch_names.size(); i++)
// 		{
// 				AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(channels_[ch_names.at(i)]);
// 				tmp->RunFFT();
// 		}
//
// 		std::cout << "\033[32;1mAnalysisEvent::RunFFT:\033[0m done!   " << "\r" << std::endl;
//
// 		double wall1 = claws::get_wall_time();
// 		double cpu1  = claws::get_cpu_time();
//
// 		cout << "Wall Time = " << wall1 - wall0 << endl;
// 		cout << "CPU Time  = " << cpu1  - cpu0  << endl;
//
// };
//
// std::tuple<bool, double, bool, double> AnalysisEvent::GetInjection()
// {
// 		return std::make_tuple( pt_.get<bool>("SuperKEKBData.LERBg"),
// 		                        pt_.get<double>("SuperKEKBData.LERInj"),
// 		                        pt_.get<bool>("SuperKEKBData.HERBg"),
// 		                        pt_.get<double>("SuperKEKBData.HERInj")  );
// }
//
//
// std::map<std::string, TH1*> AnalysisEvent::GetHistograms()
// {
// 		std::map<std::string, TH1*>    rtn;
//
// 		for(auto & itr : channels_)
// 		{
// 				AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(itr.second);
// 				/**
// 				 *  \todo When here not parameter is givin, somehow the type parameter does not go to default waveform but peak...
// 				 */
// 				rtn[itr.first]  =  tmp->GetHistogram("waveform");
// 		}
//
// 		for(auto & itr : channels_)
// 		{
// 				if(boost::algorithm::ends_with(itr.first,"4")) continue;
//
// 				AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(itr.second);
// 				rtn[itr.first + "_peak"]        =  tmp->GetHistogram("peak");
// 				rtn[itr.first + "_fft_real"]    =  tmp->GetHistogram("fft_real");
// 				rtn[itr.first + "_fft_img"]     =  tmp->GetHistogram("fft_img");
// 				rtn[itr.first + "_fft_mag"]     =  tmp->GetHistogram("fft_mag");
// 				rtn[itr.first + "_fft_phase"]   =  tmp->GetHistogram("fft_phase");
// 		}
//
// 		// for(auto & itr : channels_)
// 		// {
// 		//     if(boost::algorithm::ends_with(itr.first,"4")) continue;
// 		//
// 		//     AnalysisChannel* tmp = dynamic_cast<AnalysisChannel*>(itr.second);
// 		//     rtn[itr.first + "_fft_real"]  =  tmp->GetHistogram("fft_real");
// 		// }
//
// 		return rtn;
// }
//
// Rate& AnalysisEvent::GetRates()
// {
// 		return rates;
// }
//
// void AnalysisEvent::SaveEvent(boost::filesystem::path folder)
// {
// 		if(!boost::filesystem::is_directory( folder.parent_path() ))
// 		{
// 				boost::filesystem::create_directory( folder.parent_path() );
// 		}
//
// //    std::string fname = folder.string()+"/event_"+std::to_string(nr_)+"_" + type +"_v"+ std::to_string(GS->GetCaliPar<int>("General.CalibrationVersion")) + ".root";
// 		TFile *rfile = new TFile((folder.string()+".root").c_str(), "RECREATE");
//
// 		for(auto imap : this->GetHistograms())
// 		{
// 				imap.second->Write();
// 		}
//
// 		rfile->Close();
// 		delete rfile;
// };
