/*
 * ntp_handler.hh
 *
 *  Created on: June 9, 2018
 *      Author: mgabriel
 */

#ifndef CLAWS_NTP_HANDLER_H_
#define CLAWS_NTP_HANDLER_H_

// boost
// #include <boost/program_options.hpp>
 #include <boost/filesystem.hpp>

// ROOT includes
 #include <TFile.h>
 #include <TTree.h>

class NTP_Handler
{
public:
NTP_Handler(boost::filesystem::path search_path);
virtual ~NTP_Handler();

double GetPV(double ts, std::string pv);
double GetPVFromTree(double ts, std::string pv, TTree* tree);
std::vector<boost::filesystem::path> ntp_files;
TFile* rfile_;
TTree* ttree_;
// TBranch* br_ts_;
// TBranch* br_ts_;
};

#endif /* CLAWS_NTP_HANDLER_H_ */
