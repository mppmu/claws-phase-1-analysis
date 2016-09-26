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

// mixed
#include<boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;


//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

class Event{

    public:

        // Event();
        Event(const path &file_root, const path &file_ini);
        Event(const path &file_root, const path &file_ini, const path &file_online_rate);

        virtual ~Event();

        virtual int     LoadRootFile()   = 0;
        virtual int     LoadIniFile()    = 0;
        virtual int     LoadOnlineRate() = 0;

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

};

class PhysicsEvent : public Event{

    public:

        // PhysicsEvent();
        PhysicsEvent(const path &file_root, const path &file_ini);
        PhysicsEvent(const path &file_root, const path &file_ini, const path &file_online_rate);
        ~PhysicsEvent();

        int     LoadRootFile();
        int     LoadIniFile();
        int     LoadOnlineRate();

    protected:

    private:

};
//
// class IntEvent : public Event{
//
// };



#endif /* CLAWS_EVENT_H_ */
