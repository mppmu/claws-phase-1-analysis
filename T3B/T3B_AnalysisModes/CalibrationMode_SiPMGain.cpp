#include "CalibrationMode_SiPMGain.h"

CalibrationMode_SiPMGain::CalibrationMode_SiPMGain(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->initializeIntegralDistributionsForSiPMGainDetermination();
	this->initializeSiPMGainFile();

	this->startAnalysis();
}

void CalibrationMode_SiPMGain::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationSiPMGain")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	_useNrSpillsForGainDetermination = __globalSettingsManager->getSetting("rm_useNrSpillsForGainDetermination").toInt();

	// path to the temperature data
	_filePathTemperatureData = __globalSettingsManager->getSetting("rm_filePathTemperatureData").toString();
}

void CalibrationMode_SiPMGain::initializeAnalysisModeVariables()
{
	cout<<"Initializing Temperature Measurement Class"<<endl;
	_temperatureData = new AccessTemperatureData(_filePathTemperatureData);
	cout<<"Temperature Information is now available"<<endl;

	_lastSpillNumber = -1;
	_lastRunNumber = this->getCurrentSpillRead()->getSpillInfo()->runNumber;
	
	//_numberOfSpillsOverjumped = 0;

	_isFirstAnalyzedSpill = true;
	_gainHasNotBeenWrittenOnce = true;

	_spillNrCountDownForHistogramFilling = _useNrSpillsForGainDetermination;
}

void CalibrationMode_SiPMGain::startAnalysis()
{
	int nrAnalyzedEvents = 0;
	int nrTotalEvents = 0;
	bool eventLoopWasAccessed = false;
	while(!this->endOfDataReached() && nrAnalyzedEvents < _nrEventsToAnalyze)
	{
		eventLoopWasAccessed = true;

		// load the next waveform and get all current waveforms
		vector <Waveform*> actualWaveforms = this->loadNextWfmOnAllChannels();

		nrTotalEvents++;

		// trigger the waveforms
		bool somethingWasTriggered = this->triggerWaveformsWithSelectedTrigger(&actualWaveforms);
		if(!somethingWasTriggered) 
			continue;





		// determine if a new spill was loaded
		_isFirstAnalyzedWaveformInNewSpill = false;
		int currentSpillNumber = _spillRead->getSpillInfo()->spillNumber;
		if(currentSpillNumber != _lastSpillNumber)
		{
			//if(currentSpillNumber != _lastSpillNumber+1 && _lastSpillNumber != -1)
			//{
			//	_numberOfSpillsOverjumped = currentSpillNumber - _lastSpillNumber - 1;

			//	// exactly one spill overjumped will be tolerated
			//	if(_numberOfSpillsOverjumped == 1)
			//		cerr<<"*** Warning: One Spill was ignored (Nothing was triggered or no IntermediatRM existed)!"<<endl;
			//	else
			//	{
			//		QString errorMessage = 
			//		QString("%1%2%3%4%5").arg("*** Error: More than one Spill was ignored (Nothing was triggered or no IntermediatRM existed)!")
			//							.arg("\n*** currentSpillNumber:")
			//							.arg(currentSpillNumber)
			//							.arg(" lastSpillNumber:")
			//							.arg(_lastSpillNumber);
			//		throw GeneralException(errorMessage.toStdString().c_str());
			//	}
			//}

			if(currentSpillNumber != _lastSpillNumber+1 && _lastSpillNumber != -1)
			{
				QString errorMessage = 
				QString("%1%2%3%4%5").arg("*** Error: One or more Spills was ignored (Nothing was triggered or no IntermediatRM existed)!")
									.arg("\n*** currentSpillNumber:")
									.arg(currentSpillNumber)
									.arg(" lastSpillNumber:")
									.arg(_lastSpillNumber);
				throw GeneralException(errorMessage.toStdString().c_str());
			}

			_spillNrToCorrespondingTimeT[currentSpillNumber] = _spillRead->getSpillInfo()->timeOfDataReady.toTime_t();

			_lastSpillNumber = currentSpillNumber;
			_isFirstAnalyzedWaveformInNewSpill = true;

			this->writeAndRenewIntegralDistributionsMap();
		}

		// save integral distribution of all triggered waveforms
		this->fillIntegralDistributionsForSiPMGainDetermination(actualWaveforms);

		// plot first raw waveforms
		if(nrAnalyzedEvents<50)
		{
			// save the triggered waveforms of all _enabledOsciChannels in several canvases
			_plotPool->writeTriggeredWaveforms(actualWaveforms);
		}





		if((nrAnalyzedEvents+1)%100 == 0) 
			cout<<"Analyzing Waveform Number: "<<nrAnalyzedEvents+1
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<nrTotalEvents
			<<endl;

		nrAnalyzedEvents++;
	}
	// this is just a dirty workaround
	if(_gainHasNotBeenWrittenOnce && eventLoopWasAccessed)
		this->writeAndRenewIntegralDistributionsMap();


	this->finalizeSiPMGainFile();

	this->createAndWriteGainAndTemperatureVsTimePlot();
	this->createAndWriteGainVsTemperaturePlot();

	// move the TFile
	this->moveRootFileToCalibFolder();
}

