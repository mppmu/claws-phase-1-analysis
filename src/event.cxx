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
#include <cxxabi.h>

// --- BOOST includes ---
// #include <boost/algorithm/string/replace.hpp>
// --- OpenMP includes ---
// #include <omp.h>
// --- ROOT includes ---

// --- Project includes ---
#include "event.hh"
#include "globalsettings.hh"


// using namespace std;
// using namespace boost;

//----------------------------------------------------------------------------------------------
// Definition of the Event base class.
//----------------------------------------------------------------------------------------------


Event::Event()
{

};

Event::Event(boost::filesystem::path file, boost::filesystem::path ini_file ): path_(file.parent_path().parent_path()), file_(file), ini_file_(ini_file), state_(EVENTSTATE_INIT)
{

};

Event::~Event() {
		// TODO Auto-generated destructor stub
		// for(auto& m : channels_)
		// {
		// 		delete m.second;
		// }
}

void Event::LoadHistograms(EventState state)
{
		switch (state) {
			case EVENTSTATE_RAW:
			{
				this->LoadRaw();
				state_ = EVENTSTATE_RAW;
				break;
			}
		    case EVENTSTATE_PDSUBTRACTED:
			{
				this->LoadSubtracted();
				state_ = EVENTSTATE_PDSUBTRACTED;
				break;
			}
		}
}

void Event::LoadRaw()
{
    TFile *rfile=NULL;
	rfile = new TFile(file_.string().c_str(), "open");

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

void Event::LoadSubtracted()
{
	std::string fname = (path_/boost::filesystem::path("Calibration")/boost::filesystem::path("PDS_Calibration")/boost::filesystem::path("Waveforms")).string() + "/";

	int     status;
	char   *realname;
	const std::type_info  &ti = typeid(*this);
	realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
	fname += std::string(realname);
	free(realname);

	std::stringstream ss;
	ss << std::setw(3) << std::setfill('0') << nr_;
	fname += "_" + ss.str();
	fname += "_" + printEventState(EVENTSTATE_PDSUBTRACTED);
	fname += ".root";

	TFile *rfile = rfile = new TFile(fname.c_str(), "open");

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

void Event::PrepHistograms()
{
		for (auto channel : channels_)
		{
		    channel->PrepHistogram();
		}

    state_ = EVENTSTATE_PREP;

}

void Event::SaveEvent(boost::filesystem::path dst, bool save_pd)
{
		/**
		*  \todo Kill the path paramter and make it state dependet!
		*/

		std::string fname = dst.string() + "/";
	//	fname += std::string(typeid(*this).name());

    	int     status;
		char   *realname;
		const std::type_info  &ti = typeid(*this);
		realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
		fname += std::string(realname);
		free(realname);

		// /fname += "_" + std::to_string(nr_);
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

		//fname = (dst/boost::filesystem::path( std::string(typeid(*this).name()) + "_" +std::to_string(nr_) + printEventState(state_) + ".ini")).string();
		//boost::property_tree::write_ini(fname.c_str(), pt_);

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
};

void Event::FillPedestals()
{
		for (auto channel : channels_)
		{
				channel->FillPedestal();
		}
};

void Event::SubtractPedestals(std::vector<double> pd)
{
		if(pd.size() == channels_.size())
		{
		    for(int i = 0 ; i < pd.size(); i++ ) channels_.at(i)->SubtractPedestal(pd.at(i));
		}
		else
		{
			for(auto channel : channels_) channel->SubtractPedestal();
		}

		state_ = EVENTSTATE_PDSUBTRACTED;
}

double Event::GetTime()
{
		return unixtime_;
}

int Event::GetNumber()
{
		return nr_;
}

std::vector<Channel*> Event::GetChannels()
{
 		return channels_;
};

//----------------------------------------------------------------------------------------------
// Definition of the CalibrationEvent class derived from Event.
//----------------------------------------------------------------------------------------------

CalibrationEvent::CalibrationEvent(boost::filesystem::path file, boost::filesystem::path ini_file ) : Event(file, ini_file)
{
		nr_ = std::atoi(file.filename().string().substr(14,3).c_str());
		//nr_ += std::atoi(file.filename().string().substr(4,6).c_str()) * 1000;

		// nr_     = atoi(nr_str_.c_str());
		// GS->GetChannels("Calibration");
		for (auto name : GS->GetChannels("Calibration"))
		{
			 if( name.second.get_value<std::string>() == "true") channels_.emplace_back( new CalibrationChannel(name.first) );
		}

};

CalibrationEvent::~CalibrationEvent() {
		// TODO Auto-generated destructor stub
};


//----------------------------------------------------------------------------------------------
// Definition of the PhysicsEvent class derived from Event.
//----------------------------------------------------------------------------------------------

PhysicsEvent::PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file) : Event(file, ini_file)
{
		// fill_n(online_rate_, 6, -1);
		// fill_n(fast_rate_, 3, -1);
		// fill_n(rate_, 3, -1);

		// nr_str_ = file_root.filename().string().substr(6,9);
		nr_     = atoi(file.filename().string().substr(6,9).c_str());

		/*
		    TODO Implement a dynamic creation of channels getting the list list and therefore number of channels from somewhere else.
		 */
		// channels_["FWD1"] = new PhysicsChannel("FWD1");
		// channels_["FWD2"] = new PhysicsChannel("FWD2");
		// channels_["FWD3"] = new PhysicsChannel("FWD3");
		// channels_["FWD4"] = new PhysicsChannel("FWD4");
		//
		// channels_["BWD1"] = new PhysicsChannel("BWD1");
		// channels_["BWD2"] = new PhysicsChannel("BWD2");
		// channels_["BWD3"] = new PhysicsChannel("BWD3");
		// channels_["BWD4"] = new PhysicsChannel("BWD4");
};

PhysicsEvent::PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file, boost::filesystem::path rate_file) : PhysicsEvent(file, ini_file)
{
		rate_file_ = rate_file;
};


