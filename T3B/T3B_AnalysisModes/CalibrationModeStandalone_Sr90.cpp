#include "CalibrationModeStandalone_Sr90.h"

CalibrationModeStandalone_Sr90::CalibrationModeStandalone_Sr90(void)
{
	this->loadSettings();
	this->initializeAnalysisModeVariables();

	this->loadMipInfoOfAllRuns();

	this->startAnalysis();
}
// initializations
void CalibrationModeStandalone_Sr90::loadSettings()
{
	if(__globalSettingsManager->getSelectedAnalysisMode() != "CalibrationSr90")
		throw GeneralException("*** Error: settingsFileGroup and chosen analysis mode do not match!");

	// general
	_dataPath = __globalSettingsManager->getSetting("general_dataPath").toString();
	QString runNumbers_str = __globalSettingsManager->getSetting("general_Runs").toString();
	_runsToCalibrate = this->decomposeInputString(runNumbers_str);
	_outputFileName = __globalSettingsManager->getSetting("general_outputFileName").toString();

	// master tile
	_masterTileName = __globalSettingsManager->getSetting("rm_masterTile").toString();
}
void CalibrationModeStandalone_Sr90::initializeAnalysisModeVariables()
{
	_serializationObject_mipPeakInfo = NULL;

	this->createRootOutputFile();
}
void CalibrationModeStandalone_Sr90::loadMipInfoOfAllRuns()
{
	cout<<"Initializing MipPeak and SiPMGain Class for Run: "<<endl;
	for(int i=0; i<_runsToCalibrate.size(); i++)
	{
		cout<<_runsToCalibrate.at(i)<<", ";

		//////////////////////////MipPeak////////////////////////////////
		_serializationObject_mipPeakInfo = new Serialization_MipPeak(this->getPathToCalibDir(_runsToCalibrate.at(i),false).toStdString(),"read");

		// read in MIP peak info
		_serializationObject_mipPeakInfo->readNextSequentialInfo();

		// get mipPeakInfo struct
		MipPeakInformation *currentMipPeakInfo = _serializationObject_mipPeakInfo->getMipPeakInfo();

		// get TileName of Tile under study
		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(currentMipPeakInfo);
		if(tilePositionAndTileNameOfTUS.second == "")
			throw GeneralException("*** Error: TileName could not be identified correctly in function 'loadMipInfoOfAllRuns()'!");

		// assign mipPeakInfo
		if(tilePositionAndTileNameOfTUS.second == _masterTileName)
			_MipPeakInformationMasterTile.append(currentMipPeakInfo);
		else
			_tileName_to_MipPeakInformation[tilePositionAndTileNameOfTUS.second] = currentMipPeakInfo;

		//////////////////////////SiPMGain////////////////////////////////
		_SiPMGainData = new AccessSiPMGainData(this->getPathToCalibDir(_runsToCalibrate.at(i),true));
		double SiPMGainValue = _SiPMGainData->getSiPMGainValue(_SiPMGainData->getSmallestSpillNoForWhichSiPMGainValueWasDetermined(), tilePositionAndTileNameOfTUS.first);
		if(tilePositionAndTileNameOfTUS.second == _masterTileName)
			_SiPMGainValuesMasterTile.append(SiPMGainValue);
		else
			_tileName_to_SiPMGainValue[tilePositionAndTileNameOfTUS.second] = SiPMGainValue;
	}

	// error checks
	if(_MipPeakInformationMasterTile.size() < 2)
		throw GeneralException("*** Error: One Run is not enough for the master tile!");
	if(!_tileName_to_MipPeakInformation.size())
		throw GeneralException("*** Error: We need at least one Run of another tile than the Master Tile!");

	cout<<"\n MipPeak and SiPMGain Information is now available"<<endl<<endl;
}

