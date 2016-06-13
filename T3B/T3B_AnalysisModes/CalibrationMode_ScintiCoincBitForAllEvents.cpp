#include "CalibrationMode_ScintiCoincBitForAllEvents.h"

CalibrationMode_ScintiCoincBitForAllEvents::CalibrationMode_ScintiCoincBitForAllEvents(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->initializeSerialization();

	this->startAnalysis();
}

void CalibrationMode_ScintiCoincBitForAllEvents::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "ScintiCoincidenceBitForAllEvents")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	// settings for the dedicated channel
	_dedicatedChannel = __globalSettingsManager->getSetting("rm_dedicatedChannel").toString();
	QStringList coincidenceScintiOnOsciChannel;
	if(_dedicatedChannel.contains(":"))
	{
		QStringList tmp = _dedicatedChannel.split(":");
		if(tmp.size() != 2)
			throw GeneralException("*** Error: setting 'rm_dedicatedChannel' is set wrong!");
		_dedicatedChannel = tmp[0];
		coincidenceScintiOnOsciChannel = tmp[1].split("-");
		if(coincidenceScintiOnOsciChannel.size() != 2)
			throw GeneralException("*** Error: setting 'rm_dedicatedChannel' is set wrong!");
	}

	QString signalInverted_str = __globalSettingsManager->getSetting("rm_dedicatedChannels_SignalInverted").toString();
	bool signalInverted = false;
	if(signalInverted_str == "true")
		signalInverted = true;
	_scintiTrigger = new SW_AsyncMultiRisingAndFallingEdgeTrigger(this,false,
													 __globalSettingsManager->getSetting("rm_dedicatedChannels_RisingThresholdsMV").toDouble(), 
													 __globalSettingsManager->getSetting("rm_dedicatedChannels_FallingThresholdMV").toDouble(),
													 0,
													 0,
													 __globalSettingsManager->getSetting("rm_dedicatedChannels_MinimalSignalWidth").toDouble(),
													 signalInverted
													 );
	_typicalTriggerTime = __globalSettingsManager->getSetting("rm_dedicatedChannels_TypicalTriggerTimeNS").toDouble();
	_acceptanceRangeAroundTypicalTriggerTime = __globalSettingsManager->getSetting("rm_dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTimeNS").toDouble();

	// renaming of ScintiCoincidencChannel
	_oldAndNewNameOfScintiChannel.first = "";
	_oldAndNewNameOfScintiChannel.second = "";
	QString newNameOfScintiChannel = __globalSettingsManager->getOptionalSetting("rm_renameSciniChannelForOutputFileFromTo").toString();
	if(newNameOfScintiChannel != "") 
	{
		QStringList tmp = newNameOfScintiChannel.split("-");
		if(tmp.size() != 2)
			throw GeneralException("*** Error: Setting 'rm_renameSciniChannelForOutputFileFromTo' has format: 'oldName-newName'! e.g.: 'Sc-Scinti'!");

		_oldAndNewNameOfScintiChannel.first = tmp.at(0);
		_oldAndNewNameOfScintiChannel.second = tmp.at(1);
	}
}

void CalibrationMode_ScintiCoincBitForAllEvents::initializeAnalysisModeVariables()
{
	_EventNumber = 0;
}

