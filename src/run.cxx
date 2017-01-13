//============================================================================
// Name        : main.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================


 //std includes
 #include <iostream>
 #include <fstream>
 #include <vector>
 #include <map>
 #include <string>
 #include <cstdlib>
 #include <typeinfo>
 #include <math.h>
 #include <stdlib.h>



 // boost
 #include <boost/filesystem.hpp>
 #include <boost/lexical_cast.hpp>
 #include <boost/algorithm/string/predicate.hpp>
 #include <boost/algorithm/string/replace.hpp>
 #include <boost/property_tree/ptree.hpp>
 #include <boost/property_tree/ini_parser.hpp>
 // #include <boost/program_options.hpp>
 // #include <boost/filesystem/fstream.hpp>
 // #include <boost/algorithm/string/predicate.hpp>
 // #include <boost/foreach.hpp>

 // root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TThread.h>
// OpenMP
#include <omp.h>

// google performance tools
#include <gperftools/heap-profiler.h>
#include <gperftools/profiler.h>


// Project includes
#include "run.hh"
#include "pedestal.hh"
#include "event.hh"

#include "globalsettings.hh"


using namespace std;



//----------------------------------------------------------------------------------------------
// Definition of the Run class.
//----------------------------------------------------------------------------------------------

Run::Run(boost::filesystem::path p)
{

    path_run_ = p;
    // Extract the runnumer from the path to the folder and convert it to int.
    run_nr_     = atoi(path_run_.filename().string().substr(4,20).c_str());
    run_nr_str_ = path_run_.filename().string().substr(4,20);
    pedestal_   = new Pedestal(run_nr_);
    cout << "Created run:" << run_nr_ << endl;

};

void Run::SynchronizeFiles()
{
    /*
    This method uses the path to the run folder to determine the number of events.
    Afterwards it checks if all the files for all the events do exist. Finally it
    creates one EventClass object for each physics and each intermedieate event with
    the paths to all the files as a parameter => SynchronizeFiles().
    */

    // cout << "-------------------------------------------------------"<< endl;
    cout << "Synchronizing run" << "\r" << std::flush;

    // Check if the converted root, data & intermediate folders are available.
    if( !boost::filesystem::is_directory(path_run_/path("data_root"))  )
    {
        cout << "run folder does not exits!" << endl;
        exit(-1);
    }

    if( !boost::filesystem::is_directory(path_run_/path("data_root"))
        && boost::filesystem::is_empty(path_run_/path("data_root"))    )
    {
        cout << "data_root folder does not exits!" << endl;
        exit(-1);
    }

    if( !boost::filesystem::is_directory(path_run_/path("int_root"))
        && boost::filesystem::is_empty(path_run_/path("int_root"))     )
    {
        cout << "int_root folder does not exits!" << endl;
        exit(-1);
    }

    if(!boost::filesystem::is_directory(path_run_/path("Calibration")) )
    {
        boost::filesystem::create_directory(path_run_/path("Calibration"));
    }

    path path_data = path_run_ / path("data_root");

    vector<path> folder_content;
    copy(directory_iterator(path_data), directory_iterator(), back_inserter(folder_content));
    std::sort(folder_content.begin(),folder_content.end());

//    #pragma omp parallel num_threads(7)
//    {
//       #pragma omp for ordered schedule(dynamic,1)
        for (signed int i=0; i<folder_content.size(); ++i)
        {
    //     //claws::ProgressBar((itr - folder_content.begin()+1.)/(folder_content.end()-folder_content.begin()));
            if(    boost::filesystem::is_regular_file(folder_content.at(i))
                && starts_with(folder_content.at(i).filename().string(), "Event-")
                && ends_with(folder_content.at(i).filename().string(), ".root"))
            {
                // Get the paths to the .root file of the event.
                path path_file_root = folder_content.at(i);
                // cout << "Loading file: " << path_file_root.string() << endl;
                // Get the path to the .ini file.
                string tmp          = folder_content.at(i).filename().string();
                replace_last( tmp, ".root" , ".ini");
                path path_file_ini  = path_data / path(tmp);

                // Get the path to the file from the online monitor
                replace_first(tmp, "Event-","");
                replace_last(tmp, ".ini", "");
                string ratefile = "Rate-Run--" + to_string( atoi(tmp.substr(2,4).c_str())) + to_string( atoi(tmp.substr(6,10).c_str())-1 );
                path path_online_rate = path_run_ / ratefile;

                // Check if the .ini & online monitor exist for the event.
                if( boost::filesystem::exists( path_file_ini) && boost::filesystem::exists( path_online_rate))
                {
                        events_.push_back(new PhysicsEvent(path_file_root, path_file_ini, path_online_rate));
                }
                else{
                    //TODO put in some mechanism in case the ini or the online rate files do not exist.
                }
            }
        }
//    }

    path path_int = path_run_/path("int_root");
    folder_content.clear();
    copy(directory_iterator(path_int), directory_iterator(), back_inserter(folder_content));
    std::sort(folder_content.begin(),folder_content.end());

//    #pragma omp parallel num_threads(7)
//    {
    //    #pragma omp for ordered schedule(dynamic,1)
        for (signed int i=0; i<folder_content.size(); ++i)
        {
            if(    boost::filesystem::is_regular_file(folder_content.at(i))
                && starts_with((folder_content.at(i)).filename().string(), ("Run-"+ to_string(run_nr_) +"-Int") )
                && ends_with((folder_content.at(i)).filename().string(), ".root"))
                {
                // Get the paths to the .root file of the event.
                path path_file_root = (folder_content.at(i));

                // Get the path to the .ini file.
                string tmp          = (folder_content.at(i)).filename().string();
                replace_last( tmp, ".root" , ".ini");
                path path_file_ini  = path_int / path(tmp);

                // Check if the .ini & online monitor exist for the event.
                if( exists( path_file_ini) ){
                    int_events_.push_back(new IntEvent(path_file_root, path_file_ini));
                }
                else
                {
                    //TODO put in some mechanism in case the ini or the online rate files do not exist.
                }
            }
        }
        std::cout <<"Synchronizing done" << std::endl;

};


