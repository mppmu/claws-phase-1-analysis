//============================================================================
// Name        : run.cpp
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================


// --- C++ includes ---
#include <string>
#include <iostream>
#include <ctime>
#include <fstream>
//  #include <fstream>
//  #include <vector>
//  #include <map>
//
//  #include <cstdlib>
//  #include <typeinfo>
//  #include <math.h>
//  #include <stdlib.h>
//  #include <fstream>
//  #include <memory>
//  #include <cassert>
//
//
//
// // boost
//  #include <boost/filesystem.hpp>
//  #include <boost/lexical_cast.hpp>
 #include <boost/algorithm/string/predicate.hpp>
 #include <boost/algorithm/string/replace.hpp>
//  #include <boost/property_tree/ptree.hpp>
//  #include <boost/property_tree/ini_parser.hpp>
// // #include <boost/program_options.hpp>
// // #include <boost/filesystem/fstream.hpp>
// // #include <boost/algorithm/string/predicate.hpp>
// // #include <boost/foreach.hpp>

// --- ROOT includes ---
#include <TGraphErrors.h>
#include <TFile.h>
// #include <TH1D.h>
// #include <TH1I.h>
// #include <TLegend.h>
// #include <TApplication.h>
// #include <TCanvas.h>
// #include <TF1.h>
// #include <TThread.h>
// // OpenMP
// #include <omp.h>


// google performance tools
// #include <gperftools/heap-profiler.h>
// #include <gperftools/profiler.h>


// Project includes
#include "run.hh"
#include "pedestal.hh"
#include "event.hh"
#include "globalsettings.hh"


using namespace std;
using namespace boost;

Double_t langaufun(Double_t *x, Double_t *par) {

   //Fit parameters:
   //par[0]=Width (scale) parameter of Landau density
   //par[1]=Most Probable (MP, location) parameter of Landau density
   //par[2]=Total area (integral -inf to inf, normalization constant)
   //par[3]=Width (sigma) of convoluted Gaussian function
   //
   //In the Landau distribution (represented by the CERNLIB approximation),
   //the maximum is located at x=-0.22278298 with the location parameter=0.
   //This shift is corrected within this function, so that the actual
   //maximum is identical to the MP parameter.

      // Numeric constants
      Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
      Double_t mpshift  = -0.22278298;       // Landau maximum location

      // Control constants
      Double_t np = 100.0;      // number of convolution steps
      Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

      // Variables
      Double_t xx;
      Double_t mpc;
      Double_t fland;
      Double_t sum = 0.0;
      Double_t xlow,xupp;
      Double_t step;
      Double_t i;


      // MP shift correction
      mpc = par[1] - mpshift * par[0];

      // Range of convolution integral
      xlow = x[0] - sc * par[3];
      xupp = x[0] + sc * par[3];

      step = (xupp-xlow) / np;

      // Convolution integral of Landau and Gaussian by sum
      for(i=1.0; i<=np/2; i++) {
         xx = xlow + (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);

         xx = xupp - (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);
      }

      return (par[2] * step * sum * invsq2pi / par[3]);
}

//----------------------------------------------------------------------------------------------
// Definition of the Run class.
//----------------------------------------------------------------------------------------------
Run::Run(boost::filesystem::path p)
{
		path_ = p;

		nr_     = atoi(path_.filename().string().substr(4,20).c_str());
		// run_nr_str_ = path_run_.filename().string().substr(4,20);
		cout << "\033[1;31mRun::Created run: \033[0m" << nr_ << " - at: " << p.string() << endl;

        // current date/time based on current system
        time_t now = time(0);

        // convert now to string form
        char* dt = ctime(&now);

        std::cout << "The local date and time is: " << dt << std::endl;
};

Run::~Run()
{

};

void Run::LoadRunSettings()
{
    int phase = GS->GetParameter<int>("General.Phase");

    boost::filesystem::path settings_file;

    if(phase == 1)
    {
        settings_file = path_ / ("Run-" + std::to_string(nr_) + "-Settings.ini");
    }
    else if(phase == 2)
    {
        settings_file = path_ / ("config-" + std::to_string(nr_) + ".ini");
    }

    if( boost::filesystem::is_regular_file(settings_file) && exists(settings_file) )
    {
         boost::property_tree::ini_parser::read_ini(settings_file.string(), settings_);
    }
};
//
// std::tuple<double, double> Run::GetTime()
// {
//      return std::make_tuple(tsMin, tsMax);
// }



// int Run::GetRunNr()
// {
//      return run_nr_;
// }


//----------------------------------------------------------------------------------------------
// Definition of the CalibrationRun class.
//----------------------------------------------------------------------------------------------

CalibrationRun::CalibrationRun(boost::filesystem::path p) : Run(p)
{
	cal_nr_     = nr_;

    if(!boost::filesystem::is_directory(path_/boost::filesystem::path("Calibration")) )
    {
            boost::filesystem::create_directory(path_/boost::filesystem::path("Calibration"));
    }

    GS->SaveConfigFiles(path_/boost::filesystem::path("Calibration"));

};

CalibrationRun::~CalibrationRun() {

    for(auto evt: evts_ )
    {
        delete evt;
    }

    for(auto evt: cal_evts_ )
    {
        delete evt;
    }

};

void CalibrationRun::SynchronizePhysicsEvents()
{
     /**
      *  This method uses the path to the run folder to determine the number of events.
      *  Afterwards it checks if all the files for all the events do exist. Finally it
      *  creates one EventClass object for each physics and each intermedieate event with
      *  the paths to all the files as a parameter => SynchronizeFiles().
      */

     cout << "\033[33;1mRun::Synchronizing physics run:\033[0m running" << "\r" << std::flush;

     int phase = GS->GetParameter<int>("General.Phase");

     filesystem::path p_physics_dir = path_;
     filesystem::path p_cal_dir = path_;
     string file_start = "";

     if(phase == 1)
     {
         p_physics_dir /= boost::filesystem::path("data_root");
         p_cal_dir /= boost::filesystem::path("int_root");
         file_start = "Event-";
     }
     else if(phase == 2)
     {
         p_physics_dir /= filesystem::path("raw")/filesystem::path("physics");
         p_cal_dir /= filesystem::path("raw")/filesystem::path("intermediate");
         file_start = "physics-";
     }

     // Check if the converted root, data & folder for calibration events are available.
     if( !boost::filesystem::is_directory(p_physics_dir)  )
     {
             cout << "Run folder does not exits!" << endl;
             exit(-1);
     }

     if( !boost::filesystem::is_directory(p_physics_dir)
         || boost::filesystem::is_empty(p_physics_dir)  )
     {
             cout << "data_root folder does not exits!" << endl;
             exit(-1);
     }

     if( !boost::filesystem::is_directory(p_cal_dir) )
     {
             cout << "int_root folder does not exits!" << endl;
             exit(-1);
     }

		 // Create the physics events based on the existing root files in the Run/data_root folder

    std::vector<boost::filesystem::path> physics_files;

    copy( boost::filesystem::directory_iterator(p_physics_dir), boost::filesystem::directory_iterator(), back_inserter(physics_files)   );

    std::sort( physics_files.begin(), physics_files.end());

		for(auto file : physics_files)
		{
				std::string file_name = file.filename().string();

		    if(    boost::filesystem::is_regular_file(file)
			      && boost::starts_with( file_name, file_start)
			      && boost::ends_with( file_name, ".root")   )
			  {
				   // Get path to ini file
				   std::string tmp = file_name;
				   boost::replace_last( tmp, ".root", ".ini");

                   if(phase == 2)
                   {
                       boost::replace_first( tmp, "physics", "info");
                   }

				   boost::filesystem::path ini_file  = p_physics_dir/filesystem::path(tmp);

					 // Get the path to the file from the online monitor
				   boost::replace_first(tmp, "Event-","");
				   boost::replace_last(tmp, ".ini", "");

				   std::stringstream ss;
					 ss << std::setw(4) << std::setfill('0') << atoi(tmp.substr(2,4).c_str());
				   tmp = "Rate-Run--" + ss.str() + to_string( atoi(tmp.substr(6,10).c_str())-1 );
				                     // string ratefile = "Rate-Run--" + to_string( atoi(tmp.substr(2,4).c_str())) + to_string( atoi(tmp.substr(6,10).c_str())-1 );
				   boost::filesystem::path onrate_file = path_ / boost::filesystem::path(tmp);

				   evts_.emplace_back( new PhysicsEvent(file, ini_file, onrate_file) );

			}
		}
		std::cout << "\033[32;1mRun::Synchronizing physics files:\033[0m done!   " << "\r" << std::endl;
};



void CalibrationRun::SynchronizeCalibrationEvents()
{
    std::cout << "\033[33;1mRun::Synchronizing calibration run:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path calibration = path_/boost::filesystem::path("Calibration");
    if(!boost::filesystem::is_directory(calibration) )
    {
        boost::filesystem::create_directory(calibration);
    }

		 // Create the calibration events based on the existing root files in the Run/int_root folde

		 /** The calibration waveforms where only taken if a run Finished completely, not is it was manually stopped!
		 *  Die Scheiße hier sauber machen!!!!
		 */

     int phase = GS->GetParameter<int>("General.Phase");

    // filesystem::path p_physics_dir = path_;
    filesystem::path p_cal_dir = path_;

    if(phase == 1)
    {
        //p_physics_dir /= boost::filesystem::path("data_root");
        p_cal_dir /= boost::filesystem::path("int_root");
    }
    else if(phase == 2)
    {
    //         p_physics_dir /= filesystem::path("raw")/filesystem::path("physics");
        p_cal_dir /= filesystem::path("raw")/filesystem::path("intermediate");
    }

	//boost::filesystem::path path_int = path_/ boost::filesystem::path("int_root");

	while( !claws::CheckIntFolder(p_cal_dir) )
 	{
        int new_run = 0;

        if(phase == 1)
        {
            new_run = atoi( p_cal_dir.parent_path().filename().string().substr(4,6).c_str())-1;
            cal_nr_ = new_run;
 	        p_cal_dir = path_.parent_path()/("Run-" + to_string(new_run) );
 	        p_cal_dir /= "int_root";
        }
        else if(phase == 2)
        {
            new_run = atoi( p_cal_dir.parent_path().parent_path().filename().string().substr(4,6).c_str())-1;
            cal_nr_ = new_run;
            p_cal_dir = path_.parent_path()/("run-" + to_string(new_run) );
            p_cal_dir /= filesystem::path("raw/intermediate");
        }

 	    std::cout << "\033[1;31mIntermediate Data not valid!!! \n Switching to: "<< p_cal_dir << "\033[0m"<< "\r" << std::endl;

        if(cal_nr_ != nr_)
        {
            ofstream myfile;
            myfile.open (calibration.string()+"/CALIBRATION_FILE_WARNING");
            myfile << "Warning different calibration files are used for this run!.\n";
            myfile << "Original run:\n" << to_string(nr_) << "\n";
            myfile << "run calibration files used:\n" << to_string(new_run) << std::endl;
            myfile.close();
        }
    }


	std::vector<boost::filesystem::path> cal_files;
    copy( boost::filesystem::directory_iterator(p_cal_dir), boost::filesystem::directory_iterator(), back_inserter(cal_files));

	std::sort( cal_files.begin(), cal_files.end());

    double evt_time = -1;

    if (!evts_.empty()) evt_time = evts_.back()->GetParameter<double>("Properties.UnixTime");

	for(auto file : cal_files)
	{
        string file_start = "";
        if(phase == 1)
        {
            file_start = "Run-"+ to_string(cal_nr_) +"-Int";
        }
        else if(phase == 2)
        {
            file_start = "inter-"+ to_string(cal_nr_);
        }

		std::string file_name = file.filename().string();

		if(    boost::filesystem::is_regular_file(file)
	        && boost::starts_with(file_name,  file_start)
		    && boost::ends_with(file_name, ".root"))
		{
		                  // Get the path to the .ini file
		std::string tmp = file_name;
		boost::replace_last( tmp, ".root", ".ini");

        if(phase == 2)
        {
            tmp = "config_inter-" + to_string(cal_nr_) + ".ini";
        }

		boost::filesystem::path ini_file  = p_cal_dir / boost::filesystem::path(tmp);

		cal_evts_.emplace_back( new CalibrationEvent(file, ini_file, evt_time) );
		}

        evt_time += 0.1;
	}

    cout << "\033[32;1mRun::Synchronizing calibration files:\033[0m done!   " << "\r" << std::endl;
};