// integral Distributions
void CalibrationMode_SiPMGain::initializeIntegralDistributionsForSiPMGainDetermination()
{
	// Integral Distribution Histograms
	for(int j=0; j<_useNrSpillsForGainDetermination; j++)
	{
		// the determined SiPMGain should be assigned to the middle
		int correspondingSpillNr = _useNrSpillsForGainDetermination/2 + j + 1 + _spillRead->getSpillInfo()->spillNumber;
	
		_spillNrToTilepositionToIntegralDistributionMap[correspondingSpillNr] = 
			(this->createHistogramsForAllChannelsForIntegralDistribution(correspondingSpillNr));
	}
}
QMap<int , TH1D *> CalibrationMode_SiPMGain::createHistogramsForAllChannelsForIntegralDistribution(int correspondingSpillNr)
{
	QMap<int , TH1D *> newTilepositionToIntegralDistribution;
	for(int i=0; i<_numberOfChannels; i++)
	{
		// determine tileposition
		pair<QString,QString> *osciChannel = _allChannelDataVec->at(i)->getDataCorrespondingOsciAndChannel();
		if( !_spillRead->aSiPMisConnectedToOsciChannel(osciChannel->first, osciChannel->second) )
			continue;

		int tilePosition = _spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO[osciChannel->first][osciChannel->second];

		// create histogram
		TString histName = QString("%1-IntegralDistribution: Spill %2, Osci %3, %4, Pos %5")
			.arg(_outputFileName)
			.arg(correspondingSpillNr)
			.arg(osciChannel->first)
			.arg(osciChannel->second)
			.arg(tilePosition).toStdString().c_str();
		TH1D *h1 = new TH1D(histName,histName,500,0,3E-12);
		h1->GetXaxis()->SetTitle("Wfm Integral [C]");
		h1->GetYaxis()->SetTitle( "# Counts" );
		h1->GetYaxis()->SetTitleOffset(1.3);
		h1->GetXaxis()->SetTitleOffset(1.0);
		h1->SetFillColor(28);

		newTilepositionToIntegralDistribution[i] = h1;
	}
	//
	return newTilepositionToIntegralDistribution;
}

