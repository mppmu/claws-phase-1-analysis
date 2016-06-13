#include "CalibrationModeStandalone_T3BSyncInfo.h"

CalibrationModeStandalone_T3BSyncInfo::CalibrationModeStandalone_T3BSyncInfo(void)
{
	this->loadSettings();
	this->loadRun();
	this->initializeAnalysisModeVariables();

	this->initializeSerialization();
	this->initializeSpillSyncInfoHumanReadible();

	this->startAnalysis();
}

void CalibrationModeStandalone_T3BSyncInfo::initializeAnalysisModeVariables()
{
	// obtain the path to the calibration directory
	_pathToCalibDir = this->getPathToCalibDir();
}
void CalibrationModeStandalone_T3BSyncInfo::loadSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationT3BSyncInfoIRM" && __globalSettingsManager->getSelectedAnalysisMode() != "CalibrationT3BSyncInfoPhysics")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	// general
	_dataPath = __globalSettingsManager->getSetting("general_dataPath").toString();
	_runNumber = __globalSettingsManager->getSetting("general_Runs").toInt();
	_analyzingIntermediateRM = __globalSettingsManager->getSetting("general_analyzeIntermediateRM").toBool();

	// reading coincidence Scinti On Osci/Channel
	_coincidenceScintiChannelWasSpecified = false;
	QString string_coincidenceScintiOnOsciChannel = 
		__globalSettingsManager->getOptionalSetting("rm_coincidenceScintiOnOsciChannel").toString();
	if(!_analyzingIntermediateRM && string_coincidenceScintiOnOsciChannel != "") 
	{
		_coincidenceScintiChannelWasSpecified = true;

		// rm specific
		QStringList coincidenceScintiOnOsciChannel = string_coincidenceScintiOnOsciChannel.split("-");
		_coincidenceScintiOnOsciChannel.first = coincidenceScintiOnOsciChannel[0];
		_coincidenceScintiOnOsciChannel.second = coincidenceScintiOnOsciChannel[1];

		_thresholdOnCoincScintiMV = __globalSettingsManager->getSetting("rm_thresholdOnCoincScintiMV").toDouble();

		_ScintillatorCoincSignalInverted = __globalSettingsManager->getSetting("rm_CoincidenceScintillatorSignalInverted").toBool();
	}


	// load the spills that should be analyzed
	_minAndMaxSpillToAnalyze.first = -1;
	_minAndMaxSpillToAnalyze.second = -1;
	QString _minAndMaxSpillToAnalyze_str = __globalSettingsManager->getOptionalSetting("general_Spills").toString();
	if(_minAndMaxSpillToAnalyze_str != "") 
	{
		QStringList _minAndMaxSpillToAnalyze_strList = _minAndMaxSpillToAnalyze_str.split("-");
		if(_minAndMaxSpillToAnalyze_strList.size() == 2)
		{
			bool okMin = false;
			bool okMax = false;
			_minAndMaxSpillToAnalyze.first = _minAndMaxSpillToAnalyze_strList.at(0).toInt(&okMin);
			_minAndMaxSpillToAnalyze.second = _minAndMaxSpillToAnalyze_strList.at(1).toInt(&okMax);
			if(!okMin || !okMax)
				throw GeneralException("*** Error: The setting 'general_Spills' could not be recognized (in loadSettings())");
		}
		else
			throw GeneralException("*** Error: The setting 'general_Spills' could not be recognized (in loadSettings())");
	}
}
void CalibrationModeStandalone_T3BSyncInfo::loadRun()
{
	if(_runNumber && _dataPath != "")
		_runRead = new RunRead(QString("%1/Run_%2").arg(_dataPath).arg(_runNumber));
	else 
	{
		string errorMessage;

		errorMessage.append("DataPath ").append(_dataPath.toStdString().c_str()).append(" does not exist or no runs were specified!");
		throw GeneralException(errorMessage.c_str());
	}

	// load first Spill
	_spillRead = _runRead->getFirstSpill();

	// if setting "general_Spills" is set -> load the first spill specified
	if(_minAndMaxSpillToAnalyze.first != -1 && _minAndMaxSpillToAnalyze.second != -1)
		_spillRead = _runRead->getSpill(_minAndMaxSpillToAnalyze.first);

	// switch to intermediate run mode if specified
	if(_analyzingIntermediateRM) 
	{
		bool success = this->switchToIntermediateSpillOrLoadNextSpillIfNotExistent();
		if(!success)
		{
			cerr<< "It seems like we start calibrating with the last spill in the Run and it does not have an IRM!" <<endl;
			cerr<< "No T3B file will be created --> exiting!" <<endl;
			exit(0);
//			throw GeneralException("*** Error: The run does not seem to contain an intermediate run mode!");
		}
	}
}

