#include "CalibrationMode_PedestalSubstraction.h"

CalibrationMode_PedestalSubstraction::CalibrationMode_PedestalSubstraction(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->initializeDistributionOfOsciValues();
	this->initializeMapOfOsciValues();
	this->initializePedestalSubstractionFile();

	this->startAnalysis();
}

void CalibrationMode_PedestalSubstraction::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationPedestalSubstractionIRM" && 
	   __globalSettingsManager->getSelectedAnalysisMode() != "CalibrationPedestalSubstractionPhysics")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");
}

void CalibrationMode_PedestalSubstraction::initializeAnalysisModeVariables()
{
	_2ToThePowerOfNrBitsOfPS6000 = 256;
	//_binAcceptanceFractionOfTotalHistEntries = 0.01;
	//_binAcceptanceMaxThresholdV = 0.05;

	_lastSpillNumber = -1;
	_lastRunNumber = this->getCurrentSpillRead()->getSpillInfo()->runNumber;
	
	_isFirstAnalyzedSpill = true;
}

void CalibrationMode_PedestalSubstraction::startAnalysis()
{
	int nrAnalyzedEvents = 0;
	int nrTotalEvents = 0;
	bool eventLoopWasAccessed = false;
	while(!this->endOfDataReached() && nrAnalyzedEvents < _nrEventsToAnalyze)
	{
		eventLoopWasAccessed = true;

		// load the next waveform and get all current waveforms
		vector <Waveform*> actualWaveforms;
		actualWaveforms = this->loadNextWfmOnAllChannels();

		nrTotalEvents++;

		// trigger the waveforms
		this->triggerWaveformsWithSelectedTrigger(&actualWaveforms);




//		// save the first 50 waveforms
//		if(nrAnalyzedEvents<50)
//		{
//			for(int i=0; i<actualWaveforms.size(); i++)
//			{
////			int i=1;
//				float totalwfmint = actualWaveforms.at(i)->getTotalWaveformIntegral();
//				QString titleprefix = QString("*******%1 ").arg(totalwfmint);
//				pair <int,int> totalsamplewindow(0,actualWaveforms.at(i)->getScaledSamples()->size());
//				_plotPool->writeCanvasWithGraphOfWaveform(actualWaveforms.at(i),totalsamplewindow,titleprefix.toStdString().c_str());
//			}
//		}

		// save the first 50 waveforms
		if(nrAnalyzedEvents<50)
		{
			// save the triggered waveforms of all _enabledOsciChannels in several canvases
			_plotPool->writeTriggeredWaveforms(actualWaveforms);
		}

		// fill plots with data
		this->fillDistributionAndMapOfOsciValues(actualWaveforms);

		if((nrAnalyzedEvents+1)%100 == 0) 
			cout<<"Analyzing Waveform Number: "<<nrAnalyzedEvents+1
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<nrTotalEvents
			<<endl;

		nrAnalyzedEvents++;
	}

	if(eventLoopWasAccessed)
	{
		// todo: das kann man eleganter machen um den letzten spill auch zu bekommen
		vector <float> pedSubValueVec = this->determinePedestalSubstractionValue();
		int currentSpillNumber = this->getCurrentSpillRead()->getSpillInfo()->spillNumber;
		this->writePedestalSubstractionValueToCalibFile(pedSubValueVec, currentSpillNumber);
	}

	this->finalizePedestalSubstractionFile();

	// move the TFile
	this->moveRootFileToCalibFolder();
}

