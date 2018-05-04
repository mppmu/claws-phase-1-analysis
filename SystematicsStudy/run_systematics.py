#!/usr/bin/env python3

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

def change_parameter(cfile_name, sec, par, value):
    config = configparser.ConfigParser()
    config.optionxform = str
    config.read(cfile_name)

    config[sec][par] = str(value)

    with open(cfile_name, 'w') as cfile:
        config.write(cfile)


if __name__ == '__main__':
    cfile_name = '/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/calibration_phase1_myon_sys_study.ini'

    cmd = '/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/build/calibration -c ' + cfile_name + ' --profile-timing 1 --data.input /remote/ceph/group/ilc/claws/data/Myon/Converted/Run-900122'
    # --tasks 10

    #parameters_name = ['SystematicsStudy.threshold_tres','SystematicsStudy.range_time','SystematicsStudy.threshold_mpv','SystematicsStudy.window_length_mpv']
    parameters_name = ['SystematicsStudy.threshold_tres','SystematicsStudy.range_time','SystematicsStudy.start_mpv','SystematicsStudy.window_length_mpv']
    results_name = ['MPV_1','MPV_2','MPV_3','MPV_4', 'TRes', 'TRes_shift']

    # columns = parameters_name + results_name
    # df = pd.DataFrame(columns = columns)
    # #se = pd.Series(np.zeros(len(columns), index = columns)
    #
    # se = pd.Series( np.zeros(len(columns)), index=columns)
    # df.iloc[1] = [random.randint(-1,1) for n in range(len(columns))]
    # #df.iloc(-1) = se
    # print(df.head())

    columns = parameters_name + results_name
    df = pd.DataFrame(columns=columns)

    config = configparser.ConfigParser()
    config.optionxform = str
    config.read(cfile_name)



    #systematics_study_range_time = config['SystematicsStudy']['range_time']
    #systematics_study_threshold_mpv = config['SystematicsStudy']['threshold_mpv']
    #systematics_study_window_length_mpv = config['SystematicsStudy']['window_length_mpv']
    systematics_study_start_mpv = config['SystematicsStudy']['start_mpv']

    n = 1
    #nges = len(xrange(14980,15021,10)) + len(xrange(10, 61,10) + len(xrange(4,9)) + len( np.arange(0.2, 0.8, 0.1) )
    nges = 5*6*6*7
    for systematics_study_start_mpv in range(14980,15021,10):
        for systematics_study_window_length_mpv in range(10, 61,10):
            for systematics_study_range_time in range(4,9):

                thrtes_array = np.arange(0.2, 0.8, 0.1)
                for systematics_study_threshold_tres in thrtes_array:
                    n = n+1
                    #change config file
                    change_parameter(cfile_name, 'SystematicsStudy', 'threshold_tres', systematics_study_threshold_tres)

                    #run cmd
                    os.system(cmd + ' --tasks 10')

                    se = pd.Series( np.zeros(len(columns)), index=columns)

                    se['SystematicsStudy.threshold_tres'] = systematics_study_threshold_tres
                    se['SystematicsStudy.range_time'] = systematics_study_range_time
                    se['SystematicsStudy.start_mpv'] = systematics_study_start_mpv
                    se['SystematicsStudy.window_length_mpv'] = systematics_study_window_length_mpv

                    rfile = TFile('/remote/ceph/group/ilc/claws/data/Myon/Converted/Run-900122/Results/SystematicsStudy/run_900122_systematics_ph1v4.root', 'OPEN')
                    # #f = TFile('mytree.root','RECREATE')
                    #
                    htres = rfile.Get('time_resolution')
                    ftres = htres.GetFunction('gaus')
                    se['TRes_shift'] = ftres.GetParameter(1)/1e-9
                    se['TRes'] = ftres.GetParameter(2)/1e-9
                    for i in range(1,5):
                        hmpv = rfile.Get('FWD'+str(i) + '_pe_per_event')
                        fmpv = hmpv.GetFunction('FWD'+str(i) + '_pe_per_eventlangaus')
                        se['MPV_'+str(i)] = fmpv.GetMaximumX()

                    rfile.Close()

                    df = df.append(se,ignore_index=True)
                    print(df.tail(10))
                    print('Iteration: ' + str(n) + ' of Total Iterations:' + str(nges))
                    df.to_csv('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results.csv')
        #add to df

    df.to_csv('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results.csv')
    print(df)
    print('Finished!!!')




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
