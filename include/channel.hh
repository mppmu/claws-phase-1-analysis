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
       return "valid";
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
    *               3: Fit par sigma
    *               4: Fit chi2
    *               5: Fit NDF
    *               6: Fit p-value
    *               7: Hist mean
    *               8: Hist mean error
    *               9: Hist entries ]
    *
    */

    public:

        Channel(std::string name = "");
        virtual ~Channel();

        virtual     void        LoadHistogram(TFile* file);
        virtual     void        PrepHistogram( double range = -1);
        virtual     void        DeleteHistogram();

        virtual     void        FillPedestal() = 0;
        virtual     void        SubtractPedestal( double pd = -1000 );

        virtual std::string  GetName();
        virtual TH1*         GetHistogram(std::string type = "waveform");
        virtual double*      GetPedestal();
        virtual ChannelState GetState();
        virtual std::string  GetScopePos();

        // virtual TH1D*           Get();


        void SetName(std::string name);

    protected:
        std::string            name_;
        ChannelState           state_;
        TH1F*                   hist_;
        TH1I*                  pdhist_;
        double                 pd_[10];

        double                 range_;
        std::string scope_pos_;

//         virtual     void        SubtractPedestal(double pedestal = 0, bool backup = false);
//         virtual     void        SetPedestal(double pedestal = 0, bool backup = false);
// //        virtual     void        Subtract2(double pedestal = 0, bool backup = false);

//         void                    SetBaseline(float baseline);
//         virtual void            CalculateIntegral() = 0;
//
//         std::string             GetName();
//         std::vector<float>*     GetWaveform();
//         virtual TH1F*           GetWaveformHist();

//         float                   GetPDMean();
//         float                   GetPDError();
//         double                  GetIntegral();
//

//         virtual void            CreateHistogram();
//

};

class CalibrationChannel : public Channel
{
    public:

        CalibrationChannel(std::string name, std::string scope_pos);
        virtual ~CalibrationChannel();

        virtual void LoadHistogram(TFile* file);
        virtual     void        FillPedestal();
    protected:
        int scope_;
        std::string channel_;

//
//
//         void    CalculateIntegral();
//
//         void    PrintType();

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
};


class PhysicsChannel : public Channel
{
    public:

        PhysicsChannel(std::string ch_name, std::string scope_pos);
        virtual ~PhysicsChannel();
        virtual void PrepHistogram( double range, double offset = 0.);
        virtual     void        FillPedestal();
        virtual std::vector<OverShootResult> OverShootCorrection();
        virtual void WaveformDecomposition();

        virtual double*      GetOS();

    private:
        double    os_[10];


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
};


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
