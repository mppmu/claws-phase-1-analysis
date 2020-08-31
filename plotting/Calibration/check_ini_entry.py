#!/usr/bin/env python3


# #from rootpy.io import root_open
# from ROOT import gROOT, TCanvas, TF1

try:
    # Python modules
    import argparse
    import os
    # import timeit
    import time
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
    from ROOT import TGraphErrors, TFile, TGraph, TCanvas, TH1F
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

     #  print(bcolors.OKGREEN + "Running '{}'".format(__file__) + bcolors.ENDC)

    parser = argparse.ArgumentParser()

    parser.add_argument('-i', '--input', type=str, help='Path to day or run ', required=True)

    parser.add_argument('-o', '--output', type=str, default='', help='Path to safe ', required=False)

    parser.add_argument('-s', '--section', type=str, help='Section to check', required=True)

    parser.add_argument('-c', '--channels', type=str, nargs='+', default='FWD1', help='Channel to check', required=True)

    parser.add_argument('-sub', '--subfolder', type=str, default='', help='If selected only ini files in this subfoler are searched', required=False)

    parser.add_argument('-t', '--type', type=str, default='graph', help='Hist or graph', required=True)

    args = parser.parse_args()

    logger = logging.getLogger(__name__)


    # logger.debug('COMMAND LINE ARGUMENTS')
    # logger.debug('INPUT: {}'.format(args.input))
    # logger.debug('OUTPUT: {}'.format(args.output))
    # logger.debug('SECTION: {}'.format(args.section))
    # logger.debug('CHANNEL: {}'.format(args.channels))

    # folder = "/remote/ceph/group/ilc/claws/data/RAW/connecticut/2016-06-21"
    color = 1
    marker = 20
    graphs = []

    if args.type == 'graph':
        for ch in args.channels:
            graph = TGraph()
            graph.SetMarkerColor(color)
            color +=1
            graph.SetMarkerStyle(marker)
            marker +=1
            graph.SetName(ch)
            graphs.append(graph)
    elif args.type == 'hist':
        for ch in args.channels:
            nbins = 2000
            graph = TH1F(ch, ch, nbins, -1./(nbins), 2.-1./(nbins) )
            graph.SetLineColor(color)
            color +=1
            #graph.SetMarkerStyle(marker)
            #marker +=1
            #graph.SetName(ch)
            graphs.append(graph)



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

                        if args.type == 'graph':
                            n = graph.GetN()

                            graph.SetPoint(n, runnr, float(config[args.section][graph.GetName()]) )

                        elif args.type == 'hist':
                            graph.Fill(float(config[args.section][graph.GetName()]))

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
    if args.type == 'graph':
        for  i in range(len(graphs)):
            if i == 0:
                graphs[i].Draw('ap')
            else:
                graphs[i].Draw('same p')

    elif args.type == 'hist':
        for  i in range(len(graphs)):
            if i == 0:
                graphs[i].Draw()
            else:
                graphs[i].Draw('same')

    c1.Update()
    time.sleep(1000)


# rfile = TFile('./gain_over_runnr.root', 'RECREATE')
#
# fwd1.Write()
# fwd2.Write()
# fwd3.Write()
#
#
# rfile.Close()
                # for sec in config.sections():
                #     print(sec)
        # if sec in rename1.keys():
        #     config.add_section(rename1[sec])
        #     config.add_section(rename2[sec])
        #     tmp = config.items(sec)
        #     for option, value in tmp:
        #         if option == 'Analogue Offset':
        #             config.set(rename1[sec], 'AnalogOffset', value.strip('"'))
        #             config.set(rename2[sec], 'AnalogOffset', value.strip('"'))
        #         # elif option == 'Coupling' or option == 'Range' or option == 'Bandwidth':
        #         #     config[key][option] = value[1:-1]
        #         else:
        #             config[rename1[sec]][option] = value.strip('"')
        #             config[rename2[sec]][option] = value.strip('"')
        # else:
        #     for option, value in config.items(sec):
        #         config[sec][option] = value.strip('"')
        #
        #
        #         print('root: ' + str(root))
        #         print('dirnames: ' + str(dirnames))
        #         print('file: ' + str(file))
        #     for filename in sorted(filenames):
        #         if filename.startswith('Event-') and filename.endswith('.tdms'):

#
# (root, dirs, files) = next(os.walk(folder))
#
# for file in files:
#     print('file: ' + str(file))
#     if file.endswith('_gainstate_fitted_ph1v4.ini'):
#             # print(filename)
#             #    filename = '/home/iwsatlas1/mgabriel/workspace/claws_phaseI/MyonDAQToPhaseI/Berta_ladder7.1_56.6-mV_BC408_2x2_final_2x2-02/data/int-wf-channel-b-800.dat'
#             print('root: ' + str(root))
#             print('dirs: ' + str(dirs))
#             print('file: ' + str(file))
            # config = configparser.ConfigParser()
            # config.optionxform = str
            # cfile = os.path.join(
            #     int_folder, 'Run-' + str(run_nr) + '-Int' + str(evt_nr).zfill(3) + '.ini')
            #
            # config.read(cfile_name)
            #
            # if not 'FWD' in config.sections():
            #     config['FWD'] = {}
            #
            # config['FWD'][ch_name + '-Mean'] = '0'
            # config['FWD'][ch_name + '-Accepted'] = '0'
            #
            # if not 'BWD' in config.sections():
            #     config['BWD'] = {}
            #
            # config['BWD'][ch_name + '-Mean'] = '0'
            # config['BWD'][ch_name + '-Accepted'] = '0'