void Run::LoadRawData()
{

    std::cout << "Loading data:  " << run_nr_ << "\r" << std::flush;
    this->LoadEventFiles();
    this->LoadIntFiles();
    this->LoadWaveforms();
    this->LoadRunSettings();

    std::cout << "Loading data done!                  " << std::endl;

    //
    //
    //
    //
    // if(!boost::filesystem::is_directory(path_run_/boost::filesystem::path("Calibration/raw")) )
    // {
    //     boost::filesystem::create_directory(path_run_/boost::filesystem::path("Calibration/raw"));
    // }
    //
    // boost::filesystem::path fname = path_run_.string()/boost::filesystem::path("/Calibration/raw/Run-"+run_nr_str_+"_raw.root");
    // this->SaveEvents(fname);

    // for(auto &e : events_)
    // {
    //
    //     std::cout<< "Event: "<<e->GetNrStr()<< std::endl;
    // }

};
void Run::LoadEventFiles()
{
    // Method to load all the information that is located in the data_root folder with
    // following steps:
    //      1. Look into the data_root folder and create a PhysicalEvent object for
    //         each event.root file & check if file for online particle rate and
    //         do exist
    //      2. Loop through all objects in events_ and load the raw data from the
    //         corresponding root file
    //      3. Loop through all objects in events_ and load the oneline monitor
    //         pacout << "Loading Raw Data:  " << run_nr_ << endl;
    // Look into the data folder of the run and get a list/vector of all the events inside
    for(unsigned int i=0; i< events_.size();i++)
    {
        events_.at(i)->LoadRootFile();
    }

    for(unsigned int i=0; i< events_.size();i++)
    {
        events_.at(i)->LoadOnlineRate();
    }

    for(unsigned int i=0; i< events_.size();i++)
    {
        events_.at(i)->LoadIniFile();
    }

};

void Run::LoadIntFiles()
{
    for(unsigned int i=0; i< int_events_.size();i++)
    {
        int_events_.at(i)->LoadRootFile();
    }

    for(unsigned int i=0; i< int_events_.size();i++)
    {
        int_events_.at(i)->LoadIniFile();
    }
};

void Run::LoadWaveforms()
{
    // cout<<"Run::LoadWaveforms" <<endl;
    // double wall0 = claws::get_wall_time();
    // double cpu0  = claws::get_cpu_time();


    #pragma omp parallel num_threads(7)
    {
        #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< events_.size();i++)
        {
            events_.at(i)->LoadWaveform();
            events_.at(i)->DeleteHistograms();
        }
        #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            int_events_.at(i)->LoadWaveform();
            int_events_.at(i)->DeleteHistograms();
        }

    }

    // double wall1 = claws::get_wall_time();
    // double cpu1  = claws::get_cpu_time();
    //
    // cout << "Wall Time = " << wall1 - wall0 << endl;
    // cout << "CPU Time  = " << cpu1  - cpu0  << endl;
};

