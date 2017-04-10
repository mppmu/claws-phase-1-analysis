//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

// boost
#include <boost/algorithm/string/replace.hpp>
// OpenMP
//#include <omp.h>
// root
#include <TFile.h>


#include "event.hh"
#include "globalsettings.hh"


using namespace std;
using namespace boost;

//----------------------------------------------------------------------------------------------
// Definition of the Event base class.
//----------------------------------------------------------------------------------------------

//int Event::id_ = 0;

// int Event::GetId()
// {
//     return id_;
// }

Event::Event(const path &file_root, const path &file_ini)
{
//    ++id_;

    path_file_ini_       = file_ini;
    path_file_root_      = file_root;

};
void Event::LoadRootFile()
{
    // std::cout<< "Loading Event: " << nr_str_ << std::endl;
    // if(nr_ == 0)
    // {
    //     std::cout<< path_file_root_ << std:: endl;
    // }

    TFile *file=NULL;
    file = new TFile(path_file_root_.string().c_str(), "open");

    if(file->IsZombie())
    {
        cout << "Error openning file" << endl;
        exit(-1);
    }

    for (const auto &itr : channels_)
    {
        itr.second->LoadHistogram(file);
    }

    file->Close("R");
    delete file;
    file = NULL;

};

void Event::LoadWaveform()
{
  // for (auto &mmap : channels_)
  // {
  //     std::cout<< mmap.first << std::endl;
  // }

  for (const auto & itr : channels_)
  {
        itr.second->LoadWaveform();
  }

};

void Event::DeleteHistograms()
{

    for (const auto &itr : channels_)
    {
        itr.second->DeleteHistogram();
    }

};

void Event::DeleteWaveforms()
{

    for (const auto &itr : channels_)
    {
        itr.second->DeleteWaveform();
    }

};

void Event::LoadPedestal()
{
    for (auto &itr : channels_)
    {
        itr.second->LoadPedestal();
        // catch(std::string exception)
        // {
        //     std::cout<< "In event nr: " << nr_ << "\n" << exception << std::endl;
        // }
    }
};

// void Event::SubtractPedestal()
// {
//     /* See "void Channel::Subtract()" for more information
//     */
//     for(auto &ch : channels_)
//     {
//         ch.second->Subtract();
//     }
// };

void Event::SubtractPedestal(std::map<std::string, float> ped, bool backup)
{
    /* Subtracts the pedestal from the individual waveforms. Input options:
        1) If no parameters are specified, the pedestal from the indi -
           vidual events will be used for the subtraction. Usually the
           average run pedestal should be used, unstable pedestal over
           time like with th pickup in intermediate events in phase one
           this might lead to gains.
        2) If the first input parameter is given (i.e. not an empty map
           like default), it will be used for pedestal subtraction instead
           of the event specific one.
        3) If input pedestal is specified and backup == true, the individual
           event pedestal is used, only if this equal to zero the run-wide
           pedestal is used.
    */
    if( ped.empty() && !backup )
    {
        for(auto &ch : channels_)
        {
            ch.second->SubtractPedestal();
        }
    }
    else if(!ped.empty() && !backup)
    {
        for(auto &ch : channels_)
        {
            ch.second->SubtractPedestal( ped[ch.first] );
        }
    }
    else if(!ped.empty() && backup)
    {
        for(auto &ch : channels_)
        {
            ch.second->SubtractPedestal( ped[ch.first], backup);
        }
    }
}

