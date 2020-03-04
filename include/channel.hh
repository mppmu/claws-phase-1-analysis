//============================================================================
// Name        : channel.hh
// Author      : Miroslav Gabriel
// Version     :
// Created on  : Apr 7, 2016
// Copyright   : GNU General Public License
// Description :
//============================================================================

#ifndef CLAWS_CHANNEL_H_
#define CLAWS_CHANNEL_H_

// --- C++ includes ---
#include <tuple>

// --- ROOT includes ---
#include <TH1.h>
#include <TH1I.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <TGraphErrors.h>

//----------------------------------------------------------------------------------------------
// TODO(mgabriel@mpp.mpg.de): Add some description
//----------------------------------------------------------------------------------------------

/**
 *
 * @param  file [description]
 * @return      [description]
 */

enum ChannelState {
		CHANNELSTATE_VALID,
		CHANNELSTATE_PDFAILED,
		CHANNELSTATE_FAILED,
};

/** Usually definitions should go into the .cxx file because otherwise
 *  the complier complains about multiple definitions. To avoid this
 *  the definition is inlined.
 */

inline std::string printChannelState(ChannelState state)
{
		switch (state) {
		case CHANNELSTATE_VALID:
				return "valid";

		case CHANNELSTATE_PDFAILED:
				return "pdfailed";

		case CHANNELSTATE_FAILED:
				return "failed";

		default:
				return "Invalid Selection";
		}
}

class Channel
{
/**
 * [for description]
 * @param  pd_ [ 0: Fit status
 *               1: Fit par constant
 *               2: Fit par mean
 *               3: Fit mean error
 *               4: Fit par sigma
 *               5: Fit chi2
 *               6: Fit NDF
 *               7: Fit p-value
 *               8: Hist mean
 *               9: Hist mean error
 *               10: Hist entries ]
 *
 */

		public:

				Channel(std::string name = "");
				virtual ~Channel();

				virtual void LoadHistogram(TFile *file);
				virtual void PrepareHistogram(double range = -1);
				virtual void DeleteHistogram();

				virtual void FillPedestal() = 0;
				virtual void SubtractPedestal(double pd = -1000);

				virtual std::string GetName();
				virtual TH1 *GetHistogram(std::string type = "waveform");
				virtual double *GetPedestal();
				virtual ChannelState GetState();
				virtual std::string GetScopePos();

				void SetName(std::string name);

		protected:

				std::string name_;
				ChannelState state_;
				TH1F *wf_;
				TH1I *pdhist_;
				double pd_[11];

				double range_;
				std::string scope_pos_;
};

class CalibrationChannel : public Channel
{
		public:

				CalibrationChannel(std::string name, std::string scope_pos);
				virtual ~CalibrationChannel();

				virtual void LoadHistogram(TFile *file);
				virtual void FillPedestal();

		protected:
				int scope_;
				std::string channel_;
};


struct OverShootResult {

		double lstart = -1;
		double lstop = -1;
		double lresult = -1;
		double start = -1;
		double stop = -1;
		int result = -1;
		double par0 = -1;
		double par1 = -1;
		double par2 = -1;
		double chi2 = -1;
		int ndf = -1;
		double pval = -1;
		int n = 0;
		double area1 = 0;
		double area2 = 0;
};

struct Rate {
		Rate()
		{
				name = "";
				online = -1;
				fast = -1;
				rate = -1;
				staterr = -1;
				syserr = -1;
				err = -1;
		}

		std::string name;
		double online;
		double fast;
		double rate;
		double staterr;
		double syserr;
		double err;
};

class PhysicsChannel : public Channel
{
		public:

				PhysicsChannel(std::string ch_name, std::string scope_pos);
				virtual ~PhysicsChannel();

				virtual void LoadHistogram(TFile *rfile, std::vector<std::string> types = { "wf" });
				virtual void DeleteHistogram();

				virtual void PrepareHistogram(double range, double offset = 0.);
				virtual void FillPedestal();

				virtual std::vector<OverShootResult> OverShootCorrection();

				virtual double FastRate(TH1F *avg, double unixtime);

				virtual void PrepareTagging();
				virtual void SignalTagging();

