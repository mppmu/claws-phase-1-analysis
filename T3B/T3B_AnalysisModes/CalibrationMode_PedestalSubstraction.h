#pragma once
#include "AnalysisMode.h"

#include "TH1F.h"

#include "Exceptions.h"

#include <QTextStream>

using namespace std;

/*
The Pedestal Value is determined spillwise applying the following algorithm:
All voltage values of all waveforms of a channel are filled into one histogramm.
If a bin of that histogram has more than "_binAcceptanceFractionOfTotalHistEntries" perCent of the total number of entries it is accepted for
the determination of the Pedestal Value. An averaging is performed for the selected bins.

The Pedestal Values are saved spillwise in the file: $RunFolder/Calibration/Calibration-PedestalSubstraction.txt
*/
class CalibrationMode_PedestalSubstraction : public AnalysisMode
{
public:
	CalibrationMode_PedestalSubstraction(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	// plots
	void initializeDistributionOfOsciValues();
	void writeAndClearDistributionOfOsciValues(QString titleSuffix);

	void fillDistributionAndMapOfOsciValues(vector <Waveform*> &actualWaveforms);

	void initializeMapOfOsciValues();
	void clearMapOfOsciValues();

	// create pedestal substraction file
	void initializePedestalSubstractionFile();
	vector <float> determinePedestalSubstractionValue();
	void writePedestalSubstractionValueToCalibFile(vector <float> &pedSubValueVec, int spillNumber);
	void finalizePedestalSubstractionFile();

	~CalibrationMode_PedestalSubstraction(void);
protected:
	QString _calibDirPath;
	QString _absolutePedSubFilePath;

	int _2ToThePowerOfNrBitsOfPS6000;
	//float _binAcceptanceFractionOfTotalHistEntries;
	//float _binAcceptanceMaxThresholdV;
	//int _tilePositionOfScintillatorCoincidenceChannel;

	vector<TH1F*> _distributionOfOsciValuesVec;
	vector<QMap <float, int> > _distributionOfOsciValuesMap;

	int _lastSpillNumber;
	int _lastRunNumber;

	bool _isFirstAnalyzedSpill;

	QTextStream *_output;
	QFile *_outputFile;
	TDirectory *_tDirForPedSubValues;
	TNtuple *_tNTupleForPedSubValues;
};
