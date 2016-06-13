/*
Todo:
Example class
*/

#pragma once
#include "AnalysisMode.h"

#include "AccessTemperatureData.h"

#include "Exceptions.h"

using namespace std;

class Example : public AnalysisMode
{
public:
	Example(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	~Example(void);
private:
	AccessTemperatureData *_temperatureData;
	QString _filePathTemperatureData;
};