// start calibration
void CalibrationModeStandalone_Sr90::startAnalysis()
{
	this->VBiasVsSiPMGainPlot();
	this->VBiasVsMipPeakPlot();
	this->MipPeakVsSiPMGainPlot();

	this->MipPeakDistributionPlot();
	this->SiPMGainDistributionPlot();

	this->meanTemperatureVsVBias();
}
// create plots
void CalibrationModeStandalone_Sr90::VBiasVsSiPMGainPlot()
{
	// determine cavas ranges
	pair<double,double> minMaxVoltage(100,0);
	pair<double,double> minMaxSiPMGain(100,0);

	// create directory in root file
	QString dirTitle = "VBias vs. SiPMGain";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());

	//////////////////////create a TLegend//////////////////////////
	TLegend leg(0.12,0.48,0.42,0.88,"All T3B Tiles:");
	leg.SetFillColor(0);
	leg.SetLineColor(0);
	leg.SetFillStyle(0);

	//////////////////////fill vectors for master graph//////////////////////////
	vector<double> SiPMGainVec;
	vector<double> voltageVec;
	for(int i=0; i<_SiPMGainValuesMasterTile.size(); i++)
	{
		MipPeakInformation *currentMipPeakInfo = _MipPeakInformationMasterTile.at(i);
		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(currentMipPeakInfo);

		double voltage = currentMipPeakInfo->_tilePosition_To_appliedBiasVoltage[tilePositionAndTileNameOfTUS.first];
		double SiPMGain = _SiPMGainValuesMasterTile.at(i);

		voltageVec.push_back(voltage);
		SiPMGainVec.push_back(SiPMGain);

		if(minMaxVoltage.first > voltage)
			minMaxVoltage.first = voltage;
		if(minMaxVoltage.second < voltage)
			minMaxVoltage.second = voltage;
		if(minMaxSiPMGain.first > SiPMGain)
			minMaxSiPMGain.first = SiPMGain;
		if(minMaxSiPMGain.second < SiPMGain)
			minMaxSiPMGain.second = SiPMGain;
	}

	//////////////////////initialize master graph//////////////////////////
	TGraph *VBiasVsSiPMGain = new TGraph(voltageVec.size(),
										&voltageVec.front(),
										&SiPMGainVec.front() );
	VBiasVsSiPMGain->GetYaxis()->SetTitle("SiPMGain [C]");
	VBiasVsSiPMGain->GetXaxis()->SetTitle("V_{bias} [V]");
	QString VBiasVsSiPMGain_Title = QString("%1_graph").arg(dirTitle);
	VBiasVsSiPMGain->SetTitle(VBiasVsSiPMGain_Title.toStdString().c_str());
	VBiasVsSiPMGain->SetName(VBiasVsSiPMGain_Title.toStdString().c_str());
	VBiasVsSiPMGain->SetLineColor(2);
	VBiasVsSiPMGain->SetMarkerColor(2);

	//////////////////////draw master graph//////////////////////////
	// create canvas, draw
	QString cVBiasVsSiPMGain_title = QString("%1_canvas").arg(VBiasVsSiPMGain->GetTitle());
	TCanvas *cVBiasVsSiPMGain = new TCanvas(cVBiasVsSiPMGain_title.toStdString().c_str(),cVBiasVsSiPMGain_title.toStdString().c_str(),800,500);
	TH1F *frame = cVBiasVsSiPMGain->DrawFrame(minMaxVoltage.first - 0.5,minMaxSiPMGain.first*0.9,minMaxVoltage.second + 0.1,minMaxSiPMGain.second*1.1);
	frame->SetTitle(VBiasVsSiPMGain_Title.toStdString().c_str());
	frame->GetYaxis()->SetTitle("SiPMGain [C]");
	frame->GetXaxis()->SetTitle("V_{bias} [V]");
	cVBiasVsSiPMGain->cd();
	VBiasVsSiPMGain->Draw("L*same");
	VBiasVsSiPMGain->SetMarkerStyle(23);
	leg.AddEntry(VBiasVsSiPMGain,_masterTileName.toStdString().c_str(),"p");

	////////////////////////fit master graph//////////////////////////
	//QString fitTitle = "linearFit_Master";
	//TF1 *linearFit_Master = new TF1(fitTitle.toStdString().c_str(),"pol1");
	//VBiasVsSiPMGain->Fit(fitTitle.toStdString().c_str(),"R");
	//linearFit_Master->Draw("lsame");
	//linearFit_Master->SetLineWidth(3);
	//linearFit_Master->SetLineColor(2);
	//double slopeOfLinearFit = linearFit_Master->GetParameter(1);
	
	//////////////////////draw Graphs of other Tiles//////////////////////////
	QMapIterator<QString, MipPeakInformation*> it(_tileName_to_MipPeakInformation);
	int colourCounter = 0;
	while (it.hasNext()) 
	{
		it.next();

		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(it.value());

		double voltage = it.value()->_tilePosition_To_appliedBiasVoltage[tilePositionAndTileNameOfTUS.first];
		double SiPMGain = _tileName_to_SiPMGainValue[it.key()];

		TGraph *VBiasVsSiPMGain_otherTiles = new TGraph();
		VBiasVsSiPMGain_otherTiles->SetTitle(it.key().toStdString().c_str());
		VBiasVsSiPMGain_otherTiles->SetName(it.key().toStdString().c_str());
		VBiasVsSiPMGain_otherTiles->SetPoint(1,voltage,SiPMGain);

		VBiasVsSiPMGain_otherTiles->Draw("*same");
		VBiasVsSiPMGain_otherTiles->SetLineColor(kAzure - 9 + colourCounter);
		VBiasVsSiPMGain_otherTiles->SetMarkerStyle(23);
		VBiasVsSiPMGain_otherTiles->SetMarkerColor(kAzure - 9 + colourCounter);

		//// shift fit to data point
		//QString fitTitle = QString("linearFit_Tile %1").arg(it.key());
		//TF1 *linearFit_otherTiles = new TF1(fitTitle.toStdString().c_str(),"pol1");
		//linearFit_otherTiles->SetParameter(0,SiPMGain-slopeOfLinearFit*voltage);
		//linearFit_otherTiles->SetParameter(1,slopeOfLinearFit);
		//linearFit_otherTiles->Draw("Lsame");
		//linearFit_otherTiles->SetLineColor(kAzure - 9 + colourCounter);
		//linearFit_Master->SetLineWidth(1);

		leg.AddEntry(VBiasVsSiPMGain_otherTiles,tilePositionAndTileNameOfTUS.second.toStdString().c_str(),"p");

		colourCounter++;
	}

	//////////////////////draw legend//////////////////////////
	leg.Draw();

	//////////////////////write canvas//////////////////////////
	dir->WriteTObject(cVBiasVsSiPMGain);
}
void CalibrationModeStandalone_Sr90::VBiasVsMipPeakPlot()
{
	// create directory in root file
	QString dirTitle = "VBias vs. MipPeak";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());

	//////////////////////create a TLegend//////////////////////////
	TLegend leg(0.12,0.48,0.42,0.88,"All T3B Tiles:");
	leg.SetFillColor(0);
	leg.SetLineColor(0);
	leg.SetFillStyle(0);


	//////////////////////fill vectors for graph//////////////////////////
	vector<double> MipPeakVec;
	vector<double> voltageVec;
	vector<double> MipPeakVecError;
	vector<double> voltageVecError;
	for(int i=0; i<_MipPeakInformationMasterTile.size(); i++)
	{
		MipPeakInformation *currentMipPeakInfo = _MipPeakInformationMasterTile.at(i);
		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(currentMipPeakInfo);

		double SiPMGain = _SiPMGainValuesMasterTile.at(i);
		double voltage = currentMipPeakInfo->_tilePosition_To_appliedBiasVoltage[tilePositionAndTileNameOfTUS.first];
		double MipPeak = currentMipPeakInfo->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].first;
		double MipPeakError = currentMipPeakInfo->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].second;
		MipPeak = MipPeak / SiPMGain;
		MipPeakError = MipPeakError / SiPMGain;

		voltageVec.push_back(voltage);
		MipPeakVec.push_back(MipPeak);
		voltageVecError.push_back(0.);
		MipPeakVecError.push_back(MipPeakError);
	}

	//////////////////////initialize graph//////////////////////////
	TGraphErrors *VBiasVsMipPeak = new TGraphErrors(voltageVec.size(),
											&voltageVec.front(),
											&MipPeakVec.front(), 
											&voltageVecError.front(),
											&MipPeakVecError.front()
											);
	VBiasVsMipPeak->GetYaxis()->SetTitle("MipPeak [p.e.]");
	VBiasVsMipPeak->GetXaxis()->SetTitle("V_{bias} [V]");
	QString VBiasVsMipPeak_Title = QString("%1_graph").arg(dirTitle);
	VBiasVsMipPeak->SetTitle(VBiasVsMipPeak_Title.toStdString().c_str());
	VBiasVsMipPeak->SetName(VBiasVsMipPeak_Title.toStdString().c_str());
	VBiasVsMipPeak->SetMarkerColor(2);

	//////////////////////write graph//////////////////////////
	// create canvas, draw and write
	QString cVBiasVsMipPeak_title = QString("%1_canvas").arg(VBiasVsMipPeak->GetTitle());
	TCanvas *cVBiasVsMipPeak = new TCanvas(cVBiasVsMipPeak_title.toStdString().c_str(),cVBiasVsMipPeak_title.toStdString().c_str(),800,500);
	cVBiasVsMipPeak->cd();
	VBiasVsMipPeak->Draw("A*");
	VBiasVsMipPeak->SetMarkerStyle(23);
	VBiasVsMipPeak->SetMarkerSize(1.2);
	leg.AddEntry(VBiasVsMipPeak,_masterTileName.toStdString().c_str(),"p");

	//////////////////////fit master graph//////////////////////////
	QString fitTitle = "VBiasVsMipPeak_Fit_Master";
	TF1 *linearFit_Master = new TF1(fitTitle.toStdString().c_str(),"pol2");
	linearFit_Master->SetLineColor(2);
	linearFit_Master->SetLineWidth(2);
	VBiasVsMipPeak->Fit(fitTitle.toStdString().c_str());
	linearFit_Master->Draw("lsame");
	double slopeOfLinearFit = linearFit_Master->GetParameter(1);

	//////////////////////draw Graphs of other Tiles//////////////////////////
	QMapIterator<QString, MipPeakInformation*> it(_tileName_to_MipPeakInformation);
	int colourCounter = 0;
	while (it.hasNext()) 
	{
		it.next();

		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(it.value());

		double SiPMGain = _tileName_to_SiPMGainValue[it.key()];
		double voltage = it.value()->_tilePosition_To_appliedBiasVoltage[tilePositionAndTileNameOfTUS.first];
		double MipPeak = it.value()->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].first;
		double MipPeakError = it.value()->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].second;
		MipPeak = MipPeak / SiPMGain;
		MipPeakError = MipPeakError / SiPMGain;

		TGraphErrors *VBiasVsMipPeak_otherTiles = new TGraphErrors();
		VBiasVsMipPeak_otherTiles->SetTitle(it.key().toStdString().c_str());
		VBiasVsMipPeak_otherTiles->SetName(it.key().toStdString().c_str());
		VBiasVsMipPeak_otherTiles->SetPoint(1,voltage,MipPeak);
		VBiasVsMipPeak_otherTiles->SetPointError(1,0.,MipPeakError);

		VBiasVsMipPeak_otherTiles->Draw("*same");
		VBiasVsMipPeak_otherTiles->SetLineColor(kAzure - 9 + colourCounter);
		VBiasVsMipPeak_otherTiles->SetMarkerStyle(23);
		VBiasVsMipPeak_otherTiles->SetMarkerColor(kAzure - 9 + colourCounter);

		//// shift fit to data point
		//QString fitTitle = QString("linearFit_Tile %1").arg(it.key());
		//TF1 *linearFit_otherTiles = new TF1(fitTitle.toStdString().c_str(),"pol1");
		//linearFit_otherTiles->SetParameter(0,MipPeak-slopeOfLinearFit*voltage);
		//linearFit_otherTiles->SetParameter(1,slopeOfLinearFit);
		//linearFit_otherTiles->Draw("Lsame");
		//linearFit_otherTiles->SetLineWidth(1);
		//linearFit_otherTiles->SetLineColor(kAzure - 9 + colourCounter);

		leg.AddEntry(VBiasVsMipPeak_otherTiles,tilePositionAndTileNameOfTUS.second.toStdString().c_str(),"p");

		colourCounter++;
	}

	//////////////////////draw legend//////////////////////////
	leg.Draw();

	//////////////////////write canvas//////////////////////////
	dir->WriteTObject(cVBiasVsMipPeak);
}

