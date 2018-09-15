#!/usr/bin/env python3


# #from rootpy.io import root_open
# from ROOT import gROOT, TCanvas, TF1

try:
    # Python modules
    import argparse
    import os
    # import timeit
    # import time
    # import calendar
    # import datetime
    import logging
    # import ROOT
    import configparser
    # import shutil
    # from collections import namedtuple
    # import pandas as pd
    # import rootpy.ROOT as ROOT
#    %matplotlib inline
    # import matplotlib.pyplot as plt
    from ROOT import TGraphErrors, TFile, TGraph, TCanvas
    from rootpy.interactive import wait
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

if __name__ == '__main__':


for root, dirnames, filenames in os.walk(args.input):

   # if root.endswith('/GainDetermination'):

   for file in sorted(filenames):
       if file.endswith('.ini') and args.subfolder in root:

           config = configparser.ConfigParser()
           config.optionxform = str
           config.read(os.path.join(root,file))

           for graph in graphs:

               if config.has_option(args.section, graph.GetName()):
               # print('file: ' + str(file))
               # print('root: ' + str(root))

                   runnr = 0
                   if 'Waveforms' in root:
                       if file[13:23].isdigit():
                           runnr = int(file[13:23])
                       else:
                           runnr = int(file[13:22])

                   else:
                       start = root.find('Run-') + 4
                       runnr = int(root[start:start+6])


                   n = graph.GetN()

                   # graph.SetPoint(n, runnr, float(config[args.section][graph.GetName()]) )
                   print('Runnr: ' + str(runnr))
                   print(args.section + ' for channel: ' + graph.GetName())
                   print(config[args.section][graph.GetName()])

if not args.output == '':
   rfile = TFile( args.output, 'RECREATE')
   for graph in graphs:
       graph.Write()
   rfile.Close()

c1 = TCanvas( 'c1', 'Example ', 1200, 600 )

# graphs[0].Draw('ap')
# graphs[1].Draw('same p')
for  i in range(len(graphs)):
   if i == 0:
       graphs[i].Draw('ap')
   else:
       graphs[i].Draw('same p')
c1.Update()
wait()