void CalibrationRun::PDS_Calibration()
{
    std::cout << "\033[33;1mRun::Subtracting calibration pedestal:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path pds_calibration = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("PDS_Calibration");
    if(!boost::filesystem::is_directory(pds_calibration) )
    {
        boost::filesystem::create_directory(pds_calibration);
    }

    if(!boost::filesystem::is_directory(pds_calibration/boost::filesystem::path("Waveforms")) )
    {
        boost::filesystem::create_directory(pds_calibration/boost::filesystem::path("Waveforms"));
    }

    // Get the histograms and prepare them
	for(auto evt: cal_evts_ )
	{
	    evt->LoadFiles();
        evt->PrepareHistograms();
	}

    // Just to be sure and because they take no space, save them to disk
    for(auto evt: cal_evts_ )
    {
        evt->SaveEvent(pds_calibration/boost::filesystem::path("Waveforms"));
    }

    // Now do the pedestal subtraction
    for(auto evt: cal_evts_ )
    {
        evt->FillPedestals();
    }

    // Use the first event to get a dynamic number and name of channels.
    std::vector<TGraph *> fit_status;
    std::vector<TGraph *> fit_const;
    std::vector<TGraphErrors *> fit_mean;
    std::vector<TGraph *> fit_chi2;
    std::vector<TGraph *> fit_ndf;
    std::vector<TGraph *> fit_chi2ndf;
    vector<TGraph *> fit_pval;
    vector<TH1F *> hist_pval;
    vector<TGraphErrors *> hist_mean;
    vector<TGraph *> hist_entries;


    for(auto channel: cal_evts_.at(0)->GetChannels() )
    {

      std::string name = channel->GetName();
      boost::replace_last(name, "-INT", "");

      TGraph * stg = new TGraph();
      stg->SetName( (name+"_fit_status").c_str() );
      stg->SetMarkerStyle(23);
      stg->SetMarkerColor(kRed);
      stg->SetMarkerSize(1);
      stg->GetXaxis()->SetTitle("Time [s]");
      stg->GetYaxis()->SetTitle("Fit status");
      fit_status.push_back(stg);


      TGraph * constantg = new TGraph();
      constantg->SetName( (name +"_fit_const").c_str() );
      constantg->SetMarkerStyle(23);
      constantg->SetMarkerColor(kRed);
      constantg->SetMarkerSize(1);
      constantg->GetXaxis()->SetTitle("Time [s]");
      constantg->GetYaxis()->SetTitle("Constant term [au]");
      fit_const.push_back(constantg);

      TGraphErrors * pdg = new TGraphErrors();
      pdg->SetName( (name +"_fit_mean").c_str() );
      pdg->SetMarkerStyle(23);
      pdg->SetMarkerColor(kRed);
      pdg->SetMarkerSize(1);
      pdg->GetXaxis()->SetTitle("Time [s]");
      pdg->GetYaxis()->SetTitle("Pedestal mean [mV]");
      pdg->GetYaxis()->SetRangeUser(-128,127);
      fit_mean.push_back(pdg);

      TGraph * chig = new TGraph();
      chig->SetName( (name +"_fit_chi2").c_str() );
      chig->SetMarkerStyle(23);
      chig->SetMarkerColor(kRed);
      chig->SetMarkerSize(1);
      chig->GetXaxis()->SetTitle("Time [s]");
      chig->GetYaxis()->SetTitle("Chi2");
      fit_chi2.push_back(chig);

      TGraph * ndfg = new TGraph();
      ndfg->SetName( (name +"_fit_ndf").c_str() );
      ndfg->SetMarkerStyle(23);
      ndfg->SetMarkerColor(kRed);
      ndfg->SetMarkerSize(1);
      ndfg->GetXaxis()->SetTitle("Time [s]");
      ndfg->GetYaxis()->SetTitle("NDF");
      fit_ndf.push_back(ndfg);

      TGraph * chi2ndg = new TGraph();
      chi2ndg->SetName( (name +"_fit_chi2ndf").c_str() );
      chi2ndg->SetMarkerStyle(23);
      chi2ndg->SetMarkerColor(kRed);
      chi2ndg->SetMarkerSize(1);
      chi2ndg->GetXaxis()->SetTitle("Time [s]");
      chi2ndg->GetYaxis()->SetTitle("Chi2/NDF");
      fit_chi2ndf.push_back(chi2ndg);

    //   TH1D * pval = new TH1D((name+"_fit_pval").c_str(),(name+"_fit_pval").c_str(),100,0,1);
      TGraph * pval = new TGraph();
      pval->SetName((name+"_fit_pval").c_str());
       pval->SetMarkerStyle(23);
       pval->SetMarkerColor(kRed);
       pval->SetMarkerSize(1);
      pval->GetXaxis()->SetTitle("Time [s]");
      pval->GetYaxis()->SetTitle("Fit p-value");
      pval->GetYaxis()->SetRangeUser(-0.05,1.05);
      fit_pval.push_back(pval);

      TH1F * hpval = new TH1F((name+"_hist_pval").c_str(),(name+"_hist_pval").c_str(),101,-0.005,1.005);
      hpval->SetName((name+"_hist_pval").c_str());
      hpval->GetXaxis()->SetTitle("P-Val");
      hpval->GetYaxis()->SetTitle("Entries");
      hist_pval.push_back(hpval);

      TGraphErrors * mng = new TGraphErrors();
      mng->SetName( (name +"_hist_mean").c_str() );
      mng->SetMarkerStyle(23);
      mng->SetMarkerColor(kRed);
      mng->SetMarkerSize(1);
      mng->GetXaxis()->SetTitle("Time [s]");
      mng->GetYaxis()->SetTitle("Pedestal mean [mV]");
      mng->GetYaxis()->SetRangeUser(-128,127);
      hist_mean.push_back(mng);

      TGraph * ng = new TGraph();
      ng->SetName( (name +"_hist_entries").c_str() );
      ng->SetMarkerStyle(23);
      ng->SetMarkerColor(kRed);
      ng->SetMarkerSize(1);
      ng->GetXaxis()->SetTitle("Time [s]");
      ng->GetYaxis()->SetTitle("# entries");
      hist_entries.push_back(ng);
    }

    // Iterate over all evts and fill the pedestal for each channel in the
    // corresponding TGraphErrors.
    // Pedestals are plotted over the unixtime of the event.
    int evt_counter = 0;
    for(auto evt: cal_evts_ )
    {
        if( evt->GetState() ==  EVENTSTATE_PDFILLED )
        {
            double   evt_time = evt->GetParameter<double>("Properties.UnixTime");
            auto channels = evt->GetChannels();
            for(unsigned int i = 0; i <channels.size(); i++)
            {

                double * pd = channels.at(i)->GetPedestal();

                fit_status.at(i)->SetPoint(evt_counter, evt_time, pd[0]);
                fit_const.at(i)->SetPoint(evt_counter, evt_time, pd[1]);

                fit_mean.at(i)->SetPoint(evt_counter, evt_time, pd[2]);
                fit_mean.at(i)->SetPointError(evt_counter, 0.05, pd[3]);

                fit_chi2.at(i)->SetPoint(evt_counter, evt_time, pd[4]);

                fit_ndf.at(i)->SetPoint(evt_counter, evt_time, pd[5]);


                if( pd[5] !=0 ) fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, pd[4]/pd[5]);
                else fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, -1.);

                fit_pval.at(i)->SetPoint(evt_counter, evt_time,pd[6]);

                hist_pval.at(i)->Fill(pd[6]);

                hist_mean.at(i)->SetPoint(evt_counter, evt_time, pd[7]);
                hist_mean.at(i)->SetPointError(evt_counter, 0.05, pd[8]);

                hist_entries.at(i)->SetPoint(evt_counter, evt_time, pd[9]);
            }

            evt_counter ++;
        }
    }

    std::string fname = pds_calibration.string() + "/run_"+std::to_string(nr_)+"_pedestal"+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(int i = 0; i < fit_status.size(); i++)
    {
      fit_status.at(i)->Write();
      delete fit_status.at(i);
      fit_const.at(i)->Write();
      delete fit_const.at(i);
      fit_mean.at(i)->Write();
      delete fit_mean.at(i);
      fit_chi2.at(i)->Write();
      delete fit_chi2.at(i);
      fit_ndf.at(i)->Write();
      delete fit_ndf.at(i);
      fit_chi2ndf.at(i)->Write();
      delete fit_chi2ndf.at(i);
      fit_pval.at(i)->Write();
      delete fit_pval.at(i);
      hist_pval.at(i)->Write();
      delete hist_pval.at(i);
      hist_mean.at(i)->Write();
      delete hist_mean.at(i);
      hist_entries.at(i)->Write();
      delete hist_entries.at(i);
    }

    rfile->Close("R");

    for(auto evt: cal_evts_ )
    {
        evt->SubtractPedestals();
    }

    for(auto evt: cal_evts_ )
    {
        evt->SaveEvent(pds_calibration/boost::filesystem::path("Waveforms"), true);
        evt->DeleteHistograms();
    }

    std::cout << "\033[32;1mRun::Subtracting calibration pedestal:\033[0m done!       " << std::endl;
};

void CalibrationRun::GainDetermination()
{
    /**
    *          TODO description
    */

    std::cout << "\033[33;1mRun::Calibrating gain:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path gain_determination = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("GainDetermination");
    if(!boost::filesystem::is_directory(gain_determination) )
    {
        boost::filesystem::create_directory(gain_determination);
    }

    /** Load all the events in a pd subtracted state.
    *   If the pd subtraction failed for any of them. Erase it.
    */
    auto evt_itr = cal_evts_.begin();

    while( evt_itr != cal_evts_.end() )
    {
        (*evt_itr)->LoadFiles(EVENTSTATE_PDSUBTRACTED);

        if( (*evt_itr)->GetState() != EVENTSTATE_PDSUBTRACTED )
        {
                delete (*evt_itr);
                (*evt_itr) = NULL;
                cal_evts_.erase(evt_itr);
        }
        else
        {
                evt_itr++;
        }
    }


    Gain* gain = new Gain(path_);

    for(auto evt: cal_evts_ )
    {
        gain->AddEvent(evt);
    }

    this->DeleteCalibrationHistograms();

    gain->FitGain();

    gain->SaveGain( gain_determination );

    delete gain;

    std::cout << "\033[32;1mRun::Calibrating gain:\033[0m done!     " << std::endl;
};

void CalibrationRun::Average1PE()
{
     /*
         TODO description
      */

     std::cout << "\033[33;1mRun::Extracting average 1 pe:\033[0m running" << "\r" << std::flush;

     boost::filesystem::path average_1pe = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("Average1PE");
     if(!boost::filesystem::is_directory( average_1pe ) )
     {
         boost::filesystem::create_directory( average_1pe );
     }

     /** Load all the events in a pd subtracted state.
     *   If the pd subtraction failed for any of them. Erase it.
     */
     auto evt_itr = cal_evts_.begin();

     while( evt_itr != cal_evts_.end() )
     {
         (*evt_itr)->LoadFiles(EVENTSTATE_PDSUBTRACTED);

         if( (*evt_itr)->GetState() != EVENTSTATE_PDSUBTRACTED )
         {
                 delete (*evt_itr);
                 (*evt_itr) = NULL;
                 cal_evts_.erase(evt_itr);
         }
         else
         {
                 evt_itr++;
         }
     }


     Gain* gain = new Gain(path_, GAINSTATE_FITTED);

     // for(auto evt: cal_evts_ )
     // {
     //     evt->LoadFiles(EVENTSTATE_PDSUBTRACTED);
     // }

    for(auto evt: cal_evts_ )
    {
        gain->AddEvent(evt);
    }

    gain->Normalize();

    gain->FitAvg();

    this->DeleteCalibrationHistograms();

    gain->SaveGain( average_1pe );

    delete gain;

    std::cout << "\033[32;1mRun::Extracting average 1 pe:\033[0m done!     " << std::endl;
};