void CalibrationModeStandalone_Sr90::MipPeakVsSiPMGainPlot()
{
	// determine cavas ranges
	pair<double,double> minSiPMGain(100,0);
	pair<double,double> minMaxMipPeak(100,0);

	// create directory in root file
	QString dirTitle = "MipPeak vs. SiPMGain";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());

	//////////////////////create a TLegend//////////////////////////
	TLegend leg(0.12,0.48,0.42,0.88,"All T3B Tiles:");
	leg.SetFillColor(0);
	leg.SetLineColor(0);
	leg.SetFillStyle(0);

	//////////////////////fill vectors for graph//////////////////////////
	vector<double> MipPeakVec;
	vector<double> SiPMGainVec;
	vector<double> MipPeakVecError;
	vector<double> SiPMGainVecError;
	for(int i=0; i<_MipPeakInformationMasterTile.size(); i++)
	{
		MipPeakInformation *currentMipPeakInfo = _MipPeakInformationMasterTile.at(i);
		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(currentMipPeakInfo);

		double SiPMGain = _SiPMGainValuesMasterTile.at(i);
		double MipPeak = currentMipPeakInfo->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].first;
		double MipPeakError = currentMipPeakInfo->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].second;
		MipPeak = MipPeak / SiPMGain;
		MipPeakError = MipPeakError / SiPMGain;

		SiPMGain *= 1E12;

		MipPeakVec.push_back(MipPeak);
		MipPeakVecError.push_back(MipPeakError);
		SiPMGainVec.push_back(SiPMGain);
		SiPMGainVecError.push_back(0.);

		// determine range of graph
		if(minSiPMGain.first > SiPMGain)
			minSiPMGain.first = SiPMGain;
		if(minSiPMGain.second < SiPMGain)
			minSiPMGain.second = SiPMGain;
		if(minMaxMipPeak.first > MipPeak)
			minMaxMipPeak.first = MipPeak;
		if(minMaxMipPeak.second < MipPeak)
			minMaxMipPeak.second = MipPeak;
	}

	//////////////////////initialize graph//////////////////////////
	TGraphErrors *MipPeakVsSiPMGain = new TGraphErrors(SiPMGainVec.size(),
														&SiPMGainVec.front(),
														&MipPeakVec.front(),
														&SiPMGainVecError.front(),
														&MipPeakVecError.front());
	QString MipPeakVsSiPMGain_Title = QString("%1_graph").arg(dirTitle);
	MipPeakVsSiPMGain->SetTitle(MipPeakVsSiPMGain_Title.toStdString().c_str());
	MipPeakVsSiPMGain->SetName(MipPeakVsSiPMGain_Title.toStdString().c_str());

	//////////////////////write graph//////////////////////////
	// create canvas, draw and write
	QString cMipPeakVsSiPMGain_title = QString("%1_canvas").arg(MipPeakVsSiPMGain->GetTitle());
	TCanvas *cMipPeakVsSiPMGain = new TCanvas(cMipPeakVsSiPMGain_title.toStdString().c_str(),cMipPeakVsSiPMGain_title.toStdString().c_str(),800,500);
	TH1F *frame = cMipPeakVsSiPMGain->DrawFrame(minSiPMGain.first*0.8,minMaxMipPeak.first*0.9,minSiPMGain.second*1.1,minMaxMipPeak.second*1.1);
	frame->SetTitle(MipPeakVsSiPMGain_Title.toStdString().c_str());
	frame->GetXaxis()->SetTitle("SiPMGain [x10^{-12} C]");
	frame->GetYaxis()->SetTitle("MipPeak [p.e.]");
	cMipPeakVsSiPMGain->cd();
	MipPeakVsSiPMGain->Draw("*same");
	MipPeakVsSiPMGain->SetMarkerStyle(23);
	MipPeakVsSiPMGain->SetMarkerColor(2);
	MipPeakVsSiPMGain->SetMarkerSize(1.2);
	MipPeakVsSiPMGain->SetLineColor(2);

	leg.AddEntry(MipPeakVsSiPMGain,_masterTileName.toStdString().c_str(),"p");

	//////////////////////fit master graph//////////////////////////
	QString fitTitle = "MipPeakVsSiPMGain_Fit_Master";
	TF1 *linearFit_Master = new TF1(fitTitle.toStdString().c_str(),"pol2");

	linearFit_Master->SetLineColor(2);
	linearFit_Master->SetLineWidth(3);
	MipPeakVsSiPMGain->Fit(fitTitle.toStdString().c_str());
	linearFit_Master->Draw("lsame");
	double par0OfFit = linearFit_Master->GetParameter(0);
	double par1OfFit = linearFit_Master->GetParameter(1);
	double par2OfFit = linearFit_Master->GetParameter(2);

	//////////////////////draw Graphs of other Tiles//////////////////////////
	QMapIterator<QString, MipPeakInformation*> it(_tileName_to_MipPeakInformation);
	int colourCounter = 0;
	vector<double> MipPeakValuesAtFixedGain;
	double evaluateMipPeakAtGain = 0.16;
	while (it.hasNext()) 
	{
		it.next();

		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(it.value());

		double SiPMGain = _tileName_to_SiPMGainValue[it.key()];
		double MipPeak = it.value()->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].first;
		double MipPeakError = it.value()->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].second;
		MipPeak = MipPeak / SiPMGain;
		MipPeakError = MipPeakError / SiPMGain;

		SiPMGain *= 1E12;

		TGraphErrors *MipPeakVsSiPMGain_otherTiles = new TGraphErrors();
		MipPeakVsSiPMGain_otherTiles->SetTitle(it.key().toStdString().c_str());
		MipPeakVsSiPMGain_otherTiles->SetName(it.key().toStdString().c_str());
		MipPeakVsSiPMGain_otherTiles->SetPoint(1,SiPMGain,MipPeak);
		MipPeakVsSiPMGain_otherTiles->SetPointError(1,0.,MipPeakError);

		MipPeakVsSiPMGain_otherTiles->Draw("*same");
		leg.AddEntry(MipPeakVsSiPMGain_otherTiles,tilePositionAndTileNameOfTUS.second.toStdString().c_str(),"p");
		MipPeakVsSiPMGain_otherTiles->SetMarkerStyle(23);
		MipPeakVsSiPMGain_otherTiles->SetMarkerColor(kAzure - 9 + colourCounter);

		// shift fit to data point
		QString fitTitle = QString("MipPeakVsSiPMGain_Fit_Tile %1").arg(it.key());
		TF1 *linearFit_otherTiles = new TF1(fitTitle.toStdString().c_str(),"pol2");
		linearFit_otherTiles->SetParameter(0,MipPeak - par1OfFit*SiPMGain - par2OfFit*SiPMGain*SiPMGain);
		linearFit_otherTiles->SetParameter(1,par1OfFit);
		linearFit_otherTiles->SetParameter(2,par2OfFit);
		linearFit_otherTiles->Draw("Lsame");
		linearFit_otherTiles->SetLineColor(kAzure - 9 + colourCounter);
		linearFit_otherTiles->SetLineWidth(0.5);

		MipPeakValuesAtFixedGain.push_back( linearFit_otherTiles->Eval(evaluateMipPeakAtGain) );

		colourCounter++;
	}

	//////////////////////draw legend//////////////////////////
	leg.Draw();

	//////////////////////write canvas//////////////////////////
	dir->WriteTObject(cMipPeakVsSiPMGain);



	//////////////////////Mip Peak Distribution//////////////////////////
	QString MipPeakDistribution_title = "Mip Peak Distribution at SiPMGain = 0.16E^{-12} C";
	TH1D *MipPeakDistribution = new TH1D(MipPeakDistribution_title.toStdString().c_str(),MipPeakDistribution_title.toStdString().c_str(),100,0,100);
	MipPeakDistribution->GetXaxis()->SetTitle("Langau MPV [p.e.]");
	MipPeakDistribution->GetYaxis()->SetTitle("#");

	//////////////////////draw Hist with other Tiles//////////////////////////
	for(int i=0; i<MipPeakValuesAtFixedGain.size(); i++)
		MipPeakDistribution->Fill( MipPeakValuesAtFixedGain.at(i) );

	// create canvas, draw and write
	QString cMipPeakDistribution_title = QString("%1_canvas").arg(dirTitle);
	TCanvas *cMipPeakDistribution = new TCanvas(cMipPeakDistribution_title.toStdString().c_str(),cMipPeakDistribution_title.toStdString().c_str(),800,500);
	cMipPeakDistribution->cd();
	MipPeakDistribution->GetXaxis()->SetRangeUser(0,57);
	MipPeakDistribution->Draw();
	MipPeakDistribution->SetLineColor(2);
	MipPeakDistribution->SetFillColor(2);
	MipPeakDistribution->SetFillStyle(3003);
	
	cout<<"Mean: "<<MipPeakDistribution->GetMean()<<endl;
	cout<<"RMS: "<<MipPeakDistribution->GetRMS()<<endl;

	//////////////////////write canvas//////////////////////////
	dir->WriteTObject(cMipPeakDistribution);



	////////////////////////difference Graph-Fit for Master Tile//////////////////////////
	//vector<double> diffGraphFitVec;	
	//for(int i=0; i<(int)SiPMGainVec.size(); i++)
	//{
	//	double MipValueFit = linearFit_Master->Eval(SiPMGainVec.at(i));
	//	double diffGraphFit = MipValueFit - MipPeakVec.at(i);
	//	diffGraphFitVec.push_back(diffGraphFit);
	//}
	//QString diffGraph_title = "Master Tile: Difference of Fit to Graph Values";
	//QString cdiffGraph_title = QString("%1_canvas").arg(diffGraph_title);
	//TCanvas *cdiffGraph = new TCanvas(cdiffGraph_title.toStdString().c_str(),cdiffGraph_title.toStdString().c_str(),800,500);
	//TGraph *diffGraph = new TGraph(SiPMGainVec.size(),&SiPMGainVec.front(),&diffGraphFitVec.front());
	//diffGraph->SetTitle(diffGraph_title.toStdString().c_str());
	//diffGraph->SetName(diffGraph_title.toStdString().c_str());
	//diffGraph->GetXaxis()->SetTitle("SiPMGain [C]");
	//diffGraph->GetYaxis()->SetTitle("MPV(Fit) - MPV(Graph) [p.e.]");
	//diffGraph->Draw("AL*");
	//diffGraph->SetMarkerStyle(23);
	//dir->WriteTObject(cdiffGraph);
}