void Event::SetPedestal(std::map<std::string, float> ped, bool backup)
{
    /* Subtracts the pedestal from the individual waveforms. Input options:
        1) If no parameters are specified, the pedestal from the indi -
           vidual events will be used for the subtraction. Usually the
           average run pedestal should be used, unstable pedestal over
           time like with th pickup in intermediate events in phase one
           this might lead to gains.
        2) If the first input parameter is given (i.e. not an empty map
           like default), it will be used for pedestal subtraction instead
           of the event specific one.
        3) If input pedestal is specified and backup == true, the individual
           event pedestal is used, only if this equal to zero the run-wide
           pedestal is used.
    */
    if( ped.empty() && !backup )
    {
        for(auto &ch : channels_)
        {
            ch.second->SetPedestal();
        }
    }
    else if(!ped.empty() && !backup)
    {
        for(auto &ch : channels_)
        {
            ch.second->SetPedestal( ped[ch.first] );
        }
    }
    else if(!ped.empty() && backup)
    {
        for(auto &ch : channels_)
        {
            ch.second->SetPedestal( ped[ch.first], backup);
        }
    }
}

void Event::SetBaseline(map<std::string, float> baseline)
{
    for(auto & itr : channels_)
    {
        itr.second->SetBaseline( baseline[itr.first] );
    }
};


int Event::GetNr()     const
{
    return nr_;
}

std::string Event::GetNrStr()     const
{
    return nr_str_;
}

int Event::getCh(string ch){

    return 0;
}

// void Event::Draw(){
//
//     cout << "Drawing PhysicsEvent: "<< nr_ << endl;
//
//     TCanvas * c = new TCanvas(to_string(nr_).c_str(),to_string(nr_).c_str(), 1600, 1200);
//     c->Divide(2, channels_.size()/2);
//     unsigned int pad=0;
//
//     for(auto i : channels_)
//     {
//         pad+=+2;
//         if(pad > channels_.size()) pad =1;
//         c->cd(pad);
//         i.second->GetWaveformHist()->Draw();
//     }
//
//     string ped_can_name = to_string(nr_) + " Pedestal";
//     TCanvas * c_ped = new TCanvas(ped_can_name.c_str(), ped_can_name.c_str(), 1600, 1200);
//     c_ped->Divide(2, channels_.size()/2);
//     pad=0;
//     for(auto i : channels_)
//     {
//         pad+=+2;
//         if(pad > channels_.size() ) pad =1;
//         c_ped->cd(pad);
//         i.second->GetPedestal()->Draw();
//     }
//
//
// }

std::map<std::string, TH1I*> Event::GetPedestal()
{
    /*
        TODO description
    */
    std::map<std::string, TH1I*> rtn;
    for(auto & itr : channels_)
    {
        rtn[itr.first]  =  itr.second->GetPedestal();
    }

    return rtn;
};

Channel* Event::GetChannel(std::string name)
{
    return channels_[name];
}

std::map<std::string, Channel*> Event::GetChannels()
{
    return channels_;
};

void Event::CalculateIntegral()
{
    for(auto & itr : channels_)
    {
        itr.second->CalculateIntegral();
    }

};

std::map<std::string, double> Event::GetIntegral()
{
    std::map<std::string, double>    rtn;

    for(auto & itr : channels_)
    {
        rtn[itr.first]  =  itr.second->GetIntegral();
    }

    return rtn;
};

void Event::CreateHistograms()
{
    for(auto & itr : channels_)
    {
        itr.second->CreateHistogram();
    }
};

std::map<std::string, TH1*> Event::GetHistograms()
{
    std::map<std::string, TH1*>    rtn;

    for(auto & itr : channels_)
    {
        rtn[itr.first]  =  itr.second->GetHistogram();
    }

    return rtn;
}



std::map<std::string, std::vector<float>*> Event::GetWaveforms()
{
    std::map<std::string, std::vector<float>*>  rtn;
    for(auto& mvec : channels_)
    {
        rtn[mvec.first] = mvec.second->GetWaveform();
    }
    return rtn;
};

Event::~Event() {
	// TODO Auto-generated destructor stub
    for(auto& m : channels_)
    {
        delete m.second;
    }
}

//----------------------------------------------------------------------------------------------
// Definition of the PhysicsEvent class derived from Event.
//----------------------------------------------------------------------------------------------

PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{
    fill_n(online_rate_, 6, -1);
    fill_n(fast_rate_, 3, -1);
    fill_n(rate_, 3, -1);

    nr_     = atoi(file_root.filename().string().substr(6,15).c_str());
    nr_str_ = file_root.filename().string().substr(6,9);

    /*
        TODO Implement a dynamic creation of channels getting the list list and therefore number of channels from somewhere else.
    */
    channels_["FWD1"] = new PhysicsChannel("FWD1");
    channels_["FWD2"] = new PhysicsChannel("FWD2");
    channels_["FWD3"] = new PhysicsChannel("FWD3");
    channels_["FWD4"] = new PhysicsChannel("FWD4");

    channels_["BWD1"] = new PhysicsChannel("BWD1");
    channels_["BWD2"] = new PhysicsChannel("BWD2");
    channels_["BWD3"] = new PhysicsChannel("BWD3");
    channels_["BWD4"] = new PhysicsChannel("BWD4");
};

PhysicsEvent::PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate): PhysicsEvent(file_root, file_ini)
{
  path_online_rate_ = file_online_rate;
};


PhysicsEvent::~PhysicsEvent() {
	// TODO Auto-generated destructor stub

};

// void PhysicsEvent::LoadRootFile()
// {
//
//     // TFile and TTree classes are not thread save, therefore, you have to look them.
//     #pragma omp critical
//     {
//         file = new TFile(path_file_root_.string().c_str(), "open");
//
//         if(file->IsZombie())
//         {
//             cout << "Error openning file" << endl;
//             exit(-1);
//         }
//     }
//
//     for (auto &itr : channels_)
//
//     #pragma omp critical
//     {
//         file->Close("R");
//     }
//
//     delete file;
//     file = NULL;
//     // cout << "Listing gDirectory in PhysicsEvent::LoadRootFile() after closing the file!" << endl;
//     // gDirectory->ls();
// }


void PhysicsEvent::LoadIniFile(){


	property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);

    unixtime_       = pt_.get<double>("Properties.UnixTime");
	lerbg_          = pt_.get<int>("SuperKEKBData.LERBg");
	herbg_          = pt_.get<int>("SuperKEKBData.HERBg");
    kekb_status_    = pt_.get<string>("SuperKEKBData.SuperKEKBStatus");
    ler_status_     = pt_.get<string>("SuperKEKBData.LERSTatus");
    her_status_     = pt_.get<string>("SuperKEKBData.HERStatus");

    if (lerbg_ || herbg_ )                      injection_ = true;
    else                                        injection_ = false;
    if(kekb_status_ == "Vacuum Scrubbing"
     && ler_status_ == "Vacuum Scrubbing"
     && her_status_ == "Vacuum Scrubbing")      scrubbing_ = 3;
    else if(ler_status_ == "Vacuum Scrubbing")  scrubbing_ = 1;
    else if(her_status_ == "Vacuum Scrubbing")  scrubbing_ = 2;
    else                                        scrubbing_ = 0;

    //TODO load the rest that is written in the .ini file.
};

void PhysicsEvent::LoadOnlineRate(){

    std::ifstream ratefile(path_online_rate_.string());

    if (ratefile)
    {
        double dummy;
        ratefile >> online_rate_[0] >> online_rate_[1] >> online_rate_[2] >> dummy >> online_rate_[3] >> online_rate_[4] >> online_rate_[5] >> dummy;
    }
    else
    {
        cout << "\033[1;31mOnlinerate missing:   \033[0m" << nr_ << endl;
        for(int i=0; i<6; i++)
        {
            online_rate_[i] = 0;
        }
    }

    ratefile.close();



    for(unsigned i = 0; i < 4; i++)
    {
      if(!ends_with(GS->GetChannels(1).at(i), "4"))
      {
        pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i]);
      }
    }
    for(unsigned i = 4; i < 8; i++)
    {
      if(!ends_with(GS->GetChannels(1).at(i), "4"))
      {
        pt_.put("OnlineRate." + GS->GetChannels(1).at(i), online_rate_[i-1]);
      }
    }


};


