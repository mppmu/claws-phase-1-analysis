#include "CalibrationMode_MipPeak.h"

CalibrationMode_MipPeak::CalibrationMode_MipPeak(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->initializeIntegralDistributionsForMIPPeakDetermination();
	_plotPool->initializeTotalWfmIntegralPlots();

	this->startAnalysis();
}

void CalibrationMode_MipPeak::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationMipPeak")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	// path to the temperature data
	_filePathTemperatureData = __globalSettingsManager->getSetting("rm_filePathTemperatureData").toString();

	// bias voltages
	QString tilePositionToBiasVoltages_str = __globalSettingsManager->getSetting("rm_tilePositionToBiasVoltages").toString(); 
	QStringList tilePosAndVoltageOfChannels = tilePositionToBiasVoltages_str.split("+");
    foreach (QString tilePosAndVoltageOfCurrentChannel, tilePosAndVoltageOfChannels) 
	{
		QStringList tilePosAndVoltageOfChannels = tilePosAndVoltageOfCurrentChannel.split(">");
		if(tilePosAndVoltageOfChannels.size() != 2)
		{
			QString errorMessage = QString("*** Error: Wrong input string '%1' in rm_tilePositionToBiasVoltages!").arg(tilePosAndVoltageOfCurrentChannel);
			throw GeneralException(errorMessage.toStdString().c_str());
		}
		if(!_spillRead->getGeometryInfo()->tilePositionInnerToOuter.values().contains(tilePosAndVoltageOfChannels[0].toInt()))
		{
			QString errorMessage = QString("*** Error: Data to the tile position '%1' does not exist!").arg(tilePosAndVoltageOfChannels[0].toInt());
			throw GeneralException(errorMessage.toStdString().c_str());
		}

		int tilePosition = tilePosAndVoltageOfChannels[0].toInt();
		double biasVoltage = tilePosAndVoltageOfChannels[1].toDouble();

		_tilePosition_to_AppliedBiasVoltage[tilePosition] = biasVoltage;
	}
	if(_tilePosition_to_AppliedBiasVoltage.size() != _spillRead->getGeometryInfo()->tilePositionInnerToOuter.size())
		throw GeneralException("*** Error: You did not specify VBias for all channels containing Data!");
}

void CalibrationMode_MipPeak::initializeAnalysisModeVariables()
{
	cout<<"Initializing Temperature Measurement Class"<<endl;
	_temperatureData = new AccessTemperatureData(_filePathTemperatureData);
	cout<<"Temperature Information is now available"<<endl;
}