void CalibrationRun::PDS_Physics()
{
    std::cout << "\033[33;1mRun::Subtracting physics pedestal:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path pds_physics = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("PDS_Physics");
    if(!boost::filesystem::is_directory( pds_physics ) )
    {
        boost::filesystem::create_directory( pds_physics );
    }

    if(!boost::filesystem::is_directory( pds_physics/boost::filesystem::path("Waveforms")) )
    {
        boost::filesystem::create_directory( pds_physics/boost::filesystem::path("Waveforms"));
    }

    // // Get the histograms and prepare them
	// for(auto &evt: evts_ )
	// {
	//     evt->LoadFiles();
    //     evt->PrepareHistograms(settings_);
	// }
    //
    // // Just to be sure...
    // for(auto &evt: evts_ )
    // {
    //     evt->SaveEvent(pds_physics/boost::filesystem::path("Waveforms"));
    // }
    //
    // // Now do the pedestal subtraction
    // for(auto &evt: evts_ )
    // {
    //     evt->FillPedestals();
    // }

    // Use the first event to get a dynamic number and name of channels.
    std::vector<TGraph *> fit_status;
    std::vector<TGraph *> fit_const;
    std::vector<TGraphErrors *> fit_mean;
    std::vector<TGraph *> fit_chi2;
    std::vector<TGraph *> fit_ndf;
    std::vector<TGraph *> fit_chi2ndf;
    vector<TGraph *> fit_pval;
    vector<TH1F *> hist_pval;
    vector<TGraphErrors *> hist_mean;
    vector<TGraph *> hist_entries;

    for(auto &channel: evts_.at(0)->GetChannels() )
    {

      std::string name = channel->GetName();
     // boost::replace_last(name, "-INT", "");

      TGraph * stg = new TGraph();
      stg->SetName( (name+"_fit_status").c_str() );
      stg->SetMarkerStyle(23);
      stg->SetMarkerColor(kRed);
      stg->SetMarkerSize(1);
      stg->GetXaxis()->SetTitle("Time [s]");
      stg->GetYaxis()->SetTitle("Fit status");
      fit_status.push_back(stg);


      TGraph * constantg = new TGraph();
      constantg->SetName( (name +"_fit_const").c_str() );
      constantg->SetMarkerStyle(23);
      constantg->SetMarkerColor(kRed);
      constantg->SetMarkerSize(1);
      constantg->GetXaxis()->SetTitle("Time [s]");
      constantg->GetYaxis()->SetTitle("Constant term [au]");
      fit_const.push_back(constantg);

      TGraphErrors * pdg = new TGraphErrors();
      pdg->SetName( (name +"_fit_mean").c_str() );
      pdg->SetMarkerStyle(23);
      pdg->SetMarkerColor(kRed);
      pdg->SetMarkerSize(1);
      pdg->GetXaxis()->SetTitle("Time [s]");
      pdg->GetYaxis()->SetTitle("Pedestal mean [mV]");
      pdg->GetYaxis()->SetRangeUser(-128,127);
      fit_mean.push_back(pdg);

      TGraph * chig = new TGraph();
      chig->SetName( (name +"_fit_chi2").c_str() );
      chig->SetMarkerStyle(23);
      chig->SetMarkerColor(kRed);
      chig->SetMarkerSize(1);
      chig->GetXaxis()->SetTitle("Time [s]");
      chig->GetYaxis()->SetTitle("Chi2");
      fit_chi2.push_back(chig);

      TGraph * ndfg = new TGraph();
      ndfg->SetName( (name +"_fit_ndf").c_str() );
      ndfg->SetMarkerStyle(23);
      ndfg->SetMarkerColor(kRed);
      ndfg->SetMarkerSize(1);
      ndfg->GetXaxis()->SetTitle("Time [s]");
      ndfg->GetYaxis()->SetTitle("NDF");
      fit_ndf.push_back(ndfg);

      TGraph * chi2ndg = new TGraph();
      chi2ndg->SetName( (name +"_fit_chi2ndf").c_str() );
      chi2ndg->SetMarkerStyle(23);
      chi2ndg->SetMarkerColor(kRed);
      chi2ndg->SetMarkerSize(1);
      chi2ndg->GetXaxis()->SetTitle("Time [s]");
      chi2ndg->GetYaxis()->SetTitle("Chi2/NDF");
      fit_chi2ndf.push_back(chi2ndg);

    //   TH1D * pval = new TH1D((name+"_fit_pval").c_str(),(name+"_fit_pval").c_str(),100,0,1);
      TGraph * pval = new TGraph();
      pval->SetName((name+"_fit_pval").c_str());
      pval->SetMarkerStyle(23);
      pval->SetMarkerColor(kRed);
      pval->SetMarkerSize(1);
      pval->GetXaxis()->SetTitle("Time [s]");
      pval->GetYaxis()->SetTitle("Fit p-value");
      pval->GetYaxis()->SetRangeUser(-0.05,1.05);
      fit_pval.push_back(pval);

      TH1F * hpval = new TH1F((name+"_hist_pval").c_str(),(name+"_hist_pval").c_str(),101,-0.005,1.005);
      hpval->SetName((name+"_hist_pval").c_str());
      hpval->GetXaxis()->SetTitle("P-Val");
      hpval->GetYaxis()->SetTitle("Entries");
      hist_pval.push_back(hpval);

      TGraphErrors * mng = new TGraphErrors();
      mng->SetName( (name +"_hist_mean").c_str() );
      mng->SetMarkerStyle(23);
      mng->SetMarkerColor(kRed);
      mng->SetMarkerSize(1);
      mng->GetXaxis()->SetTitle("Time [s]");
      mng->GetYaxis()->SetTitle("Pedestal mean [mV]");
      mng->GetYaxis()->SetRangeUser(-128,127);
      hist_mean.push_back(mng);

      TGraph * ng = new TGraph();
      ng->SetName( (name +"_hist_entries").c_str() );
      ng->SetMarkerStyle(23);
      ng->SetMarkerColor(kRed);
      ng->SetMarkerSize(1);
      ng->GetXaxis()->SetTitle("Time [s]");
      ng->GetYaxis()->SetTitle("# entries");
      hist_entries.push_back(ng);
    }

    // Iterate over all evts and fill the pedestal for each channel in the
    // corresponding TGraphErrors.
    // Pedestals are plotted over the unixtime of the event.
    int evt_counter = 0;
    for(auto &evt: evts_ )
    {
        evt->LoadFiles();
        evt->PrepareHistograms(settings_);

        // Just to be sure...
        evt->SaveEvent(pds_physics/boost::filesystem::path("Waveforms"));

        // Now do the pedestal subtraction
        evt->FillPedestals();

        double   evt_time = evt->GetParameter<double>("Properties.UnixTime");
        auto channels = evt->GetChannels();

        for(unsigned int i = 0; i <channels.size(); i++)
        {
            double * pd = channels.at(i)->GetPedestal();

            fit_status.at(i)->SetPoint(evt_counter, evt_time, pd[0]);
            fit_const.at(i)->SetPoint(evt_counter, evt_time, pd[1]);

            fit_mean.at(i)->SetPoint(evt_counter, evt_time, pd[2]);
            fit_mean.at(i)->SetPointError(evt_counter, 0.05, pd[3]);

            fit_chi2.at(i)->SetPoint(evt_counter, evt_time, pd[4]);

            fit_ndf.at(i)->SetPoint(evt_counter, evt_time, pd[5]);

            if( pd[5] !=0 ) fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, pd[4]/pd[5]);
            else fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, -1.);

            fit_pval.at(i)->SetPoint(evt_counter, evt_time,pd[6]);
            hist_pval.at(i)->Fill(pd[6]);

            hist_mean.at(i)->SetPoint(evt_counter, evt_time, pd[7]);
            hist_mean.at(i)->SetPointError(evt_counter, 0.05, pd[8]);

            hist_entries.at(i)->SetPoint(evt_counter, evt_time, pd[9]);
        }

        evt->SubtractPedestals();

        evt->SaveEvent(pds_physics/boost::filesystem::path("Waveforms"));
        evt->DeleteHistograms();

        evt_counter ++;
    }

    std::string fname = pds_physics.string() + "/run_"+std::to_string(nr_)+"_pedestal"+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(int i = 0; i < fit_status.size(); i++)
    {

        fit_status.at(i)->Write();
        delete fit_status.at(i);
        fit_const.at(i)->Write();
        delete fit_const.at(i);
        fit_mean.at(i)->Write();
        delete fit_mean.at(i);
        fit_chi2.at(i)->Write();
        delete fit_chi2.at(i);
        fit_ndf.at(i)->Write();
        delete fit_ndf.at(i);
        fit_chi2ndf.at(i)->Write();
        delete fit_chi2ndf.at(i);
        fit_pval.at(i)->Write();
        delete fit_pval.at(i);

        hist_pval.at(i)->Write();
        delete hist_pval.at(i);

        hist_mean.at(i)->Write();
        delete hist_mean.at(i);
        hist_entries.at(i)->Write();
        delete hist_entries.at(i);
    }

    rfile->Close("R");

    // for(auto &evt: evts_ )
    // {
    //     evt->SubtractPedestals();
    // }
    //
    // for(auto &evt: evts_ )
    // {
    //     evt->SaveEvent(pds_physics/boost::filesystem::path("Waveforms"));
    //     evt->DeleteHistograms();
    // }

    std::cout << "\033[32;1mRun::Subtracting physics pedestal:\033[0m done!       " << std::endl;
};

void CalibrationRun::OverShootCorrection()
{
    std::cout << "\033[33;1mRun::Correcting for Amp OverShoot:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path outfolder = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("OverShootCorrection");
    if(!boost::filesystem::is_directory( outfolder ) )
    {
        boost::filesystem::create_directory( outfolder );
    }

    if(!boost::filesystem::is_directory( outfolder/boost::filesystem::path("Waveforms")) )
    {
        boost::filesystem::create_directory( outfolder/boost::filesystem::path("Waveforms"));
    }

    std::vector<std::vector<TGraph*>> graphs;
    std::string names[15] = {"_lstart", "_lstop", "_lresult", "_start", "_stop", "_result", "_par0", "_par1", "_par2", "_chi2", "_ndf", +"_pval", "_area1", "area2", "_area_diff"};
    std::string ytitles[15] = {"Time [s]", "Time [s]", "Fit status","Time [s]", "Time [s]", "Fit status", "Slope [mV/s]", "X-shift [s]", "Y-shift [s]", "Chi2", "Ndf", "P-value", "Area [a.u.]", "Area [a.u.]", "Area [a.u.]"};

    for(auto &channel: evts_.at(0)->GetChannels() )
    {
        std::vector<TGraph*> gch;

        std::string name = channel->GetName();
        for(int i =0 ; i < 15; ++i)
        {
            TGraph * g = new TGraph();
            g->SetName( (name+names[i]).c_str() );
            g->GetYaxis()->SetTitle(ytitles[i].c_str());
            g->GetXaxis()->SetTitle("Time [s]");
            g->SetMarkerStyle(23);
            g->SetMarkerColor(kRed);
            g->SetMarkerSize(1);
            gch.push_back(g);
        }

        graphs.push_back(gch);
    }

    for(auto &evt: evts_ )
    {
        // Load the histograms & .ini file. If the event
        // did not pass the pd subtraction, just skip it!
        evt->LoadFiles(EVENTSTATE_PDSUBTRACTED);

        if( evt->GetState() ==  EVENTSTATE_PDSUBTRACTED )
        {
            // Here the actual overshoot correction is done, the rest is just
            // getting the info out.
            auto channels = evt->OverShootCorrection();

            double   evt_time = evt->GetParameter<double>("Properties.UnixTime");
            for(unsigned int i = 0; i <channels.size(); i++)
            {
                for(int j = 0; j < channels.at(i).size(); ++j)
                {
                    OverShootResult result =  channels[i][j];
                    graphs[i][0]->SetPoint(graphs[i][0]->GetN(), evt_time, result.lstart );
                    graphs[i][1]->SetPoint(graphs[i][1]->GetN(), evt_time, result.lstop );
                    graphs[i][2]->SetPoint(graphs[i][2]->GetN(), evt_time, result.lresult );
                    graphs[i][3]->SetPoint(graphs[i][3]->GetN(), evt_time, result.start );
                    graphs[i][4]->SetPoint(graphs[i][4]->GetN(), evt_time, result.stop );
                    graphs[i][5]->SetPoint(graphs[i][5]->GetN(), evt_time, result.result );
                    graphs[i][6]->SetPoint(graphs[i][6]->GetN(), evt_time, result.par0 );
                    graphs[i][7]->SetPoint(graphs[i][7]->GetN(), evt_time, result.par1 );
                    graphs[i][8]->SetPoint(graphs[i][8]->GetN(), evt_time, result.par2 );
                    graphs[i][9]->SetPoint(graphs[i][9]->GetN(), evt_time, result.chi2 );
                    graphs[i][10]->SetPoint(graphs[i][10]->GetN(), evt_time, result.ndf );
                    graphs[i][11]->SetPoint(graphs[i][11]->GetN(), evt_time, result.pval );
                    graphs[i][12]->SetPoint(graphs[i][12]->GetN(), evt_time, result.area1 );
                    graphs[i][13]->SetPoint(graphs[i][13]->GetN(), evt_time, result.area2 );
                    graphs[i][14]->SetPoint(graphs[i][14]->GetN(), evt_time, result.area1 + result.area2 );
                }
            }

            evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
        }

        evt->DeleteHistograms();
    }



    std::string fname = outfolder.string() + "/run_"+std::to_string(nr_)+"_overshootcorrection_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto &channel : graphs)
    {
        for(auto & graph : channel)
        {
            graph->Write();
            delete graph;
        }
    }

    rfile->Close("R");

    // for(auto &evt: evts_ )
    // {
    //     evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
    //     evt->DeleteHistograms();
    // }

    std::cout << "\033[32;1mRun::Correcting for Amp OverShoot:\033[0m done!       " << std::endl;
};

void CalibrationRun::SignalTagging()
{
    std::cout << "\033[33;1mRun::Signal tagging:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path outfolder = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("SignalTagging");
    if(!boost::filesystem::is_directory( outfolder ) )
    {
        boost::filesystem::create_directory( outfolder );
    }

    if(!boost::filesystem::is_directory( outfolder/boost::filesystem::path("Waveforms")) )
    {
         boost::filesystem::create_directory( outfolder/boost::filesystem::path("Waveforms"));
    }

    Gain* gain = new Gain(path_, GAINSTATE_EXTENDED);

    for(auto evt : evts_)
    {
        evt->LoadFiles(EVENTSTATE_OSCORRECTED);

        if( evt->GetState() == EVENTSTATE_OSCORRECTED )
        {
    //        evt->PrepareTagging();
            evt->SignalTagging();
            evt->FastRate( gain );
            evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
        }

        evt->DeleteHistograms();
    }

    delete gain;

    std::cout << "\033[32;1mRun::Signal tagging:\033[0m done!       " << std::endl;

    //Get the histograms and prepare them
    // auto evt_itr = evts_.begin();
    //
    // while( evt_itr != evts_.end() )
    // {
    //     (*evt_itr)->LoadFiles(EVENTSTATE_OSCORRECTED);
    //
    //     if( (*evt_itr)->GetState() == EVENTSTATE_OSFAILED )
    //     {
    //             delete (*evt_itr);
    //             (*evt_itr) = NULL;
    //             evts_.erase(evt_itr);
    //     }
    //     else
    //     {
    //             evt_itr++;
    //     }
    // }
    //
    // for(auto &evt: evts_ )
    // {
    //      evt->PrepareTagging();
    // }
    //
    // int nthreads   = GS->GetParameter<int>("General.nthreads");
    // bool parallelize = GS->GetParameter<bool>("General.parallelize");
    //
    // #pragma omp parallel for if(parallelize) num_threads(nthreads)
    // for(int i = 0; i < evts_.size(); ++i)
    // {
    //     evts_.at(i)->SignalTagging();
    // }
    //
    // Gain* gain = new Gain(path_, GAINSTATE_EXTENDED);
    //
    // for(auto &evt: evts_ )
    // {
    //     evt->FastRate( gain );
    // }
    //
    // delete gain;
    //
    // for(auto &evt: evts_ )
    // {
    //      evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
    //      evt->DeleteHistograms();
    // }
};


void CalibrationRun::WaveformDecomposition()
{
    std::cout << "\033[33;1mRun::Waveform decomposition:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path outfolder = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("WaveformDecomposition");
    if(!boost::filesystem::is_directory( outfolder ) )
    {
        boost::filesystem::create_directory( outfolder );
    }

    if(!boost::filesystem::is_directory( outfolder/boost::filesystem::path("Waveforms")) )
    {
         boost::filesystem::create_directory( outfolder/boost::filesystem::path("Waveforms"));
    }

    Gain* gain = new Gain(path_, GAINSTATE_EXTENDED);

    for(auto &evt: evts_ )
    {
        evt->LoadFiles(EVENTSTATE_TAGGED);

        if( evt->GetState() == EVENTSTATE_TAGGED )
        {
            evt->PrepareTagging();
            evt->PrepareDecomposition();

            evt->WaveformDecomposition(gain);

            evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
        }

        evt->DeleteHistograms();
    }

    delete gain;

    std::cout << "\033[32;1mRun::Waveform decomposition:\033[0m done!       " << std::endl;

    //Get the histograms and prepare them
	// for(auto &evt: evts_ )
	// {
	//      evt->LoadFiles(EVENTSTATE_TAGGED);
	// }

    // Load the histograms & .ini file. If the event
    // did not pass the pd subtraction, throw it away
    // auto evt_itr = evts_.begin();
    //
    // while( evt_itr != evts_.end() )
    // {
    //     (*evt_itr)->LoadFiles(EVENTSTATE_OSCORRECTED);
    //
    //     if( (*evt_itr)->GetState() == EVENTSTATE_OSFAILED )
    //     {
    //             delete (*evt_itr);
    //             (*evt_itr) = NULL;
    //             evts_.erase(evt_itr);
    //     }
    //     else
    //     {
    //             evt_itr++;
    //     }
    // }

    // Gain* gain = new Gain(path_, GAINSTATE_EXTENDED);

    // int nthreads   = GS->GetParameter<int>("General.nthreads");
    // bool parallelize = GS->GetParameter<bool>("General.parallelize");
    //
    // //
    // // for(auto evt = evts_.begin(); evt != evts_.end(); evt++ )
    // //for(auto evt: evts_ )
    // // #pragma omp parallel for if(parallelize) num_threads(nthreads)
    // //for(std::vector<PhysicsEvent*>::iterator evt = evts_.begin(); evt != evts_.end(); ++evt)
    //
    // // Creating the mip and reco wavforsm because Clone is not threadsafe
    // for(auto &evt: evts_)
    // {
    //     evt->PrepareDecomposition();
    // }
    //
    // #pragma omp parallel for if(parallelize) num_threads(nthreads)
    // for(int i = 0; i < evts_.size(); ++i)
    // {
    //     // Here the actual waveform decomposition is done, the rest is just
    //     // getting the info out.
    //     //auto channels = evt->WaveformDecomposition(gain);
    //     evts_.at(i)->WaveformDecomposition(gain);
    // }
    //
    // // std::string fname = overshoot.string() + "/run_"+std::to_string(nr_)+"_pedestal"+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";
    // // TFile *rfile = new TFile(fname.c_str(), "RECREATE");
    // // for(auto &channel : graphs)
    // // {
    // //     for(auto & graph : channel)
    // //     {
    // //         graph->Write();
    // //         delete graph;
    // //     }
    // // }
    // //
    // // rfile->Close("R");
    // //
    // for(auto &evt: evts_ )
    // {
    //     evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
    //     evt->DeleteHistograms();
    // }
};

