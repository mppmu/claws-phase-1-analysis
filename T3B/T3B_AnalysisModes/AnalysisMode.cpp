/*
 * AnalysisMode.cpp
 *
 *  Created on: Apr 7, 2010
 *      Author: soldner
 */

#include "AnalysisMode.h"

#include"SW_DefaultTrigger.h"
#include"SW_SyncMultiEdgeTrigger.h"
#include"SW_AsyncMultiEdgeTrigger.h"
#include"SW_AsyncMultiRisingAndFallingEdgeTrigger.h"

///////////////////Constructor/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnalysisMode::AnalysisMode()
{
	// start time measurement
	_runTime = new QTime(0,0,0,0);
	_runTime->start();
	_timeAlreadyPrintedThisMin = false;

	_informCaliceT3BSynchIniIsMissing = true;
	// todo set to 1 if no software reduction of the sampling rate should be done
	_reduceSamplingByFactor = 1;
	// some initialisations
	_trig = NULL;
	_pedestalSubstractionData = NULL;
	_spillSyncInfo = NULL;

	// load general settings from analysisIniFile
	this->loadGeneralAnalysisSettings();

	// load the trigger specific settings and initialize the trigger
	this->initializeTrigger();

	// initialize channelData classes
	this->initializeAllChannels();

	// load the pedestal substraction file and make pedSub values accessible (one value per spill and channel)
	if(_selectedAnalysisMode != "CalibrationPedestalSubstractionIRM" && _selectedAnalysisMode != "CalibrationPedestalSubstractionPhysics")
	{
		if(!_spillSyncInfo->spillNoLargerThanLastSpillWithDataAndOscisInSync(_spillRead->getSpillInfo()->spillNumber))
			this->initializePedestalSubstractionData();
		else
			cerr<<"*** Warning: No Spill with Data and Oscis in sync exists for this Calibration Step -> so no pedestal substraction will be loaded! (AnalysisMode::AnalysisMode())"<<endl;
	}

	// activate all selected filters
	this->initializeFilters();

	// create .root File for output
	this->createRootOutputFile();

	// create the plot pool class which manages all plots
	_plotPool = new PlotPool(this,_tFile);

	// some initialisations
	_waveformCounter = 0;
	_numberOfSpillsOffSynch = 0;
}
///////////////////initializations/////////////////
void AnalysisMode::loadGeneralAnalysisSettings()
{
	_selectedAnalysisMode = __globalSettingsManager->getSelectedAnalysisMode();

	// are we analyzing the Intermediate run mode or not? -> always false if not set
	_analyzingIntermediateRM = false;
	if(__globalSettingsManager->settingsFileContains("general_analyzeIntermediateRM")) 
	{
		_analyzingIntermediateRM = __globalSettingsManager->getSetting("general_analyzeIntermediateRM").toBool();
		cout<<"Analyzing intermediate RunMode (0=false,1=true): "<< _analyzingIntermediateRM <<endl;
	}

	// shall we ignore if spills are async ?
	if(_analyzingIntermediateRM) // the default for IRM is yes
		_ignoreIfSpillsAsync = true;
	else // the default for Physics Mode is no
		_ignoreIfSpillsAsync = false;
	// if general_ignoreIfSpillsAsync explicitly set we will load it
	if(__globalSettingsManager->settingsFileContains("general_ignoreIfSpillsAsync")) 
		_ignoreIfSpillsAsync = __globalSettingsManager->getSetting("general_ignoreIfSpillsAsync").toBool();

	//// nameScintiCoincidencChannel
	//if(__globalSettingsManager->settingsFileContains("general_scintiCoincidenceOnOsciChannel")) 
	//{
	//	QStringList tmp = __globalSettingsManager->getSetting("general_scintiCoincidenceOnOsciChannel").toString().split("-");
	//	if(tmp.size() != 2)
	//		throw GeneralException("*** Error: Setting 'general_scintiCoincidenceOnOsciChannel' has format: 'OsciSerial-ChannelName'! e.g.: 'AY166/047-ChannelD'!");

	//	_manuallySetOsciChannelOfScintiCoincidence.first = tmp.at(0);
	//	_manuallySetOsciChannelOfScintiCoincidence.second = tmp.at(1);
	//}

	// name of output file
	_outputFileName = __globalSettingsManager->getSetting("general_outputFileName").toString();
	// location where the data is saved
	_dataPath = __globalSettingsManager->getSetting("general_dataPath").toString();
	// maximum number of events that should be analyzed
	_nrEventsToAnalyze = __globalSettingsManager->getSetting("general_nrEventsToAnalyze").toInt();
	// run energy of the analysis run (optional)
	_runEnergy = __globalSettingsManager->getOptionalSetting("general_runEnergy").toString();

	
	// run energy of the analysis run (optional)
	_doNotInvertWaveforms = __globalSettingsManager->getOptionalSetting("general_doNotInvertWaveform").toBool();

	// runs that should be analyzed
	QString allRuns = __globalSettingsManager->getSetting("general_Runs").toString();
	_runsToAnalyze = allRuns.split("+",QString::SkipEmptyParts);
	// assign iterator for all runs to analyze
	_runsToAnalyzeIT = new QStringListIterator(_runsToAnalyze);

	// spills that should be analyzed
	QString allSpills= __globalSettingsManager->getOptionalSetting("general_Spills").toString();
	_spillsToAnalyze = this->decomposeInputString(allSpills);
	// assign iterator for all spills to analyze
	_spillsToAnalyzeIT = new QListIterator<int>(_spillsToAnalyze);

	if(_runsToAnalyze.size() > 1 && _spillsToAnalyze.size() > 0)
		throw GeneralException("*** Error: You cannot specify distinct spills when analyzing multiple runs!");
}

