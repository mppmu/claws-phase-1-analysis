/*
 * Event.h
 *
 *  Created on: Apr 7, 2016
 *      Author: mgabriel
 */

#ifndef CLAWS_EVENT_H_
#define CLAWS_EVENT_H_

// std includes
#include <vector>
#include <string>
#include <map>
// root includes
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TTree.h>
#include <TBranch.h>
// Boost
#include<boost/filesystem.hpp>
// Project includes
#include "Channel.hh"
using namespace std;
using namespace boost::filesystem;


//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Event{

    public:
        //TODO -fix event name und numbger

        Event(const path &file_root, const path &file_ini);
        Event(const path &file_root, const path &file_ini, const path &file_online_rate);

        virtual ~Event();

        virtual void                   LoadRootFile()   = 0;
        virtual void                   LoadIniFile()    = 0;
        virtual void                   LoadOnlineRate() = 0;
        virtual map<string, TH1I*>     GetPedestral()   = 0;

        void             LoadPedestral();

        int     Subtract();

        bool    GetInjection() const;
        double  GetUnixtime()  const;
        int     GetLerBg()     const;
        int     GetHerBg()     const;
	    int     GetEventNr()   const;



        static int GetId();

        double* GetRateOnline();

        int getCh(string ch);
        int draw();

    // protected:

        static int id_;

        // An event relies on data/information in three different files. The .root, .ini & the online monitor.
        path path_file_root_;
        path path_file_ini_;
        path path_online_rate_;

	    int event_number        = -1;
        double unixtime_        = -1;
        int lerbg_              = -1;
        int herbg_              = -1;
        bool injection_         = false;
	    double rate_online_[8]  = {};
        double rate_offline_[8] = {};


        TFile *file;

        map<string, IntChannel*> int_chs_;
};

class PhysicsEvent : public Event{

    public:

        // PhysicsEvent();
        PhysicsEvent(const path &file_root, const path &file_ini);
        PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate);
        ~PhysicsEvent();

        void                   LoadRootFile();
        void                   LoadIniFile();
        void                   LoadOnlineRate();

        map<string, TH1I*>     GetPedestral();

        void Draw();

        map<string, PhysicsChannel*> phy_chs_;
        //map<string, PhysicsChannel*> phy_chs_;

};

#endif /* CLAWS_EVENT_H_ */