PhysicsEvent::~PhysicsEvent() {
		// TODO Auto-generated destructor stub

};



//
// void Event::LoadWaveform()
// {
// 		// for (auto &mmap : channels_)
// 		// {
// 		//     std::cout<< mmap.first << std::endl;
// 		// }
//
// 		// std::vector<std::string> vch = GS->GetChannels(1);
// 		std::vector<std::string> chs;
// 		std::pair<std::string, Channel*> pair;
//
// 		BOOST_FOREACH(pair, channels_)
// 		{
// 				chs.push_back(pair.first);
// 		}
//
// //  #pragma omp parallel for num_threads(8)
// 		for(unsigned i = 0; i < chs.size(); i++)
// 		{
// 				channels_[chs.at(i)]->LoadWaveform();
// 		}
//
// };
//

//
// void Event::DeleteWaveforms()
// {
//
// 		for (const auto &itr : channels_)
// 		{
// 				itr.second->DeleteWaveform();
// 		}
//
// };
//

//
// // void Event::SubtractPedestal()
// // {
// //     /* See "void Channel::Subtract()" for more information
// //     */
// //     for(auto &ch : channels_)
// //     {
// //         ch.second->Subtract();
// //     }
// // };
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
//
//
// int Event::GetNr()     const
// {
// 		return nr_;
// }
//
// std::string Event::GetNrStr()     const
// {
// 		return nr_str_;
// }
//
// int Event::getCh(string ch){
//
// 		return 0;
// }
//
// // void Event::Draw(){
// //
// //     cout << "Drawing PhysicsEvent: "<< nr_ << endl;
// //
// //     TCanvas * c = new TCanvas(to_string(nr_).c_str(),to_string(nr_).c_str(), 1600, 1200);
// //     c->Divide(2, channels_.size()/2);
// //     unsigned int pad=0;
// //
// //     for(auto i : channels_)
// //     {
// //         pad+=+2;
// //         if(pad > channels_.size()) pad =1;
// //         c->cd(pad);
// //         i.second->GetWaveformHist()->Draw();
// //     }
// //
// //     string ped_can_name = to_string(nr_) + " Pedestal";
// //     TCanvas * c_ped = new TCanvas(ped_can_name.c_str(), ped_can_name.c_str(), 1600, 1200);
// //     c_ped->Divide(2, channels_.size()/2);
// //     pad=0;
// //     for(auto i : channels_)
// //     {
// //         pad+=+2;
// //         if(pad > channels_.size() ) pad =1;
// //         c_ped->cd(pad);
// //         i.second->GetPedestal()->Draw();
// //     }
// //
// //
// // }
//
// std::map<std::string, TH1I*> Event::GetPedestal()
// {
// 		/*
// 		    TODO description
// 		 */
// 		std::map<std::string, TH1I*> rtn;
// 		for(auto & itr : channels_)
// 		{
// 				rtn[itr.first]  =  itr.second->GetPedestal();
// 		}
//
// 		return rtn;
// };
//
// Channel* Event::GetChannel(std::string name)
// {
// 		return channels_[name];
// }
//

