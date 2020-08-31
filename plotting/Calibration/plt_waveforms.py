#!/usr/bin/env python3


#from rootpy.io import root_open
from ROOT import gROOT, TCanvas, TF1


ph_file = root_open(
    '/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/Event-401141041.root', 'open')


# --- Calibration waveform

int_file = root_open(
    '/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/Run-401141-Int000.root', 'open')

calwf = int_file.Get('FWD1-INT')

#TH1D* calwf = new TH1D("calwf", "",230,  -dt/2., dt*230 - dt/2.);
#
#     for (size_t i = 1; i <= calwf_long->GetNbinsX(); i++)
#     {
#         double cont = -1*calwf_long->GetBinContent(i) * 50./32512 ;
#         calwf->SetBinContent(i, cont);
#     }
#
#     delete calwf_long;
#
#     calwf->GetXaxis()->SetTitle("Time [ns]");
#     calwf->GetYaxis()->SetTitle("Voltage [mV]");

multi = 200
c1 = TCanvas("c1", "c1", 4*multi, 3*multi);

calwf.Draw()

print("Done")
#     c1->Divide(1,2);
#
#     TPad* pad0 = (TPad*) c1->GetPad(0);
#     pad0->cd();
#     calwf->Draw();
#
#     TPad* pad1 = (TPad*) c1->GetPad(1);
#     pad1->cd();
#     phwf->Draw();
#
#     // c1->cd(2);
#
#
#
#     // c1->cd(1);
#     // calwf->Draw();

# void plt_waveforms()
# {
#     string fname = "./Data/Event-401141041.root";
#     TFile *rfile = new TFile(fname.c_str(), "open");
#
#     TH1I* phwf_long = (TH1I*) rfile->Get("FWD1");
#     phwf_long->SetDirectory(0);
#
#     rfile->Close("R");
#     delete rfile;
#
#     double dt = 0.8;
#     double shift  = -1;
#     double length = 625;
#
#     for (size_t i = 1; i <= phwf_long->GetNbinsX(); i++)
#     {
#         if( phwf_long->GetBinContent(i) < 24000 )
#         {
#             shift = i-132;
#             break;
#         }
#     }
#
#
#     // phwf_long->Draw();
#     // cout << shift << endl;
#
#
#     TH1D* phwf = new TH1D("phwf", "",length,  -dt/2., dt*length - dt/2.);
#
#     for (size_t i = 1; i <= phwf->GetNbinsX(); i++)
#     {
#         double cont = -(phwf_long->GetBinContent(i+shift) -32512*0.9) * 1000./32512 ;
#         phwf->SetBinContent(i, cont);
#     }
#
#     delete phwf_long;
#     // - 32512 * 0.9) / (256**2) * 1000)
#
#     phwf->GetXaxis()->SetTitle("Time [ns]");
#     phwf->GetYaxis()->SetTitle("Voltage [mV]");
#
#     // phwf->GetXaxis()->SetRangeUser(172025, 172025 + 300);
#
#
#     // phwf_long->Draw();
#     // cout << shift << endl;
#
#     // Calibration wavform
#     fname = "./Data/Run-401141-Int000.root";
#     TFile* rfile_cal = new TFile(fname.c_str(), "open");
#
#     TH1I* calwf_long = (TH1I*) rfile_cal->Get("FWD1-INT");
#     calwf_long->SetDirectory(0);
#
#     rfile_cal->Close("R");
#     //delete rfile;
#
#
#     TH1D* calwf = new TH1D("calwf", "",230,  -dt/2., dt*230 - dt/2.);
#
#     for (size_t i = 1; i <= calwf_long->GetNbinsX(); i++)
#     {
#         double cont = -1*calwf_long->GetBinContent(i) * 50./32512 ;
#         calwf->SetBinContent(i, cont);
#     }
#
#     delete calwf_long;
#
#     calwf->GetXaxis()->SetTitle("Time [ns]");
#     calwf->GetYaxis()->SetTitle("Voltage [mV]");
#
#     //calwf->GetXaxis()->SetRangeUser(-dt/2., dt*length - dt/2.);
#     calwf->SetAxisRange(0., 100, "X");
#
#     // PLotting
#     gStyle->SetOptStat(0);
#     double multi = 200;
#     TCanvas * c1 = new TCanvas("c", "c", 4*multi, 3*multi);
#     c1->Divide(1,2);
#
#     TPad* pad0 = (TPad*) c1->GetPad(0);
#     pad0->cd();
#     calwf->Draw();
#
#     TPad* pad1 = (TPad*) c1->GetPad(1);
#     pad1->cd();
#     phwf->Draw();
#
#     // c1->cd(2);
#
#
#
#     // c1->cd(1);
#     // calwf->Draw();
# }