void AnalysisMode::initializeTrigger()
{
	if(_trig) delete _trig;

	QString selectedTrigger = __globalSettingsManager->getOptionalSetting("trigger").toString();
	if(selectedTrigger == "SyncMultiEdge") 
	{
		cout<<"Trigger 'SyncMultiEdge' has been selected!"<<endl<<endl;

		bool isSynchTrigger = true;
		_trig = new SW_SyncMultiEdgeTrigger(this,isSynchTrigger);
	}
	else if(selectedTrigger == "AsyncMultiEdge") 
	{
		cout<<"Trigger 'AsyncMultiEdge' has been selected!"<<endl<<endl;

		bool isSynchTrigger = false;
		_trig = new SW_AsyncMultiEdgeTrigger(this,isSynchTrigger);
	}
	else if(selectedTrigger == "AsyncMultiRisingAndFallingEdge") 
	{
		cout<<"Trigger 'AsyncMultiRisingAndFallingEdge' has been selected!"<<endl<<endl;

		bool isSynchTrigger = false;
		_trig = new SW_AsyncMultiRisingAndFallingEdgeTrigger(this,isSynchTrigger);
	}
	else
	{
		cout<<"No Trigger has been selected! Choosing 'No Software Trigger'."<<endl<<endl;

		bool isSynchTrigger = true;
		_trig = new SW_DefaultTrigger(this,isSynchTrigger);
	}
}
void AnalysisMode::initializeFilters()
{
	QStringList allSelectedFilters = __globalSettingsManager->getOptionalSetting("filter_filterType").toString().split("+");

	// activate Filters
	if(allSelectedFilters.contains("FWHM"))
	{
		cout<<"Filter 'FWHM' was selected"<<endl;

		//todo
		_selectedFilterMap[ "FWHM" ] = new Filter_FWHM(this);

		throw GeneralException("*** Filter FWHM not yet ready to use!!");
	}
	if(allSelectedFilters.contains("TypicalTriggerTime"))
	{
		cout<<"Filter 'TypicalTriggerTime' was selected"<<endl;

		_selectedFilterMap[ "TypicalTriggerTime" ] = new Filter_TypicalTriggerTime(this);
	}
	if(allSelectedFilters.contains("TotalWfmIntegral"))
	{
		cout<<"Filter 'TotalWfmIntegral' was selected"<<endl;

		_selectedFilterMap[ "TotalWfmIntegral" ] = new Filter_TotalWfmIntegral(this);
	}
	if(allSelectedFilters.contains("ScintillatorCoincidence"))
	{
		cout<<"Filter 'ScintillatorCoincidence' was selected"<<endl;

		_selectedFilterMap[ "ScintillatorCoincidence" ] = new Filter_ScintillatorCoincidence(this);;
	}

	cout<<endl;
}