void CalibrationMode_MipPeak::startAnalysis()
{
	_timeOfRunStart = _spillRead->getSpillInfo()->timeOfDataReady.toTime_t();

	int nrAnalyzedEvents = 0;
	int nrTotalEvents = 0;
	while(!this->endOfDataReached() && nrAnalyzedEvents < _nrEventsToAnalyze)// && _spillRead->getSpillInfo()->spillNumber < 1184)
	{
		// load the next waveform and get all current waveforms
		vector <Waveform*> actualWaveforms = this->loadNextWfmOnAllChannels();

		nrTotalEvents++;

		// trigger the waveforms
		bool somethingWasTriggered = this->triggerWaveformsWithSelectedTrigger(&actualWaveforms);
		if(!somethingWasTriggered) 
			continue;

		// filter the waveforms according to the filters selected in the settings file
		somethingWasTriggered = this->filterWaveformsWithSelectedFilters(&actualWaveforms);
		if(!somethingWasTriggered)
			continue;





		// save the first 50 waveforms
		if(nrAnalyzedEvents<50)
		{
			// save the triggered waveforms of all _enabledOsciChannels in several canvases
			_plotPool->writeTriggeredWaveforms(actualWaveforms);
		}

		// fill integral distribution of all triggered waveforms
		this->fillIntegralDistributionsForMIPPeakDetermination(actualWaveforms);
		_plotPool->fillTotalWfmIntegralPlots(actualWaveforms);





		if((nrAnalyzedEvents+1)%100 == 0) 
			cout<<"Analyzing Waveform Number: "<<nrAnalyzedEvents+1
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<nrTotalEvents
			<<endl;

		nrAnalyzedEvents++;
	}
	_timeOfRunEnd = this->getCurrentSpillRead()->getSpillInfo()->timeOfDataReady.toTime_t();


	// create and write the temperature data for the analysis period
	_plotPool->createAndWriteGeneralTemperaturePlots(_temperatureData,_timeOfRunStart,_timeOfRunEnd);


	this->fitAndWriteIntegralDistribution();
	_plotPool->writeTotalWfmIntegralPlots();

	// create human readible MipPeak file
	this->initializeMipPeakFile();
	this->writeMipPeakValueToCalibFile();
	this->finalizeMipPeakFile();

	// serialization
	this->initializeSerialization();
	this->serializeMipPeakInfo();
	this->finalizeSerialization();

	this->moveRootFileToCalibFolder();
}
// plots
void CalibrationMode_MipPeak::initializeIntegralDistributionsForMIPPeakDetermination()
{
	// clean up if something if this is not the first call of the initialization
	foreach (TH1D* value, _tilePosition_to_WfmIntegralDistribution)
		delete value;

	// Integral Distribution Histograms
	for(int i=0; i<this->getNumberOfChannels(); i++)
	{
		pair<QString,QString> *osciChannel = this->getAllChannelDataVec()->at(i)->getDataCorrespondingOsciAndChannel();
		if( !_spillRead->aSiPMisConnectedToOsciChannel(osciChannel->first, osciChannel->second) )
			continue;

		TString histName = (QString("%1-IntegralDistribution: Osci %2, %3, Pos %4")
			.arg(_tFile->GetTitle())
			.arg(osciChannel->first)
			.arg(osciChannel->second)
			.arg(this->getCurrentSpillRead()->getGeometryInfo()->osciChannelToTilePosition_ItoO[osciChannel->first][osciChannel->second])).toStdString().c_str();

		TH1D *h1 = new TH1D(histName,histName,1024,0,40E-12);
		h1->GetXaxis()->SetTitle("Wfm Integral [C]");
		h1->GetYaxis()->SetTitle( "# Counts" );
		h1->GetYaxis()->SetTitleOffset(1.3);
		h1->GetXaxis()->SetTitleOffset(1.0);
		h1->SetFillColor(28);

		_tilePosition_to_WfmIntegralDistribution[i] = h1;
	}
}
void CalibrationMode_MipPeak::fillIntegralDistributionsForMIPPeakDetermination(vector <Waveform*> &actualWaveforms)
{
	for (int k = 0; k < (int)actualWaveforms.size(); k++)
	{
		for (int j = 0; j < (int)actualWaveforms[k]->getIntegralOfTriggeredWfms()->size(); j++)
		{
			_tilePosition_to_WfmIntegralDistribution[k]->Fill(actualWaveforms[k]->getIntegralOfTriggeredWfms()->at(j));
		}
	}
}

void CalibrationMode_MipPeak::fitAndWriteIntegralDistribution()
{
	TDirectory *dir = this->getTDirectory("IntegralDistribution");

	vector<int> allTilePositions;
	for(int i=0; i<_allChannelDataVec_withSiPMsAttached->size(); i++)
		allTilePositions.push_back(this->getAllChannelDataVec()->at(i)->getDataCorrespondingTilePosition());

	for(int i=0; i<allTilePositions.size(); i++)
	{
		TH1D* currentIntegralDist = _tilePosition_to_WfmIntegralDistribution[allTilePositions.at(i)];

		QString integralDistCanvas_title = QString("%1_canvas").arg(currentIntegralDist->GetTitle());
		TCanvas integralDistCanvas(integralDistCanvas_title.toStdString().c_str(),integralDistCanvas_title.toStdString().c_str(),700,500);
		integralDistCanvas.cd();
//		currentIntegralDist->Sumw2();
		currentIntegralDist->SetMarkerStyle(20);
		currentIntegralDist->Draw();

		double *peakParam = __globalLangauFit->initiateLangauFit(currentIntegralDist);
		pair<double,double> peakParam_pair(peakParam[0],peakParam[1]);
		_tilePosition_to_MipPeakAndMipPeakError[allTilePositions.at(i)] = peakParam_pair;

		pair<double,int> chiSquareAndNDFOfFit = __globalLangauFit->getChiSquareAndNDFOfLastFit();
		_tilePosition_to_ChiSquareAndNDFofFit[allTilePositions.at(i)] = chiSquareAndNDFOfFit;

		QString title = QString("%1 - %2%3%4%5").arg(currentIntegralDist->GetTitle()).arg(" LangauPeak: ").arg(peakParam[0]).arg(" PeakError: ").arg(peakParam[1]);
		currentIntegralDist->SetTitle(title.toStdString().c_str());
		integralDistCanvas.SetTitle(title.toStdString().c_str());
		integralDistCanvas.Update();

		dir->WriteTObject(&integralDistCanvas);	
	}
}