void CalibrationMode_SiPMGain::fillIntegralDistributionsForSiPMGainDetermination(vector <Waveform*> &actualWaveforms)
{
	// check the current run number
	int currentRunNumber = this->getCurrentSpillRead()->getSpillInfo()->runNumber;
	if(currentRunNumber != _lastRunNumber)
		throw GeneralException("two run numbers used->not yet implemented!");

	// decrement Spill Count Down (only necessary at the beginning of the analysis
	if(_spillNrCountDownForHistogramFilling > 0 && _isFirstAnalyzedWaveformInNewSpill)
	{
		_spillNrCountDownForHistogramFilling--;
	}

	QMapIterator<int , QMap<int , TH1D *> > it(_spillNrToTilepositionToIntegralDistributionMap);
	for(int i=0; i<(_useNrSpillsForGainDetermination - _spillNrCountDownForHistogramFilling); i++)
	{
		if(it.hasNext())
		{
			it.next();
			for(int j=0; j<(int)actualWaveforms.size(); j++)
			{
				// determine tileposition
				pair<QString,QString> *osciChannel = actualWaveforms.at(j)->getOsciChannelPair();
				if( !_spillRead->aSiPMisConnectedToOsciChannel(osciChannel->first, osciChannel->second) )
					continue;

				int tilePosition = _spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO[osciChannel->first][osciChannel->second];

				for (int k = 0; k < (int)actualWaveforms[j]->getIntegralOfTriggeredWfms()->size(); k++)
				{
					it.value()[tilePosition]->Fill(actualWaveforms[j]->getIntegralOfTriggeredWfms()->at(k));
				}
			}
		}
		else
			throw GeneralException("*** Error: QMap _spillNrToTilepositionToIntegralDistributionMap out of range access");
	}
}
void CalibrationMode_SiPMGain::writeAndRenewIntegralDistributionsMap()
{
	// we only write once the initial rm_useNrSpillsForGainDetermination spills were processed
	if(_spillNrCountDownForHistogramFilling == 0)
	{
		// determine tilePos to Distribution map which is saved now
		QMap<int , QMap<int , TH1D *> >::iterator it = _spillNrToTilepositionToIntegralDistributionMap.begin();
		int correspondingSpillNr = it.key();

		// get/create directory and SubDirectory
		QString dirName = QString("Spill_%1").arg(correspondingSpillNr);
		TDirectory *dir = this->getTDirectory(dirName);
		TDirectory *subDir = this->getTSubDirectory(dir,"IntegralDistribution");

		// determine the sipm gain
		this->determineSiPMGain(correspondingSpillNr,it.value());
		
		this->writeHistogramOfAllSiPMGainValues(subDir);
		this->writeCanvasesOfFittedIntegralDistributionsOfAllChannels(subDir);
		this->writeIntegralDistributionsOfAllChannels(subDir,correspondingSpillNr,it.value());
		
		this->writeSiPMGainValueToCalibFile(correspondingSpillNr);
		this->logGainTimeAndTemperature(correspondingSpillNr);

		// erase the gain of the spill that was just written
		foreach (TH1D* value, it.value())
			delete value;
		_spillNrToTilepositionToIntegralDistributionMap.erase(it);

		// determine the corresponding spill number for the next histogram
		int newCorrespondingSpillNr = 0;
		if(_spillNrToTilepositionToIntegralDistributionMap.size())
		{
			QMapIterator<int , QMap<int , TH1D *> > newIt(_spillNrToTilepositionToIntegralDistributionMap);
			newIt.toBack();
			newIt.previous();
			newCorrespondingSpillNr = newIt.key() + 1;
		}
		else 
			newCorrespondingSpillNr = correspondingSpillNr + 1;

		//// if spills were overjumped -> reset the variable now
		//_numberOfSpillsOverjumped = 0;

		// insert the created map into the spill map
		_spillNrToTilepositionToIntegralDistributionMap[newCorrespondingSpillNr] = 
			(this->createHistogramsForAllChannelsForIntegralDistribution(newCorrespondingSpillNr));

		// this is just a dirty workaround
		_gainHasNotBeenWrittenOnce = false;
	}
}
//determine the SiPM Gain
void CalibrationMode_SiPMGain::determineSiPMGain(int correspondingSpillNr,QMap<int , TH1D *> &tilepositionToIntegralDistribution)
{
	// clear the vector containing the pulse distribution canvases and the gainMap
	for(int i=0; i<(int)_pulseDistCanvasVec.size(); i++)
	{
		delete _pulseDistCanvasVec.at(i);
		// Todo
		//delete _pulseDistfgauss1PE.at(i);
		//delete _pulseDistfgauss2PE.at(i);
	}
	//_pulseDistCanvasVec.erase(_pulseDistCanvasVec.begin(),_pulseDistCanvasVec.end());
	_pulseDistCanvasVec.clear();
	_tilePositionToSiPMGainMap.clear();
	_tilePositionToSiPMGainErrorMap.clear();

	int nrEnabledChannels = _spillRead->getSpillInfo()->nrEnabledChannels;
	for(int j=0; j<_spillRead->getNrChannelsWithSiPMsAttached(); j++)
	{
		QString canName = tilepositionToIntegralDistribution[j]->GetTitle();

		// create the canvas and fill the pulse distribution
		TCanvas *c = new TCanvas(canName.toStdString().c_str(),canName.toStdString().c_str(),700,500);
		TH1D h1 = *tilepositionToIntegralDistribution[j];
		h1.Smooth(50,NULL);
		c->cd();
		tilepositionToIntegralDistribution[j]->Draw();

		int totalNoOfWaveforms = (int)h1.GetEntries();
		if(totalNoOfWaveforms < 3000)
		{
			cerr<<"*** Warning: Number of entries might be too low for gain determination (<3000)"<<endl;
		}

		// determine the maxBin and its content
		vector <int> binnumber;
		binnumber.push_back(h1.GetMaximumBin());

		int maximumBinNumber = h1.GetMaximumBin();

		if(binnumber.size() != 0)
		{
			double firstMaxPosition = h1.GetBinCenter(maximumBinNumber);
			//cout<<"firstMaxPosition: "<<firstMaxPosition<<endl;

			double firstMaxValue = h1.GetBinContent(maximumBinNumber);
			//cout<<"firstMaxValue: "<<firstMaxValue<<endl;

			// fit 1p.e. peak
			// GaussParameter: 0:Amplitude 1:Mittel  2:Breite
			//TF1 *fgauss1PE = new TF1("gaus1pe","gaus",firstMaxPosition*5/6,firstMaxPosition*7/6);
			TF1 *fgauss1PE = new TF1("gaus1pe","gaus",firstMaxPosition*21./24,firstMaxPosition*27./24);
			fgauss1PE->SetParameter(0,firstMaxValue);
			fgauss1PE->SetParameter(1,firstMaxPosition);
			fgauss1PE->SetParameter(2,firstMaxPosition);
			fgauss1PE->SetNpx(1000);
			h1.Fit(fgauss1PE,"NQR");//,"",firstMaxPosition*5/6,firstMaxPosition*7/6);
			fgauss1PE->Draw("lsame");

			//cout<<"1p.e.: LowerRange: "<<firstMaxPosition*5/6<<"   UpperRange: "<<firstMaxPosition*7/6<<endl;

			// fit 2p.e. peak
			//TF1 *fgauss2PE = new TF1("gaus2pe","gaus",firstMaxPosition*11/6,firstMaxPosition*15/6);
//			TF1 *fgauss2PE = new TF1("gaus2pe","gaus",firstMaxPosition*23./12,firstMaxPosition*29./12);
			TF1 *fgauss2PE = new TF1("gaus2pe","gaus",firstMaxPosition*20./12,firstMaxPosition*29./12);
			fgauss2PE->SetParameter(0,firstMaxValue*0.05);
			fgauss2PE->SetParameter(1,firstMaxPosition*2);
			fgauss2PE->SetParameter(2,firstMaxPosition);
			fgauss2PE->SetNpx(1000);
			h1.Fit(fgauss2PE,"NQR");//,"",firstMaxPosition*3/4,h1->GetBinCenter(maximumBinNumber)*5/4);
			fgauss2PE->Draw("lsame");

			//cout<<"2p.e.: LowerRange: "<<firstMaxPosition*11/6<<"   UpperRange: "<<firstMaxPosition*15/6<<endl;
			//cout<<"After Fitting: Peak 1: "<<fgauss1PE->GetParameter(1)<<"   Peak 2: "<<fgauss2PE->GetParameter(1)<<endl;

			// calculate SiPM Gain
			double siPMGain = fgauss2PE->GetParameter(1) - fgauss1PE->GetParameter(1);
			double siPMGainError = fgauss2PE->GetParError(1) + fgauss1PE->GetParError(1);
			//cout<<"SiPM Gain in Coulomb: "<<siPMGain<<endl;

			c->SetTitle((QString("%1 - SiPMGain[C]: %2").arg(canName).arg(siPMGain)).toStdString().c_str());
			_pulseDistCanvasVec.push_back(c);
			_pulseDistfgauss1PE.push_back(fgauss1PE);
			_pulseDistfgauss2PE.push_back(fgauss2PE);
			_tilePositionToSiPMGainMap[j] = siPMGain;
			_tilePositionToSiPMGainErrorMap[j] = siPMGainError;

			//// todo: eigentlich muss man die Tf1 woanders loeschen, sonst sind sie beim schreiben in die TFile
			//// nicht mehr zu verfuegung (aber was ist wenn ich die canvases garnicht schreiben will?)
			//delete fgauss1PE;
			//delete fgauss2PE;
		}
	}
}