void CalibrationMode_ScintiCoincBitForAllEvents::startAnalysis()
{
	int nrAnalyzedEvents = 0;
	int nrTotalEvents = 0;
	int lastSpillNumber = -1;
	while(!this->endOfDataReached() && nrAnalyzedEvents < _nrEventsToAnalyze)// && _spillRead->getSpillInfo()->spillNumber < 1184)
	{
		// load the next waveform and get all current waveforms
		vector <Waveform*> actualWaveforms = this->loadNextWfmOnAllChannels();



		nrTotalEvents++;
		_EventNumber++;
		int currentSpillNumber = _spillRead->getSpillInfo()->spillNumber;
		if(currentSpillNumber != lastSpillNumber) // if true: this is the first event in the current spill
		{
			// set the correct event number (even if we jump over spills)
			_EventNumber = _spillRead->getSpillInfo()->eventNr;
		}
		lastSpillNumber = _spillRead->getSpillInfo()->spillNumber;

		Waveform *currentScintiWaveform = this->triggerDedicatedChannels(&actualWaveforms);

		this->serializeScintillatorCoincidenceInfo(currentScintiWaveform);


		// save the first 50 waveforms
		if(_EventNumber<50)
		{
			for(int i=0; i<actualWaveforms.size(); i++)
			{
				pair <int,int> totalsamplewindow(0,actualWaveforms.at(i)->getScaledSamples()->size());
				_plotPool->writeCanvasWithGraphOfWaveform(actualWaveforms.at(i),totalsamplewindow);
			}
		}

		// console output
		if((nrAnalyzedEvents+1)%10 == 0) 
			cout<<"Analyzing Waveform Number: "<<nrAnalyzedEvents+1
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<nrTotalEvents
			<<" EventNumber_"<<_EventNumber
			<<endl;

		nrAnalyzedEvents++;
	}

	this->finalizeSerialization();

	this->moveRootFileToCalibFolder();
}
// trigger dedicated channel
Waveform* CalibrationMode_ScintiCoincBitForAllEvents::triggerDedicatedChannels(vector <Waveform*> *actualWaveforms)
{
	// trigger the dedicated channels with their respective trigger
	Waveform *scintiWaveform;
	for(int i=0; i<actualWaveforms->size(); i++)
	{
		if(actualWaveforms->at(i)->getChannelName() == _dedicatedChannel)
		{
			vector < Waveform* > scintiChannel;
			scintiChannel.push_back(actualWaveforms->at(i));

			_scintiTrigger->triggerWaveforms(scintiChannel);

			scintiWaveform = actualWaveforms->at(i);
			
			break;
		}
	}

	return scintiWaveform;
}