void AnalysisMode::initializeAllChannels()
{
	// initialize ChannelDatas
	cout<<"Your Data Path: "<<_dataPath.toStdString().c_str()<<endl<<endl;

	// load runRead
	_runRead = NULL;
	this->loadNextRun();

	// load either specified spills or first spill depending on settings
	if(_spillsToAnalyze.size())
	{
		int nextSpillToLoad = _spillsToAnalyzeIT->next();
		_spillRead = _runRead->getSpill(nextSpillToLoad);

		if(!_spillRead)
			throw GeneralException("*** Error: Starting with a non-existent spill number!");
	}
	else
		_spillRead = _runRead->getFirstSpill();

	if(_analyzingIntermediateRM) 
		this->switchToIntermediateSpillOrLoadNextSpillIfNotExistent(true);

	// create ChannelData for all Oscis and Channels 
	_allChannelDataVec = new vector<ChannelData*>(_spillRead->getSpillInfo()->nrEnabledChannels,NULL);
	_channelDataMap = new QMap< QString, QMap < QString, ChannelData* > >;
	_allChannelDataVec_withSiPMsAttached = new vector<ChannelData*>(_spillRead->getNrChannelsWithSiPMsAttached(), NULL);
	_ChannelDataMap_dedicatedChannels = new QMap<QString,ChannelData*>;
	QStringList *allOsciSerials = _spillRead->getAllEnabledOscis();
	_numberOfChannels = 0;
	_numberOfDedicatedChannels = 0;
	_numberChannelsWithSiPMsAttached = 0;
	for(int j=0; j< _spillRead->getAllEnabledOscis()->size(); j++)
	{	
		QStringList *allEnabledOsciChannels = _spillRead->getAllEnabledChannelsOfOsci(allOsciSerials->at(j));
		for(int k=0; k< allEnabledOsciChannels->size(); k++)
		{
			ChannelData *chData = new ChannelData(allOsciSerials->at(j),allEnabledOsciChannels->at(k),this);

			(*_channelDataMap)[allOsciSerials->at(j)][allEnabledOsciChannels->at(k)] = chData;
			if(_spillRead->aSiPMisConnectedToOsciChannel(allOsciSerials->at(j), allEnabledOsciChannels->at(k)))
			{
				int tilePosition = _spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO[allOsciSerials->at(j)][allEnabledOsciChannels->at(k)];
	 			_allChannelDataVec_withSiPMsAttached->at(tilePosition) = chData;
	
				_allChannelDataVec->at(tilePosition) = chData;
				
				_numberChannelsWithSiPMsAttached++;
			}
			else
			{
				//if(allOsciSerials->at(j) == _manuallySetOsciChannelOfScintiCoincidence.first &&
				//	allEnabledOsciChannels->at(k)) == _manuallySetOsciChannelOfScintiCoincidence.second)
				//{
				//	_spillRead->setScintiCoincidenceChannelManually()
				//}

				(*_ChannelDataMap_dedicatedChannels)[ _spillRead->getGeometryInfo()->tileAttachedToOsciChannel[allOsciSerials->at(j)][allEnabledOsciChannels->at(k)] ] = chData;
			
			 	_allChannelDataVec->at(_spillRead->getSpillInfo()->nrEnabledChannels-1 - _numberOfDedicatedChannels) = chData;
				
				_numberOfDedicatedChannels++;
			}

			// count the number of active channels
			_numberOfChannels++;
		}
	}
}

