#include "CalibrationMode_Averaged1pePeak.h"

CalibrationMode_Averaged1pePeak::CalibrationMode_Averaged1pePeak(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->initializeSerialization();

	_plotPool->initializeHistogramsForIntegralDistribution();
	_plotPool->initializeForWaveformAveragingPlots();

	std::cout << "TEST" << std::endl;

	this->startAnalysis();
}

void CalibrationMode_Averaged1pePeak::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationAveraged1pePeak")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	_averageOverNrSpills = __globalSettingsManager->getSetting("rm_averageOverNrSpills").toInt();
	_timeToleranceMinutesForSiPMGainDetermination = __globalSettingsManager->getSetting("rm_timeToleranceForSiPMGainDeterminationMinutes").toInt();
	_OnePEThreshold = __globalSettingsManager->getSetting("rm_OnePEThresholdMV").toFloat() / 1000;
	_acceptanceRangeAroundTypicalTriggerTime = __globalSettingsManager->getOptionalSetting("rm_acceptanceRangeAroundTypicalTriggerTimeNS").toFloat() * 1E-9;
	if(_acceptanceRangeAroundTypicalTriggerTime == 0)
		_acceptanceRangeAroundTypicalTriggerTime = 2.5 * 1E-9;  // 2.5ns is the default value

	_fit1peWaveformFallingEdgeFromTimeNS = __globalSettingsManager->getSetting("rm_fit1peWaveformFallingEdgeFromTimeNS").toDouble() * 1E-9;
}

void CalibrationMode_Averaged1pePeak::initializeAnalysisModeVariables()
{
	cout<<"Initializing SiPMGain Class"<<endl;
	QString filePathSiPMGainData = this->determineFilePathToCalibrationData(_analyzingIntermediateRM);
	_SiPMGainData = new AccessSiPMGainData(filePathSiPMGainData);
	cout<<"SiPMGain Information is now available"<<endl<<endl;

	_typicalTriggerTime = this->determineTypicalTriggerTime();
	this->setWfmIntegralLowerAndUpperCut();
}

void CalibrationMode_Averaged1pePeak::startAnalysis()
{
	int nrAnalyzedEvents = 0;
	int nrTotalEvents = 0;
	int nrSpillsSummedForAveraging = 0;
	while(!this->endOfDataReached() && nrAnalyzedEvents < _nrEventsToAnalyze)
	{
		// load the next waveform and get all current waveforms
		vector <Waveform*> actualWaveforms = this->loadNextWfmOnAllChannels();

		nrTotalEvents++;

		// trigger the waveforms
		bool somethingWasTriggered = this->triggerWaveformsWithSelectedTrigger(&actualWaveforms);
		if(!somethingWasTriggered) 
			continue;




		// filter all waveforms which do not contain pure 1p.e. peaks
		this->selectPure1pePeaks(&actualWaveforms);

		// average all purified 1pe peaks
		_plotPool->sumWaveformsForAveraging(&actualWaveforms);
		// fill integral distribution of all triggered waveforms
		_plotPool->fillIntegralDistribution(actualWaveforms);

		// save the first 50 waveforms
		if(nrAnalyzedEvents<50)
		{
			// save the triggered waveforms of all _enabledOsciChannels in several canvases
			_plotPool->writeTriggeredWaveforms(actualWaveforms);
		}

		if(!this->anotherEventExistsInSpill()) //nrTotalEvents % _spillRead->getAllChannelDataQList()->at(0)->getNrWaveforms() == 0)
		{
			nrSpillsSummedForAveraging++;

			// we determine the averaged 1pe waveform if the specified nr Spills over which we want to average is reached
			if(nrSpillsSummedForAveraging == _averageOverNrSpills)
			{
				int spillNumberCorrespondingToAveragedInfo = _spillRead->getSpillInfo()->spillNumber - (int)(((float)_averageOverNrSpills)/2);

				// average the waveforms that were summed
				_plotPool->averageSummedWaveforms();

				QString prefix = QString("Spill_%1 ").arg(spillNumberCorrespondingToAveragedInfo);
				_plotPool->writeIntegralDistribution(prefix);
				//vector<TGraph *> *allAveragedWaveformGraphs = _plotPool->writeWaveformAveragingPlots(prefix);

				// correct for cable reflections by fitting
				this->correctAveragedWaveformForReflections(spillNumberCorrespondingToAveragedInfo);

				// serialize the spill information on the averaged 1pe waveform
				this->serializeSpill(spillNumberCorrespondingToAveragedInfo);
				
				// reinitialize the averaging and integral distribution plots
				_plotPool->initializeHistogramsForIntegralDistribution();
				_plotPool->initializeForWaveformAveragingPlots();

				// We just reinitialize the averaging plots --> reset the number of spill summed for averaging
				nrSpillsSummedForAveraging = 0;
			}
		}

		if((nrAnalyzedEvents+1)%50 == 0) 
			cout<<"Analyzing Waveform Number: "<<nrAnalyzedEvents+1
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<nrTotalEvents
			<<endl;

		nrAnalyzedEvents++;
	}

	this->finalizeSerialization();

	// move the TFile
	this->moveRootFileToCalibFolder();
}

