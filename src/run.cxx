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
 #include <fstream>
 #include <memory>



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
//#include <omp.h>

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
    gain_       = new Gain(run_nr_);
    cout << "---------------------------------------------------------" << endl;
    cout << "\033[1;31mRun::Created run: \033[0m" << run_nr_ << endl;
    // std::ofstream hendrik_file("/home/iwsatlas1/mgabriel/Plots/forHendyDany.txt", ios::app);
    // hendrik_file << run_nr_str_;
    // hendrik_file.close();
    claws::print_local_time();
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
    cout << "\033[33;1mRun::Synchronizing run:\033[0m running" << "\r" << std::flush;

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

                // Get the path to the .ini file
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
        cout << "\033[32;1mRun::Synchronizing run:\033[0m done!   " << "\r" << std::endl;


};


void Run::LoadRawData()
{
    double wall0 = claws::get_wall_time();
    double cpu0  = claws::get_cpu_time();

    std::cout << "\033[33;1mRun::Loading data:\033[0m running" << "\r" << std::flush;

//    std::cout << "Loading data:  " << run_nr_ << "\r" << std::flush;

    this->LoadEventFiles();
    this->LoadIntFiles();
    this->LoadWaveforms();
    this->LoadRunSettings();

    std::cout << "\033[32;1mRun::Loading data:\033[0m done!   " << "\r" << std::endl;
//    std::cout << "Loading data done!                  " << std::endl;


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

    double wall1 = claws::get_wall_time();
    double cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
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
        events_.at(i)->LoadWaveform();
        events_.at(i)->DeleteHistograms();
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


//    #pragma omp parallel num_threads(7)
  //  {
        // When all the histograms are copyed into memory and than the the vectors are filled in a second (multi threaded) step, a
        // full run would use more than my full r
        // #pragma omp for schedule(dynamic,1)
        // for(unsigned int i=0; i< events_.size();i++)
        // {
        //     events_.at(i)->LoadWaveform();
        //     events_.at(i)->DeleteHistograms();
        // }
    //    #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            int_events_.at(i)->LoadWaveform();
            int_events_.at(i)->DeleteHistograms();
        }

    //}

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

    std::cout << "\033[33;1mRun::Subtracting pedestal:\033[0m running" << "\r" << std::flush;

    this->LoadPedestal();
    this->SavePedestal();
    this->Subtract();

    std::cout << "\033[32;1mRun::Subtracting pedestal:\033[0m done!       " << std::endl;

    double wall1 = claws::get_wall_time();
    double cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
};

void Run::LoadPedestal()
{
    /*
        TODO description
    */

//    #pragma omp parallel num_threads(7)
  //  {
  //      #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< events_.size();i++)
        {
            events_.at(i)->LoadPedestal();
        }

    //    #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            int_events_.at(i)->LoadPedestal();
        }

  //  }

    // There is of course only one object of class Pedestal (pedestal_) all events need to access => no multi threading.
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
    //     else if(itr.first == "BWD2")   GetChannel
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
    /*
        TODO description
    */
    if(!boost::filesystem::is_directory(path_run_/path("Calibration")) )
    {
        boost::filesystem::create_directory(path_run_/path("Calibration"));
    }

    std::string filename = path_run_.string()+"/Calibration/run-"+run_nr_str_+"_pedestal_subtraction_v1.root";
    TFile *rfile = new TFile(filename.c_str(), "RECREATE");

    pedestal_->SavePedestal(rfile);

    rfile->Close();
    delete rfile;

};

void Run::Subtract()
{
    /*
        TODO description
    */
//    #pragma omp parallel num_threads(7)
//    {
        std::map<std::string, float> tmp = pedestal_->GetPedestal(1);

  //      #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< events_.size();i++)
        {
            events_.at(i)->SubtractPedestal(tmp);
        }
  //  }

  //  #pragma omp parallel num_threads(7)
  //  {
        std::map<std::string, float> tmp_int = pedestal_->GetPedestal(2);

    //    #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            int_events_.at(i)->SubtractPedestal(tmp_int, true);
        }
  //  }
};

void Run::GainCalibration()
{
    /*
        TODO description
    */

    std::cout << "\033[33;1mRun::Calibrating gain:\033[0m running" << "\r" << std::flush;

    double wall0 = claws::get_wall_time();
    double cpu0  = claws::get_cpu_time();

    for(unsigned int i=0; i< int_events_.size();i++)
    {
        int_events_.at(i)->CalculateIntegral();
        gain_->AddValue(int_events_.at(i)->GetIntegral());
    }

    gain_->FitGain();
    gain_->SaveGain(path_run_);


    std::cout << "\033[32;1mRun::Calibrating gain:\033[0m done!     " << std::endl;

    double wall1 = claws::get_wall_time();
    double cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
}