void AnalysisMode::initializePedestalSubstractionData()
{
	cout<<"Initializing Pedestal Substraction Class"<<endl;

	QString filePathPedSubData = this->determineFilePathToCalibrationData(_analyzingIntermediateRM);
	_pedestalSubstractionData = new AccessPedestalSubstractionData(filePathPedSubData);

	cout<<"Pedestal Substraction Information is now available"<<endl<<endl;

	if(_selectedAnalysisMode != "CalibrationPedestalSubstractionIRM" && _selectedAnalysisMode != "CalibrationPedestalSubstractionPhysics")
		this->loadAndApplyPedestalSubstractionValueForCurrentSpill();
}
void AnalysisMode::initializeSpillSyncInfo()
{
	cout<<"Initializing Spill Synchronization Class"<<endl;

	try
	{
		if(_spillSyncInfo)
			_spillSyncInfo->loadNextRun(this->determineFilePathToCalibrationData(_analyzingIntermediateRM).toStdString());
		else
		{
			_spillSyncInfo = new Serialization_SpillSyncInfo(this->determineFilePathToCalibrationData(_analyzingIntermediateRM).toStdString(),"read");

			cout<<"Spill Synchronization Information is now available"<<endl<<endl;
		}
	}
	catch(FileNotFoundException &fnfE)
	{
		cerr<<fnfE.what()<<endl;

		_spillSyncInfo = NULL;
	}

	// now read the sync info for the whole run at once
	if(_spillSyncInfo)
		_spillSyncInfo->readSequentialInfoForWholeRun();
}
///////////////////Print elapsed Run Time/////////////
void AnalysisMode::printElapsedRunTime()
{
	// print elapsed time
	int seconds = (double)_runTime->elapsed() / 1000;
	int minutes = seconds / 60;
	seconds -= minutes*60;
	if(seconds == 1 && _timeAlreadyPrintedThisMin == false)
	{
		cout<<"//////////////////Elapsed Time Since Run Start: "<<minutes<<"min "<<seconds<<"s//////////////////"<<endl;

		_timeAlreadyPrintedThisMin = true;
	}
	
	if(seconds != 1)
		_timeAlreadyPrintedThisMin = false;

}
///////////////////Waveform Filtering/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AnalysisMode::filterWaveformsWithSelectedFilters(vector <Waveform*> *currentWaveforms)
{
	bool notAllTriggersWereFilteredOut = true;
	foreach (Filter *currentFilter, _selectedFilterMap)
	{
		notAllTriggersWereFilteredOut = currentFilter->filterWaveforms(currentWaveforms);
	}

	return notAllTriggersWereFilteredOut;
}
///////////////////Waveform Triggering//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AnalysisMode::triggerWaveformsWithSelectedTrigger(vector <Waveform*> *currentWaveforms)
{
	bool somethingWasTriggered = _trig->triggerWaveforms(*currentWaveforms);

	return somethingWasTriggered;
}

///////////////////dataAccess/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// some necessary dataAccess functions
bool AnalysisMode::nextRunExists()
{
	return _runsToAnalyzeIT->hasNext();
}

