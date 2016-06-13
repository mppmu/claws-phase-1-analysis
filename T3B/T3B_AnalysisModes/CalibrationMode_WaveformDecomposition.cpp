#include "CalibrationMode_WaveformDecomposition.h"

#include"SW_AsyncMultiRisingAndFallingEdgeTrigger.h"

CalibrationMode_WaveformDecomposition::CalibrationMode_WaveformDecomposition(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->initializeMapOfBanLists();

	this->initializeSerialization();

	this->startAnalysis();
}

void CalibrationMode_WaveformDecomposition::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationWaveformDecomposition")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	_lowerThresholdOfWfmMax = __globalSettingsManager->getSetting("rm_lowerThreshOfWfmMaxMV").toDouble() / 1000;
	_demandedFractionOfAveraged1peMaximum = __globalSettingsManager->getSetting("rm_demandedFracOfAveraged1peMax").toDouble();

	// reading coincidence Scinti On Osci/Channel
	QString string_coincidenceScintiOnOsciChannel = 
		__globalSettingsManager->getOptionalSetting("rm_coincidenceScintiOnOsciChannel").toString();
	if(string_coincidenceScintiOnOsciChannel != "")
	{
		QStringList coincidenceScintiOnOsciChannel = string_coincidenceScintiOnOsciChannel.split("-");
		_coincidenceScintiOnOsciChannel.first = coincidenceScintiOnOsciChannel[0];
		_coincidenceScintiOnOsciChannel.second = coincidenceScintiOnOsciChannel[1];
	}

	// settings for the dedicated channels like Scintillator Coincidence, Veto, CerenkovA, CerenkovB
	if(__globalSettingsManager->settingsFileContains("rm_dedicatedChannels_Names"))
	{
		QStringList dedicatedChannels_Names = __globalSettingsManager->getOptionalSetting("rm_dedicatedChannels_Names").toString().split("+");
	
		QStringList dedicatedChannels_risingTriggerChannelThreshold = __globalSettingsManager->getSetting("rm_dedicatedChannels_RisingThresholdsMV").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_fallingTriggerChannelThreshold = __globalSettingsManager->getSetting("rm_dedicatedChannels_FallingThresholdMV").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_preTriggerTimeNS = __globalSettingsManager->getSetting("rm_dedicatedChannels_PreTriggerTimeNS").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_postTriggerTimeNS = __globalSettingsManager->getSetting("rm_dedicatedChannels_PostTriggerTimeNS").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_MinimalSignalWidth = __globalSettingsManager->getSetting("rm_dedicatedChannels_MinimalSignalWidth").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_SignalInverted = __globalSettingsManager->getSetting("rm_dedicatedChannels_SignalInverted").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_TypicalTriggerTime = __globalSettingsManager->getSetting("rm_dedicatedChannels_TypicalTriggerTimeNS").toString().split("+",QString::SkipEmptyParts);
		QStringList dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime = __globalSettingsManager->getSetting("rm_dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTimeNS").toString().split("+",QString::SkipEmptyParts);
		if(dedicatedChannels_Names.size() != dedicatedChannels_risingTriggerChannelThreshold.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_fallingTriggerChannelThreshold.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_preTriggerTimeNS.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_postTriggerTimeNS.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_MinimalSignalWidth.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_SignalInverted.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_TypicalTriggerTime.size() ||
			dedicatedChannels_Names.size() != dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime.size()
			)
			throw GeneralException("*** Error: The number of dedicated channel settings does not match the number of specified dedicated channels!");

		for(int i=0; i<dedicatedChannels_Names.size(); i++)
		{
			// check if the specified dedicated channel does even exist
			if(_spillRead->getGeometryInfo()->allChannelNames.contains(dedicatedChannels_Names.at(i)))
			{
				_dedicatedChannels_Names.push_back(dedicatedChannels_Names.at(i));

				_dedicatedChannels_risingTriggerChannelThreshold.push_back(dedicatedChannels_risingTriggerChannelThreshold.at(i).toDouble() * 1E-3);
				_dedicatedChannels_fallingTriggerChannelThreshold.push_back(dedicatedChannels_fallingTriggerChannelThreshold.at(i).toDouble() * 1E-3);
				_dedicatedChannels_preTriggerTimeNS.push_back(dedicatedChannels_preTriggerTimeNS.at(i).toDouble() * 1E-9);
				_dedicatedChannels_postTriggerTimeNS.push_back(dedicatedChannels_postTriggerTimeNS.at(i).toDouble() * 1E-9);
				_dedicatedChannels_MinimalSignalWidth.push_back(dedicatedChannels_MinimalSignalWidth.at(i).toDouble() * 1E-9 );
				if(dedicatedChannels_SignalInverted.at(i) == "true")
					_dedicatedChannels_SignalInverted.push_back(true);
				else
					_dedicatedChannels_SignalInverted.push_back(false);
				_dedicatedChannels_TypicalTriggerTime.push_back(dedicatedChannels_TypicalTriggerTime.at(i).toDouble() * 1E-9 );
				_dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime.push_back(dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime.at(i).toDouble() * 1E-9 );
			}
			else
				throw GeneralException(QString("*** Error: You specified a dedicated channel name which does not exist! Name: %1!").arg(dedicatedChannels_Names.at(i)));

		}
	}

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

void CalibrationMode_WaveformDecomposition::initializeAnalysisModeVariables()
{
	_serializationAveraged1peInput = NULL;
	this->readInSerializedAvaragingValues();

	_breakSubstractionIfAveraged1peWfmBelowThreshold = 0.0001;

	_createStepwiseSubstractionHistogram = false;

	// initialize the specific triggers for the dedicated channels
	if(_dedicatedChannels_Names.size())
	{
		for(int i=0; i<_dedicatedChannels_Names.size(); i++)
		{
			SW_AsyncMultiRisingAndFallingEdgeTrigger *currentTrigger = 
				new SW_AsyncMultiRisingAndFallingEdgeTrigger(this,false,
															 _dedicatedChannels_risingTriggerChannelThreshold.at(i) * 1E3, 
															 _dedicatedChannels_fallingTriggerChannelThreshold.at(i) * 1E3,
															 _dedicatedChannels_preTriggerTimeNS.at(i) * 1E9,
															 _dedicatedChannels_postTriggerTimeNS.at(i) * 1E9,
															 _dedicatedChannels_MinimalSignalWidth.at(i) * 1E9,
															 _dedicatedChannels_SignalInverted.at(i)
															 );
			_dedicatedChannels_trigger[_dedicatedChannels_Names.at(i)] = currentTrigger;
		}
	}

	_chi2Distribution = new TH1D("chi2distribution","chi2distribution",1000,1E-5,1E-2);
}

