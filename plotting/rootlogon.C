
#include "TColor.h"
// Textwidth from latex in pt
// double textwidth = 418.25; // thesis
double textwidth = 495.0;

// Textsize from latex in pt;
// double textsize = 10; // Thesis
double textsize = 8; // Paper

double pt2pixel = 1.333;
double vis_multi = 2;

TColor* claws_black = new TColor(TColor::GetFreeColorIndex(), 0./255., 0./255.,0./255.,"CLAWS black");

TColor* claws_red   = new TColor(TColor::GetFreeColorIndex(), 221./255., 0./255.,0./255.,"CLAWS red");

TColor* claws_gray  = new TColor(TColor::GetFreeColorIndex(), 154./255., 154./255., 154./255.,"CLAWS gray");

TColor* claws_red2   = new TColor(TColor::GetFreeColorIndex(), 255./255., 0./255., 0./255.,"CLAWS red 2");

TColor* tab20_orange   = new TColor(TColor::GetFreeColorIndex(), 255./255., 127./255., 14./255.,"Tab 20 orange");

TColor* tab20_lorange   = new TColor(TColor::GetFreeColorIndex(), 255./255., 187./255., 120./255.,"Tab 20 orange");

TColor* tab20_blue   = new TColor(TColor::GetFreeColorIndex(), 31./255., 119./255., 180./255.,"Tab 20 blue");

TColor* tab20_lblue   = new TColor(TColor::GetFreeColorIndex(), 174./255., 199./255., 232./255.,"Ligth Tab 20 blue");

TColor* tab20_green   = new TColor(TColor::GetFreeColorIndex(), 44./255., 160./255., 44./255.,"Tab 20 green");

TColor* tab20_lila   = new TColor(TColor::GetFreeColorIndex(), 148./255., 103./255., 189./255.,"Tab 20 lila");

TColor* tab20_red   = new TColor(TColor::GetFreeColorIndex(), 214./255., 39./255., 40./255.,"Tab 20 lila");

TColor* tab20_lred   = new TColor(TColor::GetFreeColorIndex(), 255./255., 152./255., 150./255.,"Tab 20 light red");

TColor* tab20_cyan   = new TColor(TColor::GetFreeColorIndex(), 23./255., 190./255., 207./255.,"Tab 20 cyan");

TColor* tab20_magenta   = new TColor(TColor::GetFreeColorIndex(), 227./255., 119./255., 194./255.,"Tab 20 magenta");

TColor* tab20_gray   = new TColor(TColor::GetFreeColorIndex(), 127./255., 127./255., 127./255.,"Tab 20 gray");

TColor* tab20_lgray   = new TColor(TColor::GetFreeColorIndex(), 199./255., 199./255., 199./255.,"Tab 20 lgray");

TColor* tab20_yellow   = new TColor(TColor::GetFreeColorIndex(), 188./255., 189./255., 34./255.,"Tab 20 gray");

// TColor* tab20_gray   = new TColor(TColor::GetFreeColorIndex(), 127./255., 127./255., 127./255.,"Tab 20 gray");

TColor* cher = tab20_blue;
TColor* cler = tab20_red;
TColor* cfwd1 = tab20_orange;
TColor* cfwd2 = tab20_green;
TColor* cfwd3 = tab20_lila;

TGraph* set_line_width(TGraph* g, int width = 3)
{
    TGraph* clone = (TGraph*) g->Clone("clone");
    clone->SetLineWidth(width);
    clone->SetMarkerSize(2.6);
    return clone;
}

