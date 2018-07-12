/*
 * test.cxx
 *
 *  Created on: June 9, 2018
 *      Author: mgabriel
 */


//std includes
#include <iostream>

// boost
#include <boost/filesystem.hpp>

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

int main(int argc, char* argv[])
{

		cout << "---------------------------------------------------------" << endl;
		cout << "|            CLAWS phase I testbed                       |" << endl;
		cout << "---------------------------------------------------------" << endl;

		NTP_Handler* ntp_handler = new NTP_Handler(filesystem::path("/remote/ceph/group/ilc/claws/phase1/NTP/BEAST/v2"));

		double wall0 = claws::get_wall_time();
		double cpu0  = claws::get_cpu_time();

		//auto pv = ntp_handler->GetPV(10, "my pv");
		string pv = "SKB_LER_current";
		auto pv_val = ntp_handler->GetPV< vector<double>* >(1455.567000e6, pv);
		//double pv_val = ntp_handler->GetPV<double>(1455.567000e6, pv);
		auto pv_val2 = ntp_handler->GetPV< vector<double>* >(1455.567001e6, pv);

		cout << pv << ": " << (*pv_val)[0] << endl;
		cout << pv << "2: " << (*pv_val2)[0] << endl;

		double wall1 = claws::get_wall_time();
		double cpu1  = claws::get_cpu_time();

		cout << "Wall Time = " << wall1 - wall0 << endl;
		cout << "CPU Time  = " << cpu1  - cpu0  << endl;
		//cout << ntp_handler->TestMethode<double>() << endl;
}
