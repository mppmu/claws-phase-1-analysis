//============================================================================
// Name        : run.cxx
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

// --- BOOST includes ---
 #include <boost/algorithm/string/predicate.hpp>
 #include <boost/algorithm/string/replace.hpp>

// --- ROOT includes ---
#include <TGraphErrors.h>
#include <TFile.h>

// --- Project includes ---
#include "run.hh"
#include "pedestal.hh"
#include "event.hh"
#include "globalsettings.hh"


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
		Double_t invsq2pi = 0.3989422804014;                                                                                                                                                                                                                                                 // (2 pi)^(-1/2)
		Double_t mpshift  = -0.22278298;                                                                                                                                                                                                                                                     // Landau maximum location

		// Control constants
		Double_t np = 100.0;                                                                                                                                                                                                                                                    // number of convolution steps
		Double_t sc =   5.0;                                                                                                                                                                                                                                                    // convolution extends to +-sc Gaussian sigmas

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
		std::cout << "\033[1;31mRun::Created run: \033[0m" << nr_ << " - at: " << p.string() << std::endl;

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

		std::cout << "\033[33;1mRun::Synchronizing physics run:\033[0m running" << "\r" << std::flush;

		int phase = GS->GetParameter<int>("General.Phase");

		boost::filesystem::path p_physics_dir = path_;
		boost::filesystem::path p_cal_dir = path_;
		std::string file_start = "";

		if(phase == 1)
		{
				p_physics_dir /= boost::filesystem::path("data_root");
				p_cal_dir /= boost::filesystem::path("int_root");
				file_start = "Event-";
		}
		else if(phase == 2)
		{
				p_physics_dir /= boost::filesystem::path("raw")/boost::filesystem::path("physics");
				p_cal_dir /= boost::filesystem::path("raw")/boost::filesystem::path("intermediate");
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

						boost::filesystem::path ini_file  = p_physics_dir/boost::filesystem::path(tmp);

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

		boost::filesystem::path p_cal_dir = path_;

		if(phase == 1)
		{
				p_cal_dir /= boost::filesystem::path("int_root");
		}
		else if(phase == 2)
		{
				p_cal_dir /= boost::filesystem::path("raw")/boost::filesystem::path("intermediate");
		}


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
						p_cal_dir /= boost::filesystem::path("raw/intermediate");
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

		/** Use the first event to get a dynamic number and name of channels.
		 *   Messy but it works.
		 */

		std::vector<TGraph *> fit_status;
		std::vector<TGraph *> fit_const;
		std::vector<TGraphErrors *> fit_mean;
		std::vector<TGraph *> fit_sig;
		std::vector<TGraph *> fit_chi2;
		std::vector<TGraph *> fit_ndf;
		std::vector<TGraph *> fit_chi2ndf;
		std::vector<TGraph *> fit_pval;
		std::vector<TH1F *> hist_pval;
		std::vector<TGraphErrors *> hist_mean;
		std::vector<TGraph *> hist_entries;

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

				TGraph * sdg = new TGraph();
				sdg->SetName( (name +"_fit_sig").c_str() );
				sdg->SetMarkerStyle(23);
				sdg->SetMarkerColor(kRed);
				sdg->SetMarkerSize(1);
				sdg->GetXaxis()->SetTitle("Time [s]");
				sdg->GetYaxis()->SetTitle("Sigma [mV]");
				sdg->GetYaxis()->SetRangeUser(0,127);
				fit_sig.push_back(sdg);

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
						double evt_time = evt->GetParameter<double>("Properties.UnixTime");
						auto channels = evt->GetChannels();
						for(unsigned int i = 0; i <channels.size(); i++)
						{

								double * pd = channels.at(i)->GetPedestal();

								fit_status.at(i)->SetPoint(evt_counter, evt_time, pd[0]);
								fit_const.at(i)->SetPoint(evt_counter, evt_time, pd[1]);

								fit_mean.at(i)->SetPoint(evt_counter, evt_time, pd[2]);
								fit_mean.at(i)->SetPointError(evt_counter, 0.05, pd[3]);

								fit_sig.at(i)->SetPoint(evt_counter, evt_time, pd[4]);

								fit_chi2.at(i)->SetPoint(evt_counter, evt_time, pd[5]);

								fit_ndf.at(i)->SetPoint(evt_counter, evt_time, pd[6]);


								if( pd[5] !=0 ) fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, pd[5]/pd[6]);
								else fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, -1.);

								fit_pval.at(i)->SetPoint(evt_counter, evt_time,pd[7]);

								hist_pval.at(i)->Fill(pd[7]);

								hist_mean.at(i)->SetPoint(evt_counter, evt_time, pd[8]);
								hist_mean.at(i)->SetPointError(evt_counter, 0.05, pd[9]);

								hist_entries.at(i)->SetPoint(evt_counter, evt_time, pd[10]);
						}

						evt_counter++;
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
				fit_sig.at(i)->Write();
				delete fit_sig.at(i);
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

		// Use the first event to get a dynamic number and name of channels.
		std::vector<TGraph *> fit_status;
		std::vector<TGraph *> fit_const;
		std::vector<TGraphErrors *> fit_mean;
		std::vector<TGraph *> fit_sig;
		std::vector<TGraph *> fit_chi2;
		std::vector<TGraph *> fit_ndf;
		std::vector<TGraph *> fit_chi2ndf;
		std::vector<TGraph *> fit_pval;
		std::vector<TH1F *> hist_pval;
		std::vector<TGraphErrors *> hist_mean;
		std::vector<TGraph *> hist_entries;

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

				TGraph * sdg = new TGraph();
				sdg->SetName( (name +"_fit_sig").c_str() );
				sdg->SetMarkerStyle(23);
				sdg->SetMarkerColor(kRed);
				sdg->SetMarkerSize(1);
				sdg->GetXaxis()->SetTitle("Time [s]");
				sdg->GetYaxis()->SetTitle("Sigma [mV]");
				sdg->GetYaxis()->SetRangeUser(0,127);
				fit_sig.push_back(sdg);

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

				double evt_time = evt->GetParameter<double>("Properties.UnixTime");
				auto channels = evt->GetChannels();

				for(unsigned int i = 0; i <channels.size(); i++)
				{
						double * pd = channels.at(i)->GetPedestal();

						fit_status.at(i)->SetPoint(evt_counter, evt_time, pd[0]);
						fit_const.at(i)->SetPoint(evt_counter, evt_time, pd[1]);

						fit_mean.at(i)->SetPoint(evt_counter, evt_time, pd[2]);
						fit_mean.at(i)->SetPointError(evt_counter, 0.05, pd[3]);

						fit_sig.at(i)->SetPoint(evt_counter, evt_time, pd[4]);

						fit_chi2.at(i)->SetPoint(evt_counter, evt_time, pd[5]);

						fit_ndf.at(i)->SetPoint(evt_counter, evt_time, pd[6]);

						if( pd[5] !=0 ) fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, pd[5]/pd[6]);
						else fit_chi2ndf.at(i)->SetPoint(evt_counter, evt_time, -1.);

						fit_pval.at(i)->SetPoint(evt_counter, evt_time,pd[7]);
						hist_pval.at(i)->Fill(pd[7]);

						hist_mean.at(i)->SetPoint(evt_counter, evt_time, pd[8]);
						hist_mean.at(i)->SetPointError(evt_counter, 0.05, pd[9]);

						hist_entries.at(i)->SetPoint(evt_counter, evt_time, pd[10]);
				}

				evt->SubtractPedestals();

				evt->SaveEvent(pds_physics/boost::filesystem::path("Waveforms"));
				evt->DeleteHistograms();

				evt_counter++;
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
				fit_sig.at(i)->Write();
				delete fit_sig.at(i);
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

		std::vector<std::vector<TGraph*> > graphs;
		std::string names[15] = {"_lstart", "_lstop", "_lresult", "_start", "_stop", "_result", "_par0", "_par1", "_par2", "_chi2", "_ndf", +"_pval", "_area1", "area2", "_area_diff"};
		std::string ytitles[15] = {"Time [s]", "Time [s]", "Fit status","Time [s]", "Time [s]", "Fit status", "Slope [mV/s]", "X-shift [s]", "Y-shift [s]", "Chi2", "Ndf", "P-value", "Area [a.u.]", "Area [a.u.]", "Area [a.u.]"};

		for(auto &channel: evts_.at(0)->GetChannels() )
		{
				std::vector<TGraph*> gch;

				std::string name = channel->GetName();
				for(int i =0; i < 15; ++i)
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

						double evt_time = evt->GetParameter<double>("Properties.UnixTime");
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


		Gain* gain = new Gain(path_, GAINSTATE_EXTENDED);

		int nthreads   = GS->GetParameter<int>("General.nthreads");
		bool parallelize = GS->GetParameter<bool>("General.parallelize");

		for( auto evt : evts_)
		{
				// Here the actual waveform decomposition is done, the rest is just
				// getting the info out.
				evt->LoadFiles(EVENTSTATE_WFDECOMPOSED);

				if( evt->GetState() == EVENTSTATE_WFDECOMPOSED )
				{
						evt->WaveformReconstruction(gain);
						evt->SaveEvent( outfolder/boost::filesystem::path("Waveforms") );
				}

				evt->DeleteHistograms();
		}

		std::vector<std::vector<TGraph*> > graphs;
		std::string names[5] = {"_nbins", "_binerror", "_chi2", "_pval", "_chi2ndf"};
		std::string ytitles[5] = {"Ndf", "BinError", "Chi2", "P-Value","Chi2/Ndf"};

		for(auto &channel: evts_.at(0)->GetChannels() )
		{
				std::vector<TGraph*> gch;

				std::string name = channel->GetName();

				for(int i =0; i < 5; ++i)
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
				std::vector<std::vector<double> > results = evt->GetReconstruction();

				double evt_time = evt->GetParameter<double>("Properties.UnixTime");

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

		std::vector<std::vector<TGraph*> > graphs;
		std::string names[6] = { "_online_rate", "_fast_rate", "_rate", "_staterr", "_syserr", "_err"};

		for(auto &channel: evts_.at(0)->GetChannels() )
		{
				std::vector<TGraph*> gch;

				std::string name = channel->GetName();

				for(int i = 0; i < 6; ++i)
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

				for(int i =0; i < 6; ++i)
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

						vector< Rate > rates = evt->GetRates();
						double evt_time = evt->GetParameter<double>("Properties.UnixTime");
						for(unsigned int i = 0; i < rates.size(); ++i)
						{
								Rate ch_rate = rates.at(i);

								int n = graphs.at(i).at(0)->GetN();
								graphs.at(i).at(0)->SetPoint( n, evt_time, ch_rate.online );
								graphs.at(i).at(1)->SetPoint( n, evt_time, ch_rate.fast );
								graphs.at(i).at(2)->SetPoint( n, evt_time, ch_rate.rate);
								graphs.at(i).at(3)->SetPoint( n, evt_time, ch_rate.staterr );
								graphs.at(i).at(4)->SetPoint( n, evt_time, ch_rate.syserr );
								graphs.at(i).at(5)->SetPoint( n, evt_time, ch_rate.err );

						}
				}

				evt->DeleteHistograms();
		}

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

		std::vector<TH1F*> hists;
		std::string names[3] = { "_online_rate", "_fast_rate", "_rate"};

		for(auto &channel: evts_.at(0)->GetChannels() )
		{
				double pe_per_mip = 0;
				string name = channel->GetName();

				if( GS->GetParameter<string>("PEToMIP." + name) != "false")
				{
						pe_per_mip = GS->GetParameter<double>("PEToMIP." + name + "val2");
				}
				else
				{
						pe_per_mip = GS->GetParameter<double>("PEToMIP." + name + "val");
				}

				string title = channel->GetName() + "_mip_per_event";

				double nmip = 5;
				int nbinsx   = (int)round(nmip*pe_per_mip);
				double xlow  = -1./(2*pe_per_mip);
				double xup   = nmip -1./(2*pe_per_mip);

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

		double start_mpv         = GS->GetParameter<double>("SystematicsStudy.start_mpv");
		double threshold_tres    = GS->GetParameter<double>("SystematicsStudy.threshold_tres");

		for(auto & evt: evts_)
		{
				evt->LoadFiles(EVENTSTATE_CALIBRATED);

				if( evt->GetState() == EVENTSTATE_CALIBRATED)
				{


						for(int i = 0; i < evt->GetChannels().size(); ++i)
						{

								TH1F* mipwf = dynamic_cast<TH1F*>(evt->GetChannels().at(i)->GetHistogram("mip"));

								for(int j = start_mpv; j <= 500 + start_mpv; ++j)
								{
										if( mipwf->GetBinContent(j) > 0.01 )
										{
												hists.at(i)->Fill( mipwf->GetBinContent(j) );

										}
								}

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
				double rlow = hists.at(i)->GetBinLowEdge(5);
				double rup = hists.at(i)->GetBinCenter(hists.at(i)->GetMaximumBin())*4.0;

				// Stuff basically stolen from the root example:
				// https://root.cern.ch/root/html/tutorials/fit/langaus.C.html
				TF1 *langaus = new TF1(funcname.c_str(),langaufun, rlow, rup,4);

				double par1 = hists.at(i)->GetBinCenter(hists.at(i)->GetMaximumBin());

				langaus->SetParameters(0.1, par1, 500., 0.1);
				langaus->SetParNames("Width","MP","Area","GSigma");

				Double_t pllo[4], plhi[4];

				pllo[0]=0.0; pllo[1]=0.0;   pllo[2]=0; pllo[3]=0;
				plhi[0]=1.0; plhi[1]=2.0; plhi[2]=1000000.0; plhi[3]=2.0;

				for ( int j = 0; j < 4; ++j)
				{
						langaus->SetParLimits(j, pllo[j], plhi[j]);
				}

				hists.at(i)->Fit(langaus, "QRSL");

				cout << hists.at(i)->GetName() << ": " << langaus->GetMaximumX() << endl;

				delete langaus;
		}

		TF1* time_res =new TF1("gaus","[0]*exp(-0.5*((x-[1])/[2])**2) ",1,3, TF1::EAddToList::kNo);
		time_res->SetParNames("Constant", "Mean", "Sigma");
		time_res->SetParameter(0, 500);
		time_res->SetParameter(1, 0);
		time_res->SetParameter(2, dt);


		double low = -dt*GS->GetParameter<double>("SystematicsStudy.range_time");
		double up  = -low;

		hists.back()->Fit(time_res, "QSL","", low, up);

		std::cout << "TRes: " << time_res->GetParameter(2) << std::endl;

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

		std::cout << "\033[32;1mRun::Systematics study:\033[0m done!       " << std::endl;
};

void CalibrationRun::SetInjectionLimit(string type, NTP_Handler* ntp_handler)
{

		auto itr_evts = evts_.begin();

		while(itr_evts != evts_.end())
		{
				double ts = (*itr_evts)->GetParameter<double>("Properties.UnixTime");

				int ler = -1;
				int her = -1;

				if(type == "NONE")
				{
						try
						{
								ler = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_injectionFlag_verySafe"))[0];
						}
						catch(int e)
						{
								try
								{
										ler = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_injectionFlag_safe"))[0];
								}
								catch(int e)
								{
										delete (*itr_evts);
										(*itr_evts) = NULL;
										evts_.erase(itr_evts);

										continue;
								}
						}


						try
						{
								her = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_injectionFlag_verySafe"))[0];

						}
						catch(int e)
						{
								try
								{
										her = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_injectionFlag_safe"))[0];
								}
								catch(int e)
								{
										delete (*itr_evts);
										(*itr_evts) = NULL;
										evts_.erase(itr_evts);

										continue;
								}
						}
				}
				else
				{
						try
						{
								ler = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_injectionFlag"))[0];

						}
						catch(int e)
						{
								cout << "No SKB_LER_injectionFlag! Resorting to safe..." <<endl;
								try
								{
										ler = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_LER_injectionFlag_safe"))[0];
								}
								catch(int e)
								{
										delete (*itr_evts);
										(*itr_evts) = NULL;
										evts_.erase(itr_evts);

										continue;
								}
						}


						try
						{
								her = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_injectionFlag"))[0];


						}
						catch(int e)
						{
								cout << "No SKB_HER_injectionFlag! Resorting to safe..." <<endl;
								try
								{
										her = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_HER_injectionFlag_safe"))[0];
								}
								catch(int e)
								{
										delete (*itr_evts);
										(*itr_evts) = NULL;
										evts_.erase(itr_evts);

										continue;
								}
						}
				}

				if(type == "NONE")
				{
						if(ler == 0 and her == 0)
						{
								itr_evts++;
						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else if(type == "BOTH")
				{
						if(ler == 1 and her == 1)
						{
								itr_evts++;
						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else if(type == "ANY")
				{
						if(ler == 1 or her == 1)
						{
								itr_evts++;
						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else if(type == "LERONLY")
				{
						if( ler == 1 and her == 0)
						{
								itr_evts++;

						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else if(type == "HERONLY")
				{
						if( ler == 0 and her == 1)
						{
								itr_evts++;

						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else if(type == "LER")
				{
						if( ler == 1)
						{
								itr_evts++;

						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else if(type == "HER")
				{
						if(her == 1)
						{
								itr_evts++;

						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				else
				{
						itr_evts++;
				}
		}

};

void CalibrationRun::SetInjectionRate( string ring, double limit)
{
		auto itr_evts = std::begin(evts_);

		while(itr_evts != std::end(evts_))
		{
				double rate = (*itr_evts)->GetParameter<double>("SuperKEKBData."+ring+"Inj");

				if( fabs(rate - limit) < 1e-3)
				{
						itr_evts++;
				}
				else
				{
						delete (*itr_evts);
						(*itr_evts) = NULL;
						evts_.erase(itr_evts);
				}
		}
};


void CalibrationRun::SetCurrentLimit(std::string ring, double min, double max, NTP_Handler *ntp_handler)
{
		auto itr_evts = std::begin(evts_);

		while(itr_evts != std::end(evts_))
		{
				double ts = (*itr_evts)->GetParameter<double>("Properties.UnixTime");

				try
				{
						double current = (*ntp_handler->GetPV< vector<double>* >(ts, "SKB_"+ring+"_current"))[0];

						if(current >= min and current <= max)
						{
								itr_evts++;
						}
						else
						{
								delete (*itr_evts);
								(*itr_evts) = NULL;
								evts_.erase(itr_evts);
						}
				}
				catch(int e)
				{
						delete (*itr_evts);
						(*itr_evts) = NULL;
						evts_.erase(itr_evts);
				}

		}
};

void CalibrationRun::SetTSLimit(double min, double max)
{
		auto itr_evts = std::begin(evts_);

		while(itr_evts != std::end(evts_))
		{
				double ts = (*itr_evts)->GetParameter<double>("Properties.UnixTime");

				if(ts >= min and ts <= max)
				{
						itr_evts++;
				}
				else
				{
						delete (*itr_evts);
						(*itr_evts) = NULL;
						evts_.erase(itr_evts);
				}
		}
};

void CalibrationRun::SetStatus(std::string type, std::string status)
{
		auto itr_evts = std::begin(evts_);

		while(itr_evts != std::end(evts_))
		{
				string evt_status = "";

				if(type == "SUPERKEKB") evt_status = (*itr_evts)->GetParameter<string>("SuperKEKBData.SuperKEKBStatus");
				else if(type == "LER") evt_status = (*itr_evts)->GetParameter<string>("SuperKEKBData.LERSTatus");
				else if(type == "HER") evt_status = (*itr_evts)->GetParameter<string>("SuperKEKBData.HERStatus");

				if(status == evt_status)
				{
						itr_evts++;
				}
				else
				{
						delete (*itr_evts);
						(*itr_evts) = NULL;
						evts_.erase(itr_evts);
				}
		}
};

int CalibrationRun::GetNumber()
{
		return nr_;
};

int CalibrationRun::GetNEvents()
{
		return evts_.size();
};

vector<PhysicsEvent*> CalibrationRun::GetEvents()
{
		return evts_;
};

// DONE