void AnalysisMode::loadNextRun()
{
	if(_runRead) delete _runRead;
	
	QString currentRunToAnalyze = "";
	if(_runsToAnalyze.size() && _dataPath != "")
	{
		currentRunToAnalyze = _runsToAnalyzeIT->next();
		_runRead = new RunRead(QString("%1/Run_%2").arg(_dataPath).arg(currentRunToAnalyze));
	}
	else 
	{
		string errorMessage;
		errorMessage.append("DataPath ").append(_dataPath.toStdString().c_str()).append(" does not exist or no runs were specified!");
		throw GeneralException(errorMessage.c_str());
	}

	// load first Spill
	_spillRead = _runRead->getFirstSpill();
	//_spillRead = _runRead->getSpill(650);
	if(_analyzingIntermediateRM) 
		this->switchToIntermediateSpillOrLoadNextSpillIfNotExistent();

	// initialize spillSyncInfo (necessary to check if all oscis were in Sync for Spill)
	if(_selectedAnalysisMode != "CalibrationT3BSyncInfo")
		this->initializeSpillSyncInfo();
}
void AnalysisMode::switchToIntermediateSpillOrLoadNextSpillIfNotExistent(bool isFirstAnalyzedSpillInRun)
{
	if(! _spillRead->getCurrentSpillDirectory()->entryList(QDir::AllDirs).contains("IntermediateGain"))
	{
		cerr<<"*** Warning: Spill number "<<_spillRead->getSpillInfo()->spillNumber<<" does not contain IntermediateGain Data!"<<endl;

		if(isFirstAnalyzedSpillInRun)
		{
			cerr<< "It seems like we start calibrating with the last spill in the Run and it does not have an IRM!" <<endl;
			cerr<< "No T3B file will be created --> exiting!" <<endl;
			exit(0);
		}

		this->loadNextRunOrSpill();
	}
	else
		_spillRead = _spillRead->getIntermediateSpill();
}
bool AnalysisMode::nextSpillExists()
{
	bool nextSpillExists = false;

	if(_spillsToAnalyze.size())
	{
		if(!_spillsToAnalyzeIT->hasNext())
			return false;

		int nextSpillToLoad = _spillsToAnalyzeIT->peekNext();
		nextSpillExists = _runRead->spillExists(nextSpillToLoad);
	}
	else
		nextSpillExists = _runRead->nextSpillExists();

	return nextSpillExists;
} 
void AnalysisMode::loadNextSpill()
{
	cout<<"Loading spill number "<<_spillRead->getSpillInfo()->spillNumber+1<<endl;

	// load either specified spills or consecutive spill depending on settings
	if(_spillsToAnalyze.size())
	{
		int nextSpillToLoad = _spillsToAnalyzeIT->next();
		_spillRead = _runRead->getSpill(nextSpillToLoad);
	}
	else
		_spillRead = _runRead->getNextSpill();

	// switch to intermediate run mode if meant to
	if(_analyzingIntermediateRM) 
		this->switchToIntermediateSpillOrLoadNextSpillIfNotExistent();

	// load the corresponding pedestal substraction values
	if(_selectedAnalysisMode != "CalibrationPedestalSubstractionIRM" && _selectedAnalysisMode != "CalibrationPedestalSubstractionPhysics")
		this->loadAndApplyPedestalSubstractionValueForCurrentSpill();
}
bool AnalysisMode::anotherEventExistsInSpill()
{
	bool anotherEventExists = true;

	// wfmReadOfAllCH is only filled with the channels that that could be found in the spill folder
	QList <WaveformRead*> *wfmReadOfAllCH = _spillRead->getAllChannelDataQList();
	int nrPhysicsChannelsWithDataInSpill = 0;
	for(int i=0; i<(int)wfmReadOfAllCH->size();  i++)
	{
		QString currentOsciSerial = wfmReadOfAllCH->at(i)->getOsciSerial();
		QString currentChannel = wfmReadOfAllCH->at(i)->getChannelSerial();
		if(currentOsciSerial == "PS2000")
			continue;
		// in intermediate run mode it should not matter if the dedicated channels (scinti, veto, cerenkov) have data
		if(_analyzingIntermediateRM && !this->getCurrentSpillRead()->aSiPMisConnectedToOsciChannel(currentOsciSerial,currentChannel))
			continue;
		nrPhysicsChannelsWithDataInSpill++;

		// if one of the channels does not have another waveform we reached the end of the current spill
		if(!wfmReadOfAllCH->at(i)->nextWaveformExists())
		{
			anotherEventExists = false;
			break;
		}
	}

	int nrPhysicsChannelsThatShouldHaveDataInSpill = _numberOfChannels;
	// in intermediate run mode it should not matter if the dedicated channels (scinti, veto, cerenkov) have data
	if(_analyzingIntermediateRM)
		nrPhysicsChannelsThatShouldHaveDataInSpill = _numberOfChannels - _spillRead->getNrDedicatedChannels();

	// in case the spill folder does not contain data of a physics channel that should be activated (e.g. empty spill)
	if(nrPhysicsChannelsWithDataInSpill != nrPhysicsChannelsThatShouldHaveDataInSpill)
		anotherEventExists = false;

	return anotherEventExists;
}
void AnalysisMode::loadNextRunOrSpill()
{
	if(this->nextSpillExists())
	{
		this->loadNextSpill();
	}
	else if(!this->nextSpillExists() && this->nextRunExists())
	{
		this->loadNextRun();
	}
	else
		throw GeneralException("*** Error: loadNextRunOrSpill in an undefined state!");
}
// set the pedestal Substraction Value in all ChannelData objects
void AnalysisMode::loadAndApplyPedestalSubstractionValueForCurrentSpill()
{
	for(int i=0; i< _allChannelDataVec_withSiPMsAttached->size(); i++)
	{
		pair<QString,QString> *osciChannel = _allChannelDataVec->at(i)->getDataCorrespondingOsciAndChannel();
		int tilePosition = _spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO[osciChannel->first][osciChannel->second];

		double pedestalSubstractionValue = _pedestalSubstractionData->getPedestalSubstractionValue(_spillRead->getSpillInfo()->spillNumber,tilePosition);

		_allChannelDataVec->at(i)->setPedestalSubstractionValue(pedestalSubstractionValue);
	}
}
// load next waveform for all channelData objects (all channels)
vector <Waveform*> AnalysisMode::loadNextWfmOnAllChannels()
{
	// print the elapsed run time every minute (if nrElapsedSeconds == 1)
	this->printElapsedRunTime();

	// load the next Waveforms
	for (int i = 0; i < _allChannelDataVec->size(); i++)
	{
		_allChannelDataVec->at(i)->loadNextWaveform();
	}

	vector <Waveform*> actualWfms = this->getAllCurrentWaveforms();

	// validateWaveformsOrLoadNextRunOrSpill calls this function recursively if endOfSpill or empty/asynch spill
	this->validateWaveformsOrLoadNextRunOrSpill(actualWfms);

	return actualWfms;
}
// obtain current waveforms for all channels (within "startAnalysis" function)
vector <Waveform*> AnalysisMode::getAllCurrentWaveforms()
{
	vector <Waveform*> actualWaveforms;
	int nrChWithoutANextWfm = 0;
	for (int i = 0; i < _allChannelDataVec->size(); i++)
	{
		// in intermediate run mode it should not matter if the dedicated channels (scinti, veto, cerenkov) have data
		if(_analyzingIntermediateRM && !_allChannelDataVec->at(i)->getActualWaveform() && _allChannelDataVec->at(i)->isWfmOfDedicatedChannel())
			continue;

		if(!_allChannelDataVec->at(i)->getActualWaveform()) 
			nrChWithoutANextWfm++;

		actualWaveforms.push_back(_allChannelDataVec->at(i)->getActualWaveform());
	}

	//if(nrChWithoutANextWfm == (int)_allChannelDataVec->size())
	if(nrChWithoutANextWfm > 0)
		actualWaveforms.clear();

	return actualWaveforms;
}