void Run::LoadRunSettings()
{
    path ini_file  = path_run_ / ("Run-" + to_string(run_nr_) + "-Settings.ini");

    if( boost::filesystem::is_regular_file(ini_file) && exists(ini_file) )
    {
        property_tree::ini_parser::read_ini(ini_file.string(), settings_);
    }

    // Data taking in the phyics events has been conducted with an vertical offset.
    std::string ch[8]       = {"FWD1", "FWD2", "FWD3", "FWD4", "BWD1", "BWD2", "BWD3", "BWD4"};
    std::string sections[8] = {"Scope-1-Channel-Settings-A", "Scope-1-Channel-Settings-B", "Scope-1-Channel-Settings-C", "Scope-1-Channel-Settings-D",
                               "Scope-2-Channel-Settings-A", "Scope-2-Channel-Settings-B", "Scope-2-Channel-Settings-C", "Scope-2-Channel-Settings-D"};

    map<std::string, float> baseline;

    for(unsigned i=0;i<8;i++)
    {
        int offset = claws::ConvertOffset(settings_.get<double>(sections[i]+".AnalogOffset"), settings_.get<int>(sections[i]+".Range"));
        baseline[ch[i]] = offset;
    }

    for(unsigned int i=0; i< events_.size();i++)
    {
        events_.at(i)->SetBaseline(baseline);

    }
};

void Run::SubtractPedestal()
{

    double wall0 = claws::get_wall_time();
    double cpu0  = claws::get_cpu_time();
    std::cout << "Subtracting pedestal: running" << "\r" << std::flush;

    this->LoadPedestal();
    // this->CalculatePedestal();


    this->SavePedestal();
    this->Subtract();
    // if(!boost::filesystem::is_directory(path_run_/boost::filesystem::path("Calibration")) )
    // {
    //     boost::filesystem::create_directory(path_run_/boost::filesystem::path("Calibration"));
    // }
    //
    // boost::filesystem::path fname = path_run_.string()/boost::filesystem::path("/Calibration/run-"+run_nr_str_+"_events_subtracted.root");
    // this->SaveEvents(fname);

    std::cout << "Subtracting pedestal: done!   " << std::endl;

    double wall1 = claws::get_wall_time();
    double cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;

};