void CalibrationMode_WaveformDecomposition::startAnalysis()
{
	_nrAnalyzedEvents = 0;
	_nrTotalEvents = 0;
	_EventNumber = 0;
	_EventNumberInCurrentSpill = 0;
	int lastSpillNumber = -1;
	while(!this->endOfDataReached() && _nrAnalyzedEvents < _nrEventsToAnalyze)
	{
		// load the next waveform and get all current waveforms
		vector <Waveform*> actualWaveforms = this->loadNextWfmOnAllChannels();

		_nrTotalEvents++;
		_EventNumber++;
		_EventNumberInCurrentSpill++;

		int currentSpillNumber = _spillRead->getSpillInfo()->spillNumber;
		if(currentSpillNumber != lastSpillNumber) // if true: this is the first event in the current spill
		{
			// set the correct event number (even if we jump over spills)
			_EventNumber = _spillRead->getSpillInfo()->eventNr;
			// set the correct event number relative to the current spill
			_EventNumberInCurrentSpill = 1;

			// load the next averaged 1pe peak Information if necessary
			this->loadNewAveraged1peInfoIfNecessary();
		}
		lastSpillNumber = _spillRead->getSpillInfo()->spillNumber;


		// trigger the waveforms
		bool somethingWasTriggered = this->triggerWaveformsWithSelectedTrigger(&actualWaveforms);
		if(!somethingWasTriggered) 
			continue;

		// filter the waveforms according to the filters selected in the settings file
		somethingWasTriggered = this->filterWaveformsWithSelectedFilters(&actualWaveforms);
		if(!somethingWasTriggered)
			continue;

		_nrAnalyzedEvents++;



		// take care of the dedicated channels (Scinti, CerA, CerB, Veto...)
		if(_dedicatedChannels_Names.size())
			this->triggerDedicatedChannels(&actualWaveforms);



		// save histograms of current waveforms for later recreation comparison
		QMap <int,TH1F *> *tilePositionToUnmodifiedWaveforms = this->backupUnmodifiedWaveformsForRecreation(&actualWaveforms);

		// initialize control plots
		bool createStepwiseSubstractionHistogram = false;
		QMap<int,TH1F*> *tilePositionToEdepHistogramMap = this->initializePurifiedEdepHistogram(&actualWaveforms,createStepwiseSubstractionHistogram);

		// substract averaged 1pe waveforms till nothing is left
		bool substractionProcessFinishedForEvent = false;
		while(!substractionProcessFinishedForEvent)
			substractionProcessFinishedForEvent = this->substract1peWfmAtMaxPosition(&actualWaveforms);

		// fill the plots for the purifed waveforms of this event
		this->fillPurifiedEdepHistogram();
		//// write the plots for the purifed waveforms of this event
		//this->writePurifiedEdepHistogram(&actualWaveforms);

		// assemble a waveform from the edepHistogram
		// control plots - write the first n events to the root file
		bool writeRecreatedWaveform = false;
		if(_nrAnalyzedEvents < 100)
			writeRecreatedWaveform = true;
		this->recreateWaveformAndDetermineQualityFactor(tilePositionToUnmodifiedWaveforms,tilePositionToEdepHistogramMap,writeRecreatedWaveform);

		// serialize event
		this->serializeEvent(&actualWaveforms);

		// reset variables for the waveforms of this event
		this->resetVariables();
		// clean up memory
		this->cleanUpMemory();


		// in case we want to see the scintillator coincidence waveform for the first 10 analyzed waveforms
		if(_nrAnalyzedEvents < 10) 
			this->writeDedicatedChannelWaveform(&actualWaveforms);


		// ouput
		if((_nrAnalyzedEvents)%10 == 0) 
			cout<<"Analyzing WfmNo: "<<_nrAnalyzedEvents
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<_nrTotalEvents
			<<" EventNo_"<<_EventNumber
			<<endl;
	}
	_tFile->WriteTObject(_chi2Distribution);

	this->finalizeSerialization();

	// move the TFile
	this->moveRootFileToCalibFolder();
}