				virtual void PrepareDecomposition();
				virtual void WaveformDecomposition(TH1F *avg);
				virtual void SubtractWaveform(int start, TH1F *avg);
				int GetMaximumBin(double threshold, int methode, double fwhm, int stop_region, int first = 1, int last = -1);
				bool FWHM(int bin, int methode, double fwhm);
				bool Hood(int bin, int stop_region);
				void WaveformDecomposition2(TH1F *avg);

				virtual std::vector<double> WaveformReconstruction(TH1F *avg);

				virtual void PrepareRetrieval();
				virtual void MipTimeRetrieval(double unixtime = -1.);

				virtual double *GetOS();
				virtual TH1 *GetHistogram(std::string type);

				virtual Rate GetRate();


		private:
				double os_[10];

				TH1F *recowf_;
				TH1I *pewf_;
				TH1F *mipwf_;
				TH1F *mipwf_stat_;
				TH1F *mipwf_sys_;

				Rate rate_;

};

struct AnalysisChannel {
		~AnalysisChannel()
		{
				if (wf) delete wf; if (wf_stat) delete wf_stat; if (wf_sys) delete wf_sys; if (peak) delete peak; if (fft_real_h) delete fft_real_h; if (fft_img_h) delete fft_img_h; if (fft_mag_h) delete fft_mag_h; if (fft_phase_h) delete fft_phase_h; if (hit_map) delete hit_map; if (hit_energy) delete hit_energy;
				if (bunch_hit_energy) delete bunch_hit_energy; if (bunch_hit_energy_1) delete bunch_hit_energy_1; if (bunch_hit_energy_2) delete bunch_hit_energy_2;
				if (hit_energy_sync) delete hit_energy_sync; if (hit_energy_mip) delete hit_energy_mip; if (time_in_turn) delete time_in_turn; if (rate_in_turn) delete rate_in_turn; if (nhits_time) delete nhits_time;
				if (nhits_bunch) delete nhits_bunch; if (nhits_bunch_1) delete nhits_bunch_1; if (nhits_bunch_2) delete nhits_bunch_2;
				if (e_time) delete e_time; if (e_bunch) delete e_bunch; if (e_bunch_1) delete e_bunch_1; if (e_bunch_2) delete e_bunch_2;
				if (e_bunch_5mus) delete e_bunch_5mus; if (e_bunch_5mus_1) delete e_bunch_5mus_1; if (e_bunch_5mus_2) delete e_bunch_5mus_2;
				if (nhits_per_bunch) delete nhits_per_bunch; if (nhits_per_bunch_1) delete nhits_per_bunch_1; if (nhits_per_bunch_2) delete nhits_per_bunch_2;
				if (energy_per_bunch) delete energy_per_bunch; if (energy_per_bunch_1) delete energy_per_bunch_1; if (energy_per_bunch_2) delete energy_per_bunch_2;
				if (t_enery_share) delete t_enery_share; if (t_hits_share) delete t_hits_share;
				if (t_enery_bunch_share) delete t_enery_bunch_share; if (t_enery_bunch_share_1) delete t_enery_bunch_share_1; if (t_enery_bunch_share_2) delete t_enery_bunch_share_2;
				if (t_hits_bunch_share) delete t_hits_bunch_share; if (t_hits_bunch_share_1) delete t_hits_bunch_share_1; if (t_hits_bunch_share_2) delete t_hits_bunch_share_2;
				if (t_hits_bunch_belowX) delete t_hits_bunch_belowX; if (t_hits_bunch_belowX_1) delete t_hits_bunch_belowX_1; if (t_hits_bunch_belowX_1) delete t_hits_bunch_belowX_2;
				if (t_energy_bunch_belowX) delete t_energy_bunch_belowX; if (t_energy_bunch_belowX_1) delete t_energy_bunch_belowX_1; if (t_energy_bunch_belowX_2) delete t_energy_bunch_belowX_2;
				if (rate_vs_injcharge_her) delete rate_vs_injcharge_her; if (rate_vs_injcharge_ler) delete rate_vs_injcharge_ler; if (hits_vs_injcharge_her) delete hits_vs_injcharge_her; if (hits_vs_injcharge_ler) delete hits_vs_injcharge_ler; if (energy_vs_injcharge_her) delete energy_vs_injcharge_her; if (energy_vs_injcharge_ler) delete energy_vs_injcharge_ler;
				if (hitsb_vs_injcharge_her) delete hitsb_vs_injcharge_her; if (hitsb_vs_injcharge_ler) delete hitsb_vs_injcharge_ler; if (hitsb_vs_injcharge_ler_1) delete hitsb_vs_injcharge_ler_1; if (hitsb_vs_injcharge_ler_2) delete hitsb_vs_injcharge_ler_2;
				if (energyb_vs_injcharge_her) delete energyb_vs_injcharge_her; if (energyb_vs_injcharge_ler) delete energyb_vs_injcharge_ler; if (energyb_vs_injcharge_ler_1) delete energyb_vs_injcharge_ler_1; if (energyb_vs_injcharge_ler_2) delete energyb_vs_injcharge_ler_2;
				if (rate_vs_vertical_steering_1_her) delete rate_vs_vertical_steering_1_her; if (rate_vs_vertical_steering_1_ler) delete rate_vs_vertical_steering_1_ler; if (hits_vs_vertical_steering_1_her) delete hits_vs_vertical_steering_1_her; if (hits_vs_vertical_steering_1_ler) delete hits_vs_vertical_steering_1_ler; if (energy_vs_vertical_steering_1_her) delete energy_vs_vertical_steering_1_her; if (energy_vs_vertical_steering_1_ler) delete energy_vs_vertical_steering_1_ler;
				if (hitsb_vs_vertical_steering_1_her) delete hitsb_vs_vertical_steering_1_her; if (hitsb_vs_vertical_steering_1_ler) delete hitsb_vs_vertical_steering_1_ler; if (hitsb_vs_vertical_steering_1_ler_1) delete hitsb_vs_vertical_steering_1_ler_1; if (hitsb_vs_vertical_steering_1_ler_2) delete hitsb_vs_vertical_steering_1_ler_2;
				if (energyb_vs_vertical_steering_1_her) delete energyb_vs_vertical_steering_1_her; if (energyb_vs_vertical_steering_1_ler) delete energyb_vs_vertical_steering_1_ler; if (energyb_vs_vertical_steering_1_ler_1) delete energyb_vs_vertical_steering_1_ler_1; if (energyb_vs_vertical_steering_1_ler_2) delete energyb_vs_vertical_steering_1_ler_2;
				if (rate_vs_vertical_steering_2_her) delete rate_vs_vertical_steering_2_her; if (rate_vs_vertical_steering_2_ler) delete rate_vs_vertical_steering_2_ler; if (hits_vs_vertical_steering_2_her) delete hits_vs_vertical_steering_2_her; if (hits_vs_vertical_steering_2_ler) delete hits_vs_vertical_steering_2_ler; if (energy_vs_vertical_steering_2_her) delete energy_vs_vertical_steering_2_her; if (energy_vs_vertical_steering_2_ler) delete energy_vs_vertical_steering_2_ler;
				if (hitsb_vs_vertical_steering_2_her) delete hitsb_vs_vertical_steering_2_her; if (hitsb_vs_vertical_steering_2_ler) delete hitsb_vs_vertical_steering_2_ler; if (hitsb_vs_vertical_steering_2_ler_1) delete hitsb_vs_vertical_steering_2_ler_1; if (hitsb_vs_vertical_steering_2_ler_2) delete hitsb_vs_vertical_steering_2_ler_2;
				if (energyb_vs_vertical_steering_2_her) delete energyb_vs_vertical_steering_2_her; if (energyb_vs_vertical_steering_2_ler) delete energyb_vs_vertical_steering_2_ler; if (energyb_vs_vertical_steering_2_ler_1) delete energyb_vs_vertical_steering_2_ler_1; if (energyb_vs_vertical_steering_2_ler_2) delete energyb_vs_vertical_steering_2_ler_2;
				if (hitsb_vs_injeff_her) delete hitsb_vs_injeff_her; if (hitsb_vs_injeff_ler) delete hitsb_vs_injeff_ler; if (hitsb_vs_injeff_ler_1) delete hitsb_vs_injeff_ler_1; if (hitsb_vs_injeff_ler_2) delete hitsb_vs_injeff_ler_2;
				if (energyb_vs_injeff_her) delete energyb_vs_injeff_her; if (energyb_vs_injeff_ler) delete energyb_vs_injeff_ler; if (energyb_vs_injeff_ler_1) delete energyb_vs_injeff_ler_1; if (energyb_vs_injeff_ler_2) delete energyb_vs_injeff_ler_2;
				if (hitsb_vs_current_her) delete hitsb_vs_current_her; if (hitsb_vs_current_ler) delete hitsb_vs_current_ler; if (hitsb_vs_current_ler_1) delete hitsb_vs_current_ler_1; if (hitsb_vs_current_ler_2) delete hitsb_vs_current_ler_2;
				if (energyb_vs_current_her) delete energyb_vs_current_her; if (energyb_vs_current_ler) delete energyb_vs_current_ler; if (energyb_vs_current_ler_1) delete energyb_vs_current_ler_1; if (energyb_vs_current_ler_2) delete energyb_vs_current_ler_2;
				if (t_energy_bunch_belowX_vs_current_her) delete t_energy_bunch_belowX_vs_current_her; if (t_energy_bunch_belowX_vs_current_ler) delete t_energy_bunch_belowX_vs_current_ler; if (t_energy_bunch_belowX_vs_current_ler_1) delete t_energy_bunch_belowX_vs_current_ler_1; if (t_energy_bunch_belowX_vs_current_ler_2) delete t_energy_bunch_belowX_vs_current_ler_2;
				if (t_energy_bunch_belowX_vs_injeff_her) delete t_energy_bunch_belowX_vs_injeff_her; if (t_energy_bunch_belowX_vs_injeff_ler) delete t_energy_bunch_belowX_vs_injeff_ler; if (t_energy_bunch_belowX_vs_injeff_ler_1) delete t_energy_bunch_belowX_vs_injeff_ler_1; if (t_energy_bunch_belowX_vs_injeff_ler_2) delete t_energy_bunch_belowX_vs_injeff_ler_2;
				if (t_energy_bunch_belowX_vs_injcharge_her) delete t_energy_bunch_belowX_vs_injcharge_her; if (t_energy_bunch_belowX_vs_injcharge_ler) delete t_energy_bunch_belowX_vs_injcharge_ler; if (t_energy_bunch_belowX_vs_injcharge_ler_1) delete t_energy_bunch_belowX_vs_injcharge_ler_1; if (t_energy_bunch_belowX_vs_injcharge_ler_2) delete t_energy_bunch_belowX_vs_injcharge_ler_2;
				if (t_energy_bunch_belowX_vs_vertical_steering_1_her) delete t_energy_bunch_belowX_vs_vertical_steering_1_her; if (t_energy_bunch_belowX_vs_vertical_steering_1_ler) delete t_energy_bunch_belowX_vs_vertical_steering_1_ler; if (t_energy_bunch_belowX_vs_vertical_steering_1_ler_1) delete t_energy_bunch_belowX_vs_vertical_steering_1_ler_1; if (t_energy_bunch_belowX_vs_vertical_steering_1_ler_2) delete t_energy_bunch_belowX_vs_vertical_steering_1_ler_2;
				if (t_energy_bunch_belowX_vs_vertical_steering_2_her) delete t_energy_bunch_belowX_vs_vertical_steering_2_her; if (t_energy_bunch_belowX_vs_vertical_steering_2_ler) delete t_energy_bunch_belowX_vs_vertical_steering_2_ler; if (t_energy_bunch_belowX_vs_vertical_steering_2_ler_1) delete t_energy_bunch_belowX_vs_vertical_steering_2_ler_1; if (t_energy_bunch_belowX_vs_vertical_steering_2_ler_2) delete t_energy_bunch_belowX_vs_vertical_steering_2_ler_2;
				if (energy) delete energy; if (energy_bunch) delete energy_bunch; if (energy_bunch_1) delete energy_bunch_1; if (energy_bunch_2) delete energy_bunch_2;
		}