void CalibrationModeStandalone_T3BSyncInfo::startAnalysis()
{
	int spillcounter = 0;
	bool firstSpillProcessed = false;
	while ( (_runRead->nextSpillExists() && !this->lastSpillThatShouldBeAnalyzedReached()))// || spillcounter == 0)
	{
		// we shall not load the next spill the first time the loop is entered
		if(spillcounter != 0 || firstSpillProcessed)  
		{
			_spillRead = _runRead->getNextSpill();

			// switch to intermediate run mode if specified
			if(_analyzingIntermediateRM) 
			{
				bool success = this->switchToIntermediateSpillOrLoadNextSpillIfNotExistent();
				if(!success)
					continue;
			}
		}
		firstSpillProcessed = true;

		if(!this->spillContainsData())
			continue;



		// spill number
		_spillSyncInfo["T3BSpillNr"] = _spillRead->getSpillInfo()->spillNumber; 

		// spill timing
		_spillSyncInfo["timeOfDataReady"] = _spillRead->getSpillInfo()->timeOfDataReady.toTime_t(); 
		_spillSyncInfo["timeOfTransferToPcReady"] = _spillRead->getSpillInfo()->timeOfTransferToPcReady.toTime_t();
		_spillSyncInfo["timeOfAcquisitionStart"] = _spillRead->getSpillInfo()->timeOfAcquisitionStart.toTime_t();
		_spillSyncInfo["timeOfDataReadyHR"] = _spillRead->getSpillInfo()->timeOfDataReady.toString("yyyy-MM-ddThh:mm:ss:zzz"); 
		_spillSyncInfo["timeOfTransferToPcReadyHR"] = _spillRead->getSpillInfo()->timeOfTransferToPcReady.toString("yyyy-MM-ddThh:mm:ss:zzz");
		_spillSyncInfo["timeOfAcquisitionStartHR"] = _spillRead->getSpillInfo()->timeOfAcquisitionStart.toString("yyyy-MM-ddThh:mm:ss:zzz");

		// osci serials and nr captured wfms
		QStringList allChannelNames;
		allChannelNames << "ChannelA" << "ChannelB" << "ChannelC" << "ChannelD";
		QStringList *allOscis = &_spillRead->getSpillInfo()->enabledOsciSerials; 
		for(int i=0; i<allOscis->size(); i++)
		{
			for(int j=0; j<4; j++)
			{
				if(_spillRead->dataExistsForOsciChannel(allOscis->at(i), allChannelNames.at(j)))
				{
					_spillSyncInfo[QString("PS6000_%1_nrCaptures").arg(i)] = _spillRead->getChannelData(allOscis->at(i), allChannelNames.at(j))->getNrWaveforms();
					break;
				}
			}
		}

		// are the oscis off sync
		bool isOffSync = false;
		for(int i=0; i<allOscis->size(); i++)
		{
			for(int j=0; j<4; j++)
			{
				if(_spillRead->dataExistsForOsciChannel(allOscis->at(i), allChannelNames.at(j)))
				{
					if(i && _spillRead->getChannelData(allOscis->at(i-1), allChannelNames.at(j))->getNrWaveforms() != _spillRead->getChannelData(allOscis->at(i), allChannelNames.at(j))->getNrWaveforms())
					{
						isOffSync = true;
						break;
					}
				}
			}
		}
		_spillSyncInfo["oscisOffSyncBit"] = isOffSync;

		// number of coincidence events
		int countT3bCoincidenceEvents = 0;
		if(!_analyzingIntermediateRM && _coincidenceScintiChannelWasSpecified)
			countT3bCoincidenceEvents = this->getNrT3BCoincidenceEvents();
		_spillSyncInfo["#CoincEv"] = countT3bCoincidenceEvents; 



		// now write the gathered spill sync info
		this->serializeSpillSyncInfo();
		this->writeSpillSyncInfoHumanReadible();


		if(spillcounter % 10 == 0)
			cout<<"Checking spill number: "<<_spillRead->getSpillInfo()->spillNumber<<endl;


		spillcounter++;
	}

	this->finalizeSerialization();
	this->finalizeSpillSyncInfoHumanReadible();
}
int CalibrationModeStandalone_T3BSyncInfo::getNrT3BCoincidenceEvents()
{
	ChannelConfig c = _spillRead->getSpillInfo()->osciInfo[_coincidenceScintiOnOsciChannel.first].channelConfigMap[_coincidenceScintiOnOsciChannel.second];
	double threshold = (c.analogOffsetMV + _thresholdOnCoincScintiMV*(-1)) * 32512. / c.vertRangeMV;

	// get waveformRead of the osci with the scintillator coincidence which is always stopped last
	WaveformRead* w = _spillRead->getChannelData(_coincidenceScintiOnOsciChannel.first, _coincidenceScintiOnOsciChannel.second);

	int countT3bCoincidenceEvents = 0;
	while (w->nextWaveformExists())
	{
		w->readNextWaveform();
		short* buffer = w->getBuffer();

		for (int i=0; i<w->getNrSamplesPerWaveform(); i++)
		{
			short currentCheckedSample = buffer[i];

			if(_ScintillatorCoincSignalInverted)
			{
				if (currentCheckedSample > threshold)
				{
					countT3bCoincidenceEvents++;
					break;
				}
			}
			else
			{
				if (currentCheckedSample < threshold)
				{
					countT3bCoincidenceEvents++;
					break;
				}
			}
		}
	}

	return countT3bCoincidenceEvents;
}
// helper functions
bool CalibrationModeStandalone_T3BSyncInfo::switchToIntermediateSpillOrLoadNextSpillIfNotExistent()
{
	if(!_spillRead->getCurrentSpillDirectory()->entryList(QDir::AllDirs).contains("IntermediateGain"))
	{
		cerr<<"*** Warning: Spill number "<<_spillRead->getSpillInfo()->spillNumber<<" does not contain IntermediateGain Data!"<<endl;

		if(_runRead->nextSpillExists())
		{
			_spillRead = _runRead->getNextSpill();
			return this->switchToIntermediateSpillOrLoadNextSpillIfNotExistent();
		}
		else
			return false;
	}
	else
	{
		_spillRead = _spillRead->getIntermediateSpill();

		return true;
	}
}
bool CalibrationModeStandalone_T3BSyncInfo::spillContainsData()
{
	bool spillContainsData = false;

	// identify channels with tiles attached, so that they should contain data
	int nrRequiredChannelsWithData = _spillRead->getGeometryInfo()->tilePositionInnerToOuter.size();
	int nrChannelsWithData = 0;
	QList< WaveformRead* > *allChannelData = _spillRead->getAllChannelDataQList();
	for(int i=0; i<allChannelData->size(); i++)
	{
		QMap < QString, QMap < QString,int > > *osciChannelToTilePosition = &_spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO;
		if( osciChannelToTilePosition->contains(allChannelData->at(i)->getOsciSerial()) && 
			(*osciChannelToTilePosition)[allChannelData->at(i)->getOsciSerial()].contains(allChannelData->at(i)->getChannelSerial()) )
			nrChannelsWithData++;
	}

	if(nrRequiredChannelsWithData == nrChannelsWithData)	// 16 channels + ps2000
		spillContainsData = true;
	else
		cout<<"No Data found in Spill Number: "<<_spillRead->getSpillInfo()->spillNumber<<endl;

	return spillContainsData;
}
QString CalibrationModeStandalone_T3BSyncInfo::getPathToCalibDir()
{
	QString runPath = _runRead->getRunDirectory();
	QDir calibDir(runPath);
	QString calibDirName = "Calibration";
	if(!calibDir.entryList(QDir::AllDirs).contains(calibDirName))
		calibDir.mkdir(calibDirName);
	calibDir.cd(calibDirName);
	QString filePath = calibDir.absolutePath();

	QDir calibSubDir(calibDir.absolutePath());
	if(_analyzingIntermediateRM)
	{
		QString calibSubDirName = "IntermediateRM";
		if(!calibSubDir.entryList(QDir::AllDirs).contains(calibSubDirName))
			calibSubDir.mkdir(calibSubDirName);
		calibSubDir.cd(calibSubDirName);
		filePath = calibSubDir.absolutePath();
	}

	return filePath;
}
bool CalibrationModeStandalone_T3BSyncInfo::lastSpillThatShouldBeAnalyzedReached()
{
	bool lastSpillToAnalyzeReached = false;

	if(_minAndMaxSpillToAnalyze.first != -1 && _minAndMaxSpillToAnalyze.second != -1)
	{
		int currentSpillNumber = _spillRead->getSpillInfo()->spillNumber;
		if(currentSpillNumber >= _minAndMaxSpillToAnalyze.first &&
		   currentSpillNumber < _minAndMaxSpillToAnalyze.second)
		   lastSpillToAnalyzeReached = false;
		else if(currentSpillNumber == _minAndMaxSpillToAnalyze.second)
			lastSpillToAnalyzeReached = true;
		else
		{
			QString errorMessage = QString("*** Error: Trying to check spillNumber %1 which is out of the range %2-%3of general_Spills! "
										   "(in CalibrationModeStandalone_T3BSyncInfo::lastSpillThatShouldBeAnalyzedReached())")
										   .arg(currentSpillNumber)
										   .arg(_minAndMaxSpillToAnalyze.first)
										   .arg(_minAndMaxSpillToAnalyze.second);
			throw GeneralException(errorMessage.toStdString().c_str());
		}
	}

	return lastSpillToAnalyzeReached;
}
// Boost serialization///////////////////////////////////////////////////////////
void CalibrationModeStandalone_T3BSyncInfo::initializeSerialization()
{
	_serializationObject = new Serialization_SpillSyncInfo(_pathToCalibDir.toStdString(),"write");

	// set the file name if distinct spills were selected for analyzing
	QString outputFileSuffix = QString("-Run%1").arg(_runNumber);
	if(_minAndMaxSpillToAnalyze.first != -1 && _minAndMaxSpillToAnalyze.second != -1)
		outputFileSuffix.append(QString("-Spill%1-%2").arg(_minAndMaxSpillToAnalyze.first).arg(_minAndMaxSpillToAnalyze.second));
	if(_analyzingIntermediateRM)
		outputFileSuffix.append("-IRM");
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
void CalibrationModeStandalone_T3BSyncInfo::serializeSpillSyncInfo()
{
	SpillSyncInformation *spillInfo = _serializationObject->getSpillInformation();



    spillInfo->spillNumber = _spillSyncInfo["T3BSpillNr"].toInt();
    spillInfo->timeOfDataReady = _spillSyncInfo["timeOfDataReady"].toUInt();
    spillInfo->timeOfTransferToPcReady = _spillSyncInfo["timeOfTransferToPcReady"].toUInt();
    spillInfo->timeOfAcquisitionStart = _spillSyncInfo["timeOfAcquisitionStart"].toUInt();

	QStringList *allOscis = &_spillRead->getSpillInfo()->enabledOsciSerials; 
	for(int i=0; i<allOscis->size(); i++)
	{
		spillInfo->enabledOscis.push_back(allOscis->at(i).toStdString());
		spillInfo->osciSerial_To_NrCaptures[allOscis->at(i).toStdString()] = _spillSyncInfo[QString("PS6000_%1_nrCaptures").arg(i)].toInt();
	}

	spillInfo->oscisOffSync = _spillSyncInfo["oscisOffSyncBit"].toBool();
	spillInfo->nrCoincidenceEvents = _spillSyncInfo["#CoincEv"].toInt();



	_serializationObject->writeSequentialInfo();
}
void CalibrationModeStandalone_T3BSyncInfo::finalizeSerialization()
{
	_serializationObject->finalizeSerialization();
}
// write sync info in a human readible file format
void CalibrationModeStandalone_T3BSyncInfo::initializeSpillSyncInfoHumanReadible()
{
	QString fileName = QString("SpillSyncInfo-Run_%2").arg(_spillRead->getSpillInfo()->runNumber);
	if(_minAndMaxSpillToAnalyze.first != -1 && _minAndMaxSpillToAnalyze.second != -1)
		fileName.append(QString("-Spill%1-%2").arg(_minAndMaxSpillToAnalyze.first).arg(_minAndMaxSpillToAnalyze.second));
	if(_analyzingIntermediateRM)
		fileName.append("-IRM");
	fileName.append(".T3B-HumanReadible.part");
	QString absoluteFilePath = QString("%1/%2").arg(_pathToCalibDir).arg(fileName);

	_outputFile = new QFile(absoluteFilePath);
	if (!_outputFile->open(QIODevice::WriteOnly | QIODevice::Text))
		throw GeneralException("The Pedestal Substraction value file could not be opened");

	_output = new QTextStream( _outputFile );

	(*_output) << "SpillStop "<<qSetFieldWidth(25)
			<< "T3BSpillNr "<<qSetFieldWidth(16)
			<< "#CoincEv"<<qSetFieldWidth(16);
			
	QStringList *allOscis = &_spillRead->getSpillInfo()->enabledOsciSerials; 
	for(int i=0; i<allOscis->size(); i++)
		(*_output)	<< allOscis->at(i).toStdString().c_str() <<qSetFieldWidth(16);
	
	(*_output) << "offSynch"<<qSetFieldWidth(16)
			<<"\n";
}
void CalibrationModeStandalone_T3BSyncInfo::writeSpillSyncInfoHumanReadible()
{
	(*_output)	<< _spillSyncInfo["timeOfDataReadyHR"].toString().toStdString().c_str()<<qSetFieldWidth(7)
			<< _spillSyncInfo["T3BSpillNr"].toInt()<<qSetFieldWidth(16)
			<< _spillSyncInfo["#CoincEv"].toInt()<<qSetFieldWidth(16);

	QStringList *allOscis = &_spillRead->getSpillInfo()->enabledOsciSerials; 
	for(int i=0; i<allOscis->size(); i++)
		(*_output)	<< _spillSyncInfo[QString("PS6000_%1_nrCaptures").arg(i)].toInt()<<qSetFieldWidth(16);
	
	(*_output)	<< _spillSyncInfo["oscisOffSyncBit"].toBool() <<qSetFieldWidth(16)
				<<"\n";
}	
void CalibrationModeStandalone_T3BSyncInfo::finalizeSpillSyncInfoHumanReadible()
{
	// close the text file
	_outputFile->close();

	// rename the output file -> remove the .part
	QString absoluteFilePath = _outputFile->fileName();
	QString newOutputFileName = absoluteFilePath.remove(".part");
	bool renamingSucceeded = _outputFile->rename(newOutputFileName);
	if(!renamingSucceeded)
	{
		QString errorMessage = QString("*** Warning: Renaming of File \"%1\" failed!").arg(absoluteFilePath);
		cerr<<errorMessage.toStdString().c_str()<<endl;
		//throw GeneralException(errorMessage.toStdString().c_str());
	}

	// clean up memory
	delete _output;
	delete _outputFile;
}

// destructor
CalibrationModeStandalone_T3BSyncInfo::~CalibrationModeStandalone_T3BSyncInfo(void)
{
}
