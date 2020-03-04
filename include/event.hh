// Name        : event.hh
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

#ifndef CLAWS_EVENT_H_
#define CLAWS_EVENT_H_

// --- BOOST includes ---
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

// --- ROOT includes ---
#include <TH1F.h>

// --- Project includes ---
#include "ntp_handler.hh"
#include "channel.hh"
#include "gain.hh"

//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

enum EventState
{
		EVENTSTATE_INIT,
		EVENTSTATE_RAW,
		EVENTSTATE_PREP,
		EVENTSTATE_PDFILLED,
		EVENTSTATE_PDSUBTRACTED,
		EVENTSTATE_OSCORRECTED,
		EVENTSTATE_PDFAILED,
		EVENTSTATE_OSFAILED,
		EVENTSTATE_TAGGED,
		EVENTSTATE_TAGFAILED,
		EVENTSTATE_WFDECOMPOSED,
		EVENTSTATE_WFDFAILED,
		EVENTSTATE_WFRECONSTRUCTED,
		EVENTSTATE_WFRFAILED,
		EVENTSTATE_CALIBRATED,
		EVENTSTATE_FAILED,
};

/** Usually definitions should go into the .cxx file because otherwise
 *  the complier complains about multiple definitions. To avoid this
 *  the definition is inlined.
 */
inline std::string printEventState(EventState state)
{
		switch(state)
		{
		case EVENTSTATE_INIT:
				return "init";
		case EVENTSTATE_RAW:
				return "raw";
		case EVENTSTATE_PREP:
				return "prepared";
		case EVENTSTATE_PDFILLED:
				return "pd_filled";
		case EVENTSTATE_PDSUBTRACTED:
				return "pd_subtracted";
		case EVENTSTATE_OSCORRECTED:
				return "os_corrected";
		case EVENTSTATE_OSFAILED:
				return "os_failed";
		case EVENTSTATE_PDFAILED:
				return "pd_failed";
		case EVENTSTATE_TAGGED:
				return "tagged";
		case EVENTSTATE_TAGFAILED:
				return "tag_failed";
		case EVENTSTATE_WFDECOMPOSED:
				return "wf_decomposed";
		case EVENTSTATE_WFDFAILED:
				return "wfd_failed";
		case EVENTSTATE_WFRECONSTRUCTED:
				return "wf_reconstructed";
		case EVENTSTATE_WFRFAILED:
				return "wfr_failed";
		case EVENTSTATE_CALIBRATED:
				return "calibrated";
		case EVENTSTATE_FAILED:
				return "failed";
		default:
				return "Invalid Selection";
		}
}

class Event {

		public:
				Event();
				Event( boost::filesystem::path file, boost::filesystem::path ini_file );

				virtual ~Event();

				virtual void LoadFiles();
				virtual void LoadRaw();
				virtual void LoadHistograms(boost::filesystem::path file);

				virtual void SaveEvent(boost::filesystem::path dst, bool save_pd = false);

				virtual void DeleteHistograms();

				virtual void PrepareHistograms();
				virtual void FillPedestals();
				virtual void SubtractPedestals(std::vector<double> pd = {});

				virtual void                    SetTime(double unixtime);
				virtual double                  GetTime();
				virtual long                    GetNumber();
				virtual std::vector<Channel*>   GetChannels();
				virtual EventState              GetState();

				virtual boost::filesystem::path GetPath(std::string type = "path");

				template<typename T>
				T GetParameter(std::string pv)
				{
						return pt_.get<T>( pv );
				};

		protected:
				long nr_;
				EventState state_;
				double unixtime_        = -1;
				boost::filesystem::path path_;
				boost::filesystem::path file_;
				boost::filesystem::path ini_file_;
				boost::property_tree::ptree pt_;

				std::vector<Channel*> channels_;

};

class CalibrationEvent : public Event {

		public:
				CalibrationEvent(boost::filesystem::path file, boost::filesystem::path ini_file, double unixtime = -1);

				virtual void LoadFiles(EventState state = EVENTSTATE_RAW);
				virtual void LoadSubtracted();

				virtual void SaveEvent(boost::filesystem::path dst, bool save_pd = false);

				virtual ~CalibrationEvent();
				virtual void PrepareHistograms();


		protected:
				long runnr_;

};

/* Forward declaration for class Gain */
class Gain;

/**
 *   Description of the PhysicsEvent class.
 */
class PhysicsEvent : public Event {

		public:
				PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file);
				PhysicsEvent(boost::filesystem::path file, boost::filesystem::path ini_file, boost::filesystem::path rate_file);

				virtual ~PhysicsEvent();

				virtual void LoadFiles(EventState state = EVENTSTATE_RAW);
				virtual void LoadHistograms( boost::filesystem::path file, std::vector<std::string> types = {"wf"} );

				virtual void SaveEvent(boost::filesystem::path dst);

				virtual void PrepareHistograms( boost::property_tree::ptree &settings );

				virtual std::vector<std::vector<OverShootResult> > OverShootCorrection();

				virtual void FastRate( Gain* gain );

				virtual void PrepareTagging();
				virtual void SignalTagging();

				virtual void PrepareDecomposition();
				virtual void WaveformDecomposition(Gain* gain);

				virtual void WaveformReconstruction(Gain* gain);

				virtual void PrepareRetrieval();
				virtual void MipTimeRetrieval();

				virtual bool CheckInjection();

				std::vector<std::vector<double> > GetReconstruction();
				std::vector<Rate > GetRates();
				boost::property_tree::ptree GetPT();
		private:
				boost::filesystem::path rate_file_;
				std::vector<std::vector<double> > reco_;

};

class AnalysisEvent {

		public:
				AnalysisEvent(std::string suffix = "", int min_length = -1);
				AnalysisEvent(PhysicsEvent* ph_evt,std::string suffix="");

				virtual ~AnalysisEvent();

				void AddEvent(PhysicsEvent* ph_evt, NTP_Handler* ntp_handler, std::string injection = "");
				void Normalize();
				void RunFFT();
				void RunPeak();
				void HitEnergySpectrum();

				void SaveEvent(boost::filesystem::path dst, std::string prefix);

				template<typename T>
				T GetParameter(std::string pv)
				{
						return pt_.get<T>( pv );
				};

				int GetRunNr(std::string type = "first");

		protected:
				int n_;
				bool norm_;
				std::string suffix_;
				std::vector<AnalysisChannel*> channels_;

				TGraph* inj_eff_her;
				TGraph* inj_eff_ler;

				TGraph* g_inj_charge_her;
				TGraph* g_inj_charge_ler;

				TGraph* g_current_her;
				TGraph* g_current_ler;

				TGraph* g_vertical_steering_1_her;
				TGraph* g_vertical_steering_1_ler;

				TGraph* g_vertical_steering_2_her;
				TGraph* g_vertical_steering_2_ler;

				boost::property_tree::ptree pt_;
				int first_run_;
				int last_run_;

};


#endif /* CLAWS_EVENT_H_ */