bool CalibrationMode_WaveformDecomposition::substract1peWfmAtMaxPosition(vector <Waveform*> *actualWaveforms)
{
	// loop through all channels
	int nrWaveformsThatFinishedSubstractionProcess = 0;
	for(int i=0; i<_numberOfChannels; i++)
	{
		// reject the channel of the scintillator, cerenkov, veto... coincidence (no 1pe waveform exists for this tilePos)
		if(actualWaveforms->at(i)->isWfmOfDedicatedChannel())
			continue;
		if(actualWaveforms->at(i)->getOsciChannelPair()->first == _coincidenceScintiOnOsciChannel.first &&
			actualWaveforms->at(i)->getOsciChannelPair()->second == _coincidenceScintiOnOsciChannel.second)
			continue;

		// check if something was triggered
		if(actualWaveforms->at(i)->getNrOfSWTriggeredWfms() > 0)
		{
			// get the tilePosition
			int tilePosition = actualWaveforms->at(i)->getTilePosition();

			// get the scaled samples and its maximum position index: currentlyAnalyzedWfm and averaged1peWfm
			vector<float> *scaledSamplesOf1peWfm = &_tilePositionToSummedSamplesMap[tilePosition];
			int maximumIndexOf1peWfm = _tilePositionToMaximumIndexAndMaximumValueMap[tilePosition].first;
			vector<float> *scaledSamplesOfFullWfm = actualWaveforms->at(i)->getScaledSamples();
			// replace the scaledWaveform with the smoothed values -> get a graph of the current waveform and smooth it 1 time 
			if(_tilePositionToNrModificationSteps[tilePosition] % 5 == 0)
				this->performSmoothingOnWaveform(actualWaveforms->at(i));
			// now get the maximum of the currentlyAnalyzedWfm
			int maximumIndexOfFullWfm = this->getMaximumIndexOfFullWfm_respectingBanList(tilePosition,scaledSamplesOfFullWfm);

			//// check the total waveform integral
			//double totalWaveformIntegral = actualWaveforms->at(i)->getTotalWaveformIntegral();

			// continue with waveform substraction only if waveformMaximum is above a specified threshold
			float voltageMaximumOfCurrentWaveform = scaledSamplesOfFullWfm->at(maximumIndexOfFullWfm);
			if(voltageMaximumOfCurrentWaveform  > _lowerThresholdOfWfmMax)// && totalWaveformIntegral > 0.1E-12)
			{
				// ban maxima that do not fulfill the FWHM requirement
				bool maximumIndexWasBanned = this->maxIndexMustBeBanned(tilePosition,scaledSamplesOfFullWfm,maximumIndexOfFullWfm);
				if(maximumIndexWasBanned)
					continue;

				// increase the counted number of modification steps for this channel (tilePosition)
				_tilePositionToNrModificationSteps[tilePosition]++;

				// Fill time of 1pe edep vector
				// set the time of the 1peEdep = time of Maximum
				vector<float> *timingOfSamples = actualWaveforms->at(i)->getTimingOfSamples();
				_tilePositionToTimeOfEdep[tilePosition].push_back(timingOfSamples->at(maximumIndexOfFullWfm));


				////////////////now substraction starts/////////////////////////////////////////
				// substract from the maximum (inclusive) to the end of the available 1pe values
				for(int j=maximumIndexOfFullWfm; j<maximumIndexOfFullWfm + (scaledSamplesOf1peWfm->size() - maximumIndexOf1peWfm); j++)
				{
					if(j >= scaledSamplesOfFullWfm->size())
						break;

					float currentSample1peWfm = scaledSamplesOf1peWfm->at(j - maximumIndexOfFullWfm + maximumIndexOf1peWfm);

					//if(currentSample1peWfm < _breakSubstractionIfAveraged1peWfmBelowThreshold)
					//	break;

					float currentSampleFullWfm = scaledSamplesOfFullWfm->at(j);

					scaledSamplesOfFullWfm->at(j) = currentSampleFullWfm - currentSample1peWfm;
				}
				
				// substract from the maximum (exclusive) to the begin of the available 1pe values
				for(int j=maximumIndexOfFullWfm-1; j>=maximumIndexOfFullWfm - maximumIndexOf1peWfm; j--)
				{
					if(j < 0)
						break;

					float currentSample1peWfm = scaledSamplesOf1peWfm->at(j - maximumIndexOfFullWfm + maximumIndexOf1peWfm);

					if(currentSample1peWfm < _breakSubstractionIfAveraged1peWfmBelowThreshold)
						break;

					float currentSampleFullWfm = scaledSamplesOfFullWfm->at(j);

					scaledSamplesOfFullWfm->at(j) = currentSampleFullWfm - currentSample1peWfm;
				}

				// fill the 2D Histogramm
				if(_createStepwiseSubstractionHistogram)
				{
					for(int j=0; j<(int)scaledSamplesOfFullWfm->size(); j++)
					{
						_tilePositionToEdep2DHistogramMap[tilePosition]->SetBinContent(_tilePositionToNrModificationSteps[tilePosition],
																						j + 1,
																						scaledSamplesOfFullWfm->at(j));
					}
				}
			}
			else
			nrWaveformsThatFinishedSubstractionProcess++;

		}
		else
			nrWaveformsThatFinishedSubstractionProcess++;
	}

	bool substractionProcessFinishedForEvent = false;
	if(nrWaveformsThatFinishedSubstractionProcess == _nrChannelsWith1peAveragedWaveform)
		substractionProcessFinishedForEvent = true;

	return substractionProcessFinishedForEvent;
}


