#!/usr/bin/env python3

import argparse
import multiprocessing
import time
import math
import sys
import configparser
import pandas as pd
import numpy as np
import random
import os
from ROOT import TFile, TF1

# def heat(name):
#     print "Starting to heat"
#     while True:
#         print name
#         value = math.factorial(200000)

# print name
# class myThread(threading.Thread):
#     def __init__(self, threadID, name):
#         threading.Thread.__init__(self)
#         self.threadID = threadID
#         self.name_    = name
#     def run(self):
#         heat(self.name)

def change_parameter(cfile_name, par, value):
    config = configparser.ConfigParser()
    config.optionxform = str
    config.read(cfile_name)
    try:
        config[par.split('.')[0]][par.split('.')[1]] = str(value)
    except KeyError:
        print("The following sections are present in config: ")
        for sec in config.sections():
            print(sec)
        print("Parameter not found: " + str(par))
        sys.exit("KeyErrors!")

    with open(cfile_name, 'w') as cfile:
        config.write(cfile)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='CLAWS converter from LabVIEW Lab DAQ for myon measurements to phase I root format.')

    parser.add_argument('-i', '--input', type=str, default='/remote/ceph/group/ilc/claws/data/Myon/Converted/Run-900122',
                            help='Input runfolder ', required=False)

    parser.add_argument('-o', '--output', type=str, default='/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results2.csv',
                            help='File for the output. For exampel foo/bar/Run-900000', required=False)

    parser.add_argument('-c', '--cfile', type=str, default='/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/calibration_phase1_myon_sys_study.ini',
                            help='Config file to use', required=True)


    args = parser.parse_args()

    print("Config file: " +  args.cfile)
    print("Input: " +  args.input)
    print("Output: " +  args.output)

    cfile_name = args.cfile

    cmd = '/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/calibration -c ' + args.cfile + ' --profile-timing 1 --data.input ' + args.input
    print('Command: ' + cmd)
    # --tasks 10
    runnr = args.input[-6:]
    #parameters_name = ['SystematicsStudy.threshold_tres','SystematicsStudy.range_time','SystematicsStudy.threshold_mpv','SystematicsStudy.window_length_mpv']
    #parameters_name = ['MipTimeRetrieval.pe_hit_time','MipTimeRetrieval.window_length','MipTimeRetrieval.window_threshold','SystematicsStudy.threshold_tres','SystematicsStudy.range_time','SystematicsStudy.start_mpv','SystematicsStudy.window_length_mpv']

    # columns = parameters_name + results_name
    # df = pd.DataFrame(columns = columns)
    # #se = pd.Series(np.zeros(len(columns), index = columns)
    #
    # se = pd.Series( np.zeros(len(columns)), index=columns)
    # df.iloc[1] = [random.randint(-1,1) for n in range(len(columns))]
    # #df.iloc(-1) = se
    # print(df.head())

    #columns = parameters_name + results_name
    #df = pd.DataFrame(columns=columns)

    #config = configparser.ConfigParser()
    #config.optionxform = str
    #config.read(cfile_name)

    #systematics_study_threshold_mpv     = config['SystematicsStudy']['threshold_mpv']

    #df = pd.read_csv(args.output)

    # systematics_study_start_mpv_range         = range(int(df['SystematicsStudy.start_mpv'].iloc[-1]),15021,10)
    # systematics_study_window_length_mpv_range = range(int(df['SystematicsStudy.window_length_mpv'].iloc[-1]), 61,10)
    # systematics_study_range_time_range        = range(int(df['SystematicsStudy.range_time'].iloc[-1]),9)
    # systematics_study_threshold_tres_range    = np.arange(df['SystematicsStudy.threshold_tres'].iloc[-1], 0.8, 0.1)

    # systematics_study_start_mpv_start         = int(df['SystematicsStudy.start_mpv'].iloc[-1])
    # systematics_study_window_length_mpv_start = int(df['SystematicsStudy.window_length_mpv'].iloc[-1])
    # systematics_study_range_time_start        = int(df['SystematicsStudy.range_time'].iloc[-1])
    # systematics_study_threshold_tres_start    = df['SystematicsStudy.threshold_tres'].iloc[-1]

    # start = True
    # n = df.shape[0] - 2
    # df = df[:-1]

    #df = pd.DataFrame(columns=columns)
    #n = 1


    # mip_time_pe_hit_time_range       = range(1,4)
    # mip_time_window_length_range     = range(6, 17,2)
    # mip_time_window_threshold_range  = range(1,5)
    #
    # systematics_study_start_mpv_range         = range(14990,14991,10)
    # systematics_study_window_length_mpv_range = range(50, 51,10)
    # systematics_study_range_time_range        = range(4,5)
    # systematics_study_threshold_tres_range    = np.arange(0.2, 0.8, 0.1)
    #
    #
    # systematics_study_start_mpv_start         = int(df['SystematicsStudy.start_mpv'].iloc[-1])
    # systematics_study_window_length_mpv_start = int(df['SystematicsStudy.window_length_mpv'].iloc[-1])
    # systematics_study_range_time_start        = int(df['SystematicsStudy.range_time'].iloc[-1])
    # systematics_study_threshold_tres_start    = df['SystematicsStudy.threshold_tres'].iloc[-1]

    start = False
    # n = df.shape[0] - 2
    # df = df[:-1]


    # systematics_study_start_mpv         = config['SystematicsStudy']['start_mpv']
    # systematics_study_window_length_mpv = config['SystematicsStudy']['window_length_mpv']
    # systematics_study_range_time        = config['SystematicsStudy']['range_time']


    #nges = len(xrange(14980,15021,10)) + len(xrange(10, 61,10) + len(xrange(4,9)) + len( np.arange(0.2, 0.8, 0.1) )

    decomp_par_names = ['WaveformDecomposition.stop_methode','WaveformDecomposition.stop_region', 'WaveformDecomposition.threshold','WaveformDecomposition.fwhm','WaveformDecomposition.search_range','WaveformDecomposition.search_edge']
    decomp_ranges       = [[2],[5], [1.5,2.5,3.5], [4,6,8], [1000], [200]]

    miptime_par_names    = ['MipTimeRetrieval.window_threshold','MipTimeRetrieval.window_length','MipTimeRetrieval.timing_type','MipTimeRetrieval.constant_fraction', 'MipTimeRetrieval.pe_hit_time']
    miptime_ranges = [[1,2,3],[4,12,36],['constant fraction','pe hit time'],[0.1,0.3,0.5], [1,2,3]]

    sys_par_names    = ['SystematicsStudy.threshold_tres','SystematicsStudy.range_time']