		std::string name = "";
		TH1F *          wf = nullptr;
		TH1F *          wf_stat = nullptr;
		TH1F *          wf_sys = nullptr;
		TH1F *          peak = nullptr;
		TH1F *          fft_real_h = nullptr;
		TH1F *          fft_img_h = nullptr;
		TH1F *          fft_mag_h = nullptr;
		TH1F *          fft_phase_h = nullptr;
		long n = 0;
		TH2F *          hit_map = nullptr;
		TH1F *          hit_energy = nullptr;

		TH1F *          bunch_hit_energy = nullptr;

		TH1F *          hit_energy_sync = nullptr;
		TH1F *          hit_energy_mip = nullptr;
		TH2F *          time_in_turn = nullptr;
		TH1F *          rate_in_turn = nullptr;

		TH1F *          nhits_time = nullptr;
		TH1F *          nhits_bunch = nullptr;

		TH1F *          nhits_per_bunch = nullptr;
		TH1F *          energy_per_bunch = nullptr;

		TH1F *          e_time = nullptr;
		TH1F *          e_bunch = nullptr;

		TH1F *          t_enery_share = nullptr;
		TH1F *          t_hits_share = nullptr;

		TH1F *          t_enery_bunch_share = nullptr;
		TH1F *          t_hits_bunch_share = nullptr;

