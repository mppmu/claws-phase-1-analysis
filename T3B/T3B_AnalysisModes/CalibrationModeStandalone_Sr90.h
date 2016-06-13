#pragma once

#include "TDirectory.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TF1.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TLegend.h"

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

#include "Serialization_MipPeak.h"
#include "AccessSiPMGainData.h"

using namespace std;

class CalibrationModeStandalone_Sr90 //: public AnalysisMode
{
public:
	CalibrationModeStandalone_Sr90(void);

	// run mode functions
	void loadSettings();
	void initializeAnalysisModeVariables();
	void startAnalysis();

	~CalibrationModeStandalone_Sr90(void);

private:
	void loadMipInfoOfAllRuns();

	// plots
	void VBiasVsSiPMGainPlot();
	void VBiasVsMipPeakPlot();
	void MipPeakVsSiPMGainPlot();

	void MipPeakDistributionPlot();
	void SiPMGainDistributionPlot();

	void meanTemperatureVsVBias();

	// helper functions
	void createRootOutputFile();
	QList <int> decomposeInputString(QString composedString);
	QString getPathToCalibDir(int runNumber, bool _calibratingIRM);
	pair<int, QString> getTilePosAndTileNameOfTileUnderStudy(MipPeakInformation *mipPeakInfo);

private:
	// from setting
	QString _outputFileName;
	QString _dataPath;
	QList <int> _runsToCalibrate;
	QString _masterTileName;

	TFile *_tFile;

	Serialization_MipPeak *_serializationObject_mipPeakInfo;
	AccessSiPMGainData *_SiPMGainData;

	QMap<QString, MipPeakInformation*> _tileName_to_MipPeakInformation;
	QList< MipPeakInformation*> _MipPeakInformationMasterTile;

	QMap<QString, double> _tileName_to_SiPMGainValue;
	QList<double> _SiPMGainValuesMasterTile;
};
