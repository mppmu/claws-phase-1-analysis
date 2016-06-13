/*
 * AnalysisMode.h
 *
 *  Created on: Apr 7, 2010
 *      Author: soldner
 */

#ifndef AnalysisMode_H_
#define AnalysisMode_H_

#include<iostream>
#include<vector>
#include<sstream>

//#include <omp.h>

#include<TString.h>
#include<TFile.h>
#include<TGraph.h>
#include<TMultiGraph.h>
#include<TAxis.h>
#include<TCanvas.h>
#include<TNtupleD.h>
#include<TObject.h>
#include<TDirectory.h>
#include<TH1D.h>
#include<TF1.h>
#include<TLegend.h>
#include<TStyle.h>

#include<QString>
#include<QSettings>
#include<QMap>
#include<QFile>
#include<QTime>
#include<QList>
#include<QListIterator>

#include "Exceptions.h"

#include"ChannelData.h"

#include "LangauFit.h"
#include "SettingsManager.h"
#include "PlotPool.h"

#include "SW_Trigger.h"

#include "Filter.h"
class Filter;
#include "Filter_FWHM.h"
#include "Filter_TotalWfmIntegral.h"
#include "Filter_ScintillatorCoincidence.h"
#include "Filter_TypicalTriggerTime.h"

#include "AccessPedestalSubstractionData.h"
#include "Serialization_SpillSyncInfo.h"

#include "SpillRead.h"
#include "RunRead.h"

class SW_Trigger;

using namespace std;

class AnalysisMode
{
	public:
///////////////////Constructor//////////////////
		AnalysisMode();

///////////////////Functions to Inherit/////////
		virtual void startAnalysis(){};
		virtual void loadRMSpecificAnalysisSettings(){};
		
///////////////////Print elapsed Run Time/////////////
		void printElapsedRunTime();
///////////////////Waveform Filtering/////////////
		bool filterWaveformsWithSelectedFilters(vector <Waveform*> *currentWaveforms);

///////////////////Waveform Triggering/////////////
		bool triggerWaveformsWithSelectedTrigger(vector <Waveform*> *currentWaveforms);

///////////////////dataAccess/////////////////////
		vector <Waveform*> loadNextWfmOnAllChannels();
		vector <Waveform*> getAllCurrentWaveforms();

		bool endOfDataReached();
		bool lastEventReached();

		QString validateSetting(QSettings &settingsFile, QString nameOfSetting);

///////////////////Helper Functions//////
		void moveRootFileToCalibFolder();
		QString determineFilePathToCalibrationData(bool analyzingIntermediateRM);
		QList <int> decomposeInputString(QString composedString);

///////////////////Destructor//////////////////
		virtual ~AnalysisMode();

///////////////////Getter//////////////////
		SpillRead *getCurrentSpillRead()
		{ return _spillRead; }

		PlotPool *getPlotPool()
		{ return _plotPool; }

		int getNumberOfChannels()
		{ return _numberOfChannels; }
		int getNumberOfDedicatedChannels()
		{ return _numberOfDedicatedChannels; }
		int getNumberOfChannels_withSiPMsAttached()
		{ return _numberChannelsWithSiPMsAttached; }

		QString getRunEnergy()
		{ return _runEnergy; }

		SW_Trigger *getTrigger()
		{ return _trig; }

		Filter *getFilter(QString filterName)
		{ 
			if(_selectedFilterMap.contains(filterName))
				return _selectedFilterMap[filterName]; 
			else
				throw GeneralException("*** Error: You requested a Filter that was not initialized!");
		}

		vector<ChannelData*> *getAllChannelDataVec()
		{ return _allChannelDataVec; }
		vector<ChannelData*> *getAllChannelDataVec_withSiPMsAttached()
		{ return _allChannelDataVec_withSiPMsAttached; }
		QMap<QString,ChannelData*> *getChannelDataMap_dedicatedChannels()
		{ return _ChannelDataMap_dedicatedChannels; }

		QString getSelectedAnalysisMode()
		{ return _selectedAnalysisMode; }
		
		QString getAbsoluteWorkingPath()
		{
			QString path_str = QDir::currentPath();

			return path_str;
		}
		