// text file with Mip Peak values
void CalibrationMode_MipPeak::initializeMipPeakFile()
{
	QDir calibSubDir(this->determineFilePathToCalibrationData(_analyzingIntermediateRM));
	QString fileName = QString("MipPeak%1.T3B.part-HumanReadible").arg(this->getRunAndSpillTitleSuffix());
	QString absoluteMipPeakFilePath = QString("%1/%2").arg(calibSubDir.absolutePath()).arg(fileName);
	_absoluteMipPeakFilePath = absoluteMipPeakFilePath;

	_outputFile = new QFile(absoluteMipPeakFilePath);
	if (!_outputFile->open(QIODevice::WriteOnly | QIODevice::Text))
		throw GeneralException("The MipPeak value file could not be opened");

	_output = new QTextStream( _outputFile );
	(*_output)<<QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
				.arg("TilePos",30)
				.arg("TileName",30)
				.arg("MipPeak",30)
				.arg("MipPeakError",30)
				.arg("VBias",30)
				.arg("ChiSquare",30)
				.arg("NDF",30);						
	//(*_output)<<"TilePos:\t\tTileName:\t\tMipPeak:\t\tMipPeakError:\t\tVBias:\t\tChiSquare:\t\tNDF:\n";
}
void CalibrationMode_MipPeak::writeMipPeakValueToCalibFile()
{
	vector<int> allTilePositions;
	for(int i=0; i<this->getAllChannelDataVec()->size(); i++)
		allTilePositions.push_back(this->getAllChannelDataVec()->at(i)->getDataCorrespondingTilePosition());

	for(int i=0; i<allTilePositions.size(); i++)
	{
		pair<QString,QString> *osciChannel = this->getAllChannelDataVec()->at(allTilePositions.at(i))->getDataCorrespondingOsciAndChannel();

		pair<double,double> mipPeakParam = _tilePosition_to_MipPeakAndMipPeakError[allTilePositions.at(i)];
		(*_output)<<QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
						.arg(allTilePositions.at(i),30)
						.arg(_spillRead->getGeometryInfo()->tileAttachedToOsciChannel[osciChannel->first][osciChannel->second],30)
						.arg(mipPeakParam.first,30)
						.arg(mipPeakParam.second,30)
						.arg(_tilePosition_to_AppliedBiasVoltage[allTilePositions.at(i)],30)
						.arg(_tilePosition_to_ChiSquareAndNDFofFit[allTilePositions.at(i)].first,30)
						.arg(_tilePosition_to_ChiSquareAndNDFofFit[allTilePositions.at(i)].second,30);						
	}
}
void CalibrationMode_MipPeak::finalizeMipPeakFile()
{
	_outputFile->close();

	// rename the output file -> remove the .part
	QString newOutputFileName = _absoluteMipPeakFilePath.remove(".part");
	bool renamingSucceeded = _outputFile->rename(newOutputFileName);
	if(!renamingSucceeded)
	{
		QString errorMessage = QString("*** Warning: Renaming of File \"%1\" failed!").arg(_absoluteMipPeakFilePath);
		cerr<<errorMessage.toStdString().c_str()<<endl;
		//throw GeneralException(errorMessage.toStdString().c_str());
	}
}