void CalibrationMode_SiPMGain::writeHistogramOfAllSiPMGainValues(TDirectory *saveDirectory)
{
	// write a histogramm of all SiPM gains
	TH1D allSiPMGainshist("All SiPM Gains","All SiPM Gains",20,0,20);
	allSiPMGainshist.GetXaxis()->SetTitle("Tile Position (from center)");
	allSiPMGainshist.GetYaxis()->SetTitle("SiPM Gain [C]");
	
	for(int i=0; i<(int)_tilePositionToSiPMGainMap.size(); i++)
	{
		allSiPMGainshist.SetBinContent(i + 1,_tilePositionToSiPMGainMap[i]);
	}

	//save plots
	saveDirectory->WriteTObject(&allSiPMGainshist);
}

void CalibrationMode_SiPMGain::writeCanvasesOfFittedIntegralDistributionsOfAllChannels(TDirectory *saveDirectory)
{
	// save single pulse dist canvases with fits
	for (int i=0; i < (int)_pulseDistCanvasVec.size(); i++)
	{
		saveDirectory->WriteTObject(_pulseDistCanvasVec.at(i));
	}
}

void CalibrationMode_SiPMGain::writeIntegralDistributionsOfAllChannels(TDirectory *saveDirectory,int correspondingSpillNr,QMap<int , TH1D *> &tilepositionToIntegralDistribution)
{
	// save single pulse dist histograms
	for (int i=0; i < _numberOfChannels; i++)
	{
		saveDirectory->WriteTObject(tilepositionToIntegralDistribution[i]);
	}

	//write canvas with all pulse dists to file
	TCanvas c("All Pulse Distributions","All Pulse Distributions",700,500);
	c.cd();
	TLegend leg(0.6,0.5,0.9,0.9,"Tile Position(Osci,CH,Tile)");
	leg.SetFillColor(0);
	for (int i=0; i < _numberOfChannels; i++)
	{
		// get information of channelData
		pair<QString,QString> *osciChannel = _allChannelDataVec->at(i)->getDataCorrespondingOsciAndChannel();
		if( !_spillRead->aSiPMisConnectedToOsciChannel(osciChannel->first, osciChannel->second) )
			continue;
		int tilePosition = _spillRead->getGeometryInfo()->osciChannelToTilePosition_ItoO[osciChannel->first][osciChannel->second];
		QString attachedTile = _spillRead->getGeometryInfo()->tileAttachedToOsciChannel[osciChannel->first][osciChannel->second];

		// create corresponding legend entry
		QString legendEntry = QString("%1 (%2,%3,%4)")
							  .arg(tilePosition)
							  .arg(osciChannel->first)
							  .arg(osciChannel->second)
							  .arg(attachedTile);
		leg.AddEntry(tilepositionToIntegralDistribution[tilePosition],legendEntry.toStdString().c_str(),"f");

		// define the style
		if(i<2) tilepositionToIntegralDistribution[tilePosition]->SetFillColor(i+2);
		else tilepositionToIntegralDistribution[tilePosition]->SetFillColor(0);
		if(i < 2) tilepositionToIntegralDistribution[tilePosition]->SetFillStyle(i+3003);

		tilepositionToIntegralDistribution[tilePosition]->SetLineColor(i+2);

		// draw into the all histogramms canvas
		if(i) tilepositionToIntegralDistribution[tilePosition]->Draw("same");
		else 
		{
			tilepositionToIntegralDistribution[tilePosition]->SetTitle("Pulse Distribution of Different Channels");

			////todo remove this
			//tilepositionToIntegralDistribution[tilePosition]->Scale(1./4);

			tilepositionToIntegralDistribution[tilePosition]->Draw("");
		}
	}	
	leg.Draw();
	
	//save plots
	saveDirectory->WriteTObject(&c);
}
// overview plots of the SiPM Gain
void CalibrationMode_SiPMGain::logGainTimeAndTemperature(int correspondingSpillNr)
{
	for(int i=0; i<(int)_tilePositionToSiPMGainMap.size(); i++)
	{
		if(_temperatureData->temperatureArrayInfoExists())
		{
			TemperatureSensor_Array_closestMeasurement *closestMeasurement = 
				_temperatureData->getClosestTempMeasurement_TemperatureArray(_spillNrToCorrespondingTimeT[correspondingSpillNr]);

			if(closestMeasurement)
				_tilePositionToCurrentTemperature[i].push_back(closestMeasurement->_tilePosition_To_currentTemperaturePreliminary[i]);
			else if(!closestMeasurement && _temperatureData->temperatureStandaloneSensorInfoExists())
			{
				double closestTemperatureMeasurement = 
					_temperatureData->getClosestTempMeasurement_StandaloneSensor(_spillNrToCorrespondingTimeT[correspondingSpillNr]);
				// jump over spills for which no temperature measurement could be found
				if(closestTemperatureMeasurement == -274)
					continue;
				_tilePositionToCurrentTemperature[i].push_back(closestTemperatureMeasurement);
			}
		}
		else if(!_temperatureData->temperatureArrayInfoExists() && _temperatureData->temperatureStandaloneSensorInfoExists())
		{
			double closestTemperatureMeasurement = 
				_temperatureData->getClosestTempMeasurement_StandaloneSensor(_spillNrToCorrespondingTimeT[correspondingSpillNr]);
			// jump over spills for which no temperature measurement could be found
			if(closestTemperatureMeasurement == -274)
				continue;
			_tilePositionToCurrentTemperature[i].push_back(closestTemperatureMeasurement);
		}
		else // when there is no temperature information --> default to -274
		{
			cerr<<"*** Warning: No Temperature Information exists! (CalibrationMode_SiPMGain::logGainTimeAndTemperature)"<<endl;
			_tilePositionToCurrentTemperature[i].push_back(-274);
		}

		_tilePositionToTimeTOfGainDetermination[i].push_back(_spillNrToCorrespondingTimeT[correspondingSpillNr]);
		_tilePositionToCurrentGainValue[i].push_back(_tilePositionToSiPMGainMap[i]);
	}
}
void CalibrationMode_SiPMGain::createAndWriteGainAndTemperatureVsTimePlot()
{
	if(_tilePositionToTimeTOfGainDetermination.size() == 0)
		return;

	// get/create SubDirectory
	TDirectory *dir = this->getTDirectory("SiPMGain-OverviewPlots");

	// create canvas
	QString canvasTitle = "Gain vs. Time";
	TCanvas *can = new TCanvas(canvasTitle.toStdString().c_str(),canvasTitle.toStdString().c_str(),700,400);
	can->cd();

	//QString canvasTitle = "Gain vs. Time";
	//TCanvas *can = new TCanvas(canvasTitle.toStdString().c_str(),canvasTitle.toStdString().c_str(),700,400);
	//TPad *pad1 = new TPad("pad1","",0,0,1,1);
	//TPad *pad2 = new TPad("pad2","",0,0,1,1);
	//pad2->SetFillStyle(4000); //will be transparent
	//pad1->Draw();
	//pad1->cd();

	// draw the gain vs. time graphs for all channels
	TLegend legend(0.6,0.5,0.9,0.9,"Tile Position");
	legend.SetFillColor(0);
	string multiGraphTitle = QString("%1 - multiGraph name").arg(canvasTitle).toStdString();
	TMultiGraph *multiGraph = new TMultiGraph(multiGraphTitle.c_str(),multiGraphTitle.c_str());
	for(int i=0; i<(int)_tilePositionToTimeTOfGainDetermination.size(); i++)
	{
		// convert time_t from int to double for TGraph 
		vector <double> selectedTimeTtoDoubleVec;
		for(int j=0; j<(int)_tilePositionToTimeTOfGainDetermination[i].size(); j++)
			selectedTimeTtoDoubleVec.push_back((double)_tilePositionToTimeTOfGainDetermination[i].at(j));

		// create the gain vs. time graph of the current channel
		TGraph *gainVsTime = new TGraph(_tilePositionToTimeTOfGainDetermination[i].size(),
										&selectedTimeTtoDoubleVec.front(),
										&_tilePositionToCurrentGainValue[i].front() );
		QString gainVsTime_Title = QString("%1 - Tile Position: %2").arg(canvasTitle).arg(i);
		gainVsTime->SetTitle(gainVsTime_Title.toStdString().c_str());
		gainVsTime->SetName(gainVsTime_Title.toStdString().c_str());
		gainVsTime->SetLineColor(i+2);
		//if(gainVsTime->GetLineColor() == kWhite)
		//	gainVsTime->SetLineColor(i+30);
		gainVsTime->GetYaxis()->SetTitle("SiPM Gain [C]");
		gainVsTime->GetYaxis()->SetTitleOffset(1.25);
		gainVsTime->GetXaxis()->SetTitle("UTC Time");
		gainVsTime->GetXaxis()->SetTimeDisplay(1);
		//gainVsTime.GetXaxis()->LabelsOption("v");
		gainVsTime->SetMarkerColor(i+2);
		gainVsTime->SetMarkerStyle(23);
		gainVsTime->SetMarkerSize(0.5);

		multiGraph->Add(gainVsTime,"lp");

		legend.AddEntry(gainVsTime,QString("%1").arg(i).toStdString().c_str(),"lp");
	}
	multiGraph->Draw("a");		
	multiGraph->GetXaxis()->SetTimeDisplay(1);

	legend.Draw();
	dir->WriteTObject(can);


	// Todo: Temp vs. Time als einzelner plot bis ich es ins gleiche canvas plotten kann
	// http://root.cern.ch/root/html/tutorials/hist/transpad.C.html
	// create canvas
	QString canvasTempVsTime_Title = "Temperature vs. Time";
	TCanvas *canTempVsTime = new TCanvas(canvasTempVsTime_Title.toStdString().c_str(),canvasTempVsTime_Title.toStdString().c_str(),700,400);
	canTempVsTime->cd();

	//pad2->Draw();
	//pad2->cd();

	// draw the temperature vs. time graph in the same canvas
	QString timeVsTempSelected_Title = "during Data Analysis";
	int countWrittenGraphs = 0;
	TLegend leg(0.6,0.5,0.9,0.9,"Tile Position:");
	leg.SetFillColor(0);
	string multiGrTitle = QString("%1 - multiGraph name").arg(canvasTempVsTime_Title).toStdString();
	TMultiGraph *multiGr = new TMultiGraph(multiGraphTitle.c_str(),multiGraphTitle.c_str());
	for(int i=0; i<this->getNumberOfChannels(); i++)
	{
		TGraph *timeVsTempSelected = 
			this->createTemperatureVsTimePlot(timeVsTempSelected_Title,_spillNrToCorrespondingTimeT[0],_spillRead->getSpillInfo()->timeOfDataReady.toTime_t(), i);
		if(timeVsTempSelected)
		{
			timeVsTempSelected->SetLineColor(i + 2);
			timeVsTempSelected->SetMarkerColor(i + 2);
			timeVsTempSelected->SetMarkerStyle(23);
			timeVsTempSelected->SetMarkerSize(0.5);
			//pad2->Update();

			//// draw axis on the right side of the pad
			//TGaxis *axis = new TGaxis(8,-0.8,8,0.8,timeVsTempSelected_Title.toStdString().c_str(),50510,"+L");
			//axis->SetLabelColor(kBlue);
			//axis->Draw();

			multiGr->Add(timeVsTempSelected,"lp");

			leg.AddEntry(timeVsTempSelected,QString("%1").arg(i).toStdString().c_str(),"lp");
	
			countWrittenGraphs++;
		}
	}
	if(countWrittenGraphs)
	{
		multiGr->Draw("a");
		leg.Draw();
		dir->WriteTObject(canTempVsTime);
	}
}
void CalibrationMode_SiPMGain::createAndWriteGainVsTemperaturePlot()
{
	if(_tilePositionToCurrentTemperature.size() == 0)
		return;


	// get/create Directory
	TDirectory *dir = this->getTDirectory("SiPMGain-OverviewPlots");

	// create canvas
	QString canvasTitle = "Gain vs. Temperature";
	TCanvas *can = new TCanvas(canvasTitle.toStdString().c_str(),canvasTitle.toStdString().c_str(),700,400);
	can->cd();

	// draw the gain vs. temperature graphs for all channels
	TLegend leg(0.6,0.5,0.9,0.9,"Tile Position:");
	leg.SetFillColor(0);
	string multiGraphTitle = QString("%1 - multiGraph name").arg(canvasTitle).toStdString();
	TMultiGraph *multiGraph = new TMultiGraph(multiGraphTitle.c_str(),multiGraphTitle.c_str());
	for(int i=0; i<(int)_tilePositionToCurrentTemperature.size(); i++)
	{
		// create the gain vs. time graph of the current channel
		TGraph *gainVsTemperature = new TGraph(_tilePositionToCurrentTemperature[i].size(),
										&_tilePositionToCurrentTemperature[i].front(),
										&_tilePositionToCurrentGainValue[i].front() );
		QString gainVsTemperature_Title = QString("%1 - Tile Position: %2").arg(canvasTitle).arg(i);
		gainVsTemperature->SetTitle(gainVsTemperature_Title.toStdString().c_str());
		gainVsTemperature->SetName(gainVsTemperature_Title.toStdString().c_str());
		gainVsTemperature->SetLineColor(i+3);
		gainVsTemperature->GetYaxis()->SetTitle("SiPM Gain [C]");
		gainVsTemperature->GetYaxis()->SetTitleOffset(1.25);
		gainVsTemperature->GetXaxis()->SetTitle("Temperature [C]");
		gainVsTemperature->SetMarkerColor(i+3);
		gainVsTemperature->SetMarkerStyle(23);
		gainVsTemperature->SetMarkerSize(0.5);

		multiGraph->Add(gainVsTemperature,"lp");

		leg.AddEntry(gainVsTemperature,QString("%1").arg(i).toStdString().c_str(),"p");
	}
	multiGraph->Draw("a");
	leg.Draw();
	dir->WriteTObject(can);
}