		TH1F *          t_hits_bunch_belowX = nullptr;
		TH1F *          t_energy_bunch_belowX = nullptr;

		TGraphErrors *  rate_vs_injcharge_her = nullptr;
		TGraphErrors *  rate_vs_injcharge_ler = nullptr;

		TGraph *        hits_vs_injcharge_her = nullptr;
		TGraph *        hits_vs_injcharge_ler = nullptr;

		TGraph *        hitsb_vs_injcharge_her = nullptr;
		TGraph *        hitsb_vs_injcharge_ler = nullptr;

		TGraph *        energy_vs_injcharge_her = nullptr;
		TGraph *        energy_vs_injcharge_ler = nullptr;

		TGraph *        energyb_vs_injcharge_her = nullptr;
		TGraph *        energyb_vs_injcharge_ler = nullptr;

		TGraph *        energy = nullptr;
		TGraph *        energy_bunch = nullptr;
		TGraph *        energy_bunch_1 = nullptr;
		TGraph *        energy_bunch_2 = nullptr;

		TGraphErrors *  rate_vs_vertical_steering_1_her = nullptr;
		TGraphErrors *  rate_vs_vertical_steering_1_ler = nullptr;

		TGraph *        hits_vs_vertical_steering_1_her = nullptr;
		TGraph *        hits_vs_vertical_steering_1_ler = nullptr;

