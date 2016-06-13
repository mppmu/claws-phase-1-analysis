#pragma once

#include "AnalysisMode.h"
#include "AccessSiPMGainData.h"

#include "Exceptions.h"
#include "Serialization_Averaged1pePeak.h"

#include "TTree.h"
#include "TNtuple.h"
#include "TDirectory.h"

using namespace std;

class CalibrationMode_Averaged1pePeak :	public AnalysisMode
{
public:
	CalibrationMode_Averaged1pePeak(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	// a local selection filter
	void selectPure1pePeaks(vector <Waveform*> *actualWaveforms);

	~CalibrationMode_Averaged1pePeak(void);

private:
	// correct for cable reflections by fitting the falling edge
	void correctAveragedWaveformForReflections(int spillNumberCorrespondingToAveragedInfo);

	// Boost Serialization
	void initializeSerialization();
	void serializeSpill(int spillNumberCorrespondingToAveragedInfo);
	void finalizeSerialization();

	// helper functions
	float determineTypicalTriggerTime();
	void setWfmIntegralLowerAndUpperCut();

private:
	QMap<int,pair<double,double> > _tilePos_to_wfmIntegralLowerAndUpperCut;
	float _OnePEThreshold;
	float _typicalTriggerTime;
	float _acceptanceRangeAroundTypicalTriggerTime;
	int _timeToleranceMinutesForSiPMGainDetermination;
	int _averageOverNrSpills;
	double _fit1peWaveformFallingEdgeFromTimeNS;

	QMap<int, vector<float> > _tilePosition_To_AveragedSamplesMap;
	QMap<int, vector<float> > _tilePosition_To_TimingOfSamplesMap;

	AccessSiPMGainData *_SiPMGainData;

	Serialization_Averaged1pePeak *_serializationObject;
};
