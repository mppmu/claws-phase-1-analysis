/*
 * Event.h
 *
 *  Created on: Jan 20, 2017
 *      Author: mgabriel
 */
 #ifndef CLAWS_GAIN_H_
 #define CLAWS_GAIN_H_

#include <vector>
#include <map>
// boost
#include <boost/filesystem.hpp>
// root
#include <TH1I.h>
// Project
#include "channel.hh"

struct GainChannel
{
    GainChannel(std::string n, TH1I* h, double g, std::vector<float>* v):name(n),gain_hist(h), gain(g), avg_wf(v)
    {
        avg_hist = NULL;
        end = 0;
    };
    ~GainChannel()
    {
        delete gain_hist;
        delete avg_wf;
        delete avg_hist;
    };
    std::string name;
    TH1I*       gain_hist;
    double      gain;
    std::vector<float>* avg_wf;
    TH1F*       avg_hist;
    int         end;
};

class Gain
{
    public:
                                            Gain(int run_nr);
        virtual                             ~Gain();

        void                                AddValue(std::map<std::string, double> values);
        // void                                AddValue(std::vector<double> values);

        void                                FitGain();
        void                                SaveGain(boost::filesystem::path path_run);

        void                                AddIntWf(std::map<std::string, std::vector<float>*> wfs, std::map<std::string, double>integral);
        void                                AddIntWfs(std::vector<std::vector<Channel*>> int_channels);
        void                                NormalizeWaveform(int ch, double norm);
        void                                FitAvg();
        void                                SaveAvg(boost::filesystem::path path_run);

        void                                WfToHist();
        void                                HistToWf();

        std::map<std::string, double>       GetGain();
        double                              GetGain(std::string channel = "");
        std::map<std::string, std::vector<float>*>                GetWaveform();
        std::vector<float> *                GetWaveform(std::string channel = "");

    private:
        int         run_nr_;
        std::vector<GainChannel*> channels_;
        // std::vector<std::string> channel_list_;
        // std::vector<TH1I*> channels_;
        // std::vector<double> gain_;
};


#endif /* CLAWS_GAIN_H_ */