# c = TCanvas("test"+PDFname, "Graph example", 0, 0, 1200, 900);
#     c.SetFillColor(kWhite);
#     c.SetBorderMode(0);
#     c.SetBorderSize(2);
#     c.SetFrameBorderMode(0);
#     fontScale = 1.;#.8; // scale fonts to account for changed aspect ratio
#     pad1 =  TPad("pad1", "The pad 80% of the height", 0.0, 0.38, 1.0, 1.0, 21);
#     pad2 =  TPad("pad2", "The pad 20% of the height", 0.0, 0.05, 1.0, 0.4, 22);
#     pad1.Draw();
#     pad2.Draw();
#
#     #----------------pad1-------------------------------------
#     pad1.cd();
#     pad1.SetFillColor(0);
#     pad1.SetBorderMode(0);
#     pad1.SetBorderSize(2);
#     pad1.SetTickx(1);
#     pad1.SetTicky(1);
#     pad1.SetLeftMargin(0.14);
#     pad1.SetRightMargin(0.04);
#     pad1.SetTopMargin(0.04);
#     pad1.SetBottomMargin(0);
#     pad1.SetFrameLineWidth(2);
#     pad1.SetFrameBorderMode(0);
#     pad1.SetFrameLineWidth(2);
#     pad1.SetFrameBorderMode(0);
#
#     h.GetXaxis().SetTitle("");
#     h.GetXaxis().SetLabelSize(0);
#     h.GetYaxis().SetTitle("#sigma_{sc}/#sigma_{standard}");
#     h.GetXaxis().SetLimits(0, 85);
#     h.GetYaxis().SetRangeUser(0.5,1.);
#     h.GetXaxis().SetNdivisions(506);
#     h.GetXaxis().SetLabelFont(43);
#     h.GetXaxis().SetLabelOffset(0.015);
#     h.GetXaxis().SetTitleOffset(1.2);
#     h.GetXaxis().SetLabelSize(35);
#     h.GetXaxis().SetTitleSize(34);
#     h.GetXaxis().SetTitleFont(43);
#
#     h.GetYaxis().SetNdivisions(506);
#     h.GetYaxis().SetLabelFont(43);
#     h.GetYaxis().SetLabelOffset(0.015);
#     h.GetYaxis().SetLabelSize(40);
#     h.GetYaxis().SetTitleSize(40);
#     h.GetYaxis().SetTitleOffset(1.5);
#     h.GetYaxis().SetTitleFont(43);
#
#     h.SetMarkerStyle(33)
#     h_sys.SetMarkerStyle(27);
#     h.SetMarkerColor(kRed)
#     h_sys.SetLineColor(kRed+3)
#     h_sys.SetMarkerColor(kRed+3);
#     h.Draw("AP")
#     h_sys.Draw("P")
#
#     legend1 =TLegend(0.55,0.68,0.85,0.85)
#     legend1.SetBorderSize(0);
#     legend1.SetTextFont(43);
#     legend1.SetTextSize(30);
#     legend1.SetLineColor(1);
#     legend1.SetLineStyle(1);
#     legend1.SetLineWidth(2);
#     legend1.SetFillColor(10);
#     legend1.SetFillStyle(1001);
#     legend1.SetMargin(0.15);
#     legend1.AddEntry(h, "Resolution Improvement","pl")
#     legend1.AddEntry(h_sys, "Resolution Improvement sys","p")
#     legend1.Draw();
#
#     #-----------Pad2-----------------------------------------------
#     pad2.cd();
#     pad2.SetFillColor(0);
#     pad2.SetBorderMode(0);
#     pad2.SetBorderSize(2);
#     pad2.SetTickx(1);
#     pad2.SetTicky(1);
#     pad2.SetLeftMargin(0.14);
#     pad2.SetRightMargin(0.04);
#     pad2.SetTopMargin(0);
#     pad2.SetBottomMargin(0.4);
#     pad2.SetFrameLineWidth(2);
#     pad2.SetFrameBorderMode(0);
#     pad2.SetFrameLineWidth(2);
#     pad2.SetFrameBorderMode(0);
#
#
#     size =h_sys.GetN()
#     x = Double()
#     y= Double()
#     xsys = Double()
#     ysys = Double()
#     ratio  = array('f',[])
#     energy = array('f',[])
#     dratio  = array('f',[])
#     denergy = array('f',[])
#     for i in range(0,size):
#         h.GetPoint(i,x,y)
#         h_sys.GetPoint(i,xsys,ysys)
#         diff=(ysys-y)/ y
#         ratio.append(diff)
#         energy.append(x)
#         dratio.append(0)
#         denergy.append(0)
#     RSys =  TGraphErrors(size, energy, ratio,denergy,dratio);
#     RSys.SetName("RSys");
#     RSys.SetMarkerColor(kRed+3);
#
#
#     RSys.GetXaxis().SetTitle("E_{beam} [GeV]");
#     RSys.GetXaxis().SetNdivisions(506);
#     RSys.GetXaxis().SetLabelFont(43);
#     RSys.GetXaxis().SetLabelOffset(0.015);
#     RSys.GetXaxis().SetLabelSize(40);
#     RSys.GetXaxis().SetTitleSize(40);
#     RSys.GetXaxis().SetTickLength(0.0851735);
#     RSys.GetXaxis().SetTitleOffset(3.85);
#     RSys.GetXaxis().SetTitleFont(43);
#     RSys.GetYaxis().SetTitle("Relative Unc.");
#     RSys.GetYaxis().SetNdivisions(503);
#     RSys.GetYaxis().SetLabelFont(43);
#     RSys.GetYaxis().SetLabelOffset(0.015);
#     RSys.GetYaxis().SetLabelSize(40);
#     RSys.GetYaxis().SetTitleSize(40);
#     RSys.GetYaxis().SetTickLength(0.036);
#     RSys.GetYaxis().SetTitleOffset(1.5);
#     RSys.GetYaxis().SetTitleFont(43);
#
#     RSys.SetMarkerSize(1.5);
#     RSys.SetMarkerStyle(27)
#     RSys.SetMinimum(-0.1);
#     RSys.SetMaximum(0.12);
#     RSys.SetTitle("");
#     RSys.GetXaxis().SetLimits(0, 85);
#
#     RSys.Draw("AP");
#     line2 =  TLine(0.0, 0.0, 85, 0.0);
#     line2.SetLineStyle(2);
#     line2.Draw();
#
#     if dataset is "FNAL":
#         h_sys.RemovePoint(0)
#         h_sys.RemovePoint(0)
#         h_sys.RemovePoint(0)
#         RSys.RemovePoint(0)
#         RSys.RemovePoint(0)
#         RSys.RemovePoint(0)
#     if dataset is "CERN":
#         size=h_sys.GetN()
#         h_sys.RemovePoint(size-1)
#         RSys.RemovePoint(size-1)
#     if dataset is "COMBINEDCERN":
#         h_sys.RemovePoint(10)
#         h_sys.RemovePoint(10)
#         h_sys.RemovePoint(10)
#         RSys.RemovePoint(10)
#         RSys.RemovePoint(10)
#         RSys.RemovePoint(10)
#     if dataset is "COMBINEDFNAL":
#         h_sys.RemovePoint(9)
#         RSys.RemovePoint(9)
#      ##################################################
#     c.Print(output_dir+PDFname+"_"+Data_Type+".pdf", "pdf")
#     hfile_name=  output_dir+PDFname+"_"+Data_Type+".root"
#     c.SaveAs(hfile_name);
#
#     hfile =  TFile(hfile_name,"UPDATE");
#     RSys.Write("RSys")
#     hfile.Close()
#     c.SaveAs(output_dir+PDFname+"_"+Data_Type+".C");
#     del c