// a local selection filter - filter Waveforms which do not contain pure 1pe Peaks
void CalibrationMode_Averaged1pePeak::selectPure1pePeaks(vector <Waveform*> *actualWaveforms)
{
	for(int i=0; i<(int)actualWaveforms->size(); i++)
	{
		if(__globalSettingsManager->getOptionalSetting("trigger").toString() != "" )
			throw GeneralException("*** Error: The Calibration mode 'Averaged1pe' is designed for the default trigger, so don't select any!");

		bool waveformWasRejected = false;

		// demand a waveform integral of 1p.e.
		float wfmIntegral = actualWaveforms->at(i)->getIntegralOfTriggeredWfms()->at(0);
		if(wfmIntegral > _tilePos_to_wfmIntegralLowerAndUpperCut[i].second || wfmIntegral < _tilePos_to_wfmIntegralLowerAndUpperCut[i].first)
		{
			actualWaveforms->at(i)->clearAllTriggers();
			waveformWasRejected = true;
		}

		// check if time of sample above threshold is in the specified _typicalTriggerTimeRange
		if(!waveformWasRejected)
		{
			for(int j=0; j<(int)actualWaveforms->at(i)->getScaledSamples()->size(); j++)
			{
				float scaledSample = actualWaveforms->at(i)->getScaledSamples()->at(j);
				if(scaledSample > _OnePEThreshold)
				{
					float timeOfSampleAbove1peThreshold = actualWaveforms->at(i)->getTimingOfSamples()->at(j);
					if(! (timeOfSampleAbove1peThreshold > _typicalTriggerTime - _acceptanceRangeAroundTypicalTriggerTime &&
						  timeOfSampleAbove1peThreshold < _typicalTriggerTime + _acceptanceRangeAroundTypicalTriggerTime )  )
					{
						actualWaveforms->at(i)->clearAllTriggers();
						waveformWasRejected = true;
					}

					break;
				}
			}
		}

		//// control output of pure waveform
		//if(!waveformWasRejected)
		//{
		//	pair <int,int> totalSampleWindow(0,actualWaveforms->at(i)->getScaledSamples()->size());
		//	_plotPool->writeCanvasWithGraphOfWaveform(actualWaveforms->at(i),totalSampleWindow,"Pure 1pe peak");
		//}
	}
}