void CalibrationRun::WaveformReconstruction()
{
    std::cout << "\033[33;1mRun::Waveform reconstruction:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path outfolder = path_/boost::filesystem::path("Calibration")/boost::filesystem::path("WaveformReconstruction");
    if(!boost::filesystem::is_directory( outfolder ) )
    {
        boost::filesystem::create_directory( outfolder );
    }

    if(!boost::filesystem::is_directory( outfolder/boost::filesystem::path("Waveforms")) )
    {
         boost::filesystem::create_directory( outfolder/boost::filesystem::path("Waveforms"));
    }

    //Get the histograms and prepare them
    // for(auto &evt: evts_ )
    // {
    //      evt->LoadFiles(EVENTSTATE_WFDECOMPOSED);
    // }

    // Load the histograms & .ini file. If the event
    // did not pass the pd subtraction, throw it away
    // auto evt_itr = evts_.begin();
    //
    // while( evt_itr != evts_.end() )
    // {
    //     (*evt_itr)->LoadFiles(EVENTSTATE_OSCORRECTED);
    //
    //     if( (*evt_itr)->GetState() == EVENTSTATE_OSFAILED )
    //     {
    //             delete (*evt_itr);
    //             (*evt_itr) = NULL;
    //             evts_.erase(evt_itr);
    //     }
    //     else
    //     {
    //             evt_itr++;
    //     }
    // }

    Gain* gain = new Gain(path_, GAINSTATE_EXTENDED);

    int nthreads   = GS->GetParameter<int>("General.nthreads");
    bool parallelize = GS->GetParameter<bool>("General.parallelize");
    //
    // #pragma omp parallel for if(parallelize) num_threads(nthreads)
    // for(int i = 0; i < evts_.size(); ++i)
    for( auto evt : evts_)
    {
        // Here the actual waveform decomposition is done, the rest is just
        // getting the info out.
        //auto channels = evt->WaveformDecomposition(gain);
        evt->LoadFiles(EVENTSTATE_WFDECOMPOSED);

        if( evt->GetState() == EVENTSTATE_WFDECOMPOSED )
        {
            evt->WaveformReconstruction(gain);
            evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
        }

        evt->DeleteHistograms();
    }

    std::vector<std::vector<TGraph*>> graphs;
    std::string names[5] = {"_nbins", "_binerror", "_chi2", "_pval", "_chi2ndf"};
    std::string ytitles[5] = {"Ndf", "BinError", "Chi2", "P-Value","Chi2/Ndf"};

    for(auto &channel: evts_.at(0)->GetChannels() )
    {
        std::vector<TGraph*> gch;

        std::string name = channel->GetName();

        for(int i =0 ; i < 5; ++i)
        {
            TGraph * g = new TGraph();
            g->SetName( (name+names[i]).c_str() );
            g->GetYaxis()->SetTitle(ytitles[i].c_str());
            g->GetXaxis()->SetTitle("Time [s]");
            g->SetMarkerStyle(23);
            g->SetMarkerColor(kRed);
            g->SetMarkerSize(1);
            gch.push_back(g);
        }

        graphs.push_back(gch);
    }

    for(auto &evt: evts_ )
    {
        // Here the actual overshoot correction is done, the rest is just
        // getting the info out.
        std::vector<std::vector<double>> results = evt->GetReconstruction();

        double   evt_time = evt->GetParameter<double>("Properties.UnixTime");

        for(unsigned int i = 0; i < results.size(); ++i)
        {
            std::vector<double> res = results[i];
            for(unsigned int j = 0; j < 4; ++j)
            {
                TGraph* graph = graphs.at(i).at(j);
                graph->SetPoint(graph->GetN(), evt_time, res[j]);
            }

            TGraph* graph = graphs.at(i).at(4);

            graph->SetPoint(graph->GetN(), evt_time, res[2]/res[0]);
        }
    }

    std::string fname = outfolder.string() + "/run_"+std::to_string(nr_)+"_wfreco"+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto &channel : graphs)
    {
        for(auto & graph : channel)
        {
            graph->Write();
            delete graph;
        }
    }

    rfile->Close("R");


    //
    // for(auto &evt: evts_ )
    // {
    //     evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
    //     evt->DeleteHistograms();
    // }

    delete gain;

    std::cout << "\033[32;1mRun::Waveform reconstruction:\033[0m done!       " << std::endl;
};


void CalibrationRun::DeleteCalibrationHistograms()
{
    for(auto evt: cal_evts_ )
    {
        evt->DeleteHistograms();
    }
};

void CalibrationRun::MipTimeRetrieval()
{
    std::cout << "\033[33;1mRun::MIP time retrieval:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path outfolder = path_/boost::filesystem::path("Results");
    if(!boost::filesystem::is_directory( outfolder ) )
    {
        boost::filesystem::create_directory( outfolder );
    }

    if(!boost::filesystem::is_directory( outfolder/boost::filesystem::path("Waveforms")) )
    {
         boost::filesystem::create_directory( outfolder/boost::filesystem::path("Waveforms"));
    }

    //Get the histograms and prepare them
    // for(auto &evt: evts_ )
    // {
    //      evt->LoadFiles(EVENTSTATE_WFRECONSTRUCTED);
    // }
    //
    // for(auto &evt: evts_ )
    // {
    //      evt->PrepareRetrieval();
    // }

    std::vector<std::vector<TGraph*>> graphs;
    std::string names[3] = { "_online_rate", "_fast_rate", "_rate"};

    for(auto &channel: evts_.at(0)->GetChannels() )
    {
        std::vector<TGraph*> gch;

        std::string name = channel->GetName();

        for(int i = 0 ; i < 3; ++i)
        {
            TGraph * g = new TGraph();
            g->SetName( (name+names[i]).c_str() );
            g->GetYaxis()->SetTitle( "Particle rate [MIPs/s]");
            g->GetXaxis()->SetTitle("Time [s]");
            g->SetMarkerStyle(23);
            g->SetMarkerColor(kRed);
            g->SetMarkerSize(1);
            gch.push_back(g);
        }

        graphs.push_back(gch);
    }

    //Fake BWD
    vector<string> channels = {"BWD1","BWD2", "BWD3"};

    for(auto &channel: channels )
    {
         std::vector<TGraph*> gch;

         for(int i =0 ; i < 3; ++i)
         {
            TGraph * g = new TGraph();
            g->SetName( (channel + names[i]).c_str() );
            g->GetYaxis()->SetTitle( "Particle rate [MIPs/s]");
            g->GetXaxis()->SetTitle("Time [s]");
            g->SetMarkerStyle(23);
            g->SetMarkerColor(kRed);
            g->SetMarkerSize(1);
            gch.push_back(g);
        }

        graphs.push_back(gch);
    }

    // int nthreads   = GS->GetParameter<int>("General.nthreads");
    // bool parallelize = GS->GetParameter<bool>("General.parallelize");
    //
    // #pragma omp parallel for if(parallelize) num_threads(nthreads)
    // for(int i = 0; i < evts_.size(); ++i)
    for( auto evt : evts_)
    {
        // Here the actual waveform decomposition is done, the rest is just
        // getting the info out.

        evt->LoadFiles(EVENTSTATE_WFRECONSTRUCTED);

        if( evt->GetState() == EVENTSTATE_WFRECONSTRUCTED )
        {
            evt->PrepareRetrieval();
            evt->MipTimeRetrieval();
            evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );

            vector<vector<double>> rates = evt->GetRates();
            double   evt_time = evt->GetParameter<double>("Properties.UnixTime");
            for(unsigned int i = 0; i < rates.size(); ++i)
            {
                std::vector<double> ch_rate = rates.at(i);
                for(unsigned int j = 0; j < 3; ++j)
                {
                    TGraph* graph = graphs.at(i).at(j);
                    graph->SetPoint( graph->GetN(), evt_time, ch_rate.at(j) );
                }
            }
        }

        evt->DeleteHistograms();
    }

    // for(auto &evt: evts_ )
    // {
    //
    //     vector<vector<double>> rates = evt->GetRates();
    //
    //     double   evt_time = evt->GetParameter<double>("Properties.UnixTime");
    //
    //     for(unsigned int i = 0; i < rates.size(); ++i)
    //     {
    //         std::vector<double> ch_rate = rates.at(i);
    //         for(unsigned int j = 0; j < 3; ++j)
    //         {
    //             TGraph* graph = graphs.at(i).at(j);
    //             graph->SetPoint( graph->GetN(), evt_time, ch_rate.at(j) );
    //         }
    //     }
    // }

    std::string fname = outfolder.string() + "/run_"+std::to_string(nr_)+"_rate"+"_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";
    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto &channel : graphs)
    {
        for(auto & graph : channel)
        {
            graph->Write();
            delete graph;
        }
    }

    rfile->Close("R");

    //
    //
    // for(auto &evt: evts_ )
    // {
    //      evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
    //      evt->DeleteHistograms();
    // }

    std::cout << "\033[32;1mRun::MIP time retrieval:\033[0m done!       " << std::endl;
};

void CalibrationRun::SystematicsStudy()
{
    std::cout << "\033[33;1mRun::Systematics study:\033[0m running" << "\r" << std::flush;

    boost::filesystem::path outfolder = path_/boost::filesystem::path("Results")/boost::filesystem::path("SystematicsStudy");
    if(!boost::filesystem::is_directory( outfolder ) )
    {
        boost::filesystem::create_directory( outfolder );
    }

    //Get the histograms and prepare them
    // for(auto &evt: evts_ )
    // {
    //      evt->LoadFiles(EVENTSTATE_CALIBRATED);
    // }

    std::vector<TH1F*> hists;
    std::string names[3] = { "_online_rate", "_fast_rate", "_rate"};

    for(auto &channel: evts_.at(0)->GetChannels() )
    {
        string title = channel->GetName() + "_pe_per_event";
        int nbinsx   = GS->GetParameter<int>("SystematicsStudy.nbinsx_pe");
        double xlow  = - 0.5;
        double xup   = nbinsx - 0.5;

        hists.push_back( new TH1F(title.c_str(), title.c_str(), nbinsx, xlow, xup) );
    }

    string title = "time_resolution";
    int nbinsx   = GS->GetParameter<int>("SystematicsStudy.nbinsx_time");
    // Allways have an even number of bins because of bin at 0
    if( nbinsx % 2 == 0 ) ++nbinsx;

    double dt = GS->GetParameter<double>("Scope.delta_t");

    double xlow  = -dt * (nbinsx/2 + 0.5);
    double xup   =  dt * (nbinsx/2 + 0.5);

    hists.push_back( new TH1F(title.c_str(), title.c_str(), nbinsx, xlow, xup) );

    double threshold_mpv     = GS->GetParameter<double>("SystematicsStudy.threshold_mpv");
    double window_length_mpv = GS->GetParameter<double>("SystematicsStudy.window_length_mpv");
    double start_mpv         = GS->GetParameter<double>("SystematicsStudy.start_mpv");
    double threshold_tres    = GS->GetParameter<double>("SystematicsStudy.threshold_tres");

    for(auto & evt: evts_)
    {
        evt->LoadFiles(EVENTSTATE_CALIBRATED);

        if( evt->GetState() == EVENTSTATE_CALIBRATED)
        {
            for(int i = 0; i < evt->GetChannels().size(); ++i)
            {
                TH1I* pewf = dynamic_cast<TH1I*>(evt->GetChannels().at(i)->GetHistogram("pe"));

                double integral = 0;

                for(int j = start_mpv; j <= window_length_mpv + start_mpv; ++j)
                {
                    if( pewf->GetBinContent(j) >= threshold_mpv )
                    {
                        integral += pewf->GetBinContent(j);
                    }
                }

                hists.at(i)->Fill( integral );
            }

            TH1F* mipwf_fwd2 = dynamic_cast<TH1F*>(evt->GetChannels().at(1)->GetHistogram("mip"));

            double t1 = -1;

            for( int i = 1; i <= mipwf_fwd2->GetNbinsX(); ++i)
            {
                if( mipwf_fwd2->GetBinContent(i) >= threshold_tres )
                {
                    t1 = mipwf_fwd2->GetBinCenter(i);
                    break;
                }
            }

            TH1F* mipwf_fwd3 = dynamic_cast<TH1F*>(evt->GetChannels().at(2)->GetHistogram("mip"));

            double t2 = -1;

            for( int i = 1; i <= mipwf_fwd3->GetNbinsX(); ++i)
            {
                if( mipwf_fwd3->GetBinContent(i) >= threshold_tres )
                {
                    t2 = mipwf_fwd3->GetBinCenter(i);
                    break;
                }
            }

            if( t1 >= 0 && t2 >= 0)
            {
                hists.back()->Fill(t1-t2);
                // if( fabs(t1-t2) > 2e-9)
                // {
                //     cout << "Event number: " << evt->GetNumber() << ", t1: " << t1 << "t2: " << t2 << ", diff: " << (t1-t2)/(1e-9) << endl;
                // }
            }

            else hists.back()->Fill(-625*dt);
        }

        evt->DeleteHistograms();
    }

    // Now fit the pe hists with a langaus
    for(int i = 0; i < evts_.at(0)->GetChannels().size(); ++i )
    {
        string funcname = hists.at(i)->GetName();
        funcname += "langaus";
        double rlow = hists.at(i)->GetBinLowEdge(2);
        //double rup = hists.at(i)->GetBinLowEdge( hists.at(i)->GetNbinsX() ) + hists.at(i)->GetBinWidth(2);
        double rup = hists.at(i)->GetMean()*3.0;

        // Shit basically stolen from the root example:
        // https://root.cern.ch/root/html/tutorials/fit/langaus.C.html
        TF1 *langaus = new TF1(funcname.c_str(),langaufun, rlow, rup,4);

        double par1 = hists.at(i)->GetBinCenter(hists.at(i)->GetMaximumBin());

        langaus->SetParameters(1.25, par1, 225., 3.0);
        langaus->SetParNames("Width","MP","Area","GSigma");

        Double_t pllo[4], plhi[4];

        pllo[0]=0.25; pllo[1]=1.0;   pllo[2]=1.0; pllo[3]=0.1;
        plhi[0]=10.0; plhi[1]=100.0; plhi[2]=1000000.0; plhi[3]=20.0;

        for ( int j = 0; j < 4 ; ++j)
        {
            langaus->SetParLimits(j, pllo[j], plhi[j]);
        }

        hists.at(i)->Fit(langaus, "QRSL");

        delete langaus;
    }

    // Fit the time resolution with a gaussian
//    TF1* time_res =new TF1("gaus","gaus",1,3, TF1::EAddToList::kNo);
    //TF1* time_res =new TF1("gaus","gaus(0)+gaus(3)",1,3, TF1::EAddToList::kNo);
    TF1* time_res =new TF1("gaus","[0]*exp(-0.5*((x-[1])/[2])**2) + [3]*exp(-0.5*((x-[1])/[4])**2)",1,3, TF1::EAddToList::kNo);

    time_res->SetParameter(0, 500);
    time_res->SetParameter(1, 0);
    time_res->SetParameter(2, dt);

    time_res->SetParameter(3, 50);
    //time_res->SetParameter(4, 0);
    time_res->SetParameter(4, dt*4);

    double low = -dt*GS->GetParameter<double>("SystematicsStudy.range_time");
    double up  = - low;

    hists.back()->Fit(time_res, "QSL","", low, up);

    delete time_res;


    // Now save the hists to file to be sure
    std::string fname = outfolder.string() + "/run_"+std::to_string(nr_)+"_systematics_"+ GS->GetParameter<std::string>("General.CalibrationVersion")+".root";

    TFile *rfile = new TFile(fname.c_str(), "RECREATE");

    for(auto &hist : hists)
    {
            hist->Write();
            delete hist;
    }

    rfile->Close("R");


    // for(auto &evt: evts_ )
    // {
    //      evt->DeleteHistograms();
    // }

    std::cout << "\033[32;1mRun::Systematics study:\033[0m done!       " << std::endl;
};

