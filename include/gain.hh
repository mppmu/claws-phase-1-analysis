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

struct GainChannel
{
    GainChannel(std::string n, TH1I* h, double g):name(n),hist(h),gain(g){};
    std::string name;
    TH1I*       hist;
    double      gain;
};

class Gain
{
    public:
                                            Gain(int run_nr);
        virtual                             ~Gain();

        void                                AddValue(std::map<std::string, double> values);
        void                                AddValue(std::vector<double> values);

        void                                Fit();
        void                                Save(boost::filesystem::path path_run);

        std::map<std::string, double>       GetGain();
        double                              GetGain(std::string channel = "");



    private:
        int         run_nr_;
        std::vector<GainChannel> channels_;
        // std::vector<std::string> channel_list_;
        // std::vector<TH1I*> channels_;
        // std::vector<double> gain_;
};


#endif /* CLAWS_GAIN_H_ */