void Run::LoadPedestal()
{
    // Just in case we are not running this thing for the first time,
    // make sure we delete the previous stuff.

    // for(auto & itr : h_ped_)
    // {
    //     delete itr.second;
    // }
    //
    // for(auto & itr : h_ped_int_)
    // {
    //     delete itr.second;
    // }

    // Set the entries to NULL or at all if ran forstd::string channel the first time.
    // h_ped_["FWD1"]   = NULL;
    // h_ped_["FWD2"]   = NULL;
    // h_ped_["FWD3"]   = NULL;
    // h_ped_["FWD4"]   = NULL;
    //
    // h_ped_["BWD1"]   = NULL;
    // h_ped_["BWD2"]   = NULL;
    // h_ped_["BWD3"]   = NULL;
    // h_ped_["BWD4"]   = NULL;
    //
    // // Set the entries to NULL or at all if ran for the first time.
    // h_ped_int_["FWD1-INT"]   = NULL;
    // h_ped_int_["FWD2-INT"]   = NULL;
    // h_ped_int_["FWD3-INT"]   = NULL;
    //
    // h_ped_int_["BWD1-INT"]   = NULL;
    // h_ped_int_["BWD2-INT"]   = NULL;
    // h_ped_int_["BWD3-INT"]   = NULL;

    // Create the histograms
    // for(auto & itr : h_ped_)
    // {
    //     string title    = "Run-" + to_string(run_nr_) + "-" + itr.first + "_pd";
    //     //TODO Get the fucking binning right!
    //     h_ped_[itr.first] = new TH1I(title.c_str(), title.c_str(), GS->GetNBitsScope() , GS->GetXLow(), GS->GetXUp());
    //
    // }
    //
    // for(auto & itr : h_ped_int_)
    // {
    //     string title    = "Run-" + to_string(run_nr_) + "-" + itr.first + "_pd";
    //     h_ped_int_[itr.first] = new TH1I(title.c_str(), title.c_str(), GS->GetNBitsScope() , GS->GetXLow(), GS->GetXUp());
    //
    // }

    // #pragma omp parallel num_threads(7)
    // {
    //     #pragma omp for schedule(dynamic,1)
    //     for(unsigned int i=0; i< events_.size();i++)
    //     {
    //
    //         events_.at(i)->LoadPedestal();
    //         map<string, TH1I*> tmp = events_.at(i)->GetPedestal();
    //         for (auto& m : h_ped_)
    //         {
    //             m.second->Add(tmp[m.first]);
    //         }
    //     }
    //
    //     #pragma omp for schedule(dynamic,1)
    //     for(unsigned int i=0; i< int_events_.size();i++)
    //     {
    //         int_events_.at(i)->LoadPedestal();
    //         map<string, TH1I*> tmp = int_events_.at(i)->GetPedestal();
    //         for (auto& m : h_ped_int_)
    //         {
    //             h_ped_int_[m.first]->Add(tmp[m.first]);
    //         }
    //     }
    //
    // }

    // The following block might be looking a little bit redundant, but the AddEvent() Method in the pedestal class is
    // of course always accessing the same object. Therefore, raise conditions are expected to come up and you have to
    // split the pedestal generation and transfer.

    #pragma omp parallel num_threads(7)
    {
        #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< events_.size();i++)
        {

            events_.at(i)->LoadPedestal();

        }

        #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            int_events_.at(i)->LoadPedestal();

        }

    }

    for(unsigned int i=0; i< events_.size();i++)
    {
        pedestal_->AddEvent(events_.at(i)->GetPedestal());
    }

    for(unsigned int i=0; i< int_events_.size();i++)
    {
        pedestal_->AddEvent(int_events_.at(i)->GetPedestal());
    }

};
void Run::CalculatePedestal()
{

    // ped_.clear();
    // ped_int_.clear();
    //
    // for(auto & itr : h_ped_)
    // {
    //     string name = to_string(run_nr_) +"_"+ itr.first +"_pd_fit";
    //
    //     string section;
    //     if(itr.first == "FWD1")          section = "Scope-1-Channel-Settings-A";
    //     else if(itr.first == "FWD2")     section = "Scope-1-Channel-Settings-B";
    //     else if(itr.first == "FWD3")     section = "Scope-1-Channel-Settings-C";
    //     else if(itr.first == "FWD4")     section = "Scope-1-Channel-Settings-D";
    //
    //     else if(itr.first == "BWD1")     section = "Scope-2-Channel-Settings-A";
    //     else if(itr.first == "BWD2")     section = "Scope-2-Channel-Settings-B";
    //     else if(itr.first == "BWD3")     section = "Scope-2-Channel-Settings-C";
    //     else if(itr.first == "BWD4")     section = "Scope-2-Channel-Settings-D";
    //
    //     int offset = claws::ConvertOffset(settings_.get<double>(section+".AnalogOffset"), settings_.get<int>(section+".Range"));
    //
    //     // TODO Check if a gaussain really is the best option to get the pedestral. A center of gravity might work better.
    //
    //     TF1 *fit = new TF1(name.c_str(), "gaus" , offset-5 , offset +5 );
    //     fit->SetParameter(1, offset);
    //     itr.second->Fit(fit, "RQ0");
    //
    //     double constant = itr.second->GetFunction(name.c_str())->GetParameter(0);
    //     double mean     = itr.second->GetFunction(name.c_str())->GetParameter(1);
    //     double sigma    = itr.second->GetFunction(name.c_str())->GetParameter(2);
    //
    //     fit->SetParameters(constant, mean, sigma);
    //     fit->SetRange(mean - 3*sigma, mean +3*sigma);
    //
    //     itr.second->Fit(fit, "R0");
    //     const Int_t kNotDraw = 1<<9;
    //     itr.second->GetFunction(name.c_str())->ResetBit(kNotDraw);
    //
    //     ped_[itr.first] = itr.second->GetFunction(name.c_str())->GetParameter(1);
    //
    // }
    //
    // for(auto & itr : h_ped_int_)
    // {
    //     string name = to_string(run_nr_) +"_"+ itr.first +"_pd_fit";
    //
    //     // TODO Check if a gaussain really is the best option to get the pedestral. A center of gravity might work better.
    //     TF1 *fit = new TF1(name.c_str(), "gaus" , -5 , 5 );
    //     fit->SetParameter(1, 0);
    //     itr.second->Fit(fit, "R0");
    //     const Int_t kNotDraw = 1<<9;
    //     itr.second->GetFunction(name.c_str())->ResetBit(kNotDraw);
    //     ped_int_[itr.first] = itr.second->GetFunction(name.c_str())->GetParameter(1);
    //
    // }
    // pedestal_->CalculatePedestal();
};