// load next spill or run if end of spill is reached or if spill is empty or asynch
void AnalysisMode::validateWaveformsOrLoadNextRunOrSpill(vector <Waveform*> &actualWaveforms)
{
	// if all waveforms in this spill are done and the spill is not empty
	// in case our oscis were asynch during data taking
	
	// check if there is data in the spill
	bool noDataFoundInSpill = !actualWaveforms.size();

	// check if spill is async
	bool spillDataIsAsync = false;
	if(!_ignoreIfSpillsAsync && !noDataFoundInSpill) // if data was found -> check if spill is async
		spillDataIsAsync = this->spillDataIsAsynch();

	if(noDataFoundInSpill || spillDataIsAsync) 
	{
		this->loadNextRunOrSpill();

		actualWaveforms = this->loadNextWfmOnAllChannels(); 
	}
}

bool AnalysisMode::spillDataIsAsynch()
{
	bool oscisOffSynchBit = false;

	if(_spillSyncInfo)
		oscisOffSynchBit = _spillSyncInfo->getOscisOffSyncInSpill(_spillRead->getSpillInfo()->spillNumber);

	if(oscisOffSynchBit) 
	{
		cerr<<"*** Warning: Oscis off synch for Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<endl;

		_numberOfSpillsOffSynch++;

		cout<<"*** Number of off synch spills so far: "<<_numberOfSpillsOffSynch<<endl;
	}

	return oscisOffSynchBit;
}