/////////////////////////////////Take Care of the Dedicated Channels////////////////////////////////
void CalibrationMode_WaveformDecomposition::triggerDedicatedChannels(vector <Waveform*> *actualWaveforms)
{
	// trigger the dedicated channels with their respective trigger
	for(int i=0; i<actualWaveforms->size(); i++)
	{
		if(actualWaveforms->at(i)->isWfmOfDedicatedChannel())
		{
			vector < Waveform* > dedicatedChannel;
			dedicatedChannel.push_back(actualWaveforms->at(i));

			QString currentChannelName = actualWaveforms->at(i)->getChannelName();
			if(_dedicatedChannels_trigger.contains(currentChannelName))
			{
				_dedicatedChannels_trigger[currentChannelName]->triggerWaveforms(dedicatedChannel);
			}
			else
				throw GeneralException(QString("*** Error: Trying to trigger dedicated channel '%1' which does not have a corresponding trigger!").arg(currentChannelName));


			// control plots to test output
			if(_nrAnalyzedEvents < 50)
			{
				for(int k=0; k<actualWaveforms->at(i)->getNrOfSWTriggeredWfms(); k++)
				{
					_plotPool->writeCanvasWithGraphOfWaveform(actualWaveforms->at(i),
															  actualWaveforms->at(i)->getTriggeredMinMaxSampleWindows()->at(k),
															  QString("Event: %1 , Channel: %2 , TriggeredWaveform: %3").arg(_nrAnalyzedEvents).arg(currentChannelName).arg(k) );
				}
				//pair <int,int> totalSampleWindow(0,actualWaveforms->at(i)->getScaledSamples()->size());
				//_plotPool->writeCanvasWithGraphOfWaveform(actualWaveforms->at(i),
				//										  totalSampleWindow,
				//										  QString("*** Event: %1 , Channel: %2 , Total Waveform").arg(_nrAnalyzedEvents).arg(currentChannelName) );
			}
		}
	}
}
/////////////////////////////////Create Control Plots////////////////////////////////////////////////
QMap<int,TH1F*> *CalibrationMode_WaveformDecomposition::initializePurifiedEdepHistogram(vector <Waveform*> *actualWaveforms, bool createStepwiseSubstractionHistogram)
{
	_createStepwiseSubstractionHistogram = createStepwiseSubstractionHistogram;

	int nrSamples = this->getAllChannelDataVec()->at(0)->getNoOfSamples();
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		// get the tilePosition
		int tilePosition = actualWaveforms->at(i)->getTilePosition();
		if(actualWaveforms->at(i)->getNrOfSWTriggeredWfms() > 0)
		{
			// create new histogramms
			QString histTitle = QString("Purified Edep - EventNr:%1,TilePos:%2,Spill:%3").arg(_EventNumber).arg(tilePosition).arg(_spillRead->getSpillInfo()->spillNumber);
			vector<float> *timingOfSamples = actualWaveforms->at(i)->getTimingOfSamples();

			TH1F *hist = new TH1F(histTitle.toStdString().c_str(),histTitle.toStdString().c_str(),timingOfSamples->size(),timingOfSamples->at(0),timingOfSamples->at(timingOfSamples->size() - 1));
			_tilePositionToEdepHistogramMap[tilePosition] = hist;

			// create new 2D histogramms
			if(_createStepwiseSubstractionHistogram)
			{
				double assumedMaxNrModificationSteps = 500;
				QString purifiedEdep2DTitle = QString("Purified Edep 2D - Event No: %1, Tile Position: %2").arg(_EventNumber).arg(tilePosition);
				TH2F *purifiedEdepHist2D = new TH2F(purifiedEdep2DTitle.toStdString().c_str(),
													purifiedEdep2DTitle.toStdString().c_str(),
													assumedMaxNrModificationSteps,
													0,
													assumedMaxNrModificationSteps,
													nrSamples,
													0,
													nrSamples * (this->getAllChannelDataVec()->at(0)->getTimeIncrement()*1E9));
				_tilePositionToEdep2DHistogramMap[tilePosition] = purifiedEdepHist2D;
			}
		}
		else
			_tilePositionToEdepHistogramMap[tilePosition] = NULL;
	}

	return &_tilePositionToEdepHistogramMap;
}
void CalibrationMode_WaveformDecomposition::fillPurifiedEdepHistogram()
{
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		if(_tilePositionToTimeOfEdep[i].size() > 0)
		{
			// fill the 1D Histogram
			for(int j=0; j<(int)_tilePositionToTimeOfEdep[i].size(); j++)
				_tilePositionToEdepHistogramMap[i]->Fill(_tilePositionToTimeOfEdep[i].at(j));
		}
	}
}
void CalibrationMode_WaveformDecomposition::writePurifiedEdepHistogram(vector <Waveform*> *actualWaveforms)
{
	int nrSamples = this->getAllChannelDataVec()->at(0)->getNoOfSamples();
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		if(actualWaveforms->at(i)->getNrOfSWTriggeredWfms() > 0)
		{
			// get the tilePosition
			int tilePosition = actualWaveforms->at(i)->getTilePosition();

			// 1D Histogram
			_tFile->WriteTObject(_tilePositionToEdepHistogramMap[tilePosition]);

			// 2D Histogram
			if(_createStepwiseSubstractionHistogram)
			{
				TCanvas *Graph2DCan = new TCanvas(_tilePositionToEdep2DHistogramMap[tilePosition]->GetTitle(),_tilePositionToEdep2DHistogramMap[tilePosition]->GetTitle(),700,400);
				Graph2DCan->cd();
				_tilePositionToEdep2DHistogramMap[tilePosition]->Draw("ColZ");
				_tFile->WriteTObject(Graph2DCan);

				//// create stepwise 500 png files in the output folder - for .gif Animation
				//if(_nrTotalEvents < 10)
				//{
				//	for(int j=0; j<_tilePositionToEdep2DHistogramMap[tilePosition]->GetNbinsX(); j++)
				//	{
				//		QString projTitle = QString("Event%1 Tile Position%2 - Step%3").arg(_EventNumber).arg(i).arg(j);
				//		TCanvas projYCan(projTitle.toStdString().c_str(),projTitle.toStdString().c_str(),1000,400);
				//		projYCan.cd();
				//		TH1D *projY = _tilePositionToEdep2DHistogramMap[tilePosition]->ProjectionY(projTitle.toStdString().c_str(),j,j);
				//		projY->SetTitle(projTitle.toStdString().c_str());
				//		projY->SetLineColor(2);
				//		projY->SetFillColor(2);
				//		projY->SetFillStyle(3002);
				//		projY->GetXaxis()->SetRangeUser(250,600);
				//		projY->GetYaxis()->SetRangeUser(-0.01,0.11);
				//		projY->Draw();

				//		cout<<this->getAbsoluteWorkingPath().toStdString().c_str()<<endl;

				//		projTitle.append(".png");
				//		projTitle.prepend("outputFiles/");
				//		projYCan.Print(projTitle.toStdString().c_str(),"png");
				//	}
				//}

				delete Graph2DCan;
			}
		}
	}
}
/////////////////////////////////Manage Banlist//////////////////////////////////////////////////////
void CalibrationMode_WaveformDecomposition::initializeMapOfBanLists()
{
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		QList<int> *banList = new QList<int>;

		_tilePositionToIndexBanList[i] = banList;
	}
}
void CalibrationMode_WaveformDecomposition::resetMapOfBanLists()
{
	foreach (QList<int> *banList, _tilePositionToIndexBanList)
	{
		banList->clear();
	}
}
bool CalibrationMode_WaveformDecomposition::maxIndexMustBeBanned(int tilePosition,vector<float> *scaledSamplesOfFullWfm,int maximumIndexOfFullWfm)
{
	bool maximumWasBanned = false;

	// all checked sampleValues of the full waveform have to be larger than (_demandedFractionOfAveraged1peMaximum * maximumOfAveraged1peWfm)
	float acceptanceThreshold = _tilePositionToMaximumIndexAndMaximumValueMap[tilePosition].second * _demandedFractionOfAveraged1peMaximum;

	// iterate from maximumOfFullWfm to the upper index of the FWHM
	int iterateOverNrSamples_increasing
		= (_tilePositionToMaxEnclosingFWHMIndices[tilePosition].second - _tilePositionToMaximumIndexAndMaximumValueMap[tilePosition].first - 3);
	for(int i=maximumIndexOfFullWfm; i<maximumIndexOfFullWfm + iterateOverNrSamples_increasing; i++)
	{
		if(i >= scaledSamplesOfFullWfm->size())
			break;

		if(scaledSamplesOfFullWfm->at(i) < acceptanceThreshold)
		{
			maximumWasBanned = true;
			*(_tilePositionToIndexBanList[tilePosition]) << maximumIndexOfFullWfm;
			break;
		}
	}	

	if(!maximumWasBanned)
	{
		// iterate from maximumOfFullWfm to the lower index of the FWHM
		int iterateOverNrSamples_decreasing
			= (_tilePositionToMaximumIndexAndMaximumValueMap[tilePosition].first - _tilePositionToMaxEnclosingFWHMIndices[tilePosition].first);
		for(int i=maximumIndexOfFullWfm; i>=maximumIndexOfFullWfm - iterateOverNrSamples_decreasing; i--)
		{
			if(i < 0)
				break;

			if(scaledSamplesOfFullWfm->at(i) < acceptanceThreshold)
			{
				maximumWasBanned = true;
				*(_tilePositionToIndexBanList[tilePosition]) << maximumIndexOfFullWfm;
				break;
			}
		}	
	}

	return maximumWasBanned;
}
bool CalibrationMode_WaveformDecomposition::maxIndexInBanList(int tilePosition, int maximumIndex)
{
	bool maximumWasBanned = false;

	maximumWasBanned = _tilePositionToIndexBanList[tilePosition]->contains(maximumIndex);

	return maximumWasBanned;
}
/////////////////////////////////Helper Functions////////////////////////////////////////////////////
void CalibrationMode_WaveformDecomposition::writeDedicatedChannelWaveform(vector <Waveform*> *actualWaveforms)
{
	for(int i=0; i<_numberOfChannels; i++)
	{
		if( (actualWaveforms->at(i)->getOsciChannelPair()->first == _coincidenceScintiOnOsciChannel.first &&
			 actualWaveforms->at(i)->getOsciChannelPair()->second == _coincidenceScintiOnOsciChannel.second)
			 ||
		     (actualWaveforms->at(i)->isWfmOfDedicatedChannel())
		   )
		{
			Waveform *dedicatedWfm = actualWaveforms->at(i);
			pair <int,int> totalSampleWindow(0,dedicatedWfm->getScaledSamples()->size());
			_plotPool->writeCanvasWithGraphOfWaveform(dedicatedWfm,totalSampleWindow,dedicatedWfm->getChannelName());
		}
	}
}
void CalibrationMode_WaveformDecomposition::resetModificationStepsMap()
{
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
		_tilePositionToNrModificationSteps[i] = 0;
}