TGraph *CalibrationMode_SiPMGain::createTemperatureVsTimePlot(QString title, int fromTimeT, int toTimeT, int tilePosition)
{
	// error handling
	if(toTimeT < fromTimeT)
		throw GeneralException("*** Error: given start time is later than the given end time!");

	vector <double> *selectedTemperatureVec = new vector<double>;
	vector <double> *selectedTimeTtoDoubleVec = new vector<double>;
	// use array info
	if(_temperatureData->temperatureArrayInfoExists())
	{
		vector <int> *timeTIntVec = _temperatureData->getAllAvailableInfo_TemperatureArray()->_timeTOfMeasurement;
		vector < QMap<int, double> > *temperatureVec =  _temperatureData->getAllAvailableInfo_TemperatureArray()->_tilePosition_To_currentTemperaturePreliminary;

		for(int i=0; i<(int)timeTIntVec->size(); i++)
		{
			if(fromTimeT <= timeTIntVec->at(i) && toTimeT >= timeTIntVec->at(i))
			{
				selectedTimeTtoDoubleVec->push_back((double)timeTIntVec->at(i));
				selectedTemperatureVec->push_back(temperatureVec->at(i)[tilePosition]);
			}
		}
	}// use standalone sensor info
	else if(!_temperatureData->temperatureArrayInfoExists() && _temperatureData->temperatureStandaloneSensorInfoExists())
	{
		vector <int> *timeTIntVec = _temperatureData->getAllAvailableInfo_StandaloneSensor()->_timeTOfMeasurement;
		vector <double> *temperatureVec =  _temperatureData->getAllAvailableInfo_StandaloneSensor()->_currentTemperature;

		for(int i=0; i<(int)timeTIntVec->size(); i++)
		{
			if(fromTimeT <= timeTIntVec->at(i) && toTimeT >= timeTIntVec->at(i))
			{
				selectedTimeTtoDoubleVec->push_back((double)timeTIntVec->at(i));
				selectedTemperatureVec->push_back(temperatureVec->at(i));
			}
		}
	}

	TGraph *timeVsTemp = NULL;
	if(selectedTimeTtoDoubleVec->size())
	{
		timeVsTemp = new TGraph(selectedTimeTtoDoubleVec->size(),
								&selectedTimeTtoDoubleVec->front(),
								&selectedTemperatureVec->front() );
		QString timeVsTemp_Title = QString("Time vs. Temperature %1 - Tile Position: %2").arg(title).arg(tilePosition);
		timeVsTemp->SetTitle(timeVsTemp_Title.toStdString().c_str());
		timeVsTemp->SetName(timeVsTemp_Title.toStdString().c_str());
		timeVsTemp->SetLineColor(2);
		timeVsTemp->GetYaxis()->SetTitle("Temperature [C]");
		timeVsTemp->GetXaxis()->SetTitle("UTC Time");
		timeVsTemp->GetXaxis()->SetTimeDisplay(1);
		//timeVsTemp->GetXaxis()->SetTimeFormat("20%y-%m-%d %H:%M:%S%F1970-01-01 00:00:00");
		//timeVsTemp->GetXaxis()->LabelsOption("v");
	}
	else
	{
		cerr<<"*** There is not temperature information available for the selected time range or the time window is too small!"<<endl;
		return NULL;
	}

	return timeVsTemp;
}