bool AnalysisMode::lastEventReached()
{
	bool lastEventReached = false;

	// return true immediately if the current spill number is higher than the highest spill number containing data and oscis in sync --> this can happen if e.g. the last 10 spills contain no data, but the grid job sequence starts with a spill larger than the last spill containing data and beeing in sync
	if(_spillSyncInfo->spillNoLargerThanLastSpillWithDataAndOscisInSync(_spillRead->getSpillInfo()->spillNumber))
	{
		cerr<<"*** Warning: The current spill number is larger than the largest spill number with data and oscis in sync! (AnalysisMode::lastEventReached())"<<endl;
		return true;
	}

	// check if there is data in the spill
	bool noFurtherEventInSpill = !this->anotherEventExistsInSpill();

	// check if spill is async
	bool spillDataIsAsync = false;
	if(!_ignoreIfSpillsAsync && !noFurtherEventInSpill) // if data was found -> check if spill is async
		spillDataIsAsync = this->spillDataIsAsynch();

	if(noFurtherEventInSpill || spillDataIsAsync)
	{
		if(!this->nextSpillExists() && !this->nextRunExists())
			lastEventReached = true;
		else if(_spillSyncInfo && !this->nextRunExists())
		{
			if(_spillsToAnalyze.size())
			{
				if(!_spillSyncInfo->anotherSpillExistsWithDataAndOscisInSync(_spillRead->getSpillInfo()->spillNumber,_spillsToAnalyze.last()))
					lastEventReached = true;
			}
			else
			{
				if(!_spillSyncInfo->anotherSpillExistsWithDataAndOscisInSync(_spillRead->getSpillInfo()->spillNumber))
					lastEventReached = true;
			}
		}
		else if(_analyzingIntermediateRM && !this->nextSpillContainsIRM()) // we assume that only the last spill can contain no intermediate gain info
			lastEventReached = true;
	}

	return lastEventReached;
}
bool AnalysisMode::endOfDataReached()
{
	return this->lastEventReached();
//	return _endOfDataReached;
}
bool AnalysisMode::nextSpillContainsIRM()
{
	QString pathOfNextSpillDir = _runRead->peekPathToNextSpill();
	if(pathOfNextSpillDir == "") // the next spill does not even exist
		return false;

	QDir nextSpillDir(pathOfNextSpillDir);
	if( !nextSpillDir.entryList(QDir::AllDirs).contains("IntermediateGain"))
	{
		cout<<"*** Warning: Spill number "<<_spillRead->getSpillInfo()->spillNumber + 1<<" does not contain IntermediateGain Data!"<<endl;

		return false;
	}

	return true;
}
QString AnalysisMode::validateSetting(QSettings &settingsFile, QString nameOfSetting)
{
	if(!settingsFile.contains(nameOfSetting)) 
	{
		string errorMessage;
		errorMessage.append("The setting ").append(nameOfSetting.toStdString().c_str()).append(" couldn't be converted or is missing! Please review.");
		throw GeneralException(errorMessage.c_str());
	}

	return nameOfSetting;
}

