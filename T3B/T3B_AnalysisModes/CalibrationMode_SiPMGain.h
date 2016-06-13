#pragma once
#include "AnalysisMode.h"
#include "AccessTemperatureData.h"

#include "TH1F.h"
#include "TGaxis.h"
#include "TSpectrum.h"

#include "Exceptions.h"

#include <QTextStream>
#include <QDateTime>

using namespace std;

/*
*/
class CalibrationMode_SiPMGain : public AnalysisMode
{
public:
	CalibrationMode_SiPMGain(void);

	// run mode functions
	void loadRMSpecificAnalysisSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	// plots
	void initializeIntegralDistributionsForSiPMGainDetermination();
	void fillIntegralDistributionsForSiPMGainDetermination(vector <Waveform*> &actualWaveforms);
	void writeAndRenewIntegralDistributionsMap();
	QMap<int , TH1D *> createHistogramsForAllChannelsForIntegralDistribution(int correspondingSpillNr);

	void determineSiPMGain(int correspondingSpillNr,QMap<int , TH1D *> &tilepositionToIntegralDistribution);
	void writeHistogramOfAllSiPMGainValues(TDirectory *saveDirectory);
	void writeCanvasesOfFittedIntegralDistributionsOfAllChannels(TDirectory *saveDirectory);
	void writeIntegralDistributionsOfAllChannels(TDirectory *saveDirectory,int correspondingSpillNr,QMap<int , TH1D *> &tilepositionToIntegralDistribution);

	void logGainTimeAndTemperature(int correspondingSpillNr);
	void createAndWriteGainAndTemperatureVsTimePlot();
	TGraph *createTemperatureVsTimePlot(QString title, int fromTimeT, int toTimeT, int tilePosition);

	void createAndWriteGainVsTemperaturePlot();

	// create SiPM Gain file
	void initializeSiPMGainFile();
	void writeSiPMGainValueToCalibFile(int correspondingSpillNr);
	void finalizeSiPMGainFile();

	~CalibrationMode_SiPMGain(void);
protected:
	QString _calibDirPath;

	int _lastSpillNumber;
	int _lastRunNumber;

	bool _isFirstAnalyzedSpill;
	bool _gainHasNotBeenWrittenOnce;

	QTextStream *_output;
	QFile *_outputFile;

	vector <TCanvas *> _pulseDistCanvasVec;
	vector <TF1 *> _pulseDistfgauss1PE;
	vector <TF1 *> _pulseDistfgauss2PE;

private:
	QString _absoluteSiPMGainFilePath;

	map <int,double> _tilePositionToSiPMGainMap;
	map <int,double> _tilePositionToSiPMGainErrorMap;

	int _useNrSpillsForGainDetermination;
	int _spillNrCountDownForHistogramFilling;
	bool _isFirstAnalyzedWaveformInNewSpill;

	//int _numberOfSpillsOverjumped;

	QMap<int , QMap<int , TH1D *> > _spillNrToTilepositionToIntegralDistributionMap;
	QMap<int , unsigned int > _spillNrToCorrespondingTimeT;

	AccessTemperatureData *_temperatureData;
	QString _filePathTemperatureData;

	QMap<int ,vector< unsigned int > > _tilePositionToTimeTOfGainDetermination;
	QMap<int ,vector< double > > _tilePositionToCurrentGainValue;
	QMap<int ,vector< double > > _tilePositionToCurrentTemperature;
	unsigned int _timeOfFirstSpill;
};
