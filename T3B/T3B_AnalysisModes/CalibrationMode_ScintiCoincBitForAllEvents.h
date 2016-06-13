#pragma once
#include "AnalysisMode.h"

#include "Exceptions.h"

#include "Serialization_ScintillatorCoincidenceBit.h"

#include "SW_AsyncMultiRisingAndFallingEdgeTrigger.h"

using namespace std;

class CalibrationMode_ScintiCoincBitForAllEvents : public AnalysisMode
{
public:
	CalibrationMode_ScintiCoincBitForAllEvents(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	// Boost Serialization
	void initializeSerialization();
	void serializeScintillatorCoincidenceInfo(Waveform* currentWaveform);
	void finalizeSerialization();

	Waveform* triggerDedicatedChannels(vector <Waveform*> *actualWaveforms);

	~CalibrationMode_ScintiCoincBitForAllEvents(void);
private:
	Serialization_ScintillatorCoincidenceBit *_serializationObject;

	int _EventNumber;
	QString _dedicatedChannel;
	SW_AsyncMultiRisingAndFallingEdgeTrigger *_scintiTrigger;
	double _typicalTriggerTime;
	double _acceptanceRangeAroundTypicalTriggerTime;

	pair<QString, QString> _oldAndNewNameOfScintiChannel;
};