void Run::SavePedestal()
{
    if(!boost::filesystem::is_directory(path_run_/path("Calibration")) )
    {
        boost::filesystem::create_directory(path_run_/path("Calibration"));
    }

    std::string filename = path_run_.string()+"/Calibration/run-"+run_nr_str_+"_pedestal_subtraction.root";
    TFile *rfile = new TFile(filename.c_str(), "RECREATE");

    pedestal_->SavePedestal(rfile);

    rfile->Close();
    delete rfile;

};

void Run::SaveEvents(boost::filesystem::path fname)
{
    TFile *rfile = new TFile(fname.string().c_str(), "RECREATE");

    rfile->mkdir("events");
    for(auto &e : events_)
    {
        rfile->cd();
        rfile->mkdir(("events/"+e->GetNrStr()).c_str());
        rfile->cd(("events/"+e->GetNrStr()).c_str());

        std::map<std::string, Channel*> chs= e->GetChannels();

        for(auto &ch : chs)
        {
            TCanvas c(ch.first.c_str(),ch.first.c_str(),500,500);
            TH1F* hist ;
            hist = NULL;
        }

    }

    rfile->mkdir("int");

    for(auto &e : int_events_)
    {
        rfile->cd();
        rfile->mkdir(("int/"+e->GetNrStr()).c_str());
        rfile->cd(("int/"+e->GetNrStr()).c_str());

        std::map<std::string, Channel*> chs= e->GetChannels();

        for(auto &ch : chs)
        {
            TCanvas c(ch.first.c_str(),ch.first.c_str(),500,500);
            TH1F* hist = ch.second->GetWaveformHist();
            hist->Draw();
            c.Write();
            delete hist;
            hist = NULL;
        }

    }

    rfile->Close();
    delete rfile;
    rfile = NULL;

};


double Run::GetStartTime(){
    return tsMin;
};
double Run::GetStopTime(){
    return tsMax;
};

int Run::BuildOnlineTree(){
    // TODO Implentation
    return 0;
};
int Run::BuildOfflineTree(){
    // TODO Implentation
    return 0;
};
TTree *Run::GetOnlineTree(){
    this->BuildOnlineTree();
    return tree_online;
};
TTree *Run::GetOfflineTree(){
    this->BuildOfflineTree();
    return tree_offline;
};

