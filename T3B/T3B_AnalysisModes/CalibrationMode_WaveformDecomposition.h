#pragma once

#include "AnalysisMode.h"

#include <QVariant>

#include "AccessTemperatureData.h"
#include "Serialization_WaveformDecomposition.h"
#include "Serialization_Averaged1pePeak.h"

#include "Exceptions.h"

#include "TTree.h"
#include "TNtuple.h"
#include "TDirectory.h"
#include "TGraph2D.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "TLegend.h"

using namespace std;

class CalibrationMode_WaveformDecomposition :	public AnalysisMode
{
public:
	CalibrationMode_WaveformDecomposition(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	// read in averaged 1pe peaks
	void readInSerializedAvaragingValues();
	void loadNewAveraged1peInfoIfNecessary();

	// take care of the dedicated channels (Scinti, CerA, CerB, Veto...)
	// trigger the dedicated channels with their respective trigger
	void triggerDedicatedChannels(vector <Waveform*> *actualWaveforms);

	// perform waveform substraction - return if substraction Process is Finished For this Event
	bool substract1peWfmAtMaxPosition(vector <Waveform*> *actualWaveforms);

	// control plots
	QMap<int,TH1F*> *initializePurifiedEdepHistogram(vector <Waveform*> *actualWaveforms, bool createStepwiseSubstractionHistogram = false);
	void fillPurifiedEdepHistogram();
	void writePurifiedEdepHistogram(vector <Waveform*> *actualWaveforms);

	// Event Serialization
	void initializeSerialization();
	void serializeEvent(vector <Waveform*> *actualWaveforms);
	void finalizeSerialization();

	// memory cleanup
	void cleanUpMemory();
	void resetVariables();

	~CalibrationMode_WaveformDecomposition(void);

private:
	QMap <int,TH1F *> *backupUnmodifiedWaveformsForRecreation(vector <Waveform*> *actualWaveforms);
	void recreateWaveformAndDetermineQualityFactor(QMap <int,TH1F *> *unmodifiedWaveforms,QMap <int,TH1F *> *purifiedEdepHistogram,bool writeRecreatedWaveform = false);

	void initializeMapOfBanLists();
	bool maxIndexMustBeBanned(int tilePosition,vector<float> *scaledSamplesOfFullWfm,int maximumIndexOfFullWfm);
	bool maxIndexInBanList(int tilePosition, int maximumIndex);

	void resetMapOfBanLists();
	void resetModificationStepsMap();
	int getMaximumIndexOfFullWfm_respectingBanList(int tilePosition, vector<float> *sampleVector);
	void performSmoothingOnWaveform(Waveform *actualWaveform);

	// helper function
	void writeDedicatedChannelWaveform(vector <Waveform*> *actualWaveforms);
	TH1F* replaceHistoByHistoRescaledToNS(TH1F *histo);

private:
	AccessTemperatureData *_temperatureData;
	QString _filePathTemperatureData;

	double _lowerThresholdOfWfmMax;
	double _demandedFractionOfAveraged1peMaximum;

	vector < QString > _dedicatedChannels_Names;
	vector< float > _dedicatedChannels_risingTriggerChannelThreshold;
	vector< float > _dedicatedChannels_fallingTriggerChannelThreshold;
	vector< float > _dedicatedChannels_preTriggerTimeNS;
	vector< float > _dedicatedChannels_postTriggerTimeNS;
	vector< float > _dedicatedChannels_MinimalSignalWidth;
	vector< bool > _dedicatedChannels_SignalInverted;
	vector< float > _dedicatedChannels_TypicalTriggerTime;
	vector< float > _dedicatedChannels_AcceptanceRangeAroundTypicalTriggerTime;
	// trigger for dedicatedChannels
	QMap < QString, SW_Trigger* > _dedicatedChannels_trigger;

	float _breakSubstractionIfAveraged1peWfmBelowThreshold;
	
	int _nrTotalEvents;
	int _EventNumber;
	int _EventNumberInCurrentSpill;
	int _nrAnalyzedEvents;
	int _nrChannelsWith1peAveragedWaveform;

	pair <QString,QString> _coincidenceScintiOnOsciChannel;

	Serialization_Averaged1pePeak *_serializationAveraged1peInput;
	int _averaged1peWfmsAvailableEveryNrSteps;
	int _currentAveragedInfoCorrespondsToSpillNr;
	QMap<int,int> _tilePositionToNrSummedWaveformsForAveraging;
	QMap<int,vector<float> > _tilePositionToSummedSamplesMap;
	QMap<int,vector<float> > _tilePositionToTimingOfSamplesMap;
	QMap<int,pair<int,float> > _tilePositionToMaximumIndexAndMaximumValueMap;
	QMap<int,pair<int,int> > _tilePositionToMaxEnclosingFWHMIndices;

	QMap<int,QList<int> *> _tilePositionToIndexBanList;
	QMap<int,int> _tilePositionToNrModificationSteps;
	QMap<int,TH1F*> _tilePositionToEdepHistogramMap;
	QMap<int,TH2F*> _tilePositionToEdep2DHistogramMap;
	bool _createStepwiseSubstractionHistogram;
	QMap <int,TH1F *> _tilePositionToUnmodifiedWaveformBackup;

	TNtuple *_generalEventInfo;
	QMap <int, vector<float> > _tilePositionToTimeOfEdep;
	QMap <int, double> _tilePositionToCurrentChiSquare;

	Serialization_WaveformDecomposition *_serializationObject;

	pair<QString, QString> _oldAndNewNameOfScintiChannel;

	TH1D *_chi2Distribution;
};
