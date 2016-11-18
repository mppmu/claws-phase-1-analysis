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

        virtual     void    LoadWaveform(TFile* file);
        virtual     void    LoadPedestal();
        virtual     void    Subtract(double sb);

        virtual     void    PrintType() = 0;

        string  GetName();
        //vector<int8_t>*     GetWaveform();
        vector<float>*      GetWaveform();
        TH1F*               GetWaveformHist();
        TH1I*               GetPedestal();

        string  name_;
        //vector<int8_t>*     waveform_   = NULL;
        vector<float>*     waveform_   = NULL;
        TH1I*               pedestal_  = NULL;
        unsigned int        n_sample_   = 0;
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