//----------------------------------------------------------------------------------------------
// Definition of the AnalysisRun class.
//----------------------------------------------------------------------------------------------

// AnalysisRun::AnalysisRun(boost::filesystem::path p) : Run(p)
// {
//      // std::ofstream hendrik_file("/home/iwsatlas1/mgabriel/Plots/forHendyDany.txt", ios::app);
//      // hendrik_file << run_nr_str_;
//      // hendrik_file.close();
// //    claws::print_local_time();
// };
//
// AnalysisRun::~AnalysisRun()
// {
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              delete events_.at(i);
//              events_.at(i) = NULL;
//      }
// };
//
// void AnalysisRun::SynchronizeFiles()
// {
//      /*
//         This method uses the path to the run folder to determine the number of events.
//         Afterwards it checks if all the files for all the events do exist. Finally it
//         creates one EventClass object for each physics and each intermedieate event with
//         the paths to all the files as a parameter => SynchronizeFiles().
//       */
//
//      // cout << "-------------------------------------------------------"<< endl;
//      cout << "\033[33;1mRun::Synchronizing run:\033[0m running" << "\r" << std::flush;
//
//      // Check if the converted root, data & intermediate folders are available.
//      if( !boost::filesystem::is_directory(path_run_) )
//      {
//              cout << "Run folder does not exits: " <<  path_run_.string()<< std::endl;
//              exit(-1);
//      }
//
//      if( !boost::filesystem::is_directory(path_run_/path("Results")) )
//      {
//              cout << "Results folder does not exits:" << path_run_/path("Results").string()<< endl;
//              exit(-1);
//      }
//
//      if( boost::filesystem::is_empty(path_run_/path("Results")) )
//      {
//              cout << "Results folder is empty:" << path_run_/path("Results").string()<< endl;
//              exit(-1);
//      }
//
//      path path_data = path_run_ / path("Results");
//
//      vector<path> folder_content;
//      copy(directory_iterator(path_data), directory_iterator(), back_inserter(folder_content));
//      std::sort(folder_content.begin(),folder_content.end());
//
// //    #pragma omp parallel num_threads(7)
// //    {
// //       #pragma omp for ordered schedule(dynamic,1)
//      for ( unsigned int i=0; i < folder_content.size(); ++i)
//      {
//              //     //claws::ProgressBar((itr - folder_content.begin()+1.)/(folder_content.end()-folder_content.begin()));
//              if(    boost::filesystem::is_regular_file(folder_content.at(i))
//                     && starts_with(folder_content.at(i).filename().string(), "event_")
//                     && ends_with(folder_content.at(i).filename().string(), "_mip.root"))
//              {
//                      // Get the paths to the .root file of the event.
//                      path path_file_root = folder_content.at(i);
//                      // cout << "Loading file: " << path_file_root.string() << endl;
//                      // Get the path to the .ini file.
//                      string tmp          = folder_content.at(i).filename().string();
//                      replace_last( tmp, "_mip.root", ".ini");
//                      path path_file_ini  = path_data / path(tmp);
//
//                      // Check if the .ini & exist for the event.
//                      if( boost::filesystem::exists( path_file_ini) )
//                      {
//                              events_.push_back(new AnalysisEvent(path_file_root, path_file_ini));
//                      }
//                      else{
//                              //TODO put in some mechanism in case the ini or the online rate files do not exist.
//                      }
//              }
//      }
//
//      cout << "\033[32;1mRun::Synchronizing run:\033[0m done!   " << "\r" << std::endl;
//
// };
//
// void AnalysisRun::LoadMetaData()
// {
// //	#pragma omp parrallel for num_threads(7)
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadIniFile();
//              double ts = events_.at(i)->GetUnixtime();
//              //#pragma omp critical
//              {
//                      if(tsMin > ts ) tsMin = ts;
//                      if(tsMax < ts ) tsMax = ts;
//              }
//      }
//
//      this->LoadRunSettings();
//      if(settings_.get<int>("Scope-1-Acquisition-Settings.preTriggerSamples") != 0 || settings_.get<int>("Scope-2-Acquisition-Settings.preTriggerSamples") != 0 )
//      {
//              std::cout << "Warning preTriggerSamples!!!" << std::endl;
//              assert(1);
//      }
// };
//
// void AnalysisRun::DeleteEvent(int nr)
// {
//      auto itr_vec = std::begin(events_);
//
//      while(itr_vec != std::end(events_))
//      {
//              if( (*itr_vec)->GetNr() == nr)
//              {
//                      delete (*itr_vec);
//                      (*itr_vec) = NULL;
//                      events_.erase(itr_vec);
//              }
//              else
//              {
//                      itr_vec++;
//              }
//      }
// };
//
// void AnalysisRun::EraseElement(std::vector<AnalysisEvent*>::iterator itr_vec)
// {
//      delete (*itr_vec);
//      (*itr_vec) = NULL;
//      events_.erase(itr_vec);
// };
//
// void AnalysisRun::SetCurrentLimit(std::string ring, double low, double high)
// {
//      auto itr_vec = std::begin(events_);
//
//      while(itr_vec != std::end(events_))
//      {
//              auto current = (*itr_vec)->GetCurrent();
//
//              if( ring == "LER" )
//              {
//                      if(    std::get<0>(current) < low
//                             || std::get<0>(current) > high )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//              else if( ring == "HER" )
//              {
//                      if(    std::get<1>(current) < low
//                             || std::get<1>(current) > high )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//      }
// };
//
// void AnalysisRun::SetInjectionLimit(int type)
// {
//      if(type == -1 )
//      {
//              return;
//      }
//
//      auto itr_vec = events_.begin();
//
//      while(itr_vec != events_.end())
//      {
//              auto injection = (*itr_vec)->GetInjection();
//
//              if(type == 0)
//              {
//                      if(    std::get<0>(injection) == true
//                             || std::get<2>(injection) == true )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//              else if(type == 1)
//              {
//                      if(    std::get<0>(injection) == false
//                             && std::get<2>(injection) == false )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//              else if(type == 2)
//              {
//                      if(    std::get<0>(injection) == false
//                             || std::get<2>(injection) == true )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//              else if(type == 3)
//              {
//                      if(    std::get<0>(injection) == true
//                             || std::get<2>(injection) == false )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//              else if(type == 3)
//              {
//                      if(    std::get<0>(injection) == false
//                             || std::get<2>(injection) == false )
//                      {
//                              delete (*itr_vec);
//                              (*itr_vec) = NULL;
//                              events_.erase(itr_vec);
//                      }
//                      else
//                      {
//                              itr_vec++;
//                      }
//              }
//              else
//              {
//                      itr_vec++;
//              }
//      }
//
// };
//
// void AnalysisRun::LoadPhysicsData()
// {
//      // Method to load all the information that is located in the data_root folder with
//      // following steps:
//      //      1. Look into the data_root folder and create a PhysicalEvent object for
//      //         each event.root file & check if file for online particle rate and
//      //         do exist
//      //      2. Loop through all objects in events_ and load the raw data from the
//      //         corresponding root file
//      //      3. Loop through all objects in events_ and load the oneline monitor
//      //         pacout << "Loading Raw Data:  " << run_nr_ << endl;
//      // Look into the data folder of the run and get a list/vector of all the events inside
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadRootFile();
//      }
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadWaveform();
//              events_.at(i)->DeleteHistograms();
//      }
//
// };
//
// std::vector<AnalysisEvent*> AnalysisRun::GetEvents()
// {
//      return events_;
// }
//
// int AnalysisRun::NEvents()
// {
//      return events_.size();
// };





// void CalibrationRun::LoadData()
// {
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//      std::cout << "\033[33;1mRun::Loading data:\033[0m running" << "\r" << std::flush;
//
// //    std::cout << "Loading data:  " << run_nr_ << "\r" << std::flush;
//
//      this->LoadIntermediate();
//      this->LoadMetaData();
// //    this->LoadPhysicsData();
//      // this->LoadEventFiles();
//      //
//      // this->LoadWaveforms();
// //    this->LoadRunSettings();
//
//      std::cout << "\033[32;1mRun::Loading data:\033[0m done!   " << "\r" << std::endl;
// //    std::cout << "Loading data done!                  " << std::endl;
//
//
//      // if(!boost::filesystem::is_directory(path_run_/boost::filesystem::path("Calibration/raw")) )
//      // {
//      //     boost::filesystem::create_directory(path_run_/boost::filesystem::path("Calibration/raw"));
//      // }
//      //
//      // boost::filesystem::path fname = path_run_.string()/boost::filesystem::path("/Calibration/raw/Run-"+run_nr_str_+"_raw.root");
//      // this->SaveEvents(fname);
//
//      // for(auto &e : events_)
//      // {
//      //
//      //     std::cout<< "Event: "<<e->GetNrStr()<< std::endl;
//      // }
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// };
//
// void CalibrationRun::LoadIntermediate()
// {
//      for(unsigned int i=0; i< int_events_.size(); i++)
//      {
//              int_events_.at(i)->LoadRootFile();
//      }
//
// //	#pragma omp parallel num_threads(7)
//      {
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< int_events_.size(); i++)
//              {
//                      int_events_.at(i)->LoadIniFile();
//                      int_events_.at(i)->LoadWaveform();
//                      //    int_events_.at(i)->DeleteHistograms();
//              }
//      }
// };
//
// void CalibrationRun::LoadMetaData()
// {
// //    #pragma omp parallel num_threads(7)
// //    {
// //        #pragma omp for schedule(dynamic,1)
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadIniFile();
//              events_.at(i)->LoadOnlineRate();
//              double ts = events_.at(i)->GetUnixtime();
//              if(tsMin > ts ) tsMin = ts;
//              if(tsMax < ts ) tsMax = ts;
//      }
// //    }
//      this->LoadRunSettings();
// };