//
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
// // void PhysicsEvent::LoadRootFile()
// // {
// //
// //     // TFile and TTree classes are not thread save, therefore, you have to look them.
// //     #pragma omp critical
// //     {
// //         file = new TFile(path_file_root_.string().c_str(), "open");
// //
// //         if(file->IsZombie())
// //         {
// //             cout << "Error openning file" << endl;
// //             exit(-1);
// //         }
// //     }
// //
// //     for (auto &itr : channels_)
// //
// //     #pragma omp critical
// //     {
// //         file->Close("R");
// //     }
// //
// //     delete file;
// //     file = NULL;
// //     // cout << "Listing gDirectory in PhysicsEvent::LoadRootFile() after closing the file!" << endl;
// //     // gDirectory->ls();
// // }
//
//
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
// void PhysicsEvent::Decompose(std::map<std::string, std::vector<float>*> avg_waveforms)
// {
// 		/**
// 		 * [for description]
// 		 * @param  i [description]
// 		 * \todo
// 		 */
// 		// std::vector<std::string> keys;
// 		//
// 		// for(auto& mvec : avg_waveforms)
// 		// {
// 		//     keys.push_back(mvec.first);
// 		// }
// 		//
// 		// for(auto& mvec : avg_waveforms)
// 		// {
// 		//     std::string tmp_name = mvec.first;
// 		//     replace_last(tmp_name,Results." + tmp_name, chi2);
// 		// }
//
// 		std::vector<std::string> vch = GS->GetChannels(1);
//
// 	#pragma omp parallel for num_threads(8) firstprivate(avg_waveforms)
// 		for(unsigned i = 0; i < vch.size(); i++)
// 		{
//
// 				std::string ch_name = vch.at(i);
//
// //        #pragma omp critical
// //        std::cout << "Thread id: " << int(omp_get_thread_num()) << ", in channel: " << ch_name << std::endl;
//
// 				if(!ends_with(ch_name, "4"))
// 				{
// 						PhysicsChannel*     tmp     = dynamic_cast<PhysicsChannel*>(channels_[ch_name]);
// 						double chi2    = tmp->DecomposeV2(avg_waveforms[ch_name + "-INT"]);
//
// 			#pragma omp critical
// 						pt_.put("DecompositionResults." + ch_name, chi2);
//
// 						if( chi2 > GS->GetCaliPar<double>("PhysicsChannel.chi2_bound") )
// 						{
// 								std::cout << "\033[1;31mReconstruction failed! Name: "<< ch_name << " Nr: "<< nr_ <<" Chi2: "<< chi2 << "\033[0m"<< "\r" << std::endl;
// 						}
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
// {
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