// distribution and map of possible osci values
void CalibrationMode_PedestalSubstraction::initializeMapOfOsciValues()
{
	int nrChannelsWithSiPMs = this->getNumberOfChannels_withSiPMsAttached();
	for(int i=0; i<nrChannelsWithSiPMs; i++)
	{
		// Map
		QMap<float,int> tmpMap;
		_distributionOfOsciValuesMap.push_back(tmpMap);
	}
}
void CalibrationMode_PedestalSubstraction::initializeDistributionOfOsciValues()
{
	int nrChannelsWithSiPMs = this->getNumberOfChannels_withSiPMsAttached();
	for(int i=0; i<nrChannelsWithSiPMs; i++)
	{
		// TH1F
		QString distributionOfOsciValuesTitle = 
			QString("Calibration: Pedestal Subtraction - Distribution of possible Oscilloscope Values - Tile Position %1").arg(i);
		double yOffset = _allChannelDataVec->at(i)->getYOffset();
		double yRange = _allChannelDataVec->at(i)->getYRange() * 2; // todo: der yRange ist so nicht korrekt!
		double verticalRangeFraction = (fabs(yOffset) + yRange/2) / yRange;

		TH1F *distributionOfOsciValues;
		if(yOffset < 0)
			distributionOfOsciValues = new TH1F(distributionOfOsciValuesTitle.toStdString().c_str(),
												  distributionOfOsciValuesTitle.toStdString().c_str(),
												  _2ToThePowerOfNrBitsOfPS6000,
												  (-1) * yRange * (1-verticalRangeFraction),
												 yRange * verticalRangeFraction);
		else
			distributionOfOsciValues = new TH1F(distributionOfOsciValuesTitle.toStdString().c_str(),
												  distributionOfOsciValuesTitle.toStdString().c_str(),
												  _2ToThePowerOfNrBitsOfPS6000,
												  (-1) * yRange * verticalRangeFraction,
												  yRange * (1-verticalRangeFraction));
		distributionOfOsciValues->GetXaxis()->SetTitle("Possible Osciloscope Values [V]");
		distributionOfOsciValues->GetYaxis()->SetTitle("#");
		distributionOfOsciValues->GetYaxis()->SetTitleOffset(1.2);

		_distributionOfOsciValuesVec.push_back(distributionOfOsciValues);
	}
}
// text file with pedestal substraction values
void CalibrationMode_PedestalSubstraction::initializePedestalSubstractionFile()
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

	// create the txt file ///////////////////////////////////
