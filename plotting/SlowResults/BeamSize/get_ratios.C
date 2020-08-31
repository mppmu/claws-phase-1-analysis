

void get_ratios()
{


    SetPhDStyle();

    TChain *ch_skims = new TChain("tout");
    ch_skims->Add("SlowResults/BeamSize/TouschekSkims_data.root");

    TChain *ch_mc = new TChain("truth");
    ch_mc->Add("SlowResults/BeamSize/mc_v7.3.root");


    double I_SAD = 1000.;
    double Z_SAD_LER = 1.;
    double Z_SAD_HER = 2.7;
    double sigma_y_SAD_HER = 59;
    double sigma_y_SAD_LER = 110;
    double P_SAD = 0.00000133322; // 10 nTorr in Pa (from paper)

    double mc_factor = 1;
    double brem_factor    = 0.935;  // f_B(Z)/f_B(Z_SAD) for Z=2.7
    double coulomb_factor = 1.0;    // f_C(Z)/f_B(Z_SAD) for Z=2.7

    // Things to hold data
    vector<double> O_bg_data_LER, O_bg_e_data_LER, O_T_data_LER, O_T_e_data_LER;


    // Things to hold mc
    std::vector<double> O_bg_MC_LER, O_T_MC_LER;
    std::vector<double> O_bg_MC_HER, O_T_MC_HER;

    std::string correction_factor = "1";

    string chs[] = {"FWD1","FWD2","FWD3"};
    //    string chs[] = {"FWD1"};
        // Target target = GetTarget("BEAMSIZE_LER");
    for(int iCh = 0; iCh< 3 ; iCh++)
    {

        std::string times = "*";
        std::string divide = "/";
        std::string vs = ":";
        std::string squared = "^2";

        {
            // find the LER data observable
            std::stringstream plot_ss, cut_ss;
            cut_ss << "ler==1";

            // First get the pressure ratios
            std::stringstream pressure_ratio_ss;
            pressure_ratio_ss << "CLW_N_MIPs_offline_pressure_ratio[" << iCh << "]";

            int n = ch_skims->Draw(pressure_ratio_ss.str().c_str(),cut_ss.str().c_str(),"NODRAW");
            double *v0 = ch_skims->GetV1();
            double P_ratio = v0[0];
        
            std::cout << "  LER pressure ratio: " << pressure_ratio_ss.str() << " " << P_ratio << std::endl;
            //cout << "Pressure ratio for " << i << " : " << P_ratio << endl;
        //
            std::stringstream S_T_ss, S_Te_ss, S_bg_ss, S_bge_ss;
            S_bg_ss  <<  "CLW_N_MIPs_offline_offset[" << iCh << "]";
            S_bge_ss <<  "CLW_N_MIPs_offline_offset_e[" << iCh << "]";
            S_T_ss   <<  "CLW_N_MIPs_offline_slope[" << iCh << "]";
            S_Te_ss  <<  "CLW_N_MIPs_offline_slope_e[" << iCh << "]";
            plot_ss << S_bg_ss.str() << vs << S_bge_ss.str() << vs << S_T_ss.str() << vs << S_Te_ss.str();

            n = ch_skims->Draw(plot_ss.str().c_str(),cut_ss.str().c_str(),"NODRAW");
            double *v1 = ch_skims->GetV1();
            double *v2 = ch_skims->GetV2();
            double *v3 = ch_skims->GetV3();
            double *v4 = ch_skims->GetV4();

            double O_bg   = v1[0]*I_SAD*P_SAD*P_ratio*Z_SAD_LER*Z_SAD_LER;
            double O_T    = v3[0]*I_SAD*I_SAD/sigma_y_SAD_LER;
            double O_bg_e = v2[0]*I_SAD*P_SAD*P_ratio*Z_SAD_LER*Z_SAD_LER;
            double O_T_e  = v4[0]*I_SAD*I_SAD/sigma_y_SAD_LER;

            std::cout << "  LER fit results : " << std::endl;
            std::cout << "    S_bg:   " << v1[0] << " +/- " << v2[0] << std::endl;
            std::cout << "    S_T:    " << v3[0] <<  " +/- " << v4[0]<< std::endl;

            std::cout << "  LER data: " << std::endl;
            std::cout << "    O_bg:   " << O_bg << " +/- " << O_bg_e << std::endl;
            std::cout << "    O_T:    " << O_T <<  " +/- " << O_T_e << std::endl;
            O_bg_data_LER.push_back(O_bg);
            O_bg_e_data_LER.push_back(O_bg_e);
            O_T_data_LER.push_back(O_T);
            O_T_e_data_LER.push_back(O_T_e);
        }

        {
            // Find the LER mc observable
            double sum_lb = 0, sum_lc = 0, sum_lt = 0;

            for(unsigned int iSec = 0; iSec<12; ++iSec)
            {
                std::stringstream plot_ss, cut_ss;
                std::stringstream O_brem_ss, O_coulomb_ss, O_T_ss;

                O_brem_ss    << "MC_LB_CLAWS_rate" << "_" << iSec << "[" << iCh << "]" << divide << correction_factor;
                O_coulomb_ss << "MC_LC_CLAWS_rate" << "_" << iSec << "[" << iCh << "]" << divide << correction_factor;
                O_T_ss       << "MC_LT_CLAWS_rate" << "[" << iCh << "]" << divide << correction_factor;

                plot_ss << O_brem_ss.str() << vs << O_coulomb_ss.str() << vs << O_T_ss.str();
                cut_ss << "Entry$==0";

                //	  std::cout << plot_ss.str() << std::endl;

                ch_mc->Draw(plot_ss.str().c_str(),cut_ss.str().c_str(),"NODRAW");
                double *v1 = ch_mc->GetV1();
                double *v2 = ch_mc->GetV2();
                double *v3 = ch_mc->GetV3();

                sum_lb += v1[0]*mc_factor*brem_factor;
                sum_lc += v2[0]*mc_factor*coulomb_factor;
                sum_lt  = v3[0]*mc_factor; // Touschek is not separated by section

            } // END for iSec

            std::cout << "  LER MC: " << std::endl;
            std::cout << "    O_bg:   " << sum_lb+sum_lc << std::endl;
            std::cout << "    O_T:    " << sum_lt << std::endl;

            O_bg_MC_LER.push_back(sum_lb+sum_lc);
            O_T_MC_LER.push_back(sum_lt);
        }





        // O_bg_data_LER.push_back(O_bg);
        // O_bg_e_data_LER.push_back(O_bg_e);
        // O_T_data_LER.push_back(O_T);
        // O_T_e_data_LER.push_back(O_T_e);

    }


}