void CalibrationModeStandalone_Sr90::MipPeakDistributionPlot()
{
	// create directory in root file
	QString dirTitle = "Mip peak distribution - not taken from Fits";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());
	
	QString MipPeakDistribution_title = QString("%1_hist").arg(dirTitle);
	TH1D *MipPeakDistribution = new TH1D(MipPeakDistribution_title.toStdString().c_str(),MipPeakDistribution_title.toStdString().c_str(),100,0,100);
	MipPeakDistribution->GetXaxis()->SetTitle("Langau MPV [p.e.]");
	MipPeakDistribution->GetYaxis()->SetTitle("#");

	//////////////////////draw Graphs of other Tiles//////////////////////////
	QMapIterator<QString, MipPeakInformation*> it(_tileName_to_MipPeakInformation);
	while (it.hasNext()) 
	{
		it.next();

		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(it.value());

		double SiPMGain = _tileName_to_SiPMGainValue[it.key()];
		double MipPeak = it.value()->_tilePosition_To_MipPeakAndMipPeakError[tilePositionAndTileNameOfTUS.first].first;
		MipPeak = MipPeak / SiPMGain;

		MipPeakDistribution->Fill(MipPeak);
	}

	// create canvas, draw and write
	QString cMipPeakDistribution_title = QString("%1_canvas").arg(dirTitle);
	TCanvas *cMipPeakDistribution = new TCanvas(cMipPeakDistribution_title.toStdString().c_str(),cMipPeakDistribution_title.toStdString().c_str(),800,500);
	cMipPeakDistribution->cd();
	MipPeakDistribution->GetXaxis()->SetRangeUser(0,57);
	MipPeakDistribution->Draw();
	MipPeakDistribution->SetLineColor(2);
	
	//cout<<"Mean: "<<MipPeakDistribution->GetMean()<<endl;
	//cout<<"RMS: "<<MipPeakDistribution->GetRMS()<<endl;

	//////////////////////write canvas//////////////////////////
	dir->WriteTObject(cMipPeakDistribution);
}
void CalibrationModeStandalone_Sr90::SiPMGainDistributionPlot()
{
	// create directory in root file
	QString dirTitle = "SiPM Gain distribution";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());
	
	QString SiPMGainDistribution_title = QString("%1_hist").arg(dirTitle);
	TH1D *SiPMGainDistribution = new TH1D(SiPMGainDistribution_title.toStdString().c_str(),SiPMGainDistribution_title.toStdString().c_str(),100,1E-13,2E-13);
	SiPMGainDistribution->GetXaxis()->SetTitle("SiPMGain [C]");
	SiPMGainDistribution->GetYaxis()->SetTitle("#");

	//////////////////////draw Graphs of other Tiles//////////////////////////
	QMapIterator<QString, MipPeakInformation*> it(_tileName_to_MipPeakInformation);
	while (it.hasNext()) 
	{
		it.next();

		double SiPMGain = _tileName_to_SiPMGainValue[it.key()];

		SiPMGainDistribution->Fill(SiPMGain);
	}

	// create canvas, draw and write
	QString cSiPMGainDistribution_title = QString("%1_canvas").arg(dirTitle);
	TCanvas *cSiPMGainDistribution = new TCanvas(cSiPMGainDistribution_title.toStdString().c_str(),cSiPMGainDistribution_title.toStdString().c_str(),800,500);
	cSiPMGainDistribution->cd();
	SiPMGainDistribution->Draw();
	SiPMGainDistribution->SetLineColor(2);

	//////////////////////write canvas//////////////////////////
	dir->WriteTObject(cSiPMGainDistribution);
}
void CalibrationModeStandalone_Sr90::meanTemperatureVsVBias()
{
	// determine cavas ranges
	pair<double,double> minMaxVoltage(100,0);
	pair<double,double> minMaxMeanTemp(100,0);

	// create directory in root file
	QString dirTitle = "Mean Temperature vs. VBias for whole Measurement Period";
	TDirectory *dir = _tFile->GetDirectory(dirTitle.toStdString().c_str());
	if(!dir)
		dir = _tFile->mkdir(dirTitle.toStdString().c_str(),dirTitle.toStdString().c_str());

	//////////////////////create a TLegend//////////////////////////
	TLegend leg(0.12,0.48,0.42,0.88,"All T3B Tiles:");
	leg.SetFillColor(0);
	leg.SetLineColor(0);
	leg.SetFillStyle(0);

	//////////////////////fill vectors for graph//////////////////////////
	vector<double> meanTempVec;
	vector<double> voltageVec;
	for(int i=0; i<_MipPeakInformationMasterTile.size(); i++)
	{
		MipPeakInformation *currentMipPeakInfo = _MipPeakInformationMasterTile.at(i);
		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(currentMipPeakInfo);

		double voltage = currentMipPeakInfo->_tilePosition_To_appliedBiasVoltage[tilePositionAndTileNameOfTUS.first];
		double meanTemperature = currentMipPeakInfo->meanTemperatureDuringDataTaking;

		voltageVec.push_back(voltage);
		meanTempVec.push_back(meanTemperature);

		if(minMaxVoltage.first > voltage)
			minMaxVoltage.first = voltage;
		if(minMaxVoltage.second < voltage)
			minMaxVoltage.second = voltage;
		if(minMaxMeanTemp.first > meanTemperature)
			minMaxMeanTemp.first = meanTemperature;
		if(minMaxMeanTemp.second < meanTemperature)
			minMaxMeanTemp.second = meanTemperature;
	}

	//////////////////////initialize graph//////////////////////////
	TGraph *MeanTempVsVBias = new TGraph(voltageVec.size(),
										&voltageVec.front(),
										&meanTempVec.front() );
	QString MeanTempVsVBias_Title = dirTitle;
	MeanTempVsVBias->SetTitle(MeanTempVsVBias_Title.toStdString().c_str());
	MeanTempVsVBias->SetName(MeanTempVsVBias_Title.toStdString().c_str());
	MeanTempVsVBias->SetMarkerColor(2);

	//////////////////////write graph//////////////////////////
	// create canvas, draw and write
	TCanvas *cMeanTempVsVBias = new TCanvas(MeanTempVsVBias->GetTitle(),MeanTempVsVBias->GetTitle(),800,500);
	TH1F *frame = cMeanTempVsVBias->DrawFrame(minMaxVoltage.first - 0.5,minMaxMeanTemp.first - 0.2,minMaxVoltage.second + 0.1,minMaxMeanTemp.second + 0.2);
	frame->SetTitle(MeanTempVsVBias_Title.toStdString().c_str());
	frame->GetYaxis()->SetTitle("mean Temperature [C]");
	frame->GetXaxis()->SetTitle("V_{bias} [V]");
	cMeanTempVsVBias->cd();
	MeanTempVsVBias->Draw("*same");
	MeanTempVsVBias->SetMarkerStyle(23);

	leg.AddEntry(MeanTempVsVBias,_masterTileName.toStdString().c_str(),"p");

	//////////////////////draw Graphs of other Tiles//////////////////////////
	QMapIterator<QString, MipPeakInformation*> it(_tileName_to_MipPeakInformation);
	int colourCounter = 0;
	while (it.hasNext()) 
	{
		it.next();

		pair<int,QString> tilePositionAndTileNameOfTUS = this->getTilePosAndTileNameOfTileUnderStudy(it.value());

		double voltage = it.value()->_tilePosition_To_appliedBiasVoltage[tilePositionAndTileNameOfTUS.first];
		double meanTemperature = it.value()->meanTemperatureDuringDataTaking;

		TGraph *MeanTempVsVBias_otherTiles = new TGraph();
		MeanTempVsVBias_otherTiles->SetTitle(it.key().toStdString().c_str());
		MeanTempVsVBias_otherTiles->SetName(it.key().toStdString().c_str());
		MeanTempVsVBias_otherTiles->SetPoint(1,voltage,meanTemperature);

		MeanTempVsVBias_otherTiles->Draw("*same");
		leg.AddEntry(MeanTempVsVBias_otherTiles,tilePositionAndTileNameOfTUS.second.toStdString().c_str(),"p");
		MeanTempVsVBias_otherTiles->SetMarkerStyle(23);
		MeanTempVsVBias_otherTiles->SetMarkerColor(kAzure - 9 + colourCounter);

		colourCounter++;
	}

	//////////////////////draw legend//////////////////////////
	leg.Draw();

	// write to tFile
	dir->WriteTObject(cMeanTempVsVBias);
}
// helper functions
pair<int, QString> CalibrationModeStandalone_Sr90::getTilePosAndTileNameOfTileUnderStudy(MipPeakInformation *mipPeakInfo)
{
	pair<int,QString> tilePositionAndTileNameOfTUS(0,"");

	QMap<int, string> _tilePosition_To_TileName_qstring = QMap<int, string>(mipPeakInfo->_tilePosition_To_TileName);
	QMapIterator<int, string> it(_tilePosition_To_TileName_qstring);
	while (it.hasNext()) 
	{
		it.next();

		if(!QString::fromStdString(it.value()).contains("Coin"))
		{
			tilePositionAndTileNameOfTUS.first = it.key();
			tilePositionAndTileNameOfTUS.second = QString::fromStdString(it.value());

			break;
		}
	}

	return tilePositionAndTileNameOfTUS;
}
QList <int> CalibrationModeStandalone_Sr90::decomposeInputString(QString composedString)
{
	QList <int> decomposedList;

	// split by +
	QStringList plusDecomposed = composedString.split("+");

	// split by -
	for(int i=0; i<plusDecomposed.size(); i++)
	{
		QStringList minusDecomposed = plusDecomposed.at(i).split("-");
		if(minusDecomposed.size() == 1)
		{
			if(minusDecomposed.at(0) != "")
				decomposedList.append(minusDecomposed.at(0).toInt());
		}
		else if(minusDecomposed.size() == 2)
		{
			int lowerIndex = minusDecomposed.at(0).toInt();
			int upperIndex = minusDecomposed.at(1).toInt();
	
			if(upperIndex < lowerIndex)
				throw GeneralException("*** Error: String decomposition unsuccessful! \nLower Index > Upper Index!");

			for(int j=lowerIndex; j<=upperIndex; j++)
			{
				decomposedList.append(j);
			}
		}
		else
			throw GeneralException("*** Error: String decomposition unsuccessful!");
	}

	return decomposedList;
}
QString CalibrationModeStandalone_Sr90::getPathToCalibDir(int runNumber, bool _calibratingIRM)
{
	QString calibDirPath = "";
	if(_calibratingIRM)
	{
		calibDirPath = QString("%1/Run_%2/Calibration/IntermediateRM").arg(_dataPath).arg(runNumber);	
	}
	else
	{
		calibDirPath = QString("%1/Run_%2/Calibration").arg(_dataPath).arg(runNumber);	
	}

	QDir calibDir(calibDirPath);
	if(!calibDir.exists())
	{
		QString errorMessage = QString("*** Error: No Data found for Run_%1!").arg(runNumber);
		throw GeneralException(errorMessage.toStdString().c_str());
	}

	return calibDirPath;
}
void CalibrationModeStandalone_Sr90::createRootOutputFile()
{
	// check if the directory outputFiles exists
	QDir tFileDir("./");
	if(!tFileDir.entryList(QDir::AllDirs).contains("outputFiles"))
		tFileDir.mkdir("outputFiles");

	QString concatenatedOutputFileName = QString("outputFiles/%1.root").arg(_outputFileName);

	// create the root file
	_tFile = new TFile(concatenatedOutputFileName.toStdString().c_str(),"RECREATE",_outputFileName.toStdString().c_str());
}
// destructor
CalibrationModeStandalone_Sr90::~CalibrationModeStandalone_Sr90(void)
{
	_tFile->Close();
}