int CalibrationMode_WaveformDecomposition::getMaximumIndexOfFullWfm_respectingBanList(int tilePosition, vector<float> *sampleVector)
{
	int maximumIndex = -1;
	float maximum = -1E10;
	for(int i=0; i<(int)sampleVector->size(); i++)
	{
		if(sampleVector->at(i) > maximum)
		{
			// check if the maximum was banned before
			bool maximumIndexWasBanned = this->maxIndexInBanList(tilePosition,i);
			if(maximumIndexWasBanned)
				continue;

			// set the current maximum
			maximum = sampleVector->at(i);
			maximumIndex = i;
		}
	}

	return maximumIndex;
}

void CalibrationMode_WaveformDecomposition::performSmoothingOnWaveform(Waveform *wfm)
{
	// smooth the full waveform
	int tilePosition = wfm->getTilePosition();
	vector<float> *scaledSamplesOfFullWfm = wfm->getScaledSamples();

	pair <int,int> totalSampleWindow(0,scaledSamplesOfFullWfm->size());
	TH1F *histOfFullWfm = _plotPool->createHistogramOfWaveform(wfm,totalSampleWindow,QString("%1 %2").arg(_EventNumber).arg(tilePosition));
	histOfFullWfm->Smooth(1);
	//_tFile->WriteTObject(histOfFullWfm);
	for(int j=0; j<(int)scaledSamplesOfFullWfm->size(); j++)
	{
		float smoothedVolt = histOfFullWfm->GetBinContent(j+1);

		scaledSamplesOfFullWfm->at(j) = smoothedVolt;
	}

	delete histOfFullWfm;
}

/////////////////////////////////Reading Averaged WfmInfo////////////////////////////////////////////////////
void CalibrationMode_WaveformDecomposition::readInSerializedAvaragingValues()
{
	// initialize serialization only one time
	if(!_serializationAveraged1peInput)
	{
		_serializationAveraged1peInput = new Serialization_Averaged1pePeak(this->determineFilePathToCalibrationData(true).toStdString(),"read");

		QVariant averaged1peWfmsAvailableEveryNrSteps_qvar(
			_serializationAveraged1peInput->getGeneralInformation()->conversionInfo->dumpOfAllSettings["rm_averageOverNrSpills"].c_str() );
		_averaged1peWfmsAvailableEveryNrSteps = averaged1peWfmsAvailableEveryNrSteps_qvar.toInt();
	}

	// read next spill information
	_serializationAveraged1peInput->readNextSequentialInfo();

	// obtain information from the currently loaded averaged info
	SpillInformation *_spillInfo = _serializationAveraged1peInput->getSpillInformation();
	_currentAveragedInfoCorrespondsToSpillNr = _spillInfo->spillNumber;

	_nrChannelsWith1peAveragedWaveform = _spillInfo->_tilePosition_To_NrSummedWaveformsForAveraging.size();
	_tilePositionToNrSummedWaveformsForAveraging = QMap<int,int>( _spillInfo->_tilePosition_To_NrSummedWaveformsForAveraging );
	_tilePositionToTimingOfSamplesMap = QMap<int,vector<float> >( _spillInfo->_tilePosition_To_TimingOfAveragedSamplesMap );
	_tilePositionToSummedSamplesMap = QMap<int,vector<float> >( _spillInfo->_tilePosition_To_AveragedSamplesMap );

	_tilePositionToMaximumIndexAndMaximumValueMap = QMap<int,pair<int,float> >( _spillInfo->_tilePosition_To_MaxIndexAndMaxValue );
	_tilePositionToMaxEnclosingFWHMIndices = QMap<int,pair<int,int> >( _spillInfo->_tilePosition_To_LowerAndUpperFWHMIndex );
}
void CalibrationMode_WaveformDecomposition::loadNewAveraged1peInfoIfNecessary()
{
	// load the next averaged 1pe peak Information if necessary
	int currentSpillNumber = _spillRead->getSpillInfo()->spillNumber;
	bool necessarySpillNumberFound = false;
	int counter = 0;
	while(!necessarySpillNumberFound)
	{
		int previousCheckedSpillNumber = _currentAveragedInfoCorrespondsToSpillNr + counter*_averaged1peWfmsAvailableEveryNrSteps;
		int nextCheckedSpillNumber = previousCheckedSpillNumber + _averaged1peWfmsAvailableEveryNrSteps;
		
		int deltaCurrent_to_Previous = abs(currentSpillNumber - previousCheckedSpillNumber);
		int deltaCurrent_to_Next = abs(nextCheckedSpillNumber - currentSpillNumber);

		if( deltaCurrent_to_Previous <= deltaCurrent_to_Next )
		{
			necessarySpillNumberFound = true;
			int necessarySpillNumber = previousCheckedSpillNumber;

			// check if the demanded spill exists at all
			if(necessarySpillNumber >= _spillSyncInfo->getTotalNumberOfSpillsInRun() - (int)(((float)_averaged1peWfmsAvailableEveryNrSteps)/2))
			{
				QString warningMessage = QString("*** Current spill number: %1\n*** Currently loaded averaged1peInfo belongs to spill %2. Requesting for spill %3")
												.arg(currentSpillNumber).arg(_currentAveragedInfoCorrespondsToSpillNr).arg(necessarySpillNumber);
				warningMessage.prepend("*** Warning: Last available Averaged1peInfo reached. No further info will be loaded\n");
				cout<<warningMessage.toStdString().c_str()<<endl;

				break;
			}

			cout<<"Spill that needs to be loaded: "<<necessarySpillNumber<<endl;

			while(_serializationAveraged1peInput->getSpillInformation()->spillNumber != necessarySpillNumber)
			{
				this->readInSerializedAvaragingValues();

				if(_serializationAveraged1peInput->getSpillInformation()->spillNumber > necessarySpillNumber)
					throw GeneralException("*** Error: Reading in serialization iterated too far. Something went wrong!");
			}
		}

		counter++;
	}
}