TH1D* set_line_width(TH1D* wf, int width = 3)
{
    TH1D* clone = (TH1D*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}

TH1F* set_line_width(TH1F* wf, int width = 3)
{
    TH1F* clone = (TH1F*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}

TH1I* set_line_width(TH1I* wf, int width = 3)
{
    TH1I* clone = (TH1I*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}

TF1* set_line_width(TF1* wf, int width = 3)
{
    TF1* clone = (TF1*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}


TArrow* set_line_width(TArrow* wf, int width = 3)
{
    TArrow* clone = (TArrow*) wf->Clone("clone");
    clone->SetLineWidth(width);
    return clone;
}

TH1D* make_box(TH1D* hist, string type = "FULL")
{
    TH1D* clone = (TH1D*) hist->Clone("clone");
    clone->SetLineWidth(1);

    // cout << "Linecolor: " << kBlack << endl;
    if(type == "FULL")
    {
          clone->SetFillColor(clone->GetLineColor());
    }
    else if(type == "EMPTY")
    {
        clone->SetFillStyle(0);
    //    clone->SetLineWidth(1);
    }
    // else
    // {

    //}

    return clone;
}

TH1F* make_box(TH1F* hist, string type = "FULL")
{
    TH1F* clone = (TH1F*) hist->Clone("clone");
    clone->SetLineWidth(1);

    // cout << "Linecolor: " << kBlack << endl;
    if(type == "FULL")
    {
          clone->SetFillColor(clone->GetLineColor());
    }
    else if(type == "EMPTY")
    {
        clone->SetFillStyle(0);
    //    clone->SetLineWidth(1);
    }
    // else
    // {

    //}

    return clone;
}

TH1I* make_box(TH1I* hist, string type = "FULL")
{
    TH1I* clone = (TH1I*) hist->Clone("clone");
    clone->SetLineWidth(1);

    // cout << "Linecolor: " << kBlack << endl;
    if(type == "FULL")
    {
          clone->SetFillColor(clone->GetLineColor());
    }
    else if(type == "EMPTY")
    {
        clone->SetFillStyle(0);
    //    clone->SetLineWidth(1);
    }
    // else
    // {

    //}

    return clone;
}

TGraphErrors* make_box(TGraphErrors* hist, string type = "FULL")
{
    TGraphErrors* clone = (TGraphErrors*) hist->Clone("clone");
    clone->SetLineWidth(1);

    // cout << "Linecolor: " << kBlack << endl;
    if(type == "FULL")
    {
          clone->SetFillColor(clone->GetLineColor());
    }
    else if(type == "EMPTY")
    {
        clone->SetFillStyle(0);
    //    clone->SetLineWidth(1);
    }
    // else
    // {

    //}

    return clone;
}

struct Target
{
    string filein;
    string fileout;
    string pathin;
    string pathout;
    string typein;
    string data = "";
    string ring = "";
    int n = -1;
    bool aligned = false;
};

Target GetTarget(string type = "")
{

        if(type == "HER-REF")
        {
            Target her_ref;
            her_ref.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_HER_Reference_Run_401141/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_ref.filein = "AnalysisEvent_run-401141_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            her_ref.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/HER_Injections";
            her_ref.fileout = "her_reference_run-401141";
            her_ref.typein = "HER_Reference_WAVEFORM_PEAK_FFT_INJ_HER";
            her_ref.data = "HER-REF";
            her_ref.ring = "HER";
            her_ref.n = 3000000;
                her_ref.ring = "HER";
            return her_ref;
        //    targets.push_back(her_ref);
        }
        else if(type == "HER-VS")
        {
            Target her_vert;
            her_vert.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_HER_VerticalSteering_Run_401143-401144/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_vert.filein = "AnalysisEvent_run-401143_401144_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            her_vert.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/HER_Injections";
            her_vert.fileout = "her_vertical_steering_run-401143_401144";
            her_vert.typein = "HER_VertSteering_WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_vert.data = "HER-VS";
            her_vert.ring = "HER";
            her_vert.n = 3000000;
                her_vert.ring = "HER";
             // targets.push_back(her_vert);
             return her_vert;
        }
        else if(type == "HER-VS1")
        {
            Target her_vert;
            her_vert.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_HER_VerticalSteering_1_Run_401143/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_vert.filein = "AnalysisEvent_run-401143_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            her_vert.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/HER_Injections";
            her_vert.fileout = "her_vertical_steering_1_run-401143";
            her_vert.typein = "HER_VertSteering_WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_vert.data = "HER-VS1";
            her_vert.ring = "HER";
            her_vert.n = 3000000;
                her_vert.ring = "HER";
             // targets.push_back(her_vert);
             return her_vert;
        }
        else if(type == "HER-VS2")
        {
            Target her_vert;
            her_vert.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_HER_VerticalSteering_2_Run_401144/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_vert.filein = "AnalysisEvent_run-401144_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            her_vert.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/HER_Injections";
            her_vert.fileout = "her_vertical_steering_2_run_401144";
            her_vert.typein = "HER_VertSteering_WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_vert.data = "HER-VS2";
            her_vert.ring = "HER";
            her_vert.n = 3000000;
                her_vert.ring = "HER";
             // targets.push_back(her_vert);
             return her_vert;
        }
        else if(type == "HER-PS")
        {
            Target her_ps;
            her_ps.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_HER_Phase_Shift_Run_401140/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            her_ps.filein = "AnalysisEvent_run-401140_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            her_ps.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/HER_Injections";
            her_ps.fileout = "her_phase_shift_run-401140";
            her_ps.typein = "HER_PhaseShift_WAVEFORM_PEAK_FFT_INJ_HERONLY";
            //her_ps.data = "HER Phase Shift (Run-401140)";
            her_ps.data = "HER-PS";
            her_ps.ring = "HER";
            her_ps.n = 3000000;
            her_ps.ring = "HER";
            return her_ps;
        //    targets.push_back(her_ps);
        }
        else if(type == "LER-REF")
        {
            Target ler_ref;
            ler_ref.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_LER_Reference_Run_401145/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            ler_ref.filein = "AnalysisEvent_run-401145_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            ler_ref.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/LER_Injections";
            ler_ref.fileout = "ler_reference_run-401145";
            ler_ref.typein = "LER_Reference_WAVEFORM_PEAK_FFT_INJ_LERONLY";
            ler_ref.data = "LER-REF";
            ler_ref.n = 3000000;
            ler_ref.ring = "LER";
            // targets.push_back(ler_ref);
            return ler_ref;
        }
        else if(type == "LER-PS")
        {
            Target ler_ps;
            ler_ps.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_LER_Phase_Shift_Run_401130-401132/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            ler_ps.filein = "AnalysisEvent_run-401130_401132_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            ler_ps.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/LER_Injections";
            ler_ps.fileout = "ler_phase_shift_run-401130_401132";
            ler_ps.typein = "LER_PhaseShift_WAVEFORM_PEAK_FFT_INJ_LERONLY";
            ler_ps.data = "LER-PS";
            ler_ps.n = 3000000;
            ler_ps.ring = "LER";
            // targets.push_back(ler_ps);
            return ler_ps;
        }
        else if(type == "LER-VS")
        {
            Target ler_vert;
            ler_vert.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_LER_Vertical_Steering_Run_401137/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            ler_vert.filein = "AnalysisEvent_run-401137_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            ler_vert.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/LER_Injections";
            ler_vert.fileout = "ler_vertical_steering_run-401137";
            ler_vert.typein = "LER_VertSteering_WAVEFORM_PEAK_FFT_INJ_LER";
            ler_vert.data = "LER-VS";
            ler_vert.ring = "LER";
            // targets.push_back(ler_vert);
            return ler_vert;
        }
        else if(type == "LER-SA")
        {
            Target ler_sep;
            ler_sep.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/INJSTUDY_LER_SeptumAngle_Run_401148/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            ler_sep.filein = "AnalysisEvent_run-401148_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            ler_sep.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/LER_Injections";
            ler_sep.fileout = "ler_septum_angle_run-401148";
            ler_sep.typein = "LER_SeptumAngle_WAVEFORM_PEAK_FFT_INJ_LER";
            ler_sep.data = "LER-SA";
            ler_sep.ring = "LER";
            // targets.push_back(ler_sep);
            return ler_sep;
        }
        else if(type == "NJ-VACS")
        {
            Target vacscrub_noinj;
            vacscrub_noinj.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/VS_FULL_INJ_NON_ALIGNED_3e6_HER_1_2000_LER_1_2000_Run_400412-401172/WAVEFORM_PEAK_FFT_INJ_NONE";
            vacscrub_noinj.filein = "AnalysisEvent_run-400412_401172_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            vacscrub_noinj.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/NO_Injections";
            vacscrub_noinj.fileout = "nj_vacs";
            vacscrub_noinj.typein = "VacuumScrubbing_INJNON_STATFULL";
            vacscrub_noinj.data = "NI-VACS";
            vacscrub_noinj.n = 3000000;
            vacscrub_noinj.ring = "-";
                vacscrub_noinj.aligned = true;

            return vacscrub_noinj;
            // targets.push_back(vacscrub_noinj);
        }
        else if(type == "VC_STAT_FULL_INJ_HER")
        {
            Target vacscrub_herinj;
            vacscrub_herinj.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/VS_STAT_FULL_INJ_HER_3e6_Run_400407-401195/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            vacscrub_herinj.filein = "AnalysisEvent_run-400407_401195_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            vacscrub_herinj.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/HER_Injections";
            vacscrub_herinj.fileout = "vs_stat_full_inj_her_run-400407-401195";
            vacscrub_herinj.typein = "VacuumScrubbing_INJHER_STATFULL";
            vacscrub_herinj.data = "Vacuum Scrubbing (Run-400407-401195)";
            vacscrub_herinj.n = 3000000;
            // targets.push_back(vacscrub_herinj);
            return vacscrub_herinj;
        }
        else if(type == "VC_STAT_FULL_INJ_LER")
        {
            Target vacscrub_lerinj;
            vacscrub_lerinj.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/VS_STAT_FULL_INJ_LER_3e6_Run_400408-401196/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            vacscrub_lerinj.filein = "AnalysisEvent_run-400408_401196_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            vacscrub_lerinj.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/LER_Injections";
            vacscrub_lerinj.fileout = "vs_stat_full_inj_ler_run-400408-401196";
            vacscrub_lerinj.typein = "VacuumScrubbing_INJLER_STATFULL";
            vacscrub_lerinj.data = "Vacuum Scrubbing (Run-400408-401196)";
            vacscrub_lerinj.n = 3000000;
            return vacscrub_lerinj;
        //    targets.push_back(vacscrub_lerinj);
        }
        else if(type == "VC_STAT_HER_INJ_NON_3e6_HER_5_LER_-5_5")
        {
            Target vacscrub_noinj;
            vacscrub_noinj.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/VS_STAT_HER_INJ_NON_3e6_HER_5_LER_-5_5_Run_400672-401163/WAVEFORM_PEAK_FFT_INJ_NONE";
            vacscrub_noinj.filein = "AnalysisEvent_run-400672_401163_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            vacscrub_noinj.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/NO_Injections";
            vacscrub_noinj.fileout = "vs_stat_her_inj_non_her_5_ler_-5_5_run-400412_401172";
            vacscrub_noinj.typein = "VC_STAT_HER_INJ_NON_3e6_HER_5_LER_-5_5";
            vacscrub_noinj.data = "Vacuum Scrubbing (Run-400412-401172)";
            vacscrub_noinj.n = 3000000;
            return vacscrub_noinj;
            // targets.push_back(vacscrub_noinj);
        }
        else if(type == "VC_STAT_LER_INJ_NON_3e6_HER_-5_5_LER_5")
        {
            Target vacscrub_noinj;
            vacscrub_noinj.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/VS_STAT_LER_INJ_NON_3e6_HER_-5_5_LER_5_Run_400850-401223/WAVEFORM_PEAK_FFT_INJ_NONE";
            vacscrub_noinj.filein = "AnalysisEvent_run-400850_401223_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            vacscrub_noinj.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/NO_Injections";
            vacscrub_noinj.fileout = "vs_stat_her_inj_non_her_5_ler_-5_5_run-400412_401172";
            vacscrub_noinj.typein = "VC_STAT_LER_INJ_NON_3e6_HER_-5_5_LER_5";
            vacscrub_noinj.data = "Vacuum Scrubbing (Run-400412-401172)";
            vacscrub_noinj.n = 3000000;
            return vacscrub_noinj;
            // targets.push_back(vacscrub_noinj);
        }
        else if(type == "LER-VACS")
        {
            Target vacscrub_noinj;
            vacscrub_noinj.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/VS_LER_INJ_LER_3e6_HER_-1_1_LER_1_2000_Run_400849-401218/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            vacscrub_noinj.filein = "AnalysisEvent_run-400849_401218_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            vacscrub_noinj.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/LER_Injections";
            vacscrub_noinj.fileout = "vs_ler_inj_ler_her_-1_1_ler_1_2000_run-400849-401218";
            vacscrub_noinj.typein = "VC_STAT_LER_INJ_NON_3e6_HER_-5_5_LER_5";
            vacscrub_noinj.data = "LER-VACS";
            vacscrub_noinj.n = 3000000;
            vacscrub_noinj.ring = "LER";
            return vacscrub_noinj;
            // targets.push_back(vacscrub_noinj);
        }
        else if(type == "HER-ALL")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALL_INJ_HER_3e6_HER_1_2000_LER_-1_1_Run_400407-401257/WAVEFORM_PEAK_FFT_INJ_HERONLY";
            target.filein = "AnalysisEvent_run-400407_401257_WAVEFORM_PEAK_FFT_INJ_HERONLY.root";
            // target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/HER_Injections";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/CLAWS/Phase1_Analysis_Paper/Images";
            target.fileout = "her_all_run-400407-401195";
            target.typein = "All injections HER Only";
            target.data = "HER-ALL";
            target.n = 3000000;
            target.ring = "HER";
            return target;
        //    targets.push_back(target);
        }
        else if(type == "LER-ALL")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALL_INJ_LER_3e6_HER_-1_1_LER_1_2000_Run_400842-401253/WAVEFORM_PEAK_FFT_INJ_LERONLY";
            target.filein = "AnalysisEvent_run-400842_401253_WAVEFORM_PEAK_FFT_INJ_LERONLY.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/CLAWS/Phase1_Analysis_Paper/Images";
            target.fileout = "ler_all_run-400408-401232";
            target.typein = "All injections LER Only";
            target.data = "LER-ALL";
            target.n = 3000000;
            target.ring = "LER";
            return target;
        //    targets.push_back(target);
        }
        else if(type == "NJ-ALL")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALL_INJ_NON_3e6_HER_1_2000_LER_1_2000_Run_400412-401258/WAVEFORM_PEAK_FFT_INJ_NONE";
            target.filein = "AnalysisEvent_run-400412_401258_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/NO_Injections";
            target.fileout = "all_inj_non_3e6_her_1_2000_ler_1_2000_run-400407-401195";
            target.typein = "NI-ALL";
            target.data = "NI-ALL";
            target.ring = "-";
            target.n = 3000000;
            return target;
        //    targets.push_back(target);
        }
        else if(type == "NJ-HER")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALL_INJ_NON_3e6_HER_1_2000_LER_-1_1_Run_400672-401258/WAVEFORM_PEAK_FFT_INJ_NONE";
            target.filein = "AnalysisEvent_run-400672_401258_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/NO_Injections";
            target.fileout = "all_inj_non_3e6_her_1_2000_ler_-1_1_run-400407-401195";
            target.typein = "NI-HER";
            target.data = "NI-HER";
            target.ring = "-";
            target.n = 3000000;
            return target;
        //    targets.push_back(target);
        }
        else if(type == "NJ-LER")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALL_INJ_NON_3e6_HER_-1_1_LER_1_2000_Run_400842-401254/WAVEFORM_PEAK_FFT_INJ_NONE";
            target.filein = "AnalysisEvent_run-400842_401254_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Injection_Results/Images/NO_Injections";
            target.fileout = "all_inj_non_her_-1_1_ler_1_2000_run-400407-401195";
            target.typein = "NI-LER";
            target.data = "NI-LER";
            target.ring = "-";
            target.n = 3000000;
            return target;
        //    targets.push_back(target);
        }
        else if(type == "ALIGNED_ALL_NON")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALIGNED_ALL_INJ_NON_3e6_HER_5_LER_5_Run_400412-401258/WAVEFORM_PEAK_FFT_INJ_NONE";
            target.filein = "AnalysisEvent_run-400412_401258_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/NO_Injections";
            target.fileout = "aligned_all_inj_non_3e6_her_5_ler_5_run-400412_401258";
            target.typein = "Aligned All NON injections";
            target.data = "";
            target.n = 3000000;
            target.aligned = true;
            return target;
        //    targets.push_back(target);
        }
        else if(type == "ALIGNED_VS_STAT_FULL_INJ_NON_125e5_HER_5_LER_5")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/ALIGNED_VS_STAT_FULL_INJ_NON_125e5_HER_5_LER_5_Run_400412-400968/WAVEFORM_PEAK_FFT_INJ_NONE";
            target.filein = "AnalysisEvent_run-400412_400968_WAVEFORM_PEAK_FFT_INJ_NONE.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/InjectionResults/Images/Timing";
            target.fileout = "aligned_vs_stat_full_inj_non_125e5_her_5_ler_5_run-400412_401258";
            target.typein = "ALIGNED_VS_STAT_FULL_INJ_NON_125e5_HER_5_LER_5";
            target.data = "";
            target.n = 12500000;
            target.aligned = true;
            return target;
        //    targets.push_back(target);
        }

        else if(type == "BEAMSIZE_HER")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/BEAMSIZE_HER_Run-400838-400841/SCATTER:BEAMSIZE:HER";
            target.filein = ":SCATTER:BEAMSIZE:HER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize";
            target.fileout = "her_beam_size_scan";
            target.typein = "HER Size Sweep";
            target.data = "";
            target.ring = "HER";

            return target;
        //    targets.push_back(target);
        }

        else if(type == "BEAMSIZE_HER_BELOW")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/BEAMSIZE_HER_BELOW_Run-400838-400841/SCATTER:BEAMSIZE:HER";
            target.filein = ":SCATTER:BEAMSIZE:HER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize";
            target.fileout = "her_below_beam_size_scan";
            target.typein = "HER Time Resolved Size Sweep";
            target.data = "";
            target.ring = "HER";

            return target;
        //    targets.push_back(target);
        }

        else if(type == "BEAMSIZE_HER_ABOVE")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/BEAMSIZE_HER_ABOVE_Run-400838-400841/SCATTER:BEAMSIZE:HER";
            target.filein = ":SCATTER:BEAMSIZE:HER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_ThesisBeamgas_Touschek/Images/BeamSize";
            target.fileout = "her_above_beam_size_scan";
            target.typein = "HER Time Resolved Size Sweep";
            target.data = "";
            target.ring = "HER";

            return target;
        //    targets.push_back(target);
        }

        else if(type == "BEAMSIZE_LER")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/BEAMSIZE_LER_Run-400848-400849/SCATTER:BEAMSIZE:LER";
            target.filein = ":SCATTER:BEAMSIZE:LER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize";
            target.fileout = "ler_beam_size_scan";
            target.typein = "LER Size Sweep";
            target.data = "";
            target.ring = "LER";

            return target;
        //    targets.push_back(target);
        }

        else if(type == "BEAMSIZE_LER_BELOW")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/BEAMSIZE_LER_BELOW_GM_Run-400848-400849/SCATTER:BEAMSIZE:LER";
            target.filein = ":SCATTER:BEAMSIZE:LER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize";
            target.fileout = "ler_below_beam_size_scan";
            target.typein = "LER Time Resolved Size Sweep";
            target.data = "";
            target.ring = "LER";

            return target;
        //    targets.push_back(target);
        }

        else if(type == "BEAMSIZE_LER_ABOVE")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/BEAMSIZE_LER_ABOVE_GM_Run-400848-400849/SCATTER:BEAMSIZE:LER";
            target.filein = ":SCATTER:BEAMSIZE:LER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Beamgas_Touschek/Images/BeamSize";
            target.fileout = "ler_above_beam_size_scan";
            target.typein = "LER Time Resolved Size Sweep";
            target.data = "";
            target.ring = "LER";

            return target;
        //    targets.push_back(target);
        }
        else if(type == "VacuumBump")
        {
            Target target;
            target.pathin = "/remote/ceph/group/ilc/claws/phase1/Analysis/SLOW_VB_LER_Run-401003-401006/SCATTER:VACUUM_BUMP2:LER";
            target.filein = ":SCATTER:VACUUM_BUMP2:LER.root";
            target.pathout = "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/SlowResults/Images/VacuumBump";
            target.fileout = "ler_vacuum_bump";
            target.typein = "Vacuum Bump";
            target.data = "";
            target.ring = "LER";

            return target;
        //    targets.push_back(target);
        }

        return Target();
}