//	QString fileName = QString("%1.T3B").arg(__globalSettingsManager->getSetting("general_outputFileName").toString());
	QString fileName = QString("PedestalSubstraction-HumanReadible%1.T3B.part").arg(this->getRunAndSpillTitleSuffix());
	QString absolutePedSubFilePath = QString("%1/%2").arg(calibDir.absolutePath()).arg(fileName);
	if(_analyzingIntermediateRM)
		absolutePedSubFilePath = QString("%1/%2").arg(calibSubDir.absolutePath()).arg(fileName);

	_absolutePedSubFilePath = absolutePedSubFilePath;

	_outputFile = new QFile(absolutePedSubFilePath);
	if (!_outputFile->open(QIODevice::WriteOnly | QIODevice::Text))
		throw GeneralException("The Pedestal Substraction value file could not be opened");

	_output = new QTextStream( _outputFile );
	//(*_output)<<"Spill: \t Osci: \t Channel: \t TilePos: \t PedSubValue: \n";
	(*_output)<<"Spill:\tTilePos:\tPedSubValue:\n";



	// create the tuple ///////////////////////////////////
	// create or get directory to write to
	QString dirTitle = "Pedestal Substraction Values - Tuples";
	_tDirForPedSubValues = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());

	QString tupleTitle = "Pedestal Substraction Values";
	_tNTupleForPedSubValues = new TNtuple(tupleTitle.toStdString().c_str(),tupleTitle.toStdString().c_str(),"Spill:TilePos:PedV");
}
void CalibrationMode_PedestalSubstraction::fillDistributionAndMapOfOsciValues(vector <Waveform*> &actualWaveforms)
{
	// check the current run number
	int currentRunNumber = this->getCurrentSpillRead()->getSpillInfo()->runNumber;
	if(currentRunNumber != _lastRunNumber)
		throw GeneralException("*** Error: Two run numbers used->not yet implemented!");

	// if a new spill is loaded -> save the histograms and clear them afterwards
	int currentSpillNumber = this->getCurrentSpillRead()->getSpillInfo()->spillNumber;
	if(currentSpillNumber != _lastSpillNumber && !_isFirstAnalyzedSpill)
	{
		QString lastSpillNumber_str = QString("Spill %1").arg(_lastSpillNumber);

		vector <float> pedSubValueVec = this->determinePedestalSubstractionValue();
		this->writePedestalSubstractionValueToCalibFile(pedSubValueVec, _lastSpillNumber);

		// TH1F
		if(_distributionOfOsciValuesVec.size()) this->writeAndClearDistributionOfOsciValues(lastSpillNumber_str);

		// Map
		this->clearMapOfOsciValues();

		_lastSpillNumber = currentSpillNumber;
	}
	if(_isFirstAnalyzedSpill)
		_lastSpillNumber = currentSpillNumber;
	_isFirstAnalyzedSpill = false;

	// fill the histogramms and the map
	for(int i=0; i < (int)actualWaveforms.size(); i++)
	{
		if(actualWaveforms.at(i)->isWfmOfDedicatedChannel())
			continue;

		vector< pair <int, int> > *triggeredMinMaxSampleWindows = actualWaveforms[i]->getTriggeredMinMaxSampleWindows();
		int tilePosition =  actualWaveforms[i]->getTilePosition();
		int nrSamples = (int)actualWaveforms[i]->getScaledSamples()->size();
		for(int j=0; j<nrSamples; j++)
		{
			// veto all sample windows in which something was triggered
			for(int k=0; k<(int)triggeredMinMaxSampleWindows->size(); k++)
			{
				if(j >= triggeredMinMaxSampleWindows->at(k).first && j <= triggeredMinMaxSampleWindows->at(k).second)
					j = triggeredMinMaxSampleWindows->at(k).second + 1;
			}
			if(j >= nrSamples)
				break;

			float currentScaledSample = actualWaveforms[i]->getScaledSamples()->at(j);
			//if(tilePosition == _tilePositionOfScintillatorCoincidenceChannel)
			//	currentScaledSample = 0;

			//// Start temporary control plot
			//if(currentScaledSample > 0.004)
			//{
			//	_plotPool->writeTriggeredWaveforms(actualWaveforms);

			//	QString titlePrefix = QString("*******%1 ").arg(currentScaledSample);
			//	pair <int,int> totalSampleWindow(0,actualWaveforms.at(i)->getScaledSamples()->size());
			//	_plotPool->writeCanvasWithGraphOfWaveform(actualWaveforms.at(i),totalSampleWindow,titlePrefix.toStdString().c_str());
			//}
			//// End temporary control plot

			// TH1F
			if(_distributionOfOsciValuesVec.size()) _distributionOfOsciValuesVec.at(tilePosition)->Fill(currentScaledSample);

			// Map
			if(!_distributionOfOsciValuesMap.at(tilePosition).contains(currentScaledSample))
				_distributionOfOsciValuesMap.at(tilePosition)[currentScaledSample] = 0;
			_distributionOfOsciValuesMap.at(tilePosition)[currentScaledSample] += 1;
		}
	}
}
void CalibrationMode_PedestalSubstraction::clearMapOfOsciValues()
{
	// clear Map
	for(int i=0; i < (int)_distributionOfOsciValuesMap.size(); i++)
		_distributionOfOsciValuesMap.at(i).clear();
	_distributionOfOsciValuesMap.clear();

	this->initializeMapOfOsciValues();
}
void CalibrationMode_PedestalSubstraction::writeAndClearDistributionOfOsciValues(QString titleSuffix)
{
	// create directory if not existent
	QString dirTitle = "Pedestal Substraction Distribution";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());

	// create directory if not existent
	TDirectory *subDir = _tFile->GetDirectory(titleSuffix.toStdString().c_str());
	if(!subDir)
		subDir = dir->mkdir(titleSuffix.toStdString().c_str(),titleSuffix.toStdString().c_str());

	// write distributions to file
	for(int i=0; i < (int)_distributionOfOsciValuesVec.size(); i++)
	{
		QString title = QString("%1 %2").arg(_distributionOfOsciValuesVec.at(i)->GetTitle()).arg(titleSuffix);
		_distributionOfOsciValuesVec.at(i)->SetTitle(title.toStdString().c_str());
		subDir->WriteTObject(_distributionOfOsciValuesVec.at(i));

		delete _distributionOfOsciValuesVec.at(i);
	}

	// clear TH1F
	_distributionOfOsciValuesVec.clear();

	// create new histograms to fill
	this->initializeDistributionOfOsciValues();
}
// determination of pedestal substraction value
vector <float> CalibrationMode_PedestalSubstraction::determinePedestalSubstractionValue()
{
	////////////////// Bestimmung mit (leicht fehlerhaftem) TH1F/////////////
	//vector <float> pedSubVec;
	//for(int i=0; i < (int)_distributionOfOsciValuesVec.size(); i++)
	//{
	//	int nrTotalHistEntries = _distributionOfOsciValuesVec.at(i)->GetEntries();
	//	int nrEntriesToAverage = 0;
	//	float sumWeightedBin = 0;
	//	for(int j=0; j < (int)_distributionOfOsciValuesVec.at(i)->GetNbinsX(); j++)
	//	{
	//		int binContent = _distributionOfOsciValuesVec.at(i)->GetBinContent(j);
	//		double binLowEdge = _distributionOfOsciValuesVec.at(i)->GetBinLowEdge(j); // Todo: binCenter ist falsch
	//		if(binContent > nrTotalHistEntries*_binAcceptanceFractionOfTotalHistEntries)
	//		{
	//			nrEntriesToAverage += binContent;
	//
	//			sumWeightedBin += binContent*binLowEdge;
	//		}
	//	}
	//	float pedSubValue = sumWeightedBin/nrEntriesToAverage;
	//	pedSubVec.push_back(pedSubValue);
	//}
	//return pedSubVec;

	////////////////// Bestimmung mit QMap/////////////
	vector <float> pedSubVec;
	for(int i=0; i < (int)_distributionOfOsciValuesMap.size(); i++)
	{
		int nrTotalHistEntries = 0;
		foreach (int value, _distributionOfOsciValuesMap.at(i))
			nrTotalHistEntries += value;
		int nrEntriesToAverage = 0;
		float sumWeightedBin = 0;
		QMapIterator<float, int> it(_distributionOfOsciValuesMap.at(i));
		while (it.hasNext()) 
		{
			it.next();

			int binContent = it.value();
			float binValue = it.key(); 
			//if(binContent > nrTotalHistEntries*_binAcceptanceFractionOfTotalHistEntries && binValue < _binAcceptanceMaxThresholdV)
			//{
				nrEntriesToAverage += binContent;
	
				sumWeightedBin += binContent*binValue;
			//}
		}
		float pedSubValue = sumWeightedBin/nrEntriesToAverage;
		pedSubVec.push_back(pedSubValue);
	}

	return pedSubVec;
}
void CalibrationMode_PedestalSubstraction::writePedestalSubstractionValueToCalibFile(vector <float> &pedSubValueVec, int spillNumber)
{
	// root file ///////////////////////////////////
	// fill the tuple
	for(int j=0; j<(int)pedSubValueVec.size(); j++)
	{
		_tNTupleForPedSubValues->Fill(spillNumber,j,pedSubValueVec.at(j));
	}

	// text file ///////////////////////////////////
	for(int i=0; i<(int)pedSubValueVec.size(); i++)
	{
		(*_output)<<QString("%1\t%2\t%3\n").arg(spillNumber).arg(i).arg(pedSubValueVec.at(i));
	}
}

void CalibrationMode_PedestalSubstraction::finalizePedestalSubstractionFile()
{
	// close the text file
	_outputFile->close();

	// rename the output file -> remove the .part
	QString newOutputFileName = _absolutePedSubFilePath.remove(".part");
	bool renamingSucceeded = _outputFile->rename(newOutputFileName);
	if(!renamingSucceeded)
	{
		QString errorMessage = QString("*** Warning: Renaming of File \"%1\" failed!").arg(_absolutePedSubFilePath);
		cerr<<errorMessage.toStdString().c_str()<<endl;
		//throw GeneralException(errorMessage.toStdString().c_str());
	}



	// write the filled tuple to the root file
	_tDirForPedSubValues->WriteTObject(_tNTupleForPedSubValues);
}
// destructor
CalibrationMode_PedestalSubstraction::~CalibrationMode_PedestalSubstraction(void)
{
}