/////////////////////////////////Rescale Waveforms to nanosec/////////////////////////////////////////////////
TH1F* CalibrationMode_WaveformDecomposition::replaceHistoByHistoRescaledToNS(TH1F *histo)
{
	TH1F *histoNew = new TH1F(histo->GetTitle(),histo->GetName(),histo->GetNbinsX(),0,histo->GetBinLowEdge(histo->GetNbinsX()+1)*1E9);

	for(int i=0; i<(int)histo->GetNbinsX(); i++)
		histoNew->SetBinContent(i,histo->GetBinContent(i));
	histoNew->GetXaxis()->SetTitle("Time [ns]");

	delete histo;

	return histoNew;
}
/////////////////////////////////Waveform Recreation/////////////////////////////////////////////////
QMap <int,TH1F *> *CalibrationMode_WaveformDecomposition::backupUnmodifiedWaveformsForRecreation(vector <Waveform*> *actualWaveforms)
{
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		if(actualWaveforms->at(i)->getNrOfSWTriggeredWfms())
		{
			pair <int,int> totalSampleWindow(0,actualWaveforms->at(i)->getScaledSamples()->size());
			_tilePositionToUnmodifiedWaveformBackup[i] = _plotPool->createHistogramOfWaveform(actualWaveforms->at(i),totalSampleWindow);
		}
	}
	
	return &_tilePositionToUnmodifiedWaveformBackup;
}
void CalibrationMode_WaveformDecomposition::recreateWaveformAndDetermineQualityFactor(QMap <int,TH1F *> *unmodifiedWaveforms,QMap <int,TH1F *> *purifiedEdepHistogram, bool writeRecreatedWaveform)
{
	// assemble a waveform from the edepHistogram
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		if((*purifiedEdepHistogram)[i])
		{
			TH1F *edepHist = (*purifiedEdepHistogram)[i];

			int edepNBinsX = edepHist->GetNbinsX();
			QString titleRecoWfm = QString("%1 - Reconstructed").arg(edepHist->GetTitle());
			TH1F *reconstructedWaveform = new TH1F(titleRecoWfm.toStdString().c_str(),titleRecoWfm.toStdString().c_str(),
													edepNBinsX,0,edepNBinsX*edepHist->GetBinWidth(1));

			// go through all entries of the 1pe edep Histogramm
			vector<float> *scaledSamplesOf1peWfm = &_tilePositionToSummedSamplesMap[i];
			double lastTimeOfHit = 0;
			double firstTimeOfHit = 0;
			bool firstTimeOfHitWasDetermined = false;
			int maximumIndexOf1peWfm = _tilePositionToMaximumIndexAndMaximumValueMap[i].first;
			for(int j=0; j<edepHist->GetNbinsX(); j++)
			{
				int edepBinContent = edepHist->GetBinContent(j+1);
				if(edepBinContent == 0)
					continue;
	
				// determine the time of the first and last filled bin
				if(!firstTimeOfHitWasDetermined)
				{
					firstTimeOfHit = edepHist->GetBinCenter(j+1);
					firstTimeOfHitWasDetermined = true;
				}
				lastTimeOfHit = edepHist->GetBinCenter(j+1);

				int indexOf1peEdep = j;
				////////////////now adding starts/////////////////////////////////////////
				// add from the maximum (inclusive) to the end of the available 1pe values
				for(int k=indexOf1peEdep; k<indexOf1peEdep + (scaledSamplesOf1peWfm->size() - maximumIndexOf1peWfm); k++)
				{
					float currentSample1peWfm = scaledSamplesOf1peWfm->at(k - indexOf1peEdep + maximumIndexOf1peWfm);

					//if(k >= edepNBinsX || currentSample1peWfm < _breakSubstractionIfAveraged1peWfmBelowThreshold)
					//	break;
					if(k >= edepNBinsX)
						break;

					float prevoiusBinContent = reconstructedWaveform->GetBinContent(k+1);
					reconstructedWaveform->SetBinContent(k+1,prevoiusBinContent + (currentSample1peWfm * edepBinContent) );
				}

				// add from the maximum (exclusive) to the begin of the available 1pe values
				for(int k=indexOf1peEdep-1; k>=indexOf1peEdep - maximumIndexOf1peWfm; k--)
				{
					float currentSample1peWfm = scaledSamplesOf1peWfm->at(k - indexOf1peEdep + maximumIndexOf1peWfm);

					if(k < 0 || currentSample1peWfm < _breakSubstractionIfAveraged1peWfmBelowThreshold)
						break;

					float prevoiusBinContent = reconstructedWaveform->GetBinContent(k+1);
					reconstructedWaveform->SetBinContent(k+1,prevoiusBinContent + (currentSample1peWfm * edepBinContent) );
				}
			}

			// determine Quality factor
			double chiSquare = 0;
			//int nrDegreesOfFreedom = 0;
			for(int j=0; j<(*unmodifiedWaveforms)[i]->GetNbinsX(); j++)
			{
				float originalBinContent = (*unmodifiedWaveforms)[i]->GetBinContent(j+1);
				float reconstructedBinContent = reconstructedWaveform->GetBinContent(j+1);
				
				//if(reconstructedBinContent == 0)
				//	continue;
				//nrDegreesOfFreedom++;
				chiSquare += ( (originalBinContent - reconstructedBinContent)*(originalBinContent - reconstructedBinContent) ) / fabs(originalBinContent);
			}
			//chiSquare /= nrDegreesOfFreedom;
			_chi2Distribution->Fill(chiSquare);
			if(chiSquare != 0)
				_tilePositionToCurrentChiSquare[i] = chiSquare;

			// draw the original and reconstructed waveform in one canvas
			if(writeRecreatedWaveform)
			{
				// for scaling to nanosec
				//(*unmodifiedWaveforms)[i] = this->replaceHistoByHistoRescaledToNS((*unmodifiedWaveforms)[i]);
				//_tilePositionToUnmodifiedWaveformBackup[i] = (*unmodifiedWaveforms)[i];
				//reconstructedWaveform = this->replaceHistoByHistoRescaledToNS(reconstructedWaveform);
				//edepHist = this->replaceHistoByHistoRescaledToNS(edepHist);
				//_tilePositionToEdepHistogramMap[i] = edepHist;
				//firstTimeOfHit *= 1E9;
				//lastTimeOfHit *= 1E9;

				QString canvasTitle = QString("Compare: Orig vs. Reconstructed Waveform - ChiSquare:%1 - %2").arg(chiSquare).arg(edepHist->GetTitle());
				TCanvas can(canvasTitle.toStdString().c_str(),canvasTitle.toStdString().c_str(),700,400);

				TPad pad1("pad1","",0,0,1,1);
				TPad pad2("pad2","",0,0,1,1);
				pad2.SetFillStyle(4000);
				pad1.Draw();
				pad1.cd();
				//can.cd();

				(*unmodifiedWaveforms)[i]->SetLineColor(2);
				(*unmodifiedWaveforms)[i]->SetFillColor(2);
				(*unmodifiedWaveforms)[i]->SetFillStyle(3004);
				double histXRangeMin = firstTimeOfHit * 0.9; //reconstructedWaveform->GetXaxis()->GetXmin();
				double histXRangeMax = lastTimeOfHit * 1.1;  //reconstructedWaveform->GetXaxis()->GetXmax();
				if(histXRangeMax > edepNBinsX*edepHist->GetBinWidth(1))
					histXRangeMax = edepNBinsX*edepHist->GetBinWidth(1);
				double histYRangeMax = (*unmodifiedWaveforms)[i]->GetMaximum() * 1.1;
				double histYRangeMin = -histYRangeMax * 0.05;
				(*unmodifiedWaveforms)[i]->GetXaxis()->SetRangeUser(histXRangeMin,histXRangeMax);
				(*unmodifiedWaveforms)[i]->GetYaxis()->SetRangeUser(histYRangeMin,histYRangeMax);
				(*unmodifiedWaveforms)[i]->Draw();
				(*unmodifiedWaveforms)[i]->GetYaxis()->SetTickLength(0.01);
				(*unmodifiedWaveforms)[i]->GetYaxis()->SetTicks("");
				reconstructedWaveform->SetLineWidth(3);
				reconstructedWaveform->GetXaxis()->SetRangeUser(histXRangeMin,histXRangeMax);
				reconstructedWaveform->Draw("same");

				//compute the pad range with suitable margins
				Double_t ymax = edepHist->GetMaximum() * 1.1;
				double zeroLineFraction = histYRangeMax / (fabs(histYRangeMax) + fabs(histYRangeMin));
				Double_t ymin = (-1) * ymax * (1/zeroLineFraction) * (1-zeroLineFraction);
				Double_t dy = (ymax-ymin)/0.8; //10 per cent margins top and bottom
				Double_t xmin = histXRangeMin;
				Double_t xmax = histXRangeMax;
				Double_t dx = (xmax-xmin)/0.8; //10 per cent margins left and right
				pad2.Range(xmin-0.1*dx,ymin-0.1*dy,xmax+0.1*dx,ymax+0.1*dy);
				pad2.Draw();
				pad2.cd();

				edepHist->SetLineColor(4);
				edepHist->SetFillColor(4);
				edepHist->SetFillStyle(3001);
				edepHist->GetXaxis()->SetRangeUser(histXRangeMin,histXRangeMax);
				edepHist->Draw("][sames");
				pad2.Update();

				// draw axis on the right side of the pad
				TGaxis axis(xmax,ymin,xmax,ymax,ymin,ymax,50510,"+L");
				axis.SetLabelColor(4);
				axis.SetLineColor(4);
				axis.Draw();

				// create corresponding legend entry
				QString legTitle = QString("Compare: Orig vs. Reco Waveform: #chi^{2}=%1").arg(chiSquare);
				TLegend legend(0.5,0.7,0.9,0.9,legTitle.toStdString().c_str());
				legend.SetFillColor(0);
				legend.AddEntry((*unmodifiedWaveforms)[i],"Unmodified Waveform","l");
				legend.AddEntry(reconstructedWaveform,"Reconstructed Waveform","l");
				legend.AddEntry(edepHist,"Distribution of 1pe Hits","l");
				legend.Draw();

				// now save the reconstructed Waveform
				_tFile->WriteTObject(&can);
				//delete can;

				//_tFile->WriteTObject((*unmodifiedWaveforms)[i]);
				//_tFile->WriteTObject(reconstructedWaveform);
				//_tFile->WriteTObject(edepHist);
				//_tFile->WriteTObject(&legend);
			}

			delete reconstructedWaveform;
		}
	}
}

