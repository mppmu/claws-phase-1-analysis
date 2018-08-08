/*
 *  Channel.hh
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_CHANNEL_H_
#define CLAWS_CHANNEL_H_

// --- C++ includes ---
#include <tuple>
//#include <iostream>
// #include <cstdint>
// #include <cstdio>
// #include <cinttypes>

// --- ROOT includes ---
#include <TH1.h>
#include <TH1I.h>
#include <TFile.h>

// Project includes
// #include "GlobalSettings.hh"




//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

/**
 *
 * @param  file [description]
 * @return      [description]
 */

enum ChannelState
{
		CHANNELSTATE_VALID,
		CHANNELSTATE_PDFAILED,
		CHANNELSTATE_FAILED,
};

/** Usually definitions should go into the .cxx file because otherwise
 *  the complier complains about multiple definitions. To avoid this
 *  the definition is inlined.
 */
inline std::string printChannelState(ChannelState state)
{
		switch(state)
		{
		case CHANNELSTATE_VALID:
				return "valid";
		case CHANNELSTATE_PDFAILED:
				return "pdfailed";
		case CHANNELSTATE_FAILED:
				return "failed";
		default:
				return "Invalid Selection";
		}
}


class Channel
{
/**
 * [for description]
 * @param  pd_ [ 0: Fit status
 *               1: Fit par constant
 *               2: Fit par mean
 *               3: Fit mean error
 *               4: Fit par sigma
 *               5: Fit chi2
 *               6: Fit NDF
 *               7: Fit p-value
 *               8: Hist mean
 *               9: Hist mean error
 *               10: Hist entries ]
 *
 */

public:

Channel(std::string name = "");
virtual ~Channel();

virtual void        LoadHistogram(TFile* file);
virtual void        PrepareHistogram( double range = -1);
virtual void        DeleteHistogram();

virtual void        FillPedestal() = 0;
virtual void        SubtractPedestal( double pd = -1000 );

virtual std::string  GetName();
virtual TH1*         GetHistogram(std::string type = "waveform");
virtual double*      GetPedestal();
virtual ChannelState GetState();
virtual std::string  GetScopePos();

void SetName(std::string name);

protected:
std::string name_;
ChannelState state_;
TH1F*                  wf_;
TH1I*                  pdhist_;
double pd_[11];

double range_;
std::string scope_pos_;
};

class CalibrationChannel : public Channel
{
public:

CalibrationChannel(std::string name, std::string scope_pos);
virtual ~CalibrationChannel();

virtual void LoadHistogram(TFile* file);
virtual void        FillPedestal();
protected:
int scope_;
std::string channel_;
};

struct OverShootResult
{
		double lstart = -1;
		double lstop  = -1;
		double lresult = -1;
		double start   = -1;
		double stop   = -1;
		int result   = -1;
		double par0  = -1;
		double par1  = -1;
		double par2  = -1;
		double chi2  = -1;
		int ndf      = -1;
		double pval  = -1;
		int n        =  0;
		double area1 = 0;
		double area2 = 0;
};


class PhysicsChannel : public Channel
{
public:

PhysicsChannel(std::string ch_name, std::string scope_pos);
virtual ~PhysicsChannel();

virtual void LoadHistogram(TFile* rfile, std::vector<std::string> types = {"wf"});
virtual void        DeleteHistogram();

virtual void PrepareHistogram( double range, double offset = 0.);
virtual void FillPedestal();

virtual std::vector<OverShootResult> OverShootCorrection();

virtual double FastRate(TH1F* avg, double unixtime);

virtual void PrepareTagging();
virtual void SignalTagging();

virtual void PrepareDecomposition();
virtual void WaveformDecomposition(TH1F* avg);
virtual void SubtractWaveform(int start, TH1F* avg);
int GetMaximumBin(double threshold, int methode, double fwhm, int stop_region, int first = 1, int last = -1);
bool FWHM(int bin, int methode, double fwhm);
bool Hood(int bin, int stop_region);
void WaveformDecomposition2(TH1F* avg);

virtual std::vector<double> WaveformReconstruction(TH1F* avg);

virtual void PrepareRetrieval();
virtual void MipTimeRetrieval(double unixtime = -1.);


virtual double*                 GetOS();
virtual TH1*                    GetHistogram(std::string type);

virtual double     GetFastRate();
virtual double     GetRate();

//    virtual double*     GetReco();
private:
double os_[10];
TH1F*                  recowf_;
TH1I*                  pewf_;
TH1F*                  mipwf_;

/** [0] == online rate
 *   [1] == fast offline
 *   [2] == foll reco
 */
double fast_rate_;
double rate_;
//     double              reco_res_[4];

};