// text file with SiPM Gain values
void CalibrationMode_SiPMGain::initializeSiPMGainFile()
{
	QString filePath = _runRead->getRunDirectory();
	QDir calibDir(filePath);
	QString calibDirName = "Calibration";
	if(!calibDir.entryList(QDir::AllDirs).contains(calibDirName));
		calibDir.mkdir(calibDirName);
	calibDir.cd(calibDirName);
	_calibDirPath = calibDir.absolutePath();

	// create new subdirectory if the intermediate RM is analyzed
	QDir calibSubDir(calibDir.absolutePath());
	if(_analyzingIntermediateRM)
	{
		QString calibSubDirName = "IntermediateRM";
		if(!calibSubDir.entryList(QDir::AllDirs).contains(calibSubDirName))
			calibSubDir.mkdir(calibSubDirName);
		calibSubDir.cd(calibSubDirName);
		_calibDirPath = calibSubDir.absolutePath();
	}

	QString fileName = QString("SiPMGain-HumanReadible%1.T3B.part").arg(this->getRunAndSpillTitleSuffix());
	QString absoluteSiPMGainFilePath = QString("%1/%2").arg(calibDir.absolutePath()).arg(fileName);
	if(_analyzingIntermediateRM)
		absoluteSiPMGainFilePath = QString("%1/%2").arg(calibSubDir.absolutePath()).arg(fileName);

	_absoluteSiPMGainFilePath = absoluteSiPMGainFilePath;

	_outputFile = new QFile(absoluteSiPMGainFilePath);
	if (!_outputFile->open(QIODevice::WriteOnly | QIODevice::Text))
		throw GeneralException("The SiPMGain value file could not be opened");

	_output = new QTextStream( _outputFile );
	(*_output)<<"Spill:\tTilePos:  SiPMGain:\tTime:\t\t\tTime_t:\t\tTemperature:\t\tSiPMGainError:\n";
}
void CalibrationMode_SiPMGain::writeSiPMGainValueToCalibFile(int correspondingSpillNr)
{
	// error handling
	if(!_temperatureData)
		throw GeneralException("The Temperature access class was not initialized!");

	for(int i=0; i<_spillRead->getNrChannelsWithSiPMsAttached(); i++)
	{
//		(*_output)<<"Spill:\tTilePos:\tSiPMGain:\tTime:\tTime_t:\tTemperature:\n";
		QDateTime dt;
		dt.setTime_t(_spillNrToCorrespondingTimeT[correspondingSpillNr]);

		// well this is stupid - but fast
		double currTemperature = -274;
		if(_temperatureData->temperatureArrayInfoExists())
		{
			TemperatureSensor_Array_closestMeasurement *closestMeas = _temperatureData->getClosestTempMeasurement_TemperatureArray(_spillNrToCorrespondingTimeT[correspondingSpillNr]);
			if(closestMeas)
				currTemperature = closestMeas->_tilePosition_To_currentTemperaturePreliminary[i];
		}
		else if(!_temperatureData->temperatureArrayInfoExists() && _temperatureData->temperatureStandaloneSensorInfoExists())
		{
			currTemperature = _temperatureData->getClosestTempMeasurement_StandaloneSensor(_spillNrToCorrespondingTimeT[correspondingSpillNr]);
		}

		(*_output)<<QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
				.arg(correspondingSpillNr)
				.arg(i)
				.arg(_tilePositionToSiPMGainMap[i])
				.arg(dt.toString("yyyy-MM-ddThh:mm:ss"))
				.arg(_spillNrToCorrespondingTimeT[correspondingSpillNr])
				.arg(currTemperature)
				.arg(_tilePositionToSiPMGainErrorMap[i])
				;

	}
}
void CalibrationMode_SiPMGain::finalizeSiPMGainFile()
{
	_outputFile->close();

	// rename the output file -> remove the .part
	QString newOutputFileName = _absoluteSiPMGainFilePath.remove(".part");
	bool renamingSucceeded = _outputFile->rename(newOutputFileName);
	if(!renamingSucceeded)
	{
		QString errorMessage = QString("*** Warning: Renaming of File \"%1\" failed!").arg(_absoluteSiPMGainFilePath);
		cerr<<errorMessage.toStdString().c_str()<<endl;
		//throw GeneralException(errorMessage.toStdString().c_str());
	}
}

// destructor
CalibrationMode_SiPMGain::~CalibrationMode_SiPMGain(void)
{
}