void Run::Average1PE()
{
    /*
        TODO description
    */

    std::cout << "\033[33;1mRun::Extracting average 1 pe:\033[0m running" << "\r" << std::flush;

    double wall0 = claws::get_wall_time();
    double cpu0  = claws::get_cpu_time();

    // for(unsigned int i=0; i< int_events_.size();i++)
    // {
    //     gain_->AddIntWf(int_events_.at(i)->GetWaveforms(), int_events_.at(i)->GetIntegral());
    // }
    std::vector<std::vector<IntChannel*>> vec;
    for(auto ivec : GS->GetChannels(2))
    {
        vec.push_back(this->GetIntChannel(ivec));
    }

    gain_->AddIntWfs(vec);
//    gain_->NormalizeWaveforms(int_events_.size());
    gain_->WfToHist();
    gain_->FitAvg();
    gain_->HistToWf();
    // gain_->WfToHist();
    gain_->SaveAvg(path_run_);

    std::cout << "\033[32;1mRun::Extracting average 1 pe:\033[0m done!     " << std::endl;

    double wall1 = claws::get_wall_time();
    double cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
};

void Run::WaveformDecomposition()
{
    std::cout << "\033[33;1mRun::Decomposing waveforms:\033[0m running" << "\r" << std::flush;

    double wall0 = claws::get_wall_time();
    double cpu0  = claws::get_cpu_time();

    this->Decompose();


    std::cout << "\033[32;1mRun::Decomposing waveforms:\033[0m done!     " << std::endl;

    double wall1 = claws::get_wall_time();
    double cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;

    wall0 = claws::get_wall_time();
    cpu0  = claws::get_cpu_time();

    this->SaveEvents();

    // std::cout << "\033[32;1mRun::Decomposing waveforms:\033[0m done!     " << std::endl;

    wall1 = claws::get_wall_time();
    cpu1  = claws::get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
}

void Run::Decompose()
{


    std::map<std::string, std::vector<float>*> avg_waveforms = gain_->GetWaveform();
    std::map<std::string, double> pe_to_mips = GS->GetPEtoMIPs();
//    #pragma omp parallel for num_threads(5) firstprivate(avg_waveforms)
    for(unsigned int i=0; i< events_.size(); i++)
    {
        events_.at(i)->SetUpWaveforms();
        events_.at(i)->FastRate(avg_waveforms, pe_to_mips);
//        events_.at(i)->Decompose(avg_waveforms);
//        events_.at(i)->Reconstruct(avg_waveforms);
//        events_.at(i)->CalculateChi2();
    }

    //TODO Finish implentation
};

void Run::Reconstruct()
{
    //TODO Implentation
};

void Run::CalculateChi2()
{
    //TODO Implentation
};

void Run::SaveEvents()
{
    std::cout << "Now saving events!" << std::endl;


    boost::filesystem::path folder = path_run_/boost::filesystem::path("Calibration");
    if(!boost::filesystem::is_directory(folder) )
    {
        boost::filesystem::create_directory(folder);
    }

    std::string fname = folder.string()+"/run_"+std::to_string(run_nr_)+"_snapshoot_selectedv1.root";

    TFile *rfile = new TFile(fname.c_str(), "RECREATE");
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

            TH1F* hist = ch.second->GetWaveformHist();
            hist->Draw();
            c.Write();
            // c.SaveAs((folder.string()+"/Original/run_"+std::to_string(run_nr_)+"_"+e->GetNrStr()+"_"+ch.second->GetName()+"_original.pdf").c_str());
            delete hist;
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

std::vector<IntChannel*> Run::GetIntChannel(std::string name)
{
    std::vector<IntChannel*> channel;
    for(auto & ivec : int_events_)
    {
        IntChannel* tmp = dynamic_cast<IntChannel*>(ivec->GetChannel(name));
        channel.push_back(tmp);
    }
    return channel;
};

std::vector<PhysicsChannel*> Run::GetPhysicsChannel(std::string name)
{
    std::vector<PhysicsChannel*> channel;
    for(auto & ivec : events_)
    {
        PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(ivec->GetChannel(name));
        channel.push_back(tmp);
    }
    return channel;
}


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
    std::cout << "Deleteing Run object!" << std::endl;
  //  #pragma omp parallel num_threads(7)
  //  {
    //    #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< events_.size();i++)
        {
            delete events_.at(i);
            events_.at(i) = NULL;
        }

  //      #pragma omp for schedule(dynamic,1)
        for(unsigned int i=0; i< int_events_.size();i++)
        {
            delete int_events_.at(i);
            int_events_.at(i) = NULL;
        }
  //  }
    delete pedestal_;
    delete gain_;

};