struct AnalysisChannel {
		~AnalysisChannel(){
				if(wf) delete wf;
				if(peak) delete peak;
				if(fft_real_h) delete fft_real_h;
				if(fft_img_h) delete fft_img_h;
				if(fft_mag_h) delete fft_mag_h;
				if(fft_phase_h) delete fft_phase_h;
				if(hit_energy) delete hit_energy;
		};

		std::string name ="";
		TH1F* wf= nullptr;
		TH1F* peak = nullptr;
		TH1F* fft_real_h = nullptr;
		TH1F* fft_img_h = nullptr;
		TH1F* fft_mag_h = nullptr;
		TH1F* fft_phase_h = nullptr;
		TH1F* hit_energy = nullptr;
		TH1F* hit_energy_sync = nullptr;
		TH1F* hit_energy_mip = nullptr;
};
//
//         void    WaveformDecomposition();
//         void    PrintType();
//
//         void               CalculateIntegral();  // Pure Placeholder so far.
//         virtual     void        DeleteWaveform();
//
//         void SetUpWaveforms();
//         void SetUpWaveformsV2();
//
//             void InitCleanWF();
//             void InitWorkhorseWF();
//             void InitMipWF();
//
//             void BuildCleanWF();
//             void BuildWorkhorseWF();
//
//         double DecomposeV2(std::vector<float>* avg_wf);
//
//                 void    Subtract1PE(std::vector<float>* avg_wf);
//                 void    ReconstructV2(std::vector<float>* avg_waveform);
//                 void    CalculateChi2V2();
//                 double  GetChi2();
//
//         void RunFFT();
//
//         void FastRate(std::vector<float>* avg_waveform, double pe_to_mip = 16.);
//         void Rate(double pe_to_mip = 16.);
//
//         void Decompose(std::vector<float>* avg_waveform);
//         void Reconstruct(std::vector<float>* avg_waveform);
//         void CalculateChi2();
//         //void Decompose();
//
//        std::vector<float>*           waveform_workon_   = NULL;
//        std::vector<std::uint8_t>*    waveform_photon_   = NULL;
//
//        std::vector<float>*         clean_wf_     = NULL;
//        std::vector<float>*         wh_wf_     = NULL;
//        std::vector<float>*         mip_wf_       = NULL;
//
//        double                               chi2_ = 0;
//        int                                  nr_ph_ = 0;
//
//        TH1F*                   GetWaveformHist();
//
//        void                 CreateHistogram(std::string type = "raw");
//        TH1*                 GetHistogram(std::string type = "raw");
//
//        // Type 1: fast offline double[3]
//        // Type 2: offline      double[3]
//        double                  GetRate(int type = 1);
//
//
//        double                  fast_rate_ = 0;
//        double                  rate_      = 0;



//
// class AnalysisChannel : public Channel
// {
//     public:
//         AnalysisChannel(std::string ch_name);
//         virtual ~AnalysisChannel();
//
//         virtual     void        LoadHistogram(TFile* file);
//         void CreateHistogram();
//         void                    Normalize(double n);
//         void SetErrors(double err = 0);
//         void RunPeak();
//         void RunFFT();
//
//         virtual void            CalculateIntegral();
//         TH1*                    GetHistogram(std::string type ="waveform");
//
//         void    PrintType();
//
//
//         TH1D*                   peak_h_         = NULL;
//         TH1D*                   fft_real_h_     = NULL;
//         TH1D*                   fft_img_h_      = NULL;
//         TH1D*                   fft_mag_h_      = NULL;
//         TH1D*                   fft_phase_h_    = NULL;
//
// };

#endif /* CLAWS_CHANNEL_H_ */
