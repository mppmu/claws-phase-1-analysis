//============================================================================
// Name        : run.chh
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================
#ifndef CLAWS_RUN_H_
#define CLAWS_RUN_H_


// --- BOOST includes ---
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>

// --- Project includes ---
// #include "pedestal.hh"
#include "gain.hh"
#include "event.hh"
#include "ntp_handler.hh"
// #include "globalsettings.hh"


//----------------------------------------------------------------------------------------------
// Definition of the Run class. This class is supposed to do all gthe organization of a run.
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Run
{
		public:
				Run(boost::filesystem::path p);
				virtual ~Run();

				virtual void LoadRunSettings();

		protected:
				boost::filesystem::path path_;
				int nr_;

				boost::property_tree::ptree settings_;

};

class CalibrationRun : public Run
{
		public:
				CalibrationRun(boost::filesystem::path p);
				virtual ~CalibrationRun();

				void SynchronizeCalibrationEvents();
				void PDS_Calibration();
				void GainDetermination();
				void Average1PE();

				void SynchronizePhysicsEvents();
				void PDS_Physics();
				void OverShootCorrection();
				void SignalTagging();
				void WaveformDecomposition();
				void WaveformReconstruction();
				void MipTimeRetrieval();
				void SystematicsStudy();
				void DeleteCalibrationHistograms();

				void SetInjectionLimit( std::string type, NTP_Handler* ntp_handler);
				void SetInjectionRate( std::string ring, double limit);
				void SetCurrentLimit(std::string ring, double min, double max, NTP_Handler *ntp_handler);
				void SetTSLimit(double min, double max);
				void SetStatus(std::string type, std::string status);

				int GetNumber();
				int GetNEvents();
				std::vector<PhysicsEvent*> GetEvents();

		private:
				int cal_nr_;

				std::vector<PhysicsEvent*>           evts_;
				std::vector<CalibrationEvent*>       cal_evts_;

};

#endif /* CLAWS_RUN_H_ */