// void CalibrationRun::LoadRunSettings()
// {
//      // path ini_file  = path_run_ / ("Run-" + to_string(run_nr_) + "-Settings.ini");
//      //
//      // if( boost::filesystem::is_regular_file(ini_file) && exists(ini_file) )
//      // {
//      //     property_tree::ini_parser::read_ini(ini_file.string(), settings_);
//      // }
//      this->Run::LoadRunSettings();
//
//      // Data taking in the phyics events has been conducted with an vertical offset.
//      std::string ch[8]       = {"FWD1", "FWD2", "FWD3", "FWD4", "BWD1", "BWD2", "BWD3", "BWD4"};
//      std::string sections[8] = {"Scope-1-Channel-Settings-A", "Scope-1-Channel-Settings-B", "Scope-1-Channel-Settings-C", "Scope-1-Channel-Settings-D",
//                                 "Scope-2-Channel-Settings-A", "Scope-2-Channel-Settings-B", "Scope-2-Channel-Settings-C", "Scope-2-Channel-Settings-D"};
//
//      map<std::string, float> baseline;
//
//      for(unsigned i=0; i<8; i++)
//      {
//              int offset = claws::ConvertOffset(settings_.get<double>(sections[i]+".AnalogOffset"), settings_.get<int>(sections[i]+".Range"));
//              baseline[ch[i]] = offset;
//      }
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->SetBaseline(baseline);
//
//      }
// };
//
// void CalibrationRun::LoadPhysicsData()
// {
//      // Method to load all the information that is located in the data_root folder with
//      // following steps:
//      //      1. Look into the data_root folder and create a PhysicalEvent object for
//      //         each event.root file & check if file for online particle rate and
//      //         do exist
//      //      2. Loop through all objects in events_ and load the raw data from the
//      //         corresponding root file
//      //      3. Loop through all objects in events_ and load the oneline monitor
//      //         pacout << "Loading Raw Data:  " << run_nr_ << endl;
//      // Look into the data folder of the run and get a list/vector of all the events inside
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadRootFile();
//      }
//
// //	#pragma omp parallel num_threads(7)
//      {
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< events_.size(); i++)
//              {
//                      events_.at(i)->LoadWaveform();
//                      events_.at(i)->DeleteHistograms();
//              }
//      }
// }
//
// void CalibrationRun::LoadEventFiles()
// {
//
//      // Legacy Methode - Use is deprechiated!
//
//      // Method to load all the information that is located in the data_root folder with
//      // following steps:
//      //      1. Look into the data_root folder and create a PhysicalEvent object for
//      //         each event.root file & check if file for online particle rate and
//      //         do exist
//      //      2. Loop through all objects in events_ and load the raw data from the
//      //         corresponding root file
//      //      3. Loop through all objects in events_ and load the oneline monitor
//      //         pacout << "Loading Raw Data:  " << run_nr_ << endl;
//      // Look into the data folder of the run and get a list/vector of all the events inside
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadRootFile();
//              events_.at(i)->LoadWaveform();
//              events_.at(i)->DeleteHistograms();
//      }
// };
//
//
//
// void CalibrationRun::LoadWaveforms()
// {
//
//      // Legacy Methode - Use is deprechiated!
//
//      // cout<<"Run::LoadWaveforms" <<endl;
//      // double wall0 = claws::get_wall_time();
//      // double cpu0  = claws::get_cpu_time();
//
//
// //    #pragma omp parallel num_threads(7)
//      //  {
//      // When all the histograms are copyed into memory and than the the vectors are filled in a second (multi threaded) step, a
//      // full run would use more than my full r
//      // #pragma omp for schedule(dynamic,1)
//      // for(unsigned int i=0; i< events_.size();i++)
//      // {
//      //     events_.at(i)->LoadWaveform();
//      //     events_.at(i)->DeleteHistograms();
//      // }
//      //    #pragma omp for schedule(dynamic,1)
//      for(unsigned int i=0; i< int_events_.size(); i++)
//      {
//              int_events_.at(i)->LoadWaveform();
//              int_events_.at(i)->DeleteHistograms();
//      }
//
//      //}
//
//      // double wall1 = claws::get_wall_time();
//      // double cpu1  = claws::get_cpu_time();
//      //
//      // cout << "Wall Time = " << wall1 - wall0 << endl;
//      // cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// };
//
//
//
// void CalibrationRun::SubtractPedestal()
// {
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//      std::cout << "\033[33;1mRun::Subtracting pedestal:\033[0m running" << "\r" << std::flush;
//
//      this->LoadPedestal();
//      this->SavePedestal();
//      this->Subtract();
//
//      std::cout << "\033[32;1mRun::Subtracting pedestal:\033[0m done!       " << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// };
//
// void CalibrationRun::LoadPedestal()
// {
//      /*
//          TODO description
//       */
//
// //	#pragma omp parallel num_threads(7)
//      {
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< events_.size(); i++)
//              {
//                      events_.at(i)->LoadPedestal();
//              }
//
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< int_events_.size(); i++)
//              {
//                      int_events_.at(i)->LoadPedestal();
//              }
//
//      }
//
//      // There is of course only one object of class Pedestal (pedestal_) all events need to access => no multi threading.
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              pedestal_->AddEvent(events_.at(i)->GetPedestal());
//      }
//
//      for(unsigned int i=0; i< int_events_.size(); i++)
//      {
//              pedestal_->AddEvent(int_events_.at(i)->GetPedestal());
//      }
//
// };
// void CalibrationRun::CalculatePedestal()
// {
//
//      // ped_.clear();
//      // ped_int_.clear();
//      //
//      // for(auto & itr : h_ped_)
//      // {
//      //     string name = to_string(run_nr_) +"_"+ itr.first +"_pd_fit";
//      //
//      //     string section;
//      //     if(itr.first == "FWD1")          section = "Scope-1-Channel-Settings-A";
//      //     else if(itr.first == "FWD2")     section = "Scope-1-Channel-Settings-B";
//      //     else if(itr.first == "FWD3")     section = "Scope-1-Channel-Settings-C";
//      //     else if(itr.first == "FWD4")     section = "Scope-1-Channel-Settings-D";
//      //
//      //     else if(itr.first == "BWD1")     section = "Scope-2-Channel-Settings-A";
//      //     else if(itr.first == "BWD2")   GetChannel
//      //     int offset = claws::ConvertOffset(settings_.get<double>(section+".AnalogOffset"), settings_.get<int>(section+".Range"));
//      //
//      //     // TODO Check if a gaussain really is the best option to get the pedestral. A center of gravity might work better.
//      //
//      //     TF1 *fit = new TF1(name.c_str(), "gaus" , offset-5 , offset +5 );
//      //     fit->SetParameter(1, offset);
//      //     itr.second->Fit(fit, "RQ0");
//      //
//      //     double constant = itr.second->GetFunction(name.c_str())->GetParameter(0);
//      //     double mean     = itr.second->GetFunction(name.c_str())->GetParameter(1);
//      //     double sigma    = itr.second->GetFunction(name.c_str())->GetParameter(2);
//      //
//      //     fit->SetParameters(constant, mean, sigma);
//      //     fit->SetRange(mean - 3*sigma, mean +3*sigma);
//      //
//      //     itr.second->Fit(fit, "R0");
//      //     const Int_t kNotDraw = 1<<9;
//      //     itr.second->GetFunction(name.c_str())->ResetBit(kNotDraw);
//      //
//      //     ped_[itr.first] = itr.second->GetFunction(name.c_str())->GetParameter(1);
//      //
//      // }
//      //
//      // for(auto & itr : h_ped_int_)
//      // {
//      //     string name = to_string(run_nr_) +"_"+ itr.first +"_pd_fit";
//      //
//      //     // TODO Check if a gaussain really is the best option to get the pedestral. A center of gravity might work better.
//      //     TF1 *fit = new TF1(name.c_str(), "gaus" , -5 , 5 );
//      //     fit->SetParameter(1, 0);
//      //     itr.second->Fit(fit, "R0");
//      //     const Int_t kNotDraw = 1<<9;
//      //     itr.second->GetFunction(name.c_str())->ResetBit(kNotDraw);
//      //     ped_int_[itr.first] = itr.second->GetFunction(name.c_str())->GetParameter(1);
//      //
//      // }
//      // pedestal_->CalculatePedestal();
// };
//
// void CalibrationRun::SavePedestal()
// {
//      /*
//          TODO description
//       */
//      if(!boost::filesystem::is_directory(path_run_/path("Calibration")) )
//      {
//              boost::filesystem::create_directory(path_run_/path("Calibration"));
//      }
//
//      std::string filename = path_run_.string()+"/Calibration/run-"+run_nr_str_+"_pedestal_subtraction"+"_v"+ std::to_string(GS->GetCaliPar<int>("General.CalibrationVersion"))+".root";
//      TFile *rfile = new TFile(filename.c_str(), "RECREATE");
//
//      pedestal_->SavePedestal(rfile);
//
//      rfile->Close();
//      delete rfile;
// };
//
// void CalibrationRun::Subtract()
// {
//      /*
//          TODO description
//       */
// //	#pragma omp parallel num_threads(7)
//      {
//              std::map<std::string, float> tmp = pedestal_->GetPedestal(1);
//
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< events_.size(); i++)
//              {
//                      events_.at(i)->SubtractPedestal(tmp);
//              }
//      }
//
// //	#pragma omp parallel num_threads(7)
//      {
//              std::map<std::string, float> tmp_int = pedestal_->GetPedestal(2);
//
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< int_events_.size(); i++)
//              {
//                      int_events_.at(i)->SubtractPedestal(tmp_int, true);
//              }
//      }
// };
//
// void CalibrationRun::SubtractPedestal2()
// {
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//      std::cout << "\033[33;1mRun::Subtracting pedestal:\033[0m running" << "\r" << std::flush;
//
//      // First intermediate
//
// //	#pragma omp parallel num_threads(7)
//      {
// //		#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< int_events_.size(); i++)
//              {
//                      int_events_.at(i)->LoadPedestal();
//              }
//      }
//
//      for(unsigned int i=0; i< int_events_.size(); i++)
//      {
//              pedestal_->AddEvent(int_events_.at(i)->GetPedestal());
//      }
//
// //	#pragma omp parallel num_threads(7)
//      {
//              std::map<std::string, float> tmp_int = pedestal_->GetPedestal(2);
//
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< int_events_.size(); i++)
//              {
//                      int_events_.at(i)->SubtractPedestal(tmp_int, true);
//              }
//      }
//
//      // Now Physics
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadRootFile();
//              events_.at(i)->LoadWaveform();
//
//              events_.at(i)->LoadPedestal();
//
//              pedestal_->AddEvent(events_.at(i)->GetPedestal());
//
//              events_.at(i)->DeleteHistograms();
//              events_.at(i)->DeleteWaveforms();
//      }
//
// //    #pragma omp parallel num_threads(7)
// //    {
//      std::map<std::string, float> tmp = pedestal_->GetPedestal(1);
//
// //        #pragma omp for schedule(dynamic,1)
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->SetPedestal(tmp);
//      }
// //    }
//
//      this->SavePedestal();
//
//      std::cout << "\033[32;1mRun::Subtracting pedestal:\033[0m done!       " << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// };
//
//
//
// void CalibrationRun::DeletePhysicsData()
// {
//      //#pragma omp parallel num_threads(GS->GetNThreads())
//      {
//              //	#pragma omp for schedule(dynamic,1)
//              for(unsigned int i=0; i< events_.size(); i++)
//              {
//                      events_.at(i)->DeleteHistograms();
//                      events_.at(i)->DeleteWaveforms();
//              }
//      }
// };
//
// void CalibrationRun::GainCalibration()
// {
//      /*
//          TODO description
//       */
//
//      std::cout << "\033[33;1mRun::Calibrating gain:\033[0m running" << "\r" << std::flush;
//
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//      for(unsigned int i=0; i< int_events_.size(); i++)
//      {
//              int_events_.at(i)->CalculateIntegral();
//              gain_->AddValue(int_events_.at(i)->GetIntegral());
//      }
//
//      gain_->FitGain();
//      gain_->SaveGain(path_run_);
//
//
//      std::cout << "\033[32;1mRun::Calibrating gain:\033[0m done!     " << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// }
//

