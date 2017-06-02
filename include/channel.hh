/*
 *  Channel.hh
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_CHANNEL_H_
#define CLAWS_CHANNEL_H_

// Std includes
 #include <iostream>
// #include <cstdint>
// #include <cstdio>
// #include <cinttypes>

// Root includes
#include <TH1I.h>
#include "TApplication.h"
#include <TCanvas.h>
#include <TFile.h>

// Project includes
// #include "GlobalSettings.hh"

using namespace std;



//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

/**
 *
 * @param  file [description]
 * @return      [description]
 */
class Channel
{
    public:

        Channel(std::string ch_name);
        virtual ~Channel();

        virtual     void        LoadHistogram(TFile* file);
        virtual     void        LoadWaveform();
        virtual     void        DeleteHistogram();
        virtual     void        DeleteWaveform();

        virtual     void        LoadPedestal();
//        virtual     void    Subtract();
        virtual     void        SubtractPedestal(double pedestal = 0, bool backup = false);
        virtual     void        SetPedestal(double pedestal = 0, bool backup = false);
//        virtual     void        Subtract2(double pedestal = 0, bool backup = false);


        virtual     void        PrintType() ;

        void                    SetBaseline(float baseline);
        virtual void            CalculateIntegral() = 0;

        std::string             GetName();
        std::vector<float>*     GetWaveform();
        virtual TH1F*           GetWaveformHist();
        TH1I*                   GetPedestal();
        float                   GetPDMean();
        float                   GetPDError();
        double                  GetIntegral();

        virtual TH1*            GetHistogram();
        virtual void            CreateHistogram();

    protected:

        std::string             name_;

        std::vector<float>*     waveform_      = NULL;

        TH1*                   hist_          = NULL;
        TH1I*                   pedestal_hist_      = NULL;

        float                   pedestal_   = 0;
        float                   baseline_   = 0;

        float                   pd_mean_;
        float                   pd_error_;

        double                  integral_     = 0 ;
        unsigned int            n_sample_   = 0;
        int                     n_bits_;

        int                     pd_gap_      = 25;
        float                   pd_delta_   = 4;


        // void Draw();

};

class PhysicsChannel : public Channel
{
    public:

        PhysicsChannel(std::string ch_name);
        virtual ~PhysicsChannel();

        void    WaveformDecomposition();
        void    PrintType();

        void               CalculateIntegral();  // Pure Placeholder so far.
        virtual     void        DeleteWaveform();

        void SetUpWaveforms();
        void SetUpWaveformsV2();

            void InitCleanWF();
            void InitWorkhorseWF();
            void InitMipWF();

            void BuildCleanWF();
            void BuildWorkhorseWF();

        double DecomposeV2(std::vector<float>* avg_wf);

                void    Subtract1PE(std::vector<float>* avg_wf);
                void    ReconstructV2(std::vector<float>* avg_waveform);
                void    CalculateChi2V2();
                double  GetChi2();

        void RunFFT();

        void FastRate(std::vector<float>* avg_waveform, double pe_to_mip = 16.);
        void Rate(double pe_to_mip = 16.);

        void Decompose(std::vector<float>* avg_waveform);
        void Reconstruct(std::vector<float>* avg_waveform);
        void CalculateChi2();
        //void Decompose();

       std::vector<float>*           waveform_workon_   = NULL;
       std::vector<std::uint8_t>*    waveform_photon_   = NULL;

       std::vector<float>*                clean_wf_     = NULL;
       std::vector<float>*                wh_wf_     = NULL;
       std::vector<float>*         mip_wf_       = NULL;

       double                               chi2_ = 0;
       int                                  nr_ph_ = 0;

       TH1F*                   GetWaveformHist();

       void                 CreateHistogram(std::string type = "raw");
       TH1*                 GetHistogram(std::string type = "raw");

       // Type 1: fast offline double[3]
       // Type 2: offline      double[3]
       double                  GetRate(int type = 1);


       double                  fast_rate_ = 0;
       double                  rate_      = 0;
};

class IntChannel : public Channel
{
    public:

        IntChannel(string ch_name);
        virtual ~IntChannel();



        void    CalculateIntegral();

        void    PrintType();

};

class AnalysisChannel : public Channel
{
    public:
        AnalysisChannel(std::string ch_name);
        virtual ~AnalysisChannel();

        virtual     void        LoadHistogram(TFile* file);
        void CreateHistogram();
        void                    Normalize(double n);
        void SetErrors(double err = 0);

        virtual void            CalculateIntegral();
        TH1*                   GetHistogram();

        void    PrintType();

};

#endif /* CLAWS_CHANNEL_H_ */
