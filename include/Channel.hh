/*
 *  Channel.hh
 *
 *  Created on: Sep 26, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_CHANNEl_H_
#define CLAWS_CHANNEL_H_

// Std includes
 #include <iostream>
// Root includes
#include <TH1I.h>
#include "TApplication.h"
#include <TCanvas.h>
#include <TFile.h>
// Project includes
#include "GlobalSettings.hh"

using namespace std;



//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------


class Channel
{
    public:

        Channel(string ch_name);
        virtual ~Channel();

        virtual     void    LoadHistogram(TFile* file);
        virtual     void    LoadWaveform();
        virtual     void    DeleteHistogram();

        virtual     void    LoadPedestal();
        virtual     void    Subtract(double sb);

        virtual     void    PrintType() = 0;

        void                SetBaseline(float baseline);
        string              GetName();
        //vector<int8_t>*     GetWaveform();
        vector<float>*      GetWaveform();
        TH1F*               GetWaveformHist();
        TH1I*               GetPedestal();

        string  name_;

        vector<float>*      waveform_   = NULL;
        TH1I*               hist_       = NULL;
        TH1I*               pedestal_   = NULL;
        unsigned int        n_sample_   = 0;
        int                 n_bits_;

        int                 pd_gap_      = 25;
        float               pd_delta_   = 4;
        float               baseline_;
        // void Draw();

};

class PhysicsChannel : public Channel
{
    public:

        PhysicsChannel(string ch_name);
        virtual ~PhysicsChannel();

        void    WaveformDecomposition();
        void    PrintType();

};

class IntChannel : public Channel
{
    public:

        IntChannel(string ch_name);
        virtual ~IntChannel();

        void    CalcGain();
        void    LoadAverage1PE();

        void    PrintType();

};

#endif /* CLAWS_CHANNEL_H_ */