		bool getVariable_doNotInvertWaveforms()
		{
			return _doNotInvertWaveforms;
		}

		// create or get directory to write to
		TDirectory *getTDirectory(QString dirName)
		{ 
			TDirectory *dir = _tFile->GetDirectory(dirName.toStdString().c_str());
			if(!dir)
				dir = _tFile->mkdir(dirName.toStdString().c_str(),dirName.toStdString().c_str());

			return dir; 
		}
		// create or get directory to write to
		TDirectory *getTSubDirectory(TDirectory *dir,QString subDirName)
		{ 
			TDirectory *subDir = dir->GetDirectory(subDirName.toStdString().c_str());
			if(!subDir)
				subDir = dir->mkdir(subDirName.toStdString().c_str(),subDirName.toStdString().c_str());

			return subDir; 
		}

	private:
///////////////////Initializations/////////////////
		void loadGeneralAnalysisSettings();
		void initializeTrigger();
		void initializeFilters();
		void initializeAllChannels();
		void initializePedestalSubstractionData();
		void initializeSpillSyncInfo();

///////////////////dataAccess/////////////////////
		bool nextRunExists();
		void loadNextRun();
	protected:
		bool anotherEventExistsInSpill();
	private:
		bool nextSpillExists();
		void loadNextSpill();
		void switchToIntermediateSpillOrLoadNextSpillIfNotExistent(bool isFirstAnalyzedSpillInRun = false);
		void loadNextRunOrSpill();

		void loadAndApplyPedestalSubstractionValueForCurrentSpill();

		void validateWaveformsOrLoadNextRunOrSpill(vector <Waveform*> &actualWaveforms);

		bool spillDataIsAsynch();
		bool nextSpillContainsIRM();
///////////////////Helper Functions//////
		void createRootOutputFile();
	protected:
		QString getRunAndSpillTitleSuffix();

	protected:
		QTime *_runTime;
		bool _timeAlreadyPrintedThisMin;

		bool _analyzingIntermediateRM;

		double _reduceSamplingByFactor;

		TFile *_tFile;
		PlotPool *_plotPool;

		SW_Trigger *_trig;

		int _waveformCounter;
		int _numberOfChannels;
		int _numberOfDedicatedChannels;
		int _numberChannelsWithSiPMsAttached;
		int _numberOfSpillsOffSynch;
		int _nrEventsToAnalyze;

		bool _informCaliceT3BSynchIniIsMissing;

		QString _selectedAnalysisMode;

		QMap <QString,Filter*> _selectedFilterMap;

		pair <QString, QString> _manuallySetOsciChannelOfScintiCoincidence;

		QString _outputFileName;
		QString _dataPath;
		QString _runEnergy;

		QStringList _runsToAnalyze;
		QStringListIterator *_runsToAnalyzeIT;
		QList <int> _spillsToAnalyze;
		QListIterator<int> *_spillsToAnalyzeIT;

		QMap <QString,QMap<QString,ChannelData*> > *_channelDataMap;
		vector<ChannelData*> *_allChannelDataVec;
		vector<ChannelData*> *_allChannelDataVec_withSiPMsAttached;
		QMap<QString,ChannelData*> *_ChannelDataMap_dedicatedChannels;

		RunRead *_runRead;
		SpillRead *_spillRead;

		AccessPedestalSubstractionData *_pedestalSubstractionData;
		Serialization_SpillSyncInfo *_spillSyncInfo;
		bool _ignoreIfSpillsAsync;

	protected:
		bool _doNotInvertWaveforms;

		//bool _isFirstWaveformInSpill;
		//bool _isFirstSpillInRun;
};

#endif /* MANAGERUNMODES_H_ */

////		void displayActualWaveforms(vector <Waveform*> &actualWaveforms);
////		void saveTimeIntegratedEngDeposOfAveragedWaveforms();
////		void addWaveformsToAverage(vector <Waveform*> &actualWaveforms);
////		void saveWaveformsToAverage();
////		void addWaveformsToTimeDistributionOfEngDeposits(vector <Waveform*> &actualWaveforms);
////		void saveIntegralOfTriggeredWaveforms(vector <Waveform*> &actualWaveforms);
