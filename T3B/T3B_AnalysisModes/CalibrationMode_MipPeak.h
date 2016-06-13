/*
***************************************************************************
This class is obsolete for the calibration, but working!
***************************************************************************

This class was originally used for the Sr90 Calibration.
The purpose was to fit and extract the MipPeak information.

As we run on the raw data here, a new additional class was created that runs on the waveformdecomposed data to have perfect comparability for the
calibration to the MIP Scale, which also runs on the decomposed data.
*/
#pragma once
#include "AnalysisMode.h"

#include "AccessTemperatureData.h"

#include "Serialization_MipPeak.h"

#include "Exceptions.h"

using namespace std;

class CalibrationMode_MipPeak : public AnalysisMode
{
public:
	CalibrationMode_MipPeak(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	~CalibrationMode_MipPeak(void);
protected:
	// integral plot
	void initializeIntegralDistributionsForMIPPeakDetermination();
	void fillIntegralDistributionsForMIPPeakDetermination(vector <Waveform*> &actualWaveforms);
	void fitAndWriteIntegralDistribution();

	// create human readible MipPeak file
	void initializeMipPeakFile();
	void writeMipPeakValueToCalibFile();
	void finalizeMipPeakFile();

	// Boost Serialization
	void initializeSerialization();
	void serializeMipPeakInfo();
	void finalizeSerialization();

private:
	// helper
	double determineMeanTemperatureDuringDataTaking();

private:
	AccessTemperatureData *_temperatureData;
	QString _filePathTemperatureData;

	int _timeOfRunStart;
	int _timeOfRunEnd;

	QMap <int,TH1D*> _tilePosition_to_WfmIntegralDistribution;
	QMap <int,pair<double,double> > _tilePosition_to_MipPeakAndMipPeakError;
	QMap <int,pair<double,int> > _tilePosition_to_ChiSquareAndNDFofFit;
	QMap <int,double> _tilePosition_to_AppliedBiasVoltage;

	QString _absoluteMipPeakFilePath;
	QTextStream *_output;
	QFile *_outputFile;

	Serialization_MipPeak *_serializationObject;
};
