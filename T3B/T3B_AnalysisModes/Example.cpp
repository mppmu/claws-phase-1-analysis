#include "Example.h"

Example::Example(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	_plotPool->initializeHistogramsForIntegralDistribution();
	//_plotPool->initializeTimeOfFirstHit();
	//_plotPool->initializeTimeOfHit();
	//_plotPool->initializeScintillatorCoincidencePlots();
	//_plotPool->initializeWfmWidthPlots();
	//_plotPool->initializeEdepVsTimeHist();
	_plotPool->initializeTotalWfmIntegralPlots();

	this->startAnalysis();
}

void Example::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "Example")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	//// path to the temperature data
	//_filePathTemperatureData = __globalSettingsManager->getSetting("rm_filePathTemperatureData").toString();
}

void Example::initializeAnalysisModeVariables()
{
	//cout<<"Initializing Temperature Measurement Class"<<endl;
	//_temperatureData = new AccessTemperatureData(_filePathTemperatureData);
	//cout<<"Temperature Information is now available"<<endl;
}

void Example::startAnalysis()
{
	int timeOfDataAnalysisStart = 0;
	int timeOfDataAnalysisEnd = 2147483647; // maximum int value

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
		//// fill scintillator coincidence plots
		//_plotPool->fillScintillatorCoincidencePlots();
//
//		Filter_ScintillatorCoincidence* coinFilter = dynamic_cast<Filter_ScintillatorCoincidence*>(_selectedFilterMap[ "ScintillatorCoincidence" ]);
//		for(int i=0; i<coinFilter->getTimeOfScintiCoincidences()->size(); i++)
//		{
//			float timeOfScintiCoin = coinFilter->getTimeOfScintiCoincidences()->at(i);
//			if(timeOfScintiCoin < 480*1E-9 && timeOfScintiCoin > 365*1E-9)
//			{
////if(coinFilter->getNrScintCoinInEvent() > 1)
//			_plotPool->writeTriggeredWaveforms(actualWaveforms);
//			}
//		}
		if(!somethingWasTriggered)
			continue;











		// save the first 50 waveforms
		if(nrAnalyzedEvents<50)
		{
			// save the triggered waveforms of all _enabledOsciChannels in several canvases
			_plotPool->writeTriggeredWaveforms(actualWaveforms);
		}

		// fill integral distribution of all triggered waveforms
		_plotPool->fillIntegralDistribution(actualWaveforms);
		//// fill time of first hit
		//_plotPool->fillTimeOfFirstHit(actualWaveforms);
		//// fill time of hit
		//_plotPool->fillTimeOfHit(actualWaveforms);
		//// fill WfmWidth Plots
		//_plotPool->fillWfmWidthPlots(actualWaveforms);
		//// fill Energy Deposition vs. Time Histogramm
		//_plotPool->fillEdepVsTimeHist(actualWaveforms);
		// fill Total Waveform Integral Plots
		_plotPool->fillTotalWfmIntegralPlots(actualWaveforms);

		if((nrAnalyzedEvents+1)%10 == 0) 
			cout<<"Analyzing Waveform Number: "<<nrAnalyzedEvents+1
			<<"\tin Run_"<<_spillRead->getSpillInfo()->runNumber
			<<" Spill_"<<_spillRead->getSpillInfo()->spillNumber
			<<" nrTotalEvents_"<<nrTotalEvents
			<<endl;

		if(nrAnalyzedEvents == 0)
			timeOfDataAnalysisStart = this->getCurrentSpillRead()->getSpillInfo()->timeOfDataReady.toTime_t();

		int timeOfDataAnalysis = this->getCurrentSpillRead()->getSpillInfo()->timeOfDataReady.toTime_t();

		nrAnalyzedEvents++;
	}
	timeOfDataAnalysisEnd = this->getCurrentSpillRead()->getSpillInfo()->timeOfDataReady.toTime_t();
//	// create and write the temperature data for the analysis period
//	_plotPool->createAndWriteGeneralTemperaturePlots(_temperatureData,timeOfDataAnalysisStart,timeOfDataAnalysisEnd);

	_plotPool->writeIntegralDistribution();
	//_plotPool->writeTimeOfFirstHit();
	//_plotPool->writeTimeOfHit();
	//_plotPool->writeWfmWidthPlots();
	//_plotPool->writeEdepVsTimeHist();
	//_plotPool->writeScintillatorCoincidencePlots();
	_plotPool->writeTotalWfmIntegralPlots();

	this->moveRootFileToCalibFolder();
}
// destructor
Example::~Example(void)
{
}