int Run::WriteOnlineTree(TFile* file)
{
    TTree *tout = new TTree("tout","tout");
    TTree *tout_inj = new TTree("tout_inj","tout_inj");
    TTree *tscrub = new TTree("tscrub","tscrub");
    TTree *tscrub_inj = new TTree("tscrub_inj","tscrub_inj");

    double ts;
    double rate_on[6] = {0};

    tout->Branch("ts", &ts,     "ts/D");
    tout->Branch("rate_on", rate_on,     "rate_on[6]/D");

    tout_inj->Branch("ts", &ts,     "ts/D");
    tout_inj->Branch("rate_on", rate_on,     "rate_on[6]/D");

    tscrub->Branch("ts", &ts,     "ts/D");
    tscrub->Branch("rate_on", rate_on,     "rate_on[6]/D");

    tscrub_inj->Branch("ts", &ts,     "ts/D");
    tscrub_inj->Branch("rate_on", rate_on,     "rate_on[6]/D");

    for(unsigned int i=0; i < events_.size(); i++){

        ts = events_.at(i)->GetUnixtime();

        rate_on[0] = events_.at(i)->GetRateOnline()[0];
        rate_on[1] = events_.at(i)->GetRateOnline()[1];
        rate_on[2] = events_.at(i)->GetRateOnline()[2];
        rate_on[3] = events_.at(i)->GetRateOnline()[3];
        rate_on[4] = events_.at(i)->GetRateOnline()[4];
        rate_on[5] = events_.at(i)->GetRateOnline()[5];

        if(events_.at(i)->GetInjection())
        {
            tout_inj->Fill();
            if(events_.at(i)->GetScrubbing() == 3)   tscrub_inj->Fill();
        }

        else
        {
            tout->Fill();
            if(events_.at(i)->GetScrubbing() == 3)   tscrub->Fill();
        }

    }

    // double ts, rate_fwd1, rate_fwd2, rate_fwd3, rate_fwd4, rate_bwd1, rate_bwd2, rate_bwd3, rate_bwd4;
    // bool injection;
    //
    // tout->Branch("ts", &ts,     "ts/D");
    // tout->Branch("fwd1_rate_on", &rate_fwd1,     "fwd1_rate_on/D");
    // tout->Branch("fwd2_rate_on", &rate_fwd2,     "fwd2_rate_on/D");
    // tout->Branch("fwd3_rate_on", &rate_fwd3,     "fwd3_rate_on/D");
    // tout->Branch("fwd4_rate_on", &rate_fwd4,     "fwd4_rate_on/D");
    // tout->Branch("bwd1_rate_on", &rate_bwd1,     "bwd1_rate_on/D");
    // tout->Branch("bwd2_rate_on", &rate_bwd2,     "bwd2_rate_on/D");
    // tout->Branch("bwd3_rate_on", &rate_bwd3,     "bwd3_rate_on/D");
    // tout->Branch("bwd4_rate_on", &rate_bwd4,     "bwd4_rate_on/D");
    //
    // tout_inj->Branch("ts", &ts,     "ts/D");
    // tout_inj->Branch("fwd1_rate_on", &rate_fwd1,     "fwd1_rate_on/D");
    // tout_inj->Branch("fwd2_rate_on", &rate_fwd2,     "fwd2_rate_on/D");
    // tout_inj->Branch("fwd3_rate_on", &rate_fwd3,     "fwd3_rate_on/D");
    // tout_inj->Branch("fwd4_rate_on", &rate_fwd4,     "fwd4_rate_on/D");
    // tout_inj->Branch("bwd1_rate_on", &rate_bwd1,     "bwd1_rate_on/D");
    // tout_inj->Branch("bwd2_rate_on", &rate_bwd2,     "bwd2_rate_on/D");
    // tout_inj->Branch("bwd3_rate_on", &rate_bwd3,     "bwd3_rate_on/D");
    // tout_inj->Branch("bwd4_rate_on", &rate_bwd4,     "bwd4_rate_on/D");
    //
    // for(unsigned int i=0; i < events_.size(); i++){
    //
    //     ts = events_.at(i)->GetUnixtime();
    //
    //     rate_fwd1 = events_.at(i)->GetRateOnline()[0];
    //     rate_fwd2 = events_.at(i)->GetRateOnline()[1];
    //     rate_fwd3 = events_.at(i)->GetRateOnline()[2];
    //     rate_fwd4 = events_.at(i)->GetRateOnline()[3];
    //     rate_bwd1 = events_.at(i)->GetRateOnline()[4];
    //     rate_bwd2 = events_.at(i)->GetRateOnline()[5];
    //     rate_bwd3 = events_.at(i)->GetRateOnline()[6];
    //     rate_bwd4 = events_.at(i)->GetRateOnline()[7];
    //
    //     if(events_.at(i)->GetInjection()) tout_inj->Fill();
    //     else                             tout->Fill();
    //
    // }

    file->cd();
    tout->Write();
    tout_inj->Write();
    tscrub->Write();
    tscrub_inj->Write();

    delete tout;
    delete tout_inj;
    delete tscrub;
    delete tscrub_inj;

    return 0;
};

int Run::WriteTimeStamp(TFile* file)
{
    TTree *tout = new TTree("tout","tout");

    double ts;
    bool injection;

    tout->Branch("ts", &ts,     "ts/D");
    tout->Branch("inj", &injection,     "inj/O");

    for(unsigned int i=0; i < events_.size(); i++){

        injection = events_.at(i)->GetInjection();
        ts        = events_.at(i)->GetUnixtime();

        tout->Fill();


    }

    file->cd();
    tout->Write();

    delete tout;

    return 0;
};