// Boost serialization///////////////////////////////////////////////////////////
void CalibrationMode_ScintiCoincBitForAllEvents::initializeSerialization()
{
	_serializationObject = new Serialization_ScintillatorCoincidenceBit(this->determineFilePathToCalibrationData(_analyzingIntermediateRM).toStdString(),"write");

	// set the file name if distinct spills were selected for analyzing
	QString outputFileSuffix = QString("-Run%1").arg(_spillRead->getSpillInfo()->runNumber);
	if(_spillsToAnalyze.size() > 1)
		outputFileSuffix.append(QString("-Spill%1-%2").arg(_spillsToAnalyze.at(0)).arg(_spillsToAnalyze.at(_spillsToAnalyze.size() -1 )));
	_serializationObject->setOutputFileSuffix(outputFileSuffix.toStdString());

	// set general run information
	RunInformation *runInfo = _serializationObject->getGeneralInformation()->runInfo;
	runInfo->runDirectory = _runRead->getRunDirectory().toStdString();
	runInfo->runMode = _runRead->getDataRunMode().toStdString();
	runInfo->runNumber = _spillRead->getSpillInfo()->runNumber;
	runInfo->nrEnabledChannels = _spillRead->getSpillInfo()->nrEnabledChannels;
	runInfo->runEnergy = 0;

	// set general conversion information
	ConversionInformation *convInfo = _serializationObject->getGeneralInformation()->conversionInfo;
	convInfo->analysisMode = __globalSettingsManager->getSelectedAnalysisMode().toStdString();
	convInfo->dumpOfAllSettings = __globalSettingsManager->getAllSettings_STLStyle();

	// set general hardware information
	HardwareInformation *hwInfo = _serializationObject->getGeneralInformation()->hardwareInfo;
	QStringList *allOscis = &_spillRead->getSpillInfo()->enabledOsciSerials; 
	for(int i=0; i<allOscis->size(); i++)
	{
		OsciInfo *infoOfCurrentOsci = &_spillRead->getSpillInfo()->osciInfo[allOscis->at(i)];
		QStringList allChannels = infoOfCurrentOsci->enabledChannels;
		for(int j=0; j<infoOfCurrentOsci->enabledChannels.size(); j++)
		{
			ChannelInformation channelInfo;
			channelInfo.nrSamples = infoOfCurrentOsci->noOfSamples;
			channelInfo.channelName = _spillRead->getGeometryInfo()->tileAttachedToOsciChannel[allOscis->at(i)][allChannels.at(j)].toStdString();
			channelInfo.time_increment = infoOfCurrentOsci->timeBasePerSamplePS *1E-12;
			channelInfo.time_totalWindow = infoOfCurrentOsci->noOfSamples * channelInfo.time_increment;
			channelInfo.time_triggerOffset = -infoOfCurrentOsci->triggerNoOfPreTriggerSamples*channelInfo.time_increment;
			channelInfo.volt_plusMinusRange = (double)infoOfCurrentOsci->channelConfigMap[allChannels.at(j)].vertRangeMV / 1000;
			channelInfo.volt_increment = (double)infoOfCurrentOsci->channelConfigMap[allChannels.at(j)].vertRangeMV / 1000 / 32512;
			channelInfo.volt_offset = -infoOfCurrentOsci->channelConfigMap[allChannels.at(j)].analogOffsetMV / 1000;
			channelInfo.volt_triggerThreshold = infoOfCurrentOsci->channelConfigMap[allChannels.at(j)].triggerThreshUpMV;

			if( _spillRead->aSiPMisConnectedToOsciChannel(allOscis->at(i), allChannels.at(j)) )
			{
				int tilePosition = _spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO[allOscis->at(i)][allChannels.at(j)];
				hwInfo->tilePositionToChannelInfo[tilePosition] = channelInfo;
			}
			else
			{
				string dedicatedChannelName = _spillRead->getGeometryInfo()->tileAttachedToOsciChannel[allOscis->at(i)][allChannels.at(j)].toStdString();
				hwInfo->dedicatedChannelNameToChannelInfo[dedicatedChannelName] = channelInfo;
			}
		}
	}

	_serializationObject->writeGeneral();
}
void CalibrationMode_ScintiCoincBitForAllEvents::serializeScintillatorCoincidenceInfo(Waveform* currentWaveform)
{
	ScintillatorCoincidenceInformation *scintillatorCoincidenceInformation = _serializationObject->getScintillatorCoincidenceInformation();

	scintillatorCoincidenceInformation->T3B_spillNumber = _spillRead->getSpillInfo()->spillNumber;
    scintillatorCoincidenceInformation->T3B_eventNumber = _EventNumber;
	scintillatorCoincidenceInformation->dedicatedChanelName = currentWaveform->getChannelName().toStdString();
	if(_oldAndNewNameOfScintiChannel.second != "")
	{
		scintillatorCoincidenceInformation->dedicatedChanelName = _oldAndNewNameOfScintiChannel.second.toStdString();
	}

	scintillatorCoincidenceInformation->TypicalTriggerTime = _typicalTriggerTime;
	scintillatorCoincidenceInformation->AcceptanceRangeAroundTypicalTriggerTime = _acceptanceRangeAroundTypicalTriggerTime;

	bool somethingWasTriggered = currentWaveform->getNrOfSWTriggeredWfms();
	if(somethingWasTriggered)
	{
		vector<float> *timingOfSamples = currentWaveform->getTimingOfSamples();
		for(int k=0; k<currentWaveform->getNrOfSWTriggeredWfms(); k++)
		{
			float triggerTime = currentWaveform->getTimeOfTriggerings()->at(k) *1E9;
			scintillatorCoincidenceInformation->TriggerTime.push_back( triggerTime );

			// was there a signal at the expected trigger time
			scintillatorCoincidenceInformation->T3B_scintillatorCoincidenceBitOn = false;
			if(triggerTime < (_typicalTriggerTime + _acceptanceRangeAroundTypicalTriggerTime) &&
			   triggerTime > (_typicalTriggerTime - _acceptanceRangeAroundTypicalTriggerTime) )
			{
				scintillatorCoincidenceInformation->T3B_scintillatorCoincidenceBitOn = true;
				break;
			}
		}
	}
	else
		scintillatorCoincidenceInformation->T3B_scintillatorCoincidenceBitOn = false;

	 
	_serializationObject->writeSequentialInfo();
}
void CalibrationMode_ScintiCoincBitForAllEvents::finalizeSerialization()
{
	_serializationObject->finalizeSerialization();
}

// destructor
CalibrationMode_ScintiCoincBitForAllEvents::~CalibrationMode_ScintiCoincBitForAllEvents(void)
{
}
