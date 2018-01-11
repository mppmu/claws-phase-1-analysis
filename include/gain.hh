/*
 * Event.h
 *
 *  Created on: Jan 20, 2017
 *      Author: mgabriel
 */
 #ifndef CLAWS_GAIN_H_
 #define CLAWS_GAIN_H_

#include <vector>
//#include <map>
// boost
#include <boost/filesystem.hpp>
// root
#include <TH1I.h>
#include <TGraph.h>
// Project
#include "event.hh"
#include "channel.hh"

class GainChannel
{
    public:
        GainChannel(std::string name);
        ~GainChannel();

        void AddChannel(CalibrationChannel * channel, double t = -1);
        void FitGain();
        void Normalize();
        TH1I* GetHistogram();
        TGraph* GetGraph();
        TH1D* GetAvg();

        std::string name_;
        TH1I*       hist_;
        TGraph*     gain_otime_;
        TH1D*       avg_;
        double      gain_;
        int         n_;

        //int         end;
};

enum GainState
{
    GAINSTATE_INIT,
    GAINSTATE_LOADED,
    GAINSTATE_NORMALIZED,
    GAINSTATE_FITTED,
};

/** Usually definitions should go into the .cxx file because otherwise
*  the complier complains about multiple definitions. To avoid this
*  the definition is inlined.
*/
inline std::string printGainState(GainState state)
{
  switch(state)
  {
    case GAINSTATE_INIT:
      return "init";
    case GAINSTATE_LOADED:
      return "loaded";
    case GAINSTATE_NORMALIZED:
      return "normalized";
    case GAINSTATE_FITTED:
      return "fitted";
    default:
      return "Invalid Selection";
  }
};

class Gain
{
    public:
                                            // Gain(boost::filesystem::path path, int nr);
                                            Gain(int nr);
        virtual                             ~Gain();

        virtual void AddEvent( CalibrationEvent* evt );
        virtual void FitGain();

        virtual void Normalize();
        virtual void SaveGain(boost::filesystem::path dst);
//    protected:
        // boost::filesystem::path path_;
        // int nr_;
    protected:
        GainState state_;
        std::vector<GainChannel*> channels_;
        int nr_;
        // std::vector<TH1I*> channels_;
        // std::vector<TGraph*> gain_otime_;

};

// struct GainChannel
// {
//     GainChannel(std::string n, TH1I* h, double g, std::vector<float>* v):name(n),gain_hist(h), gain(g), avg_wf(v)
//     {
//         avg_hist = NULL;
//         end = 0;
//         norm = 0;
//     };
//     ~GainChannel()
//     {
//         delete gain_hist;
//         delete avg_wf;
//         delete avg_hist;
//     };
//     std::string name;
//     TH1I*       gain_hist;
//     double      gain;
//     std::vector<float>* avg_wf;
//     TH1F*       avg_hist;
//     int         end;
//     int         norm;
// };
//
// class Gain
// {
//     public:
//                                             Gain(int int_nr);
//         virtual                             ~Gain();
//
//         void                                AddValue(std::map<std::string, double> values);
//         // void                                AddValue(std::vector<double> values);
//
//         void                                FitGain();
//         void                                SaveGain(boost::filesystem::path path_run);
//
//         void                                AddIntWf(std::map<std::string, std::vector<float>*> wfs, std::map<std::string, double>integral);
//         void                                AddIntWfs(std::vector<std::vector<IntChannel*>> int_channels);
//         void                                NormalizeWaveform(int ch, double norm);
//         void                                FitAvg();
//         void                                SaveAvg(boost::filesystem::path path_run);
//
//         void                                WfToHist();
//         void                                HistToWf();
//
//         std::map<std::string, double>       GetGain();
//         double                              GetGain(std::string channel);
//         std::map<std::string, std::vector<float> *>                GetWaveform();
//         std::vector<float> *                GetWaveform(std::string channel);
//
//     private:
//         int         int_nr_;
//         std::vector<GainChannel*> channels_;
//         // std::vector<std::string> channel_list_;
//         // std::vector<TH1I*> channels_;
//         // std::vector<double> gain_;
// };


#endif /* CLAWS_GAIN_H_ */