// Boost serialization///////////////////////////////////////////////////////////
void CalibrationMode_MipPeak::initializeSerialization()
{
	_serializationObject = new Serialization_MipPeak(this->determineFilePathToCalibrationData(_analyzingIntermediateRM).toStdString(),"write");

	// set the file name if distinct spills were selected for analyzing
	_serializationObject->setOutputFileSuffix(this->getRunAndSpillTitleSuffix().toStdString());

	// set general run information
	RunInformation *runInfo = _serializationObject->getGeneralInformation()->runInfo;
	runInfo->runDirectory = _runRead->getRunDirectory().toStdString();
	runInfo->runMode = _runRead->getDataRunMode().toStdString();
	runInfo->runNumber = _spillRead->getSpillInfo()->runNumber;
	runInfo->nrEnabledChannels = _numberOfChannels;
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
void CalibrationMode_MipPeak::serializeMipPeakInfo()
{
	MipPeakInformation *mipPeakInfo = _serializationObject->getMipPeakInfo();

	mipPeakInfo->timeTOfRun = _spillRead->getSpillInfo()->timeOfDataReady.toTime_t();

	mipPeakInfo->meanTemperatureDuringDataTaking = this->determineMeanTemperatureDuringDataTaking();

	// determine tilePositionToTileNameMap
	map<int, string> tilePos_to_TileNameMap;
	for(int i=0; i<this->getAllChannelDataVec()->size(); i++)
	{
		pair<QString,QString> *osciChannel = this->getAllChannelDataVec()->at(i)->getDataCorrespondingOsciAndChannel();
		string tileName = _spillRead->getGeometryInfo()->tileAttachedToOsciChannel[osciChannel->first][osciChannel->second].toStdString();

		tilePos_to_TileNameMap[i] = tileName;
	}
	mipPeakInfo->_tilePosition_To_TileName = tilePos_to_TileNameMap;

	mipPeakInfo->_tilePosition_To_appliedBiasVoltage = _tilePosition_to_AppliedBiasVoltage.toStdMap();

	mipPeakInfo->_tilePosition_To_MipPeakAndMipPeakError = _tilePosition_to_MipPeakAndMipPeakError.toStdMap();

	mipPeakInfo->_tilePosition_To_FitChi2AndNDF = _tilePosition_to_ChiSquareAndNDFofFit.toStdMap();

	_serializationObject->writeSequentialInfo();
}
void CalibrationMode_MipPeak::finalizeSerialization()
{
	_serializationObject->finalizeSerialization();
}
// helper
double CalibrationMode_MipPeak::determineMeanTemperatureDuringDataTaking()
{
	double meanTempDuringDataTaking = 0;

	if(!_temperatureData->temperatureStandaloneSensorInfoExists())
		throw GeneralException("*** Error: No Temperature Information Standalone Sensor exists for the current run -> need to extend the code here!");

	vector <int> *timeTIntVec = _temperatureData->getAllAvailableInfo_StandaloneSensor()->_timeTOfMeasurement;
	vector <double> *temperatureVec =  _temperatureData->getAllAvailableInfo_StandaloneSensor()->_currentTemperature;
	double temperatureSum = 0;
	int nrTemperatureSummations = 0;
	for(int i=0; i<(int)timeTIntVec->size(); i++)
	{
		if(_timeOfRunStart <= timeTIntVec->at(i) && _timeOfRunEnd >= timeTIntVec->at(i))
		{
			temperatureSum += temperatureVec->at(i);
			nrTemperatureSummations += 1;
		}
	}
	meanTempDuringDataTaking = temperatureSum / nrTemperatureSummations;

	return meanTempDuringDataTaking;
}

// destructor
CalibrationMode_MipPeak::~CalibrationMode_MipPeak(void)
{
	delete _temperatureData;

	// TODO
	//foreach (TH1D* hist, _tilePosition_to_WfmIntegralDistribution)
	//	delete hist;
}