// correct for cable reflections by fitting the falling edge
void CalibrationMode_Averaged1pePeak::correctAveragedWaveformForReflections(int spillNumberCorrespondingToAveragedInfo)
{
	// create or get directory to write to
	TDirectory *dir = this->getTDirectory("Averaged Full Waveforms");

	for(int i=0; i<this->getNumberOfChannels_withSiPMsAttached(); i++)
	{
		QString title = QString("Averaged Full Waveform @ Tile Position: %2").arg(i);
		TGraph *currentGraph = new TGraph((int)_plotPool->getTilePositionToSummedSamplesMap()[i].size(),&_plotPool->getTilePositionToTimingOfSamplesMap()[i].front(),&_plotPool->getTilePositionToSummedSamplesMap()[i].front());
		currentGraph->GetXaxis()->SetTitle("Time [sek]");
		currentGraph->GetYaxis()->SetTitle("Amplitude [V]");
		currentGraph->SetName(title.toStdString().c_str());
		currentGraph->SetTitle(title.toStdString().c_str());
		currentGraph->SetLineColor(2);

		// find maximum
		double timeOfMax = 0;
		double maximumTime = 0;
		double max = 0;
		vector<float> cleanedX;
		vector<float> cleanedY;
		for(int j=0; j<currentGraph->GetN(); j++)
		{	
			double x,y;
			currentGraph->GetPoint(j,x,y);
			if(y > max)
			{
				max = y;
				timeOfMax = x;
			}
			if(j < currentGraph->GetN() -9)
				maximumTime = x;

			if(x > 50E-9 && x < 62E-9)
				continue;

			cleanedX.push_back(x);
			cleanedY.push_back(y);
		}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
		// set time from which we fit on manually -  comment out to go back fitting from the max
		timeOfMax = _fit1peWaveformFallingEdgeFromTimeNS;
/////////////////////////////////////////////////////////////////////////////////////////////////////////

		TGraph *gr = new TGraph((int)cleanedX.size(),&cleanedX.front(),&cleanedY.front());
		gr->SetName(TString::Format("%s - new",currentGraph->GetTitle()));
		gr->SetTitle(TString::Format("%s - new",currentGraph->GetTitle()));
		gr->SetLineColor(3);

		TF1 *fit = new TF1(QString("charge decay function - TilePos: %1").arg(i).toStdString().c_str(), "expo", timeOfMax, maximumTime);
		gr->Fit(fit,"","", timeOfMax, maximumTime);

		// retrieve new values from fit 
		// the time window will be extended until the fit value drops below the largest value that can ever create a fake pixel firing at the consecutive 1pe substration
		vector<float> timingOfSamples = _plotPool->getTilePositionToTimingOfSamplesMap()[i];
		vector<float> averagedSamples = _plotPool->getTilePositionToSummedSamplesMap()[i];
		int maximumIterationIndex = timingOfSamples.size();
		for(int j=0; j<maximumIterationIndex; j++)
		{
			double currentTimingValue = 0;
			if(j < timingOfSamples.size())
				currentTimingValue = timingOfSamples.at(j);
			else
				currentTimingValue = timingOfSamples.at(timingOfSamples.size() -1) + (timingOfSamples.at(1)-timingOfSamples.at(0))*(j - (timingOfSamples.size()-1) );


			if(currentTimingValue <= timeOfMax)
				_tilePosition_To_AveragedSamplesMap[i].push_back(	averagedSamples.at(j)	);
			else
			{
				double fitEvaluatedValue = fit->Eval(currentTimingValue);
				_tilePosition_To_AveragedSamplesMap[i].push_back( fitEvaluatedValue );

				double stoppingConditionValue = 0.002 / 400;
				if(j >= (timingOfSamples.size() -1) && fitEvaluatedValue > stoppingConditionValue)
					maximumIterationIndex++;
			}

			_tilePosition_To_TimingOfSamplesMap[i].push_back(currentTimingValue);

			if(j > timingOfSamples.size() *2)
				break;
		}

		TGraph *gr_fromFit = new TGraph((int)_tilePosition_To_TimingOfSamplesMap[i].size(),&_tilePosition_To_TimingOfSamplesMap[i].front(),&_tilePosition_To_AveragedSamplesMap[i].front());
		gr_fromFit->SetMarkerStyle(26);

		TString canTitle = TString::Format("Spill_%i - %s",spillNumberCorrespondingToAveragedInfo,currentGraph->GetTitle());
		TCanvas *cAveragedWfms = new TCanvas(canTitle,canTitle,800,500);
		cAveragedWfms->cd();
		gr_fromFit->Draw("*A");
		currentGraph->Draw("lsame");
		gr->Draw("same");
		fit->Draw("same");

		TLegend leg(0.5,0.75,0.9,0.9,"Correction step");
		leg.AddEntry(currentGraph,"Original averaged graph","l");
		leg.AddEntry(gr,"Cut out reflection dip","l");
		leg.AddEntry(gr_fromFit,"Graph corrected with Fit","p");
		leg.Draw();

		dir->WriteTObject(cAveragedWfms);
	}
}
// helper functions
float CalibrationMode_Averaged1pePeak::determineTypicalTriggerTime()
{
	QStringList osciSerials = _spillRead->getSpillInfo()->enabledOsciSerials;
	int nrPreTriggerSamplesLastCheckedOsci = 0;
	float timeBaseLastCheckedOsci = 0;
	float preTriggerTimeNS = 0;
	for(int i=0; i<osciSerials.size(); i++)
	{
		int nrPreTriggerSamples = _spillRead->getSpillInfo()->osciInfo[osciSerials.at(i)].triggerNoOfPreTriggerSamples;
		float timeBase = _spillRead->getSpillInfo()->osciInfo[osciSerials.at(i)].timeBasePerSamplePS / 1000.;

		// check if there are differences between the oscis
		if(i != 0)
			if(nrPreTriggerSamplesLastCheckedOsci != nrPreTriggerSamples || timeBaseLastCheckedOsci != timeBase)
				throw GeneralException("*** Error: Two or more Oscis have different timeBase or nrPreTriggerSamples!");
			
		nrPreTriggerSamplesLastCheckedOsci = nrPreTriggerSamples;
		timeBaseLastCheckedOsci = timeBase;

		preTriggerTimeNS = nrPreTriggerSamples * timeBase;
	}

	return preTriggerTimeNS * 1E-9;
}
void CalibrationMode_Averaged1pePeak::setWfmIntegralLowerAndUpperCut()
{
	// determine the closest measurement existing for the current spillNumber
	QMap<int,double> *tilePosition_to_closestSiPMGainValues
		= _SiPMGainData->getClosestAvailableSiPMGainValueForAllTilePositions(_spillRead->getSpillInfo()->spillNumber,
																			 _spillRead->getSpillInfo()->timeOfDataReady.toTime_t(),
																			 _timeToleranceMinutesForSiPMGainDetermination);

	// set the new cut values for the waveform integral
	QMapIterator<int,double> it(*tilePosition_to_closestSiPMGainValues);
	while (it.hasNext()) 
	{
		it.next();
		_tilePos_to_wfmIntegralLowerAndUpperCut[it.key()].first = it.value() * 0.75;
		_tilePos_to_wfmIntegralLowerAndUpperCut[it.key()].second = it.value() * 1.25;
	}
}
// Boost serialization///////////////////////////////////////////////////////////
void CalibrationMode_Averaged1pePeak::initializeSerialization()
{
	_serializationObject = new Serialization_Averaged1pePeak(this->determineFilePathToCalibrationData(_analyzingIntermediateRM).toStdString(),"write");

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
void CalibrationMode_Averaged1pePeak::serializeSpill(int spillNumberCorrespondingToAveragedInfo)
{
	SpillInformation *spillInfo = _serializationObject->getSpillInformation();
	spillInfo->spillNumber = spillNumberCorrespondingToAveragedInfo;
	spillInfo->timeOfDataReady = _spillRead->getSpillInfo()->timeOfDataReady.toTime_t();

	//spillInfo->_tilePosition_To_TimingOfAveragedSamplesMap = _plotPool->getTilePositionToTimingOfSamplesMap().toStdMap();
	//spillInfo->_tilePosition_To_AveragedSamplesMap = _plotPool->getTilePositionToSummedSamplesMap().toStdMap();
	spillInfo->_tilePosition_To_TimingOfAveragedSamplesMap = _tilePosition_To_TimingOfSamplesMap.toStdMap();
	spillInfo->_tilePosition_To_AveragedSamplesMap = _tilePosition_To_AveragedSamplesMap.toStdMap();
	spillInfo->_tilePosition_To_NrSummedWaveformsForAveraging = _plotPool->getTilePositionToNrSummedWaveformsMap().toStdMap();

	// determine the lower and upper FWHM indices and the MaxIndex and MaxValue
	for(int j=0; j<_spillRead->getNrChannelsWithSiPMsAttached(); j++)
	{
		// determine maximum
		int maximumIndex = -1;
		float maximum = -1E10;
		for (int i=0; i<(int)spillInfo->_tilePosition_To_AveragedSamplesMap[j].size(); i++)
		{
			float currentSampleValue = spillInfo->_tilePosition_To_AveragedSamplesMap[j].at(i);
			if(currentSampleValue > maximum)
			{
				// set the current maximum
				maximum = currentSampleValue;
				maximumIndex = i;
			}
		}
		float halfMaximum = maximum/2;

		// determine the upper FWHM index
		int upperFWHMIndex = -1;
		for (int i=maximumIndex; i<(int)spillInfo->_tilePosition_To_AveragedSamplesMap[j].size(); i++)
		{
			float currentSampleValue = spillInfo->_tilePosition_To_AveragedSamplesMap[j].at(i);
			if(currentSampleValue < halfMaximum)
				break;

			upperFWHMIndex = i;
		}

		// determine the lower FWHM index
		int lowerFWHMIndex = -1;
		for (int i=maximumIndex; i>=0; i--)
		{
			float currentSampleValue = spillInfo->_tilePosition_To_AveragedSamplesMap[j].at(i);
			if(currentSampleValue < halfMaximum)
				break;

			lowerFWHMIndex = i;
		}

		spillInfo->_tilePosition_To_LowerAndUpperFWHMIndex[j] = pair<int,int>(lowerFWHMIndex,upperFWHMIndex);
		spillInfo->_tilePosition_To_MaxIndexAndMaxValue[j] = pair<int,float>(maximumIndex,maximum);
	}

	_serializationObject->writeSequentialInfo();

	// clear class specific containers
	_tilePosition_To_AveragedSamplesMap.clear();
	_tilePosition_To_TimingOfSamplesMap.clear();
}
void CalibrationMode_Averaged1pePeak::finalizeSerialization()
{
	_serializationObject->finalizeSerialization();
}
// destructor
CalibrationMode_Averaged1pePeak::~CalibrationMode_Averaged1pePeak(void)
{
}
