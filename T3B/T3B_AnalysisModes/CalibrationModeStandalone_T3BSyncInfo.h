#pragma once
//#include "AnalysisMode.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QFile>
#include <QTextStream>

#include "Exceptions.h"

#include "SettingsManager.h"

#include "SpillRead.h"
#include "RunRead.h"

#include "Serialization_SpillSyncInfo.h"

using namespace std;

class CalibrationModeStandalone_T3BSyncInfo //: public AnalysisMode
{
public:
	CalibrationModeStandalone_T3BSyncInfo(void);

	// run mode functions
	void loadSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	~CalibrationModeStandalone_T3BSyncInfo(void);

private:
	void loadRun();
	bool spillContainsData();

	int getNrT3BCoincidenceEvents();

	// helper functions
	bool switchToIntermediateSpillOrLoadNextSpillIfNotExistent();
	QString getPathToCalibDir();
	bool lastSpillThatShouldBeAnalyzedReached();

	// Boost Serialization
	void initializeSerialization();
	void serializeSpillSyncInfo();
	void finalizeSerialization();

	// write sync info in a human readible file format
	void initializeSpillSyncInfoHumanReadible();
	void writeSpillSyncInfoHumanReadible();
	void finalizeSpillSyncInfoHumanReadible();

private:
	QString _dataPath;
	int _runNumber;
	bool _analyzingIntermediateRM;
	RunRead *_runRead;
	SpillRead *_spillRead;

	QTextStream *_output;
	QFile *_outputFile;
	QString _pathToCalibDir;
	pair<int,int> _minAndMaxSpillToAnalyze;

	QMap<QString,QVariant> _spillSyncInfo;

	Serialization_SpillSyncInfo *_serializationObject;

	pair <QString,QString> _coincidenceScintiOnOsciChannel;
	bool _coincidenceScintiChannelWasSpecified;
	double _thresholdOnCoincScintiMV;
	bool _ScintillatorCoincSignalInverted;
};
