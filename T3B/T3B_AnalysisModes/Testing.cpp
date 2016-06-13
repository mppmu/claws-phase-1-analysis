#include "Testing.h"

Testing::Testing(void)
{
	this->loadRMSpecificAnalysisSettings();
	this->initializeAnalysisModeVariables();

	this->startAnalysis();
}

void Testing::loadRMSpecificAnalysisSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "Testing")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	// path to the temperature data
	_filePathTemperatureData = __globalSettingsManager->getSetting("rm_filePathTemperatureData").toString();
}

void Testing::initializeAnalysisModeVariables()
{
	cout<<"Initializing Temperature Measurement Class"<<endl;
	_temperatureData = new AccessTemperatureData(_filePathTemperatureData);
	cout<<"Temperature Information is now available"<<endl;
}

void Testing::startAnalysis()
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
		if(nrAnalyzedEvents<10)
		{
			// save the triggered waveforms of all _enabledOsciChannels in several canvases
			_plotPool->writeTriggeredWaveforms(actualWaveforms);
		}


		// in case we want to see the scintillator coincidence waveform for the first 10 analyzed waveforms
		pair <QString, QString> scintiCoin_OsciChannel("AY166/047","ChannelD");
		if(nrAnalyzedEvents < 10) 
			_plotPool->writeScintillatorCoincidenceWaveform(&actualWaveforms,scintiCoin_OsciChannel);



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

	// move root file to calibration folder
	this->moveRootFileToCalibFolder();
}
// destructor
Testing::~Testing(void)
{
	delete _temperatureData;
}