		TGraph *        hitsb_vs_vertical_steering_1_her = nullptr;
		TGraph *        hitsb_vs_vertical_steering_1_ler = nullptr;

		TGraph *        energy_vs_vertical_steering_1_her = nullptr;
		TGraph *        energy_vs_vertical_steering_1_ler = nullptr;

		TGraph *        energyb_vs_vertical_steering_1_her = nullptr;
		TGraph *        energyb_vs_vertical_steering_1_ler = nullptr;


		TGraphErrors *  rate_vs_vertical_steering_2_her = nullptr;
		TGraphErrors *  rate_vs_vertical_steering_2_ler = nullptr;

		TGraph *        hits_vs_vertical_steering_2_her = nullptr;
		TGraph *        hits_vs_vertical_steering_2_ler = nullptr;

		TGraph *        hitsb_vs_vertical_steering_2_her = nullptr;
		TGraph *        hitsb_vs_vertical_steering_2_ler = nullptr;

		TGraph *        energy_vs_vertical_steering_2_her = nullptr;
		TGraph *        energy_vs_vertical_steering_2_ler = nullptr;

		TGraph *        energyb_vs_vertical_steering_2_her = nullptr;
		TGraph *        energyb_vs_vertical_steering_2_ler = nullptr;


		TH1F *          bunch_hit_energy_1 = nullptr;
		TH1F *          bunch_hit_energy_2 = nullptr;

		TH1F *          nhits_bunch_1 = nullptr;
		TH1F *          nhits_bunch_2 = nullptr;