int Run::WriteNTuple(path path_ntuple){

    path_ntuple = path_ntuple / ("CLWv0.1-" +to_string(run_nr_) +"-" + to_string((int)tsMin) +".root");
    TFile * root_file  = new TFile(path_ntuple.string().c_str(), "RECREATE");

    this->WriteTimeStamp(root_file);
    this->WriteOnlineTree(root_file);

    root_file->Close();

    return 0;
};







void Run::Subtract()
{
    #pragma omp parallel num_threads(7)
    {
        std::map<std::string, float> tmp = pedestal_->GetPedestal(1);

        #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< events_.size();i++)
        {
            events_.at(i)->SubtractPedestal(tmp);
        }
    }

    #pragma omp parallel num_threads(7)
    {
        std::map<std::string, float> tmp = pedestal_->GetPedestal(2);

        #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            int_events_.at(i)->SubtractPedestal(tmp);
        }
    }
}



void Run::DrawPedestal()
{
    // string title = to_string(run_nr_);
    // TCanvas * c = new TCanvas(title.c_str(), title.c_str(), 1600, 1200);
    // c->Divide(2,h_ped_.size()/2);
    // unsigned int pad=0;
    // for(auto i : h_ped_)
    // {
    //     pad+=+2;
    //     if(pad > h_ped_.size()) pad =1;
    //     c->cd(pad);
    //     i.second->Draw();
    // }
    //
    // title += "-Int";
    // TCanvas * c_int = new TCanvas(title.c_str(), title.c_str(), 1600, 1200);
    // c_int->Divide(2, h_ped_int_.size()/2);
    // pad=0;
    // for(auto i : h_ped_int_)
    // {
    //     pad+=+2;
    //     if(pad > h_ped_int_.size() ) pad =1;
    //     c_int->cd(pad);
    //     i.second->Draw();
    //
    // }
}