void PhysicsEvent::SetUpWaveforms()
{
    /**
     * [PhysicsEvent::SetUpWaveforms2 description]
     */
    for(auto& mvec : channels_)
    {
        if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
        {
            PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
            tmp->SetUpWaveforms();
        }
        // std::string tmp_name = mvec.first;
        // replace_last(tmp_name, "-INT", "");
        //
        // tmp->Decompose(mvec.second);
        //        tmp->Decompose();
    }
};

void PhysicsEvent::SetUpWaveforms2()
{
    //TODO Validation
    for(auto& mvec : channels_)
    {
        if(!ends_with(mvec.first, "-INT") && !ends_with(mvec.first, "4"))
        {
            PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mvec.second);
            tmp->SetUpWaveforms2();
        }
    }
};

void PhysicsEvent::FastRate(std::map<std::string, std::vector<float>*> avg_waveforms, std::map<std::string, double> pe_to_mips)
{
    //TODO Validation
    for(auto& mvec : avg_waveforms)
    {
        std::string tmp_name = mvec.first;
        replace_last(tmp_name, "-INT", "");
        PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
        tmp->FastRate(mvec.second, pe_to_mips[tmp_name]);
        //        tmp->Decompose();
        // double rate = tmp->GetRate();

        if(tmp_name == "FWD1")
        {
          fast_rate_[0] = tmp->GetRate();
          pt_.put("FastRate." + tmp_name, fast_rate_[0]);
        }
        else if(tmp_name == "FWD2")
        {
          fast_rate_[1] = tmp->GetRate();
          pt_.put("FastRate." + tmp_name, fast_rate_[1]);
        }
        else if(tmp_name == "FWD3")
        {
          fast_rate_[2] = tmp->GetRate();
          pt_.put("FastRate." + tmp_name, fast_rate_[2]);
        }

    }


};

void PhysicsEvent::Decompose(std::map<std::string, std::vector<float>*> avg_waveforms)
{
    //TODO Validation
    for(auto& mvec : avg_waveforms)
    {
        std::string tmp_name = mvec.first;
        replace_last(tmp_name, "-INT", "");
        PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
        tmp->Decompose(mvec.second);
        //        tmp->Decompose();
    }
};

void PhysicsEvent::Reconstruct(std::map<std::string, std::vector<float>*> avg_waveforms)
{
    //TODO Implentation    // channels_["BWD1-INT"] = new IntChannel("BWD1");
    for(auto& mvec : avg_waveforms)
    {

    // channels_["BWD2-INT"] = new IntChannel("BWD2");
    // channels_["BWD3-INT"] = new IntChannel("BWD3");
        std::string tmp_name = mvec.first;
        replace_last(tmp_name, "-INT", "");
        PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(channels_[tmp_name]);
        tmp->Reconstruct(mvec.second);
    }
};

void PhysicsEvent::CalculateChi2()
{
    //TODO Implentation
    for(auto& mmap : channels_)
    {
        // std::string tmp_name = mvec.first;
        // replace_last(tmp_name, "-INT", "");
        if( !ends_with(mmap.first, "4"))
        {
            PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(mmap.second);
            tmp->CalculateChi2();
        }
    }
};

void PhysicsEvent::SaveEvent(boost::filesystem::path folder, std::string type)
{
    if(!boost::filesystem::is_directory( folder ))
    {
        boost::filesystem::create_directory( folder );
    }

//    std::string fname = folder.string()+"/event_"+std::to_string(nr_)+"_" + type +"_v"+ std::to_string(GS->GetCaliPar<int>("General.CalibrationVersion")) + ".root";
    std::string fname = folder.string()+"/event_"+std::to_string(nr_)+"_" + type + ".root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto imap : this->GetHistograms(type))
    {
        imap.second->Write();
    }

    rfile->Close();
    delete rfile;


    boost::property_tree::write_ini(folder.string()+"/event_"+std::to_string(nr_)+".ini", pt_);

    // boost::filesystem::path dest = folder/path_file_ini_.filename();
    // boost::filesystem::copy_file(path_file_ini_, dest, copy_option::overwrite_if_exists );
};