///////////////////Helper Functions///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString AnalysisMode::determineFilePathToCalibrationData(bool analyzingIntermediateRM)
{
	QString runPath = _runRead->getRunDirectory();
	QDir calibDir(runPath);
	QString calibDirName = "Calibration";
	if(!calibDir.entryList(QDir::AllDirs).contains(calibDirName))
		calibDir.mkdir(calibDirName);
	calibDir.cd(calibDirName);
	QString filePath = calibDir.absolutePath();

	QDir calibSubDir(calibDir.absolutePath());
	if(analyzingIntermediateRM)
	{
		QString calibSubDirName = "IntermediateRM";
		if(!calibSubDir.entryList(QDir::AllDirs).contains(calibSubDirName))
			calibSubDir.mkdir(calibSubDirName);
		calibSubDir.cd(calibSubDirName);
		filePath = calibSubDir.absolutePath();
	}

	return filePath;
}
void AnalysisMode::moveRootFileToCalibFolder()
{
	QString filePath = _runRead->getRunDirectory();
	QDir calibDir(filePath);
	QString calibDirName = "Calibration";
	if(!calibDir.entryList(QDir::AllDirs).contains(calibDirName));
		calibDir.mkdir(calibDirName);
	calibDir.cd(calibDirName);
	QString calibDirPath = calibDir.absolutePath();

	// create new subdirectory if the intermediate RM is analyzed
	QDir calibSubDir(calibDir.absolutePath());
	if(_analyzingIntermediateRM)
	{
		QString calibSubDirName = "IntermediateRM";
		if(!calibSubDir.entryList(QDir::AllDirs).contains(calibSubDirName))
			calibSubDir.mkdir(calibSubDirName);
		calibSubDir.cd(calibSubDirName);
		calibDirPath = calibSubDir.absolutePath();
	}

	// make sure the TFile is closed
	_tFile->Close();

	// get current absulte path
	QString path = this->getAbsoluteWorkingPath();

	// concatenate file path
	QString pathOfFileToMove = QString("%1/%2").arg(path).arg(_tFile->GetPath());
	pathOfFileToMove.chop(2);

	// make sure the moving is white space safe
	pathOfFileToMove.prepend("\"");
	pathOfFileToMove.append("\"");
	calibDirPath.prepend("\"");
	calibDirPath.append("\"");

	// do the moving
	QString moveCommand = QString("mv %1 %2").arg(pathOfFileToMove).arg(calibDirPath);
	system(moveCommand.toStdString().c_str());
}
QList <int> AnalysisMode::decomposeInputString(QString composedString)
{
	QList <int> decomposedList;

	// split by +
	QStringList plusDecomposed = composedString.split("+");

	// split by -
	for(int i=0; i<plusDecomposed.size(); i++)
	{
		QStringList minusDecomposed = plusDecomposed.at(i).split("-");
		if(minusDecomposed.size() == 1)
		{
			if(minusDecomposed.at(0) != "")
				decomposedList.append(minusDecomposed.at(0).toInt());
		}
		else if(minusDecomposed.size() == 2)
		{
			int lowerIndex = minusDecomposed.at(0).toInt();
			int upperIndex = minusDecomposed.at(1).toInt();
	
			if(upperIndex < lowerIndex)
				throw GeneralException("*** Error: String decomposition unsuccessful! \nLower Index > Upper Index!");

			for(int j=lowerIndex; j<=upperIndex; j++)
			{
				decomposedList.append(j);
			}
		}
		else
			throw GeneralException("*** Error: String decomposition unsuccessful!");
	}

	return decomposedList;
}
void AnalysisMode::createRootOutputFile()
{
	// check if the directory outputFiles exists
	QString path = this->getAbsoluteWorkingPath();
	QDir tFileDir(path);
	if(!tFileDir.entryList(QDir::AllDirs).contains("outputFiles"))
		tFileDir.mkdir("outputFiles");

	QString concatenatedOutputFileName = QString("outputFiles/%1").arg(_outputFileName);
	QString concatenatedOutputFileName_justSpillSuffix = _outputFileName;

	// append the suffix indicating which spills are analyzed
	QString suffix = this->getRunAndSpillTitleSuffix();

	// rename the _outputFileName variable accordingly
	concatenatedOutputFileName_justSpillSuffix.append(suffix);
	_outputFileName = concatenatedOutputFileName_justSpillSuffix;

	// create the root file name containing its relative path from the current directory
	concatenatedOutputFileName.append(suffix);
	concatenatedOutputFileName.append(".root");

	// create the root file
	_tFile = new TFile(concatenatedOutputFileName.toStdString().c_str(),"RECREATE",_outputFileName.toStdString().c_str());
}
QString AnalysisMode::getRunAndSpillTitleSuffix()
{
	QString suffix = "";

	// for Run
	QString runSuffix = "";
	if(_runsToAnalyze.size() > 1)
		runSuffix = QString("-Run%1-%2").arg(_runsToAnalyze.at(0)).arg(_runsToAnalyze.at(_runsToAnalyze.size() - 1));
	else
		runSuffix = QString("-Run%1").arg(_runsToAnalyze.at(0));
	
	// append the run suffix
	suffix.append(runSuffix);

	// for spills
	if(_spillsToAnalyze.size())
	{
		QString spillSuffix = QString("-Spill%1-%2").arg(_spillsToAnalyze.at(0)).arg(_spillsToAnalyze.at(_spillsToAnalyze.size() - 1));

		// append the spill suffix
		suffix.append(spillSuffix);
	}

	if(_analyzingIntermediateRM)	
		suffix.append("-IRM");

	return suffix;
}

///////////////////Destructor///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AnalysisMode::~AnalysisMode()
{
	if(_tFile)
		_tFile->Close();
}