// vector <Target> targets = GetTargets();

std::vector<std::string> GetDataSets()
{
    string arr[] = {"NJ-ALL", "NJ-HER", "NJ-LER", "NJ-VACS", "", "HER-ALL", "LER-ALL", "LER-VACS", "", "HER-REF" , "HER-PS" , "HER-VS1", "HER-VS2", "", "LER-REF", "LER-PS" , "LER-VS" , "LER-SA"};
    std::vector<std::string> vecOfStr(arr, arr + sizeof(arr)/sizeof(std::string));
    return vecOfStr;
}
void rootlogon()
{
  // Load ATLAS style
  gROOT->LoadMacro("phd_style.C");



 // Beast note
 //TColor* claws_red   = new TColor(1501, 215./255., 39./255.,41./255.,"CLAWS red");
 // TColor* claws_gray  = new TColor(1502, 160./255., 160./255.,160./255.,"CLAWS gray");
 // Beast note
 // TColor* claws_gray  = new TColor(1502, 165./255., 172./255.,175./255.,"CLAWS gray");

}


// {// Add my own options here:
//     printf("\nSetting up MyPhDStyle!\n");
//
//     TStyle* phd_style = new TStyle("phd_style","Style for Miro's phd thesis.");
//
//     // // phd_style->SetPalette(1,0); // avoid horrible default color scheme
//     // phd_style->SetOptStat(0);
//     // phd_style->SetOptTitle(0);
//     // phd_style->SetOptDate(0);
//     //
//     // // // --- Fonts ---
//     //
//     // //Int_t font=72; // Helvetica italics
//     // Int_t font=62; // Helvetica
//     // Double_t tsize=0.05;
//     // phd_style->SetTextFont(font);
//     //
//     // phd_style->SetLabelFont(font,"xyz");
//     // phd_style->SetTitleFont(font,"xyz");
//     //
//     // phd_style->SetTextSize(tsize);
//     //
//     // phd_style->SetLabelSize(tsize,"xyz");
//     // phd_style->SetTitleSize(tsize,"xyz");
//     //
//     // // phd_style->SetLabelSize(0.03,"xyz"); // size of axis value font
//     // // phd_style->SetTitleSize(0.035,"xyz"); // size of axis title font
//     // // phd_style->SetTitleFont(22,"xyz"); // font option
//     // // phd_style->SetLabelFont(22,"xyz");
//     // // phd_style->SetTitleOffset(1.2,"y");
//     // //
//     // // // --- default canvas options ---
//     // // phd_style->SetCanvasDefW(600);
//     // // phd_style->SetCanvasDefH(600);
//     //
//     // // --- Pad stuff ---
//     // phd_style->SetPadTopMargin(0.05);
//     // phd_style->SetPadRightMargin(0.05);
//     // phd_style->SetPadBottomMargin(0.16);
//     // phd_style->SetPadLeftMargin(0.16);
//     //
//     // phd_style->SetPadTickX(1);
//     // phd_style->SetPadTickY(1);
//     //
//     // // use plain black on white colors
//     // Int_t icol=0; // WHITE
//     // phd_style->SetFrameBorderMode(icol);
//     // phd_style->SetFrameFillColor(icol);
//     // phd_style->SetCanvasBorderMode(icol);
//     // phd_style->SetCanvasColor(icol);
//     // phd_style->SetPadBorderMode(icol);
//     // phd_style->SetPadColor(icol);
//     // phd_style->SetStatColor(icol);
//     //
//     //
//     // // --- Axis and titles ---
//     // phd_style->SetTitleXOffset(1.4);
//     // phd_style->SetTitleYOffset(1.4);
//     //
//     //
//     // // --- Standard hist and graph options ---
//     // phd_style->SetMarkerStyle(20);
//     // phd_style->SetMarkerSize(1.2);
//     // phd_style->SetHistLineWidth(2.);
//     // phd_style->SetLineStyleString(2,"[12 12]"); // postscript dashes
//
//
//     // phd_style->SetCanvasColor(0); // canvas...
//     // phd_style->SetCanvasBorderMode(0);
//     // phd_style->SetCanvasBorderSize(0);
//     // phd_style->SetPadBottomMargin(0.1); //margins...
//     // phd_style->SetPadTopMargin(0.1);
//     // phd_style->SetPadLeftMargin(0.1);
//     // phd_style->SetPadRightMargin(0.1);
//     // phd_style->SetPadGridX(0); // grids, tickmarks
//     // phd_style->SetPadGridY(0);
//     // phd_style->SetPadTickX(1);
//     // phd_style->SetPadTickY(1);
//     // phd_style->SetFrameBorderMode(0);
//     //phd_style->SetPaperSize(20,24); // US letter size
//
//     //cout << "Styles are Set!" << endl;
//
//     return;
// }
