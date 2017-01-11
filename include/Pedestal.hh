/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

 #include <TFile.h>
 #include <TH1I.h>
 #include <TGraphErrors.h>
 #include <vector>
#include <map>

#ifndef CLAWS_PEDESTAL_H_
#define CLAWS_PEDESTAL_H_


class Pedestal{

    public:
                                            Pedestal(int run_nr);
        virtual                             ~Pedestal();

        void                                AddEvent(map<string, TH1I*> event);
        void                                SavePedestal(TFile* file);

        std::map<string, float>             GetPedestal();
        std::map<string, float>             GetError();

        std::map<string, TH1I*>             GetHistograms();
        std::map<string, TGraphErrors*>     GetGraphs();

        std::map<string, TH1I*>             h_;
        std::map<string, TGraphErrors*>     g_;
        int                                 run_nr_;



};
#endif /* CLAWS_PEDESTAL_H_ */
