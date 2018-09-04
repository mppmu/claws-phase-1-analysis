/*
 * test.cxx
 *
 *  Created on: June 9, 2018
 *      Author: mgabriel
 */


//std includes
#include <iostream>
#include <stdio.h>
// boost
//#include <boost/filesystem.hpp>

// Project includes
#include "ntp_handler.hh"
#include "globalsettings.hh"

using namespace std;
using namespace boost;
// template <class T>
// ostream& operator<<(ostream& os, const vector<T>& v)
// {
//      copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
//      return os;
// }
void func2()
{
		int count = 0;
		for(count=0; count < 0XFFFFF; count++) ;

		return;
}

void func1(void)
{
		int count = 0;
		for(count=0; count < 0XFF; count++)
				func2();

		return;
}

int main(int argc, char* argv[])
{

		cout << "---------------------------------------------------------" << endl;
		cout << "|            CLAWS phase I testbed                       |" << endl;
		cout << "---------------------------------------------------------" << endl;

		NTP_Handler* ntp_handler = new NTP_Handler(filesystem::path("/remote/pcilc3/RaidGroup/claws/data/phase1/NTP/BEAST/v3"));
		//
		double wall0 = claws::get_wall_time();
		double cpu0  = claws::get_cpu_time();

		//auto pv = ntp_handler->GetPV(10, "my pv");

		double ts = 1467052259.9273696;
		string pv1 = "SKB_LER_current";
		string pv3 = "SKB_HER_current";
		string pv2 = "SKB_LER_integratedCurrent";
		auto pv_val = ntp_handler->GetPV< vector<double>* >(ts, pv1);
		cout << pv1 << ": " << (*pv_val)[0] << endl;
		//double pv_val = ntp_handler->GetPV<double>(1455.567000e6, pv);
		auto pv_val2 = ntp_handler->GetPV< vector<double>* >(ts, pv2);
		cout << pv2 << "2: " << (*pv_val2)[0] << endl;
		auto pv_val3 = ntp_handler->GetPV< vector<double>* >(ts, pv3);



		cout << pv3 << "3: " << (*pv_val3)[0] << endl;

		double wall1 = claws::get_wall_time();
		double cpu1  = claws::get_cpu_time();
		cout << "Wall Time = " << wall1 - wall0 << endl;
		cout << "CPU Time  = " << cpu1  - cpu0  << endl;
		// cout << "Wall Time = " << 0 << endl;
		// cout << "CPU Time  = " << 0  << endl;
		//cout << ntp_handler->TestMethode<double>() << endl;

		// printf("\n Hello World! \n");
		// func1();
		// func2();
		return 0;
}
