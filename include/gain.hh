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
#include <TFile.h>
// Project
#include "event.hh"
#include "channel.hh"

class GainChannel
{
    /**
    * [for description]
    * @param  gain_     [ 0: Fit status
    *                     1: Fit par const1
    *                     2: Fit par mean1
    *                     3: Fit par sigma1
    *                     4: Fit par const2
    *                     5: Fit par mean2
    *                     6: Fit par sigma2
    *                     7: Fit Chi2
    *                     8: Fit NDF
    *                     9: Fit p-value
    *                     10: Hist entries
    *                     11: Stage
    *                     12: Gain ]
    *
    * @param  avg_res_ [ 0: Fit status
    *                    1: Fit par const1
    *                    2: Fit par mean1
    *                    3: Fit par sigma1
    *                    4: Fit par const2
    *                    5: Fit par mean2
    *                    6: Fit par sigma2
    *                    7: Fit Chi2 ]
    */
    public:
        GainChannel(std::string name);
        GainChannel(std::string name, TFile* rfile);
        ~GainChannel();

        void AddGain(CalibrationChannel * channel, double t = -1);
        double* FitGain();

        void AddWaveform(CalibrationChannel * channel);

        void Normalize();
        double* FitAvg();

        std::string GetName();
        TH1I*   GetHistogram();
        TGraph* GetGraph();
        TH1D*   GetAvg();
        virtual double*      GetGain();
        virtual void         SetGain(double* gain);
        virtual double*      GetAvgResults();
        std::string name_;
        TH1I*       hist_;
        TGraph*     gain_otime_;
        TH1D*       avg_;
        // double      gain_;
        int         n_;
        double      gain_[13];
        double      avg_res_[1];

        //int         end;
};

enum GainState
{
    GAINSTATE_INIT,
    GAINSTATE_LOADED,
    GAINSTATE_FITTED,
    GAINSTATE_AVGED,
    GAINSTATE_NORMALIZED,
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
      return "gainstate_init";
    case GAINSTATE_LOADED:
      return "gainstate_loaded";
    case GAINSTATE_FITTED:
      return "gainstate_fitted";
      case GAINSTATE_AVGED:
        return "gainstate_averaged";
     case GAINSTATE_NORMALIZED:
      return "gainstate_normalized";
    default:
      return "Invalid Selection";
  }
};

class Gain
{
    public:
                                            // Gain(boost::filesystem::path path, int nr);
                                            Gain(boost::filesystem::path path, GainState state = GAINSTATE_INIT);
        virtual                             ~Gain();
        virtual void CreateChannels();
        virtual void LoadChannels(GainState state);
        virtual void AddEvent( CalibrationEvent* evt );
        virtual void FitGain();

        virtual void Normalize();
        virtual void SaveGain(boost::filesystem::path dst);

    protected:
        GainState state_;
        std::vector<GainChannel*> channels_;
        int nr_;
        boost::filesystem::path path_;
        boost::property_tree::ptree pt_;


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
