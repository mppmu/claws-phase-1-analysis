#pragma once
#include "AnalysisMode.h"

#include "AccessTemperatureData.h"

#include "Exceptions.h"

using namespace std;

class Testing : public AnalysisMode
{
public:
	Testing(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	~Testing(void);

private:
	AccessTemperatureData *_temperatureData;
	QString _filePathTemperatureData;

	int _timeOfRunStart;
	int _timeOfRunEnd;
};