void PhysicsEvent::CreateHistograms(std::string type)
{
    for(auto & imap : channels_)
    {
        PhysicsChannel* p_ch = dynamic_cast<PhysicsChannel*>(imap.second);
        p_ch->CreateHistogram(type);
    }
};

std::map<std::string, TH1*> PhysicsEvent::GetHistograms(std::string type)
{
    std::map<std::string, TH1*>    rtn;

    for(auto & imap : channels_)
    {
        PhysicsChannel* p_ch = dynamic_cast<PhysicsChannel*>(imap.second);
        rtn[imap.first]  =  p_ch->GetHistogram(type);
    }
    return rtn;
}

double* PhysicsEvent::GetRate(int type){
    if( type == 0 )
    {
        return online_rate_;
    }
    else if( type == 1 )
    {
        return fast_rate_;
    }
    else if( type == 2 )
    {
        return rate_;
    }
    else
    {
        return NULL;
    }
}

bool PhysicsEvent::GetInjection()  const
{
    return injection_;
}

int PhysicsEvent::GetScrubbing()  const
{
    return scrubbing_;
}

int PhysicsEvent::GetLerBg()       const
{
    return lerbg_;
}

int PhysicsEvent::GetHerBg()       const
{
    return herbg_;
}
double PhysicsEvent::GetUnixtime() const
{
    return unixtime_;
}

// template<typename T>;
// T PhysicsEvent::GetPV(std::string pv)
// {
//     return pt_.get<T>( "SuperKEKBData." + pv );
// };

// std::map<std::string, double> PhysicsEvent::GetIntegral()
// {
//     /*
//     TODO implementation
//     */
//
//     std::map<std::string, double>    rtn;
//     return rtn;
// }

//----------------------------------------------------------------------------------------------
// Definition of the IntEvent class derived from Event.
//----------------------------------------------------------------------------------------------
IntEvent::IntEvent(const path &file_root, const path &file_ini): Event(file_root ,file_ini)
{
    nr_str_ = file_root.filename().string().substr(14,3);
    nr_     = atoi(nr_str_.c_str());
    /*
        TODO Implement a dynamic creation of channels getting the list list and therefore number of channels from somewhere else.
    */
    channels_["FWD1-INT"] = new IntChannel("FWD1");
    channels_["FWD2-INT"] = new IntChannel("FWD2");
    channels_["FWD3-INT"] = new IntChannel("FWD3");

    channels_["BWD1-INT"] = new IntChannel("BWD1");
    channels_["BWD2-INT"] = new IntChannel("BWD2");
    channels_["BWD3-INT"] = new IntChannel("BWD3");

};

void IntEvent::LoadIniFile()
{
    property_tree::ptree pt_;
    property_tree::ini_parser::read_ini(path_file_ini_.string(), pt_);

    for(int i=1;i<5;i++)
    {
        try
        {
            mean_online_[i-1] = pt_.get<double>("FWD."+std::to_string(i)+"-Mean");
        }
        catch(...)
        {
            mean_online_[i-1] = 0;
        }
        try
        {
        accepted_online_[i-1] = pt_.get<double>("FWD."+std::to_string(i)+"-Accepted");
        }
        catch(...)
        {
            accepted_online_[i-1] = 0;
        }
    }
    for(int i=1;i<5;i++)
    {
        try
        {
            mean_online_[i+4-1] = pt_.get<double>("BWD."+std::to_string(i)+"-Mean");
        }
        catch(...)
        {
            mean_online_[i+4-1] =0;
        }
        try
        {
            accepted_online_[i+4-1] = pt_.get<double>("BWD."+std::to_string(i)+"-Accepted");
        }
        catch(...)
        {
            accepted_online_[i+4-1] = 0;
        }
    }
};

// IntChannel* IntEvent::GetChannel(std::string name)
// {
//     return channels_[name];
// }

IntEvent::~IntEvent() {
	// TODO Auto-generated destructor stub
};