//
// void CalibrationRun::WaveformDecomposition()
// {
//      std::cout << "\033[33;1mRun::Decomposing waveforms:\033[0m running" << "\r" << std::flush;
//
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//      // this->SetUpWaveforms();
//      // this->FastRate();
// //    this->SaveEvents();
//
//      // New Version
//
//      std::map<std::string, std::vector<float>*> avg_waveforms = gain_->GetWaveform();
//      std::map<std::string, double> pe_to_mips = GS->GetPEtoMIPs();
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadRootFile();
//              events_.at(i)->LoadWaveform();
//
//              events_.at(i)->SetUpWaveforms();
//
//              events_.at(i)->FastRate(avg_waveforms, pe_to_mips);
//              events_.at(i)->Rate(pe_to_mips);
//              //  events_.at(i)->Decompose(avg_waveforms);
//              //  events_.at(i)->Reconstruct(avg_waveforms);
//              //  events_.at(i)->CalculateChi2();
//              //
//              // events_.at(i)->SaveEvent(path_run_/boost::filesystem::path("ResultsRoot"));
//
//              events_.at(i)->DeleteHistograms();
//              events_.at(i)->DeleteWaveforms();
//      }
//
//
//
//      std::cout << "\033[32;1mRun::Decomposing waveforms:\033[0m done!     " << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
//
//      // wall0 = claws::get_wall_time();
//      // cpu0  = claws::get_cpu_time();
//      //
//      //
//      //
//      //
//      // // std::cout << "\033[32;1mRun::Decomposing waveforms:\033[0m done!     " << std::endl;
//      //
//      // wall1 = claws::get_wall_time();
//      // cpu1  = claws::get_cpu_time();
//      //
//      // cout << "Wall Time = " << wall1 - wall0 << endl;
//      // cout << "CPU Time  = " << cpu1  - cpu0  << endl;
// }
//
// void CalibrationRun::SetUpWaveforms()
// {
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->SetUpWaveforms();
//      }
// };
//
// void CalibrationRun::FastRate()
// {
//      std::map<std::string, std::vector<float>*> avg_waveforms = gain_->GetWaveform();
//      std::map<std::string, double> pe_to_mips = GS->GetPEtoMIPs();
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->FastRate(avg_waveforms, pe_to_mips);
//      }
//
// };
//
// void CalibrationRun::Decompose()
// {
//
//
//      std::map<std::string, std::vector<float>*> avg_waveforms = gain_->GetWaveform();
//      std::map<std::string, double> pe_to_mips = GS->GetPEtoMIPs();
// //    #pragma omp parallel for num_threads(5) firstprivate(avg_waveforms)
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->Decompose(avg_waveforms);
//              events_.at(i)->Reconstruct(avg_waveforms);
//              events_.at(i)->CalculateChi2();
//      }
//
//      //TODO Finish implentation
// };
//
// void CalibrationRun::Reconstruct()
// {
//      //TODO Implentation
// };
//
// void CalibrationRun::CalculateChi2()
// {
//      //TODO Implentation
// };
//
// void CalibrationRun::WaveformDecompositionV2()
// {
//      std::cout << "\033[33;1mRun::Decomposing waveforms:\033[0m running" << "\r" << std::flush;
//
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//
//      std::map<std::string, std::vector<float>*> avg_waveforms = gain_->GetWaveform();
//      std::map<std::string, double> pe_to_mips = GS->GetPEtoMIPs();
//
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              events_.at(i)->LoadRootFile();
//              events_.at(i)->LoadWaveform();
//
//              events_.at(i)->SetUpWaveformsV2();
//              events_.at(i)->Decompose(avg_waveforms);
//              events_.at(i)->Rate(pe_to_mips);
//              events_.at(i)->FastRate(avg_waveforms, pe_to_mips);
//
//              // events_.at(i)->Reconstruct(avg_waveforms);
//              // events_.at(i)->CalculateChi2();
//              // std::string folder = "Results_SignalFlagThreshold_"+ std::to_string(GS->GetCaliPar<double>("PhysicsChannel.SignalFlagThreshold"))
//              // +"_BinsOverThreshold_"+ std::to_string(GS->GetCaliPar<int>("PhysicsChannel.BinsOverThreshold"))
//              // +"_TailLength_"+ std::to_string(GS->GetCaliPar<int>("PhysicsChannel.TailLength"));
//              // events_.at(i)->SaveEvent(path_run_/boost::filesystem::path(folder), "clean");
//              // events_.at(i)->SaveEvent(path_run_/boost::filesystem::path(folder), "raw");
//
//              events_.at(i)->SaveEvent(path_run_/boost::filesystem::path("Results"), "raw");
//              events_.at(i)->SaveEvent(path_run_/boost::filesystem::path("Results"), "clean");
//              events_.at(i)->SaveEvent(path_run_/boost::filesystem::path("Results"), "workhorse");
//              events_.at(i)->SaveEvent(path_run_/boost::filesystem::path("Results"), "mip");
//
//              events_.at(i)->DeleteHistograms();
//              events_.at(i)->DeleteWaveforms();
//      }
//
//      std::cout << "\033[32;1mRun::Decomposing waveforms:\033[0m done!     " << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
//
// };
//
// void CalibrationRun::SaveEvents()
// {
//      std::cout << "Now saving events!" << std::endl;
//
//
//      boost::filesystem::path folder = path_run_/boost::filesystem::path("Calibration");
//      if(!boost::filesystem::is_directory(folder) )
//      {
//              boost::filesystem::create_directory(folder);
//      }
//
//      std::string fname = folder.string()+"/run_"+std::to_string(run_nr_)+"_snapshoot_selectedv1.root";
//
//      TFile *rfile = new TFile(fname.c_str(), "RECREATE");
//      rfile->mkdir("events");
//      for(auto &e : events_)
//      {
//              rfile->cd();
//              rfile->mkdir(("events/"+e->GetNrStr()).c_str());
//              rfile->cd(("events/"+e->GetNrStr()).c_str());
//
//              std::map<std::string, Channel*> chs= e->GetChannels();
//
//              for(auto &ch : chs)
//              {
//                      TCanvas c(ch.first.c_str(),ch.first.c_str(),500,500);
//
//                      TH1F* hist = ch.second->GetWaveformHist();
//                      hist->Draw();
//                      c.Write();
//                      // c.SaveAs((folder.string()+"/Original/run_"+std::to_string(run_nr_)+"_"+e->GetNrStr()+"_"+ch.second->GetName()+"_original.pdf").c_str());
//                      delete hist;
//                      hist = NULL;
//              }
//
//      }
//
//      rfile->mkdir("int");
//
//      for(auto &e : int_events_)
//      {
//
//              rfile->cd();
//              rfile->mkdir(("int/"+e->GetNrStr()).c_str());
//              rfile->cd(("int/"+e->GetNrStr()).c_str());
//
//              std::map<std::string, Channel*> chs= e->GetChannels();
//
//              for(auto &ch : chs)
//              {
//                      TCanvas c(ch.first.c_str(),ch.first.c_str(),500,500);
//                      TH1F* hist = ch.second->GetWaveformHist();
//                      hist->Draw();
//                      c.Write();
//                      delete hist;
//                      hist = NULL;
//              }
//
//      }
//
//      rfile->Close();
//      delete rfile;
//      rfile = NULL;
//
// };
//
// void CalibrationRun::SaveRates()
// {
//      /*
//          TODO description
//       */
//      if(!boost::filesystem::is_directory(path_run_/path("Calibration")) )
//      {
//              boost::filesystem::create_directory(path_run_/path("Calibration"));
//      }
//
//      std::string filename = path_run_.string()+"/Calibration/run-"+run_nr_str_+"_rates_version_"+ std::to_string(GS->GetCaliPar<int>("General.CalibrationVersion"))+".root";
//      TFile *rfile = new TFile(filename.c_str(), "RECREATE");
//
//      TGraph* rates[3];
//      TGraph* fast_rates[3];
//      TGraph* online_rates[6];
//      TGraph* current[2];
//      TGraph* injection[2];
//      TGraph* injection_bg[2];
//      TGraph* ratios[3]; // Fastrate/OnlineRate
//      TGraph* ratios2[3];
//
//      for(int i = 0; i < 3; i++)
//      {
//              online_rates[i] = new TGraph();
//              online_rates[i]->SetName(("OnlineRateFWD"+std::to_string(i+1)).c_str());
//              online_rates[i]->SetMarkerColor(kBlue+i*2);
//              online_rates[i]->SetMarkerStyle(33);
//              online_rates[i]->SetMarkerSize(2.0);
//              online_rates[i]->SetMinimum(0);
//              online_rates[i]->SetMaximum(700000);
//              online_rates[i]->GetXaxis()->SetTitle("time [Ms]");
//              online_rates[i]->GetXaxis()->SetNdivisions(405);
//
//              fast_rates[i] = new TGraph();
//              fast_rates[i]->SetName(("FastRateFWD"+std::to_string(i+1)).c_str());
//              fast_rates[i]->SetMarkerColor(kRed+i*2);
//              fast_rates[i]->SetMarkerStyle(34);
//              fast_rates[i]->SetMarkerSize(2.0);
//              fast_rates[i]->SetMinimum(0);
//              fast_rates[i]->SetMaximum(700000);
//              fast_rates[i]->GetXaxis()->SetTitle("time [Ms]");
//              fast_rates[i]->GetXaxis()->SetNdivisions(405);
//
//              rates[i] = new TGraph();
//              rates[i]->SetName(("RateFWD"+std::to_string(i+1)).c_str());
//              rates[i]->SetMarkerColor(kGreen+i*2);
//              rates[i]->SetMarkerStyle(21);
//              rates[i]->SetMarkerSize(2.0);
//              rates[i]->SetMinimum(0);
//              rates[i]->SetMaximum(700000);
//              rates[i]->GetXaxis()->SetTitle("time [Ms]");
//              rates[i]->GetXaxis()->SetNdivisions(405);
//
//              ratios[i] = new TGraph();
//              ratios[i]->SetName(("FastRateOnlineRateFWD"+std::to_string(i+1)).c_str());
//              ratios[i]->SetTitle(("FastRate/OnlineRate FWD"+std::to_string(i+1)).c_str());
//              ratios[i]->SetMarkerStyle(20);
//              ratios[i]->SetMarkerSize(2.0);
//              ratios[i]->SetMinimum(0);
//              ratios[i]->SetMaximum(10);
//              ratios[i]->GetXaxis()->SetTitle("time [Ms]");
//              ratios[i]->GetXaxis()->SetNdivisions(405);
//
//              ratios2[i] = new TGraph();
//              ratios2[i]->SetName(("RateFastRateFWD"+std::to_string(i+1)).c_str());
//              ratios2[i]->SetTitle(("Rate/FastRate FWD"+std::to_string(i+1)).c_str());
//              ratios2[i]->SetMarkerStyle(20);
//              ratios2[i]->SetMarkerSize(2.0);
//              ratios2[i]->SetMinimum(0);
//              ratios2[i]->SetMaximum(10);
//              ratios2[i]->GetXaxis()->SetTitle("time [Ms]");
//              ratios2[i]->GetXaxis()->SetNdivisions(405);
//      }
//      for(int i = 3; i < 6; i++)
//      {
//              online_rates[i] = new TGraph();
//              online_rates[i]->SetName(("OnlineRateBWD"+std::to_string(i+1)).c_str());
//              online_rates[i]->SetMarkerColor(kOrange+i-2);
//              online_rates[i]->SetMarkerStyle(29);
//              online_rates[i]->SetMarkerSize(2.0);
//              online_rates[i]->SetMinimum(0);
//              online_rates[i]->SetMaximum(700000);
//              online_rates[i]->GetXaxis()->SetTitle("time [Ms]");
//              online_rates[i]->GetXaxis()->SetNdivisions(405);
//
//      }
//
//      current[0] = new TGraph();
//      current[0]->SetName("LERCurrent");
//      current[0]->GetXaxis()->SetTitle("time [s x 10^6]");
//      current[0]->GetXaxis()->SetNdivisions(405);
//      current[0]->SetMarkerColor(kRed);
//      current[0]->SetMarkerStyle(22);
//      current[0]->SetMinimum(-25);
//      current[0]->SetMaximum(850);
//
//      current[1] = new TGraph();
//      current[1]->SetName("HERCurrent");
//      current[1]->GetXaxis()->SetTitle("time [s x 10^6]");
//      current[1]->GetXaxis()->SetNdivisions(405);
//      current[1]->SetMarkerColor(kBlue);
//      current[1]->SetMarkerStyle(23);
//      current[1]->SetMinimum(-25);
//      current[1]->SetMaximum(850);
//
//      injection[0] = new TGraph();
//      injection[0]->SetName("LERInj");
//      injection[0]->GetXaxis()->SetTitle("time [Ms]");
//      injection[0]->GetXaxis()->SetTitle("time [s x 10^6]");
//      injection[0]->GetXaxis()->SetNdivisions(405);
//      injection[0]->SetMarkerColor(kRed);
//      injection[0]->SetMarkerStyle(22);
//      injection[0]->SetMinimum(0);
//      injection[0]->SetMaximum(30);
//
//      injection[1] = new TGraph();
//      injection[1]->SetName("HERInj");
//      injection[1]->GetXaxis()->SetTitle("time [Ms]");
//      injection[1]->GetXaxis()->SetTitle("time [s x 10^6]");
//      injection[1]->GetXaxis()->SetNdivisions(405);
//      injection[1]->SetMarkerColor(kBlue);
//      injection[1]->SetMarkerStyle(23);
//      injection[1]->SetMinimum(0);
//      injection[1]->SetMaximum(30);
//
//      injection_bg[0] = new TGraph();
//      injection_bg[0]->SetName("LERInj_BG");
//      injection_bg[0]->GetXaxis()->SetTitle("time [Ms]");
//      injection_bg[0]->GetXaxis()->SetTitle("time [s x 10^6]");
//      injection_bg[0]->GetXaxis()->SetNdivisions(505);
//      injection_bg[0]->SetMarkerColor(kRed);
//      injection_bg[0]->SetMarkerStyle(22);
//      injection_bg[0]->SetMinimum(0);
//      injection_bg[0]->SetMaximum(2);
//
//      injection_bg[1] = new TGraph();
//      injection_bg[1]->SetName("HERInj_BG");
//      injection_bg[1]->GetXaxis()->SetTitle("time [Ms]");
//      injection_bg[1]->GetXaxis()->SetTitle("time [s x 10^6]");
//      injection_bg[1]->GetXaxis()->SetNdivisions(505);
//      injection_bg[1]->SetMarkerColor(kBlue);
//      injection_bg[1]->SetMarkerStyle(23);
//      injection_bg[1]->SetMinimum(0);
//      injection_bg[1]->SetMaximum(2);
//
//      for(unsigned int i=0; i < events_.size(); i++)
//      {
//              double ts = events_.at(i)->GetUnixtime()/10e6;
//
//              for(int j = 0; j < 3; j++)
//              {
//
//                      double online = events_.at(i)->GetRate()[j];
//                      double fast   = events_.at(i)->GetRate(1)[j];
//                      double rate   = events_.at(i)->GetRate(2)[j];
//
//
//                      online_rates[j]->SetPoint(i, ts, online);
//                      fast_rates[j]->SetPoint(i, ts, fast);
//                      rates[j]->SetPoint(i, ts, rate);
//                      //ratios[j]->SetPoint(i, ts, i);
//                      ratios[j]->SetPoint(i, ts, (fast/online));
//                      ratios2[j]->SetPoint(i, ts, (rate/fast));
//              }
//              for(int j = 3; j < 6; j++)
//              {
//                      online_rates[j]->SetPoint(i, ts, events_.at(i)->GetRate()[j]);
//              }
//
//              try
//              {
//                      current[0]->SetPoint(i, ts, events_.at(i)->GetPV<double>("LERCurrent"));
//                      current[1]->SetPoint(i, ts, events_.at(i)->GetPV<double>("HERCurrent"));
//              }
//              catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> >)
//              {
//                      current[0]->SetPoint(i, ts, -1);
//                      current[1]->SetPoint(i, ts, -1);
//              }
//              injection[0]->SetPoint(i, ts, events_.at(i)->GetPV<double>("LERInj"));
//              injection[1]->SetPoint(i, ts, events_.at(i)->GetPV<double>("HERInj"));
//
//              injection_bg[0]->SetPoint(i, ts, events_.at(i)->GetPV<double>("LERBg"));
//              injection_bg[1]->SetPoint(i, ts, events_.at(i)->GetPV<double>("HERBg"));
//      }
//
//      ratios[0]->SetMarkerColor(kRed);
//      ratios[1]->SetMarkerColor(kBlue);
//      ratios[2]->SetMarkerColor(kGreen+2);
//
//      ratios2[0]->SetMarkerColor(kRed);
//      ratios2[1]->SetMarkerColor(kBlue);
//      ratios2[2]->SetMarkerColor(kGreen+2);
//
//      TF1 *fit = new TF1("fit","[0]");
//      fast_rates[0]->Fit(fit,"Q");
//      delete fit;
//      for(int i = 0; i < 3; i++)
//      {
//              TF1 *fit = new TF1("fit","[0]");
//              fast_rates[i]->Fit(fit,"Q");
//              fast_rates[i]->Write();
//              delete fit;
//
//              rates[i]->Write();
//              online_rates[i]->Write();
//              ratios[i]->Write();
//              ratios2[i]->Write();
//      }
//      for(int i = 3; i < 6; i++)
//      {
//              online_rates[i]->Write();
//      }
//      current[0]->Write();
//      current[1]->Write();
//
//      injection[0]->Write();
//      injection[1]->Write();
//
//      injection_bg[0]->Write();
//      injection_bg[1]->Write();
//
//      std::string title = "Rates";
//      TCanvas c(title.c_str(),title.c_str(),500,500);
//
//      fast_rates[0]->Draw("AP");
//      fast_rates[1]->Draw("P");
//      fast_rates[2]->Draw("P");
//
//      rates[0]->Draw("P");
//      rates[1]->Draw("P");
//      rates[2]->Draw("P");
//
//      online_rates[0]->Draw("P");
//      online_rates[1]->Draw("P");
//      online_rates[2]->Draw("P");
//      online_rates[3]->Draw("P");
//      online_rates[4]->Draw("P");
//      online_rates[5]->Draw("P");
//
//      TLegend* leg = new TLegend(0.1,0.7,0.48,0.9);
//      leg->AddEntry(fast_rates[0], fast_rates[0]->GetName(),"P");
//      leg->AddEntry(fast_rates[1], fast_rates[1]->GetName(),"P");
//      leg->AddEntry(fast_rates[2], fast_rates[2]->GetName(),"P");
//
//      leg->AddEntry(rates[0], rates[0]->GetName(),"P");
//      leg->AddEntry(rates[1], rates[1]->GetName(),"P");
//      leg->AddEntry(rates[2], rates[2]->GetName(),"P");
//
//      leg->AddEntry(online_rates[0], online_rates[0]->GetName(),"P");
//      leg->AddEntry(online_rates[1], online_rates[1]->GetName(),"P");
//      leg->AddEntry(online_rates[2], online_rates[2]->GetName(),"P");
//      leg->AddEntry(online_rates[3], online_rates[3]->GetName(),"P");
//      leg->AddEntry(online_rates[4], online_rates[4]->GetName(),"P");
//      leg->AddEntry(online_rates[5], online_rates[5]->GetName(),"P");
//
//      leg->Draw("same");
//      c.Write();
//
//      title += "_ratios";
//      TCanvas c2(title.c_str(),title.c_str(),500,500);
//      c2.cd();
//      ratios[0]->Draw("AP");
//      ratios[1]->Draw("P");
//      ratios[2]->Draw("P");
//
//      leg->Clear();
//      leg->AddEntry(ratios[0], ratios[0]->GetTitle(),"P");
//      leg->AddEntry(ratios[1], ratios[1]->GetTitle(),"P");
//      leg->AddEntry(ratios[2], ratios[2]->GetTitle(),"P");
//      leg->Draw("same");
//
//      c2.Write();
//
//      title = "Rates_ratios2";
//      TCanvas c3(title.c_str(),title.c_str(),500,500);
//      c3.cd();
//      ratios2[0]->Draw("AP");
//      ratios2[1]->Draw("P");
//      ratios2[2]->Draw("P");
//
//      leg->Clear();
//      leg->AddEntry(ratios2[0], ratios2[0]->GetTitle(),"P");
//      leg->AddEntry(ratios2[1], ratios2[1]->GetTitle(),"P");
//      leg->AddEntry(ratios2[2], ratios2[2]->GetTitle(),"P");
//      leg->Draw("same");
//
//      c3.Write();
//
//      rfile->Close();
//      delete rfile;
//
// };
// // double CalibrationRun::GetStartTime(){
// //     return tsMin;
// // };
// // double CalibrationRun::GetStopTime(){
// //     return tsMax;
// // };
//
// int CalibrationRun::BuildOnlineTree(){
//      // TODO Implentation
//      return 0;
// };
// int CalibrationRun::BuildOfflineTree(){
//      // TODO Implentation
//      return 0;
// };
// TTree *CalibrationRun::GetOnlineTree(){
//      this->BuildOnlineTree();
//      return tree_online;
// };
// TTree *CalibrationRun::GetOfflineTree(){
//      this->BuildOfflineTree();
//      return tree_offline;
// };
//
// std::vector<IntChannel*> CalibrationRun::GetIntChannel(std::string name)
// {
//      std::vector<IntChannel*> channel;
//      for(auto & ivec : int_events_)
//      {
//              IntChannel* tmp = dynamic_cast<IntChannel*>(ivec->GetChannel(name));
//              channel.push_back(tmp);
//      }
//      return channel;
// };
//
// std::vector<PhysicsChannel*> CalibrationRun::GetPhysicsChannel(std::string name)
// {
//      std::vector<PhysicsChannel*> channel;
//      for(auto & ivec : events_)
//      {
//              PhysicsChannel* tmp = dynamic_cast<PhysicsChannel*>(ivec->GetChannel(name));
//              channel.push_back(tmp);
//      }
//      return channel;
// };
//
// int CalibrationRun::WriteNTuple(path path_ntuple){
//
//      std::cout << "\033[33;1mRun::Writing NTuples:\033[0m running" << "\r" << std::flush;
//
//      double wall0 = claws::get_wall_time();
//      double cpu0  = claws::get_cpu_time();
//
//
//      if(path_ntuple.string() == "")
//      {
//              if(!boost::filesystem::is_directory(path_run_/boost::filesystem::path("Calibration")) )
//              {
//                      boost::filesystem::create_directory(path_run_/boost::filesystem::path("Calibration"));
//              }
//              path_ntuple = path_run_/boost::filesystem::path("Calibration");
//      }
//      path_ntuple = path_ntuple / ("CLW_" +to_string(run_nr_) + "_" + to_string(int(tsMin)) + "_v"+std::to_string(GS->GetCaliPar<int>("General.CalibrationVersion"))+".root" );
//
//      TFile * root_file  = new TFile(path_ntuple.string().c_str(), "RECREATE");
//
//      // this->WriteTimeStamp(root_file);
//      // this->WriteOnlineTree(root_file);
//      this->WriteTree(root_file);
//
//      root_file->Close();
//
//      std::cout << "\033[32;1mRun::Writing NTuples:\033[0m done!     " << std::endl;
//
//      double wall1 = claws::get_wall_time();
//      double cpu1  = claws::get_cpu_time();
//
//      cout << "Wall Time = " << wall1 - wall0 << endl;
//      cout << "CPU Time  = " << cpu1  - cpu0  << endl;
//
//
//      return 0;
// };
//
// int CalibrationRun::WriteOnlineTree(TFile* file)
// {
//      TTree *tout = new TTree("tout","tout");
//      TTree *tout_inj = new TTree("tout_inj","tout_inj");
//      TTree *tscrub = new TTree("tscrub","tscrub");
//      TTree *tscrub_inj = new TTree("tscrub_inj","tscrub_inj");
//
//      double ts;
//      double rate_on[6] = {0};
//
//      tout->Branch("ts", &ts,     "ts/D");
//      tout->Branch("rate_on", rate_on,     "rate_on[6]/D");
//
//      tout_inj->Branch("ts", &ts,     "ts/D");
//      tout_inj->Branch("rate_on", rate_on,     "rate_on[6]/D");
//
//      tscrub->Branch("ts", &ts,     "ts/D");
//      tscrub->Branch("rate_on", rate_on,     "rate_on[6]/D");
//
//      tscrub_inj->Branch("ts", &ts,     "ts/D");
//      tscrub_inj->Branch("rate_on", rate_on,     "rate_on[6]/D");
//
//      for(unsigned int i=0; i < events_.size(); i++) {
//
//              ts = events_.at(i)->GetUnixtime();
//
//              rate_on[0] = events_.at(i)->GetRate()[0];
//              rate_on[1] = events_.at(i)->GetRate()[1];
//              rate_on[2] = events_.at(i)->GetRate()[2];
//              rate_on[3] = events_.at(i)->GetRate()[3];
//              rate_on[4] = events_.at(i)->GetRate()[4];
//              rate_on[5] = events_.at(i)->GetRate()[5];
//
//              if(events_.at(i)->GetInjection())
//              {
//                      tout_inj->Fill();
//                      if(events_.at(i)->GetScrubbing() == 3) tscrub_inj->Fill();
//              }
//              else
//              {
//                      tout->Fill();
//                      if(events_.at(i)->GetScrubbing() == 3) tscrub->Fill();
//              }
//
//      }
//
//      file->cd();
//      tout->Write();
//      tout_inj->Write();
//      tscrub->Write();
//      tscrub_inj->Write();
//
//      delete tout;
//      delete tout_inj;
//      delete tscrub;
//      delete tscrub_inj;
//
//      return 0;
// };
//
// int CalibrationRun::WriteTimeStamp(TFile* file)
// {
//      TTree *tout = new TTree("tout","tout");
//
//      double ts;
//      bool injection;
//
//      tout->Branch("ts", &ts,     "ts/D");
//      tout->Branch("inj", &injection,     "inj/O");
//
//      for(unsigned int i=0; i < events_.size(); i++) {
//
//              injection = events_.at(i)->GetInjection();
//              ts        = events_.at(i)->GetUnixtime();
//              tout->Fill();
//      }
//
//      file->cd();
//      tout->Write();
//
//      delete tout;
//
//      return 0;
// };
//
// int CalibrationRun::WriteTree(TFile* file, std::string type)
// {
//      //TODO Validate
//      TTree* t_auto = new TTree("t_auto", "t_auto");
//      TTree* t_inj = new TTree("t_inj", "t_inj");
//      TTree* t_comb = new TTree("t_comb", "t_comb");
//
//      double ts;
//      double rate_online[6];
//      double fast_rate[3];
//      double rate[3];
//      double current[2];
//      double injection[2];
//
//      t_auto->Branch("ts", &ts,     "ts/D");
//      t_auto->Branch("rate_online",       rate_online,           "rate_online[6]/D");
//      t_auto->Branch("fast_rate", fast_rate,     "fast_rate[3]/D");
//      t_auto->Branch("rate", rate,     "rate[3]/D");
//
//      t_inj->Branch("ts", &ts,     "ts/D");
//      t_inj->Branch("rate_online",       rate_online,           "rate_online[6]/D");
//      t_inj->Branch("fast_rate", fast_rate,     "fast_rate[3]/D");
//      t_inj->Branch("rate", rate,     "rate[3]/D");
//
//      t_comb->Branch("ts", &ts,     "ts/D");
//      t_comb->Branch("rate_online",       rate_online,           "rate_online[6]/D");
//      t_comb->Branch("fast_rate", fast_rate,     "fast_rate[3]/D");
//      t_comb->Branch("rate", rate,     "rate[3]/D");
//      t_comb->Branch("current", current,     "current[2]/D");
//      t_comb->Branch("injection", current,     "injection[2]/D");
//
//      for(unsigned int i=0; i < events_.size(); i++)
//      {
//              ts                  = events_.at(i)->GetUnixtime();
//
//              rate_online[0] = events_.at(i)->GetRate()[0];
//              rate_online[1] = events_.at(i)->GetRate()[1];
//              rate_online[2] = events_.at(i)->GetRate()[2];
//              rate_online[3] = events_.at(i)->GetRate()[3];
//              rate_online[4] = events_.at(i)->GetRate()[4];
//              rate_online[5] = events_.at(i)->GetRate()[5];
//
//              fast_rate[0] = events_.at(i)->GetRate(1)[0];
//              fast_rate[1] = events_.at(i)->GetRate(1)[1];
//              fast_rate[2] = events_.at(i)->GetRate(1)[2];
//
//              rate[0] = events_.at(i)->GetRate(2)[0];
//              rate[1] = events_.at(i)->GetRate(2)[1];
//              rate[2] = events_.at(i)->GetRate(2)[2];
//              try
//              {
//                      current[0] = events_.at(i)->GetPV<double>("LERCurrent");
//                      current[1] = events_.at(i)->GetPV<double>("HERCurrent");
//              }
//              catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> >)
//              {
//                      current[0] = -1;
//                      current[1] = -1;
//              }
//
//              injection[0] = events_.at(i)->GetPV<double>("LERInj");
//              injection[1] = events_.at(i)->GetPV<double>("HERInj");
//
//              if(events_.at(i)->GetInjection()) t_inj->Fill();
//              else t_auto->Fill();
//              t_comb->Fill();
//      }
//
//      file->cd();
//      t_auto->Write();
//      t_inj->Write();
//      t_comb->Write();
//      return 0;
// };
//
// void CalibrationRun::DrawPedestal()
// {
//      // string title = to_string(run_nr_);
//      // TCanvas * c = new TCanvas(title.c_str(), title.c_str(), 1600, 1200);
//      // c->Divide(2,h_ped_.size()/2);
//      // unsigned int pad=0;
//      // for(auto i : h_ped_)
//      // {
//      //     pad+=+2;
//      //     if(pad > h_ped_.size()) pad =1;
//      //     c->cd(pad);
//      //     i.second->Draw();
//      // }
//      //
//      // title += "-Int";
//      // TCanvas * c_int = new TCanvas(title.c_str(), title.c_str(), 1600, 1200);
//      // c_int->Divide(2, h_ped_int_.size()/2);
//      // pad=0;
//      // for(auto i : h_ped_int_)
//      // {
//      //     pad+=+2;
//      //     if(pad > h_ped_int_.size() ) pad =1;
//      //     c_int->cd(pad);
//      //     i.second->Draw();
//      //
//      // }
// }
//
// CalibrationRun::~CalibrationRun() {
//      // TODO Auto-generated destructor stub
//      std::cout << "Deleteing Run object!" << std::endl;
//      //  #pragma omp parallel num_threads(7)
//      //  {
//      //    #pragma omp for schedule(dynamic,1)
//      for(unsigned int i=0; i< events_.size(); i++)
//      {
//              delete events_.at(i);
//              events_.at(i) = NULL;
//      }
//
//      //      #pragma omp for schedule(dynamic,1)
//      for(unsigned int i=0; i< int_events_.size(); i++)
//      {
//              delete int_events_.at(i);
//              int_events_.at(i) = NULL;
//      }
//
//      //  }
//
//      if(!boost::filesystem::is_directory(path_run_/path("Calibration")) )
//      {
//              boost::filesystem::create_directory(path_run_/path("Calibration"));
//      }
//      GS->SaveConfigFiles(path_run_/path("Calibration"));
//
//      delete pedestal_;
//      delete gain_;
//
// };
//
//