#    sys_ranges = [np.arange(0., 0.7, 0.1), [4,6,8]]
    sys_ranges = [ [0.1, 0.5], [4,6,8]]

    result_par_names = ['MPV_1','MPV_2','MPV_3','MPV_4', 'TRes', 'TRes_shift']



    par_names = decomp_par_names + miptime_par_names + sys_par_names + result_par_names
    par       = [0] * len(par_names)

    n = 1
    nges = 1
    for r in (decomp_ranges+miptime_ranges+sys_ranges):
        nges =  nges*len(r)

    df = pd.DataFrame(columns=par_names)

    print("Parnames: " + str(par_names))
    print("Par: " + str(par))
    print("Niterations: " + str(nges))


    for par[-len(result_par_names) - len(sys_par_names) - len(miptime_par_names) - 6] in decomp_ranges[-6]:
        change_parameter(args.cfile, decomp_par_names[-6],  par[-len(result_par_names)- len(sys_par_names) - len(miptime_par_names)-6])

        for par[-len(result_par_names) - len(sys_par_names) - len(miptime_par_names) - 5] in decomp_ranges[-5]:
            change_parameter(args.cfile, decomp_par_names[-5],  par[-len(result_par_names)- len(sys_par_names) - len(miptime_par_names)-5])

            for par[-len(result_par_names) - len(sys_par_names) - len(miptime_par_names) - 4] in decomp_ranges[-4]:
                change_parameter(args.cfile, decomp_par_names[-4],  par[-len(result_par_names)- len(sys_par_names) - len(miptime_par_names)-4])

                for par[-len(result_par_names) - len(sys_par_names) - len(miptime_par_names)- 3] in decomp_ranges[-3]:
                    change_parameter(args.cfile, decomp_par_names[-3],  par[-len(result_par_names)- len(sys_par_names) - len(miptime_par_names)-3])

                    for par[-len(result_par_names) - len(sys_par_names) - len(miptime_par_names)- 2] in decomp_ranges[-2]:
                        change_parameter(args.cfile, decomp_par_names[-2],  par[-len(result_par_names)- len(sys_par_names) - len(miptime_par_names)-2])

                        for par[-len(result_par_names) - len(sys_par_names) - len(miptime_par_names)- 1] in decomp_ranges[-1]:
                            change_parameter(args.cfile, decomp_par_names[-1],  par[-len(result_par_names)- len(sys_par_names) - len(miptime_par_names)-1])

                            for i in range(len(par_names)):
                                print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                            os.system(cmd + ' --tasks 7-8')

                            # for par[-len(result_par_names) - len(sys_par_names)- 3] in miptime_ranges[-3]:
                            #     change_parameter(args.cfile, miptime_par_names[-3],  par[-len(result_par_names)- len(sys_par_names) -3])
                            #
                            #     for par[-len(result_par_names) - len(sys_par_names)- 2] in miptime_ranges[-2]:
                            #         change_parameter(args.cfile, miptime_par_names[-2],  par[-len(result_par_names)- len(sys_par_names) -2])
                            #
                            #         for par[-len(result_par_names) - len(sys_par_names)- 1] in miptime_ranges[-1]:
                            #             change_parameter(args.cfile, miptime_par_names[-1],  par[-len(result_par_names)- len(sys_par_names) -1])

                            for par[-len(result_par_names) - len(sys_par_names)- 5] in miptime_ranges[-5]:
                                change_parameter(args.cfile, miptime_par_names[-5],  par[-len(result_par_names)- len(sys_par_names) -5])

                                for par[-len(result_par_names) - len(sys_par_names)- 4] in miptime_ranges[-4]:
                                    change_parameter(args.cfile, miptime_par_names[-4],  par[-len(result_par_names)- len(sys_par_names) -4])

                                    for par[-len(result_par_names) - len(sys_par_names)- 3] in miptime_ranges[-3]:
                                        change_parameter(args.cfile, miptime_par_names[-3],  par[-len(result_par_names)- len(sys_par_names) -3])

                                        if(miptime_par_names[-3] == 'constant fraction'):

                                            for par[-len(result_par_names) - len(sys_par_names)- 2] in miptime_ranges[-2]:
                                                change_parameter(args.cfile, miptime_par_names[-2],  par[-len(result_par_names)- len(sys_par_names) -2])

                                                for i in range(len(par_names)):
                                                    print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                                                os.system(cmd + ' --tasks 9')

                                                for par[-len(result_par_names) -2] in sys_ranges[-2]:
                                                    change_parameter(args.cfile, sys_par_names[-2],  par[-len(result_par_names) -2])

                                                    for  par[-len(result_par_names) -1] in sys_ranges[-1]:

                                                        change_parameter(args.cfile, sys_par_names[-1],  par[-len(result_par_names) -1])
                                                            #                 print('systematics_study_threshold_tres: ' + str(systematics_study_threshold_tres))
                                                            #
                                                            #                 #run cmd
                                                        for i in range(len(par_names)):
                                                            print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                                                        os.system(cmd + ' --tasks 10')

                                                        rfile = TFile( args.input + '/Results/SystematicsStudy/run_'+str(runnr)+'_systematics_ph1v4.root', 'OPEN')

                                                        htres = rfile.Get('time_resolution')
                                                        ftres = htres.GetFunction('gaus')
                                                        par[-1] = ftres.GetParameter(1)/1e-9
                                                        par[-2] = ftres.GetParameter(2)/1e-9

                                                        for i in range(1,5):
                                                            try:
                                                                hmpv = rfile.Get('FWD'+str(i) + '_pe_per_event')
                                                                fmpv = hmpv.GetFunction('FWD'+str(i) + '_pe_per_eventlangaus')
                                                                par[-7+i] = fmpv.GetMaximumX()
                                                            except AttributeError:
                                                                print("i: " + str(i))
                                                                sys.exit(0)
                                                        rfile.Close()

                                                        se = pd.Series( np.zeros(len(par)), index=par_names)
                                                        for i in range(0, len(par)):
                                                            se[par_names[i]] = par[i]

                                                        for i in range(len(par_names)):
                                                            print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                                                        df = df.append(se,ignore_index=True)
                                                        #print(df.tail(10))
                                                        print('Iteration: ' + str(n) + ' of a total of: ' + str(nges))
                                                        df.to_csv(args.output)
                                                        n += 1

                                        elif(miptime_par_names[-3] == 'pe hit time'):

                                            for par[-len(result_par_names) - len(sys_par_names)- 1] in miptime_ranges[-1]:
                                                change_parameter(args.cfile, miptime_par_names[-1],  par[-len(result_par_names)- len(sys_par_names) -1])

                                                for i in range(len(par_names)):
                                                    print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                                                os.system(cmd + ' --tasks 9')

                                                for par[-len(result_par_names) -2] in sys_ranges[-2]:
                                                    change_parameter(args.cfile, sys_par_names[-2],  par[-len(result_par_names) -2])

                                                    for  par[-len(result_par_names) -1] in sys_ranges[-1]:

                                                        change_parameter(args.cfile, sys_par_names[-1],  par[-len(result_par_names) -1])
                                                            #                 print('systematics_study_threshold_tres: ' + str(systematics_study_threshold_tres))
                                                            #
                                                            #                 #run cmd
                                                        for i in range(len(par_names)):
                                                            print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                                                        os.system(cmd + ' --tasks 10')

                                                        rfile = TFile( args.input + '/Results/SystematicsStudy/run_'+str(runnr)+'_systematics_ph1v4.root', 'OPEN')

                                                        htres = rfile.Get('time_resolution')
                                                        ftres = htres.GetFunction('gaus')
                                                        par[-1] = ftres.GetParameter(1)/1e-9
                                                        par[-2] = ftres.GetParameter(2)/1e-9

                                                        for i in range(1,5):
                                                            try:
                                                                hmpv = rfile.Get('FWD'+str(i) + '_pe_per_event')
                                                                fmpv = hmpv.GetFunction('FWD'+str(i) + '_pe_per_eventlangaus')
                                                                par[-7+i] = fmpv.GetMaximumX()
                                                            except AttributeError:
                                                                print("i: " + str(i))
                                                                sys.exit(0)
                                                        rfile.Close()

                                                        se = pd.Series( np.zeros(len(par)), index=par_names)
                                                        for i in range(0, len(par)):
                                                            se[par_names[i]] = par[i]

                                                        for i in range(len(par_names)):
                                                            print("Name: " + str(par_names[i]) + ' Value: ' + str(par[i]))

                                                        df = df.append(se,ignore_index=True)
                                                        #print(df.tail(10))
                                                        print('Iteration: ' + str(n) + ' of a total of: ' + str(nges))
                                                        df.to_csv(args.output)
                                                        n += 1


    df.to_csv(args.output)
    print(df)
    print('Finished!!!')
    os.system('spd-say "your program has finished"')
    os.system('play --no-show-progress --null --channels 1 synth %s sine %f' % (1, 440))
    # nges = 5*6*6*7
    # for systematics_study_start_mpv in systematics_study_start_mpv_range:
    #
    #     if start:
    #         systematics_study_start_mpv = systematics_study_start_mpv_start
    #     change_parameter(cfile_name, 'SystematicsStudy', 'start_mpv', systematics_study_start_mpv)
    #     print('systematics_study_start_mpv: ' + str(systematics_study_start_mpv))
    #
    #     for systematics_study_window_length_mpv in systematics_study_window_length_mpv_range:
    #
    #         if start:
    #             systematics_study_window_length_mpv = systematics_study_window_length_mpv_start
    #         change_parameter(cfile_name, 'SystematicsStudy', 'window_length_mpv', systematics_study_window_length_mpv)
    #         print('systematics_study_window_length_mpv: ' + str(systematics_study_window_length_mpv))
    #
    #         for systematics_study_range_time in systematics_study_range_time_range:
    #
    #             if start:
    #                 systematics_study_range_time = systematics_study_range_time_start
    #             change_parameter(cfile_name, 'SystematicsStudy', 'range_time', systematics_study_range_time)
    #             print('systematics_study_range_time: ' + str(systematics_study_range_time))
    #
    #             for systematics_study_threshold_tres in systematics_study_threshold_tres_range:
    #                 n = n+1
    #                 #change config file
    #                 if start:
    #                     systematics_study_threshold_tres = systematics_study_threshold_tres_start
    #                 change_parameter(cfile_name, 'SystematicsStudy', 'threshold_tres', systematics_study_threshold_tres)
    #                 print('systematics_study_threshold_tres: ' + str(systematics_study_threshold_tres))
    #
    #                 #run cmd
    #                 os.system(cmd + ' --tasks 10')
    #
    #                 se = pd.Series( np.zeros(len(columns)), index=columns)
    #
    #                 se['MipTimeRetrieval.pe_hit_time'] =mip_time_pe_hit_time
    #                 se['MipTimeRetrieval.window_length'] = systematics_study_range_time
    #                 se['SystematicsStudy.start_mpv'] = systematics_study_start_mpv
    #
    #
    #                 se['SystematicsStudy.threshold_tres'] = systematics_study_threshold_tres
    #                 se['SystematicsStudy.range_time'] = systematics_study_range_time
    #                 se['SystematicsStudy.start_mpv'] = systematics_study_start_mpv
    #                 se['SystematicsStudy.window_length_mpv'] = systematics_study_window_length_mpv
    #
    #                 rfile = TFile( args.input + '/Results/SystematicsStudy/run_900122_systematics_ph1v4.root', 'OPEN')
    #                 # #f = TFile('mytree.root','RECREATE')
    #                 #
    #                 htres = rfile.Get('time_resolution')
    #                 ftres = htres.GetFunction('gaus')
    #                 se['TRes_shift'] = ftres.GetParameter(1)/1e-9
    #                 se['TRes'] = ftres.GetParameter(2)/1e-9
    #                 for i in range(1,5):
    #                     hmpv = rfile.Get('FWD'+str(i) + '_pe_per_event')
    #                     fmpv = hmpv.GetFunction('FWD'+str(i) + '_pe_per_eventlangaus')
    #                     se['MPV_'+str(i)] = fmpv.GetMaximumX()
    #
    #                 rfile.Close()
    #
    #                 df = df.append(se,ignore_index=True)
    #                 print(df.tail(10))
    #                 print('Iteration: ' + str(n) + ' of Total Iterations:' + str(nges))
    #                 df.to_csv(args.output)
    #
    #                 start = False
    #     #add to df
    #
    # df.to_csv(args.output)
    # print(df)
    # print('Finished!!!')




    #print()
    # df[-1] = np.zeros(len(columns))

    #print(df.head())
    # for task in range(10,11):
    #     for par in

            #change_paramter(cfile_name, sec, par, value)
            #os.system(cmd + ' --tasks ' + str(task))

    # jobs = []
    # for i in range(int(sys.argv[1])):
    #     p = multiprocessing.Process(target = heat, args=('Thread-'+str(i),))
    #     jobs.append(p)
    #     p.start()




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