Run::~Run() {
	// TODO Auto-generated destructor stub
};
// int setStyle(TGraph* graph1,TGraph* graph2,TGraph* graph3,TGraph* graph4){
//     double markersize=0.1;
//
//     graph1->SetMarkerColor(kAzure-3);
//     graph1->SetMarkerStyle(20);
//     graph1->SetMarkerSize(markersize);
//     graph1->GetYaxis()->SetRangeUser(-32512., 32512);
//
//     graph2->SetMarkerColor(kOrange+7);
//     graph2->SetMarkerStyle(20);
//     graph2->SetMarkerSize(markersize);
//
//     graph3->SetMarkerColor(kGreen+2);
//     graph3->SetMarkerStyle(20);
//     graph3->SetMarkerSize(markersize);
//
//     graph4->SetMarkerColor(kRed);
//     graph4->SetMarkerStyle(20);
//     graph4->SetMarkerSize(markersize);
//
//     return 0;
// }
//
// Event::Event(TTree* meta, TTree* data) {
//
//     // Check if everything is there.
//     if (meta == NULL || data == NULL){
//         std::cout << "Meta tree or data tree not found" << std::endl;
//     }
//
//     // Extract the meta data aka timestamp and evt number
//     meta->SetBranchAddress("evt_nr", &evt_nr);
//     meta->SetBranchAddress("unixtime", &unixtime);
//     meta->GetEntry(0);
//
//     delete meta;
//     meta = NULL;
//
//     //TODO make this here dynamic, now only 8 channels are possible
//     int16_t fwd1;
//     data->SetBranchAddress("FWD1", &fwd1);
//     FWD1=new TGraph();
//
//     int16_t fwd2;
//     data->SetBranchAddress("FWD2", &fwd2);
//     FWD2=new TGraph();
//
//     int16_t fwd3;
//     data->SetBranchAddress("FWD3", &fwd3);
//     FWD3=new TGraph();
//
//     int16_t fwd4;
//     data->SetBranchAddress("FWD4", &fwd4);
//     FWD4=new TGraph();
//
//     int16_t bwd1;
//     data->SetBranchAddress("BWD1", &bwd1);
//     BWD1=new TGraph();
//
//     int16_t bwd2;
//     data->SetBranchAddress("BWD2", &bwd2);
//     BWD2=new TGraph();
//
//     int16_t bwd3;
//     data->SetBranchAddress("BWD3", &bwd3);
//     BWD3=new TGraph();
//
//     int16_t bwd4;
//     data->SetBranchAddress("BWD4", &bwd4);
//     BWD4=new TGraph();
//
//     Long64_t n_entries = data->GetEntries();
//
//     //TODO Instead of i*0.8 use the appropriate timebase from the scope.
//     for (Long64_t i = 0 ; i < n_entries ; i++) {
//         data->GetEntry(i);
//         FWD1->SetPoint(i, i * 0.8, fwd1);
//         FWD2->SetPoint(i, i * 0.8, fwd2);
//         FWD3->SetPoint(i, i * 0.8, fwd3);
//         FWD4->SetPoint(i, i * 0.8, fwd4);
//         BWD1->SetPoint(i, i * 0.8, bwd1);
//         BWD2->SetPoint(i, i * 0.8, bwd2);
//         BWD3->SetPoint(i, i * 0.8, bwd3);
//         BWD4->SetPoint(i, i * 0.8, bwd4);
//     }
//
//     delete data;
//     data = NULL;
//
//     // TODO Auto-generated constructor stub
// 	// TODO Autoflag on TGraph to check if is_clock=true, maybe GetYAxis->GetMean ~ (max - min)/2 + min ~ Wert
//
// }
//
// Event::~Event() {
// 	// TODO Auto-generated destructor stub
// }
//
//
// TGraph* Event::getChannel(std::string channel){
//
//     if(channel == "FWD1"){
//         return FWD1;
//     }else if (channel == "FWD2"){
//         return FWD2;
//     }else if (channel == "FWD3"){
//         return FWD3;
//     }else if (channel == "FWD4"){
//         return FWD4;
//     }else if (channel == "BWD1"){
//         return BWD1;
//     }else if (channel == "BWD2"){
//         return BWD2;
//     }else if (channel == "BWD3"){
//         return BWD3;
//     }else if (channel == "BWD4"){
//         return BWD4;
//     }else{
//         std::cout << "Channel " << channel << " not found! " << std::endl;
//         return NULL;
//     }
//
// }
//
// int Event::eventToPdf(std::string file){
//
//     setStyle(FWD1, FWD2, FWD3, FWD4);
//     setStyle(BWD1, BWD2, BWD3, BWD4);
//
//     TCanvas *c1 = new TCanvas("c1","multipads",1200,750);
//     c1->Divide(1,2,0,0);
//
//     c1->cd(1);
//     FWD1->Draw("AP");
//     FWD2->Draw("P");
//     FWD3->Draw("P");
//     FWD4->Draw("P");
//
//     c1->cd(2);
//     BWD1->Draw("AP");
//     BWD2->Draw("P");
//     BWD3->Draw("P");
//     BWD4->Draw("P");
//
//     c1->SaveAs(file.c_str());
//
//     return 0;
//
// }
//
// int Event::calcRate(std::string channel){
//
//
//     return 0;
// }
//
//
// double Event::getRate(std::string channel){
//     //TODO implementation
//     return 0;
// }
//
//
//
// Data::Data(TDirectory* dir){
//
//
//
// //    Event* event = new Event(meta, data);
//     map<int,Event*> events;
//     map<int,Event*>::iterator it;
//
//
//     TIter next(dir->GetListOfKeys());
//     TKey* key;
//     while ((key = (TKey*)next())){
//     		int evt_nr = std::stoi(string(key->GetName()).substr(0,9));
//     		it = events.find(evt_nr);
//     		if (it == events.end()){
//                 TTree* meta = (TTree*)dir->Get((to_string(evt_nr)+"-meta").c_str());
//                 TTree* data = (TTree*)dir->Get((to_string(evt_nr)+"-data").c_str());
//                 Event* event=new Event(meta,data);
//     			events.insert(pair<int,Event*>(evt_nr,event));
//     		}
//
//     	}
//
//
// //    it = events.begin();
// //    while(it !=events.end()){
// ////        data = (TTree*)dir->Get((to_string(it->first)+"-data").c_str());
// ////        meta = (TTree*)dir->Get((to_string(it->first)+"-meta").c_str());
// ////        it->second=new Event(meta, data);
// //        cout << it->first << ", " << it->second->getEvtnr() <<endl;
// //        ++it;
// //    	}
//
// //    cout << events.size()<< endl;
// }
//
// Data::~Data() {
//     // TODO Auto-generated destructor stub
// }
//
// Event* getEvent(int evt_nr){
//     return NULL;
// };
//
// int Data::appendEvent(Event* event){
//
//
//     return 0;
// }
