#!/usr/bin/env python3


# #from rootpy.io import root_open
# from ROOT import gROOT, TCanvas, TF1

try:
    # Python modules
    # import argparse
    import os
    # import timeit
    # import time
    # import calendar
    # import datetime
    # import logging
    # import ROOT
    import configparser
    # import shutil
    # from collections import namedtuple
    # import pandas as pd
    # import rootpy.ROOT as ROOT
#    %matplotlib inline
    # import matplotlib.pyplot as plt
    from ROOT import TGraphErrors, TFile
except ImportError:
    print("Couldn't import some shit in plt_gain_over_time.py")

# ph_file = root_open(
#     '/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/Event-401141041.root', 'open')
#

# day = os.path.join(in_folder, 'data')
#
#     int_folder = os.path.join(out_folder, 'int_root')
#
#     try:
#         os.mkdir(int_folder)
#         os.chmod(int_folder, mode=0o777)
#     except FileExistsError:
#         logger.debug(
#             int_folder + ' does allready exist, only changing permissions.')
#         os.chmod(int_folder, mode=0o777)

folder = "/remote/ceph/group/ilc/claws/data/RAW/connecticut/"
day = "2016-06-08"

fwd1 = TGraphErrors()
fwd1.SetName('fwd1')
fwd2 = TGraphErrors()
fwd2.SetName('fwd2')
fwd3 = TGraphErrors()
fwd3.SetName('fwd3')

files = []

for root, dirnames, filenames in os.walk(folder+day):
    if root.endswith('/GainDetermination'):
        for file in sorted(filenames):
            #print(file)
            if file.endswith('_gainstate_fitted_ph1v4.ini'):
                # print("File: " + str(file))
                # print("Root: " + str(root))
                files.append(root + "/" + file)
                # config = configparser.ConfigParser()
                # config.optionxform = str
                # config.read(os.path.join(root,file))
                # start = root.find('Run-') + 4
                #
                # runnr = int(root[start:start+6])
                # n = fwd1.GetN()
                # # print('N: ' + str(n))
                # # print('Y: ' + str(float(config['Gain_FitMean1']['FWD1'])))
                # # print('Err: ' + str(float(config['Gain_FitSigma1']['FWD1'])))
                # print(runnr)
                # fwd1.SetPoint(n, runnr, float(config['Gain_FitMean1']['FWD1']))
                # fwd1.SetPointError(n, 0, float(config['Gain_FitSigma1']['FWD1']))
                #
                # fwd2.SetPoint(n, runnr, float(config['Gain_FitMean1']['FWD2']))
                # fwd2.SetPointError(n, 0, float(config['Gain_FitSigma1']['FWD2']))
                #
                # fwd3.SetPoint(n, runnr, float(config['Gain_FitMean1']['FWD3']))
                # fwd3.SetPointError(n, 0, float(config['Gain_FitSigma1']['FWD3']))
#print(files)

for file in sorted(files):

    config = configparser.ConfigParser()
    config.optionxform = str
    config.read(file)

    start = file.find('Run-') + 4

    #print('File: ' + file)
    runnr = int(file[start:start+6])
    #print('Runnr: ' + str(runnr))
    n = fwd1.GetN()
    #print('N: ' + str(n))
    # print('Y: ' + str(float(config['Gain_FitMean1']['FWD1'])))
    # print('Err: ' + str(float(config['Gain_FitSigma1']['FWD1'])))
    # print(runnr)
    fwd1.SetPoint(n, runnr, float(config['Gain_FitMean1']['FWD1']))
    fwd1.SetPointError(n, 0, float(config['Gain_FitMean1Error']['FWD1']))

    fwd2.SetPoint(n, runnr, float(config['Gain_FitMean1']['FWD2']))
    fwd2.SetPointError(n, 0, float(config['Gain_FitMean1Error']['FWD2']))

    fwd3.SetPoint(n, runnr, float(config['Gain_FitMean1']['FWD3']))
    fwd3.SetPointError(n, 0, float(config['Gain_FitMean1Error']['FWD3']))

rfile = TFile('./Calibration/Data/gain_over_runnr_'+day+'.root', 'RECREATE')

fwd1.Write()
fwd2.Write()
fwd3.Write()


rfile.Close()