// cleanup and reset//////////////////////////////////////////////////
void CalibrationMode_WaveformDecomposition::resetVariables()
{
	this->resetModificationStepsMap();
	this->resetMapOfBanLists();

	// chiSquare Quality factor
	_tilePositionToCurrentChiSquare.clear();

	// Time of 1pe Edep
	for(int i=0; i<_tilePositionToTimeOfEdep.size(); i++)
		_tilePositionToTimeOfEdep[i].clear();
}
void CalibrationMode_WaveformDecomposition::cleanUpMemory()
{
	// delete unmodified waveform backup
	if(_tilePositionToUnmodifiedWaveformBackup.size())
	{
		foreach (TH1F* value, _tilePositionToUnmodifiedWaveformBackup)
			if(value)
				delete value;
		_tilePositionToUnmodifiedWaveformBackup.clear();
	}

	// delete 1D purified edep histogram
	if(_tilePositionToEdepHistogramMap.size())
	{
		foreach (TH1F* value, _tilePositionToEdepHistogramMap)
			if(value)
				delete value;
		_tilePositionToEdepHistogramMap.clear();
	}
	
	// delete 2D purified edep histogram
	if(_tilePositionToEdep2DHistogramMap.size())
	{
		foreach (TH2F* value, _tilePositionToEdep2DHistogramMap)
			if(value)
				delete value;
		_tilePositionToEdep2DHistogramMap.clear();
	}
}
// Boost serialization///////////////////////////////////////////////////////////
void CalibrationMode_WaveformDecomposition::initializeSerialization()
{
	_serializationObject = new Serialization_WaveformDecomposition(this->determineFilePathToCalibrationData(_analyzingIntermediateRM).toStdString(),"write");

	// set the file name if distinct spills were selected for analyzing
	string tmp = this->getRunAndSpillTitleSuffix().toStdString();
	_serializationObject->setOutputFileSuffix(this->getRunAndSpillTitleSuffix().toStdString());

	// set general run information
	RunInformation *runInfo = _serializationObject->getGeneralInformation()->runInfo;
	runInfo->runDirectory = _runRead->getRunDirectory().toStdString();
	runInfo->runMode = _runRead->getDataRunMode().toStdString();
	runInfo->runNumber = _spillRead->getSpillInfo()->runNumber;
	runInfo->nrEnabledChannels = _nrChannelsWith1peAveragedWaveform;
	runInfo->runEnergy = __globalSettingsManager->getSetting("general_runEnergy").toInt();

	// set general conversion information
	ConversionInformation *convInfo = _serializationObject->getGeneralInformation()->conversionInfo;
	convInfo->analysisMode = __globalSettingsManager->getSelectedAnalysisMode().toStdString();
	convInfo->stopSubstractionAtThresh = __globalSettingsManager->getSetting("rm_lowerThreshOfWfmMaxMV").toFloat();
	convInfo->demandedFractionOf1peMaximum = __globalSettingsManager->getSetting("rm_demandedFracOfAveraged1peMax").toFloat();
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
void CalibrationMode_WaveformDecomposition::serializeEvent(vector <Waveform*> *actualWaveforms)
{
	EventInformation *eventInfo = _serializationObject->getEventInformation();
	eventInfo->eventNumber = _EventNumber;
	eventInfo->goodEvents = _nrTotalEvents;
	eventInfo->eventNumberInCurrentSpill = _EventNumberInCurrentSpill;
	eventInfo->spillNumber = _spillRead->getSpillInfo()->spillNumber;
	eventInfo->timeOfDataReady = _spillRead->getSpillInfo()->timeOfDataReady.toTime_t();
	for(int i=0; i<_nrChannelsWith1peAveragedWaveform; i++)
	{
		if(_tilePositionToTimeOfEdep[i].size())
		{
			eventInfo->tilePosition_To_waveformOverflowOccurred[i] = actualWaveforms->at(i)->getOverflowBit();
	
			eventInfo->triggeredTilePositions.push_back(i);
			
			eventInfo->tilePosition_To_ConversionQualityFactor[i] = _tilePositionToCurrentChiSquare[i];

			vector<float> *timingOfSamples = actualWaveforms->at(i)->getTimingOfSamples();
			for (int j=0; j<(int)actualWaveforms->at(i)->getNrOfSWTriggeredWfms(); j++)
			{
				// time window of software triggered waveform
				pair<float,float> timeWindow(timingOfSamples->at(actualWaveforms->at(i)->getTriggeredMinMaxSampleWindows()->at(j).first),
											 timingOfSamples->at(actualWaveforms->at(i)->getTriggeredMinMaxSampleWindows()->at(j).second) );
				eventInfo->tilePosition_To_TriggeredTimeWindow[i].push_back(timeWindow);
				eventInfo->tilePosition_To_TriggeredWaveformIntegral[i].push_back(actualWaveforms->at(i)->getIntegralOfTriggeredWfms()->at(j));
				eventInfo->tilePosition_To_TriggerTime[i].push_back(actualWaveforms->at(i)->getTimeOfTriggerings()->at(j));
				eventInfo->tilePosition_To_WaveformMax[i].push_back(actualWaveforms->at(i)->getMaxOfSWTriggeredWfm()->at(j));
				eventInfo->tilePosition_To_TimeOfWaveformMax[i].push_back(timingOfSamples->at(actualWaveforms->at(i)->getMaxSampleOfSWTriggeredWfm()->at(j)));
			}

			for (int j=0; j<(int)_tilePositionToTimeOfEdep[i].size(); j++)
			{
				float timeOfEdep = _tilePositionToTimeOfEdep[i].at(j);
				eventInfo->tilePosition_To_TimeOf1peHit[i].push_back(timeOfEdep);
			}
		}
	}

	// dedicated channel info
	for(int j=0; j<actualWaveforms->size(); j++)
	{
		for(int i=0; i<this->getNumberOfDedicatedChannels(); i++)
		{
			if(actualWaveforms->at(j)->getChannelName() == _dedicatedChannels_Names.at(i))
			{
				string dedicatedChName = _dedicatedChannels_Names.at(i).toStdString();

				if(_oldAndNewNameOfScintiChannel.second != "")
					if(dedicatedChName == _oldAndNewNameOfScintiChannel.first.toStdString())
						dedicatedChName = _oldAndNewNameOfScintiChannel.second.toStdString();

				bool somethingWasTriggered = actualWaveforms->at(j)->getNrOfSWTriggeredWfms();

				eventInfo->dedicatedChanelNames.push_back( dedicatedChName );

				if(somethingWasTriggered)
				{
					eventInfo->triggeredDedicatedChannels.push_back( dedicatedChName );
					eventInfo->dedicatedChannelName_To_TypicalTriggerTime[dedicatedChName].push_back(_dedicatedChannels_TypicalTriggerTime.at(i));
					eventInfo->dedicatedChannelName_To_AcceptanceRangeAroundTypicalTriggerTime[dedicatedChName].push_back(_dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime.at(i));

					vector<float> *timingOfSamples = actualWaveforms->at(j)->getTimingOfSamples();
					for(int k=0; k<actualWaveforms->at(j)->getNrOfSWTriggeredWfms(); k++)
					{
						// time window of software triggered waveform
						pair<float,float> timeWindow(timingOfSamples->at(actualWaveforms->at(j)->getTriggeredMinMaxSampleWindows()->at(k).first),
													 timingOfSamples->at(actualWaveforms->at(j)->getTriggeredMinMaxSampleWindows()->at(k).second) );
						eventInfo->dedicatedChannelName_To_TriggeredTimeWindow[dedicatedChName].push_back(timeWindow);
						eventInfo->dedicatedChannelName_To_TriggeredWaveformIntegral[dedicatedChName].push_back(actualWaveforms->at(j)->getIntegralOfTriggeredWfms()->at(k));
						
						float triggerTime = actualWaveforms->at(j)->getTimeOfTriggerings()->at(k);
						eventInfo->dedicatedChannelName_To_TriggerTime[dedicatedChName].push_back( triggerTime );
	
						// was there a signal at the expected trigger time
						eventInfo->dedicatedChannelName_To_FiredAtTypicalTriggerTime[dedicatedChName] = false;
						if(triggerTime < (_dedicatedChannels_TypicalTriggerTime.at(i) + _dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime.at(i)) &&
						   triggerTime > (_dedicatedChannels_TypicalTriggerTime.at(i) - _dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime.at(i)) )
						{
							eventInfo->dedicatedChannelName_To_FiredAtTypicalTriggerTime[dedicatedChName] = true;
							break;
						}
					}
				}
				else
					eventInfo->dedicatedChannelName_To_FiredAtTypicalTriggerTime[dedicatedChName] = false;
			}
		}
	}

	// finally write the event
	_serializationObject->writeSequentialInfo();
}
void CalibrationMode_WaveformDecomposition::finalizeSerialization()
{
	_serializationObject->finalizeSerialization();
}
// destructor///////////////////////////////////////////////////////////
CalibrationMode_WaveformDecomposition::~CalibrationMode_WaveformDecomposition(void)
{
}