		TH1F *          e_bunch_1 = nullptr;
		TH1F *          e_bunch_2 = nullptr;

		TH1F *          e_bunch_5mus = nullptr;
		TH1F *          e_bunch_5mus_1 = nullptr;
		TH1F *          e_bunch_5mus_2 = nullptr;

		TH1F *          nhits_per_bunch_1 = nullptr;
		TH1F *          nhits_per_bunch_2 = nullptr;

		TH1F *          energy_per_bunch_1 = nullptr;
		TH1F *          energy_per_bunch_2 = nullptr;

		TH1F *          t_enery_bunch_share_1 = nullptr;
		TH1F *          t_enery_bunch_share_2 = nullptr;

		TH1F *          t_hits_bunch_share_1 = nullptr;
		TH1F *          t_hits_bunch_share_2 = nullptr;

		TH1F *          t_hits_bunch_belowX_1 = nullptr;
		TH1F *          t_hits_bunch_belowX_2 = nullptr;
		TH1F *          t_energy_bunch_belowX_1 = nullptr;
		TH1F *          t_energy_bunch_belowX_2 = nullptr;

		TGraph *        hitsb_vs_injcharge_ler_1 = nullptr;
		TGraph *        hitsb_vs_injcharge_ler_2 = nullptr;

		TGraph *        energyb_vs_injcharge_ler_1 = nullptr;
		TGraph *        energyb_vs_injcharge_ler_2 = nullptr;

		TGraph *        hitsb_vs_vertical_steering_1_ler_1 = nullptr;
		TGraph *        hitsb_vs_vertical_steering_1_ler_2 = nullptr;

		TGraph *        energyb_vs_vertical_steering_1_ler_1 = nullptr;
		TGraph *        energyb_vs_vertical_steering_1_ler_2 = nullptr;

		TGraph *        hitsb_vs_vertical_steering_2_ler_1 = nullptr;
		TGraph *        hitsb_vs_vertical_steering_2_ler_2 = nullptr;

		TGraph *        energyb_vs_vertical_steering_2_ler_1 = nullptr;
		TGraph *        energyb_vs_vertical_steering_2_ler_2 = nullptr;

		TGraph *        hitsb_vs_injeff_her = nullptr;
		TGraph *        hitsb_vs_injeff_ler = nullptr;
		TGraph *        hitsb_vs_injeff_ler_1 = nullptr;
		TGraph *        hitsb_vs_injeff_ler_2 = nullptr;

		TGraph *        energyb_vs_injeff_her = nullptr;
		TGraph *        energyb_vs_injeff_ler = nullptr;
		TGraph *        energyb_vs_injeff_ler_1 = nullptr;
		TGraph *        energyb_vs_injeff_ler_2 = nullptr;

		TGraph *        hitsb_vs_current_her = nullptr;
		TGraph *        hitsb_vs_current_ler = nullptr;
		TGraph *        hitsb_vs_current_ler_1 = nullptr;
		TGraph *        hitsb_vs_current_ler_2 = nullptr;

		TGraph *        energyb_vs_current_her = nullptr;
		TGraph *        energyb_vs_current_ler = nullptr;
		TGraph *        energyb_vs_current_ler_1 = nullptr;
		TGraph *        energyb_vs_current_ler_2 = nullptr;

		TGraph *        t_energy_bunch_belowX_vs_current_her = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_current_ler = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_current_ler_1 = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_current_ler_2 = nullptr;

		TGraph *        t_energy_bunch_belowX_vs_injeff_her = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_injeff_ler = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_injeff_ler_1 = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_injeff_ler_2 = nullptr;

		TGraph *        t_energy_bunch_belowX_vs_injcharge_her = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_injcharge_ler = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_injcharge_ler_1 = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_injcharge_ler_2 = nullptr;

		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_1_her = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_1_ler = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_1_ler_1 = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_1_ler_2 = nullptr;

		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_2_her = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_2_ler = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_2_ler_1 = nullptr;
		TGraph *        t_energy_bunch_belowX_vs_vertical_steering_2_ler_2 = nullptr;
};

#endif /* CLAWS_CHANNEL_H_ */
