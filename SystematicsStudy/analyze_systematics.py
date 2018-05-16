#!/usr/bin/env python3


import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
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


if __name__ == '__main__':

    df = pd.read_csv('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results.csv')
    print(df.tail(10))

    quant = 0.8
    quant2 = 0.7

    #df = df[(df['TRes']<df['TRes'].quantile(quant))&(df['MPV_1']>= df['MPV_1'].quantile(quant2))&(df['MPV_2']>= df['MPV_2'].quantile(quant2))&(df['MPV_3']>= df['MPV_3'].quantile(quant2))&(df['MPV_4']>= df['MPV_4'].quantile(quant2))]

    df = df.drop(['Unnamed: 0'], axis=1)
    df = df.drop(['Unnamed: 0.1'], axis=1)
    plt.figure()
    # sns.pairplot(data=df,hue="Survived", dropna=True)
    sns.pairplot(data=df)
    plt.savefig('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_pair_plot.png')


    print('MINIMUM TIME RESOLUTION:')
    print( df.ix[df['TRes'].idxmin()] )
    print('MINIMUM TIME SHIFT:')
    print( df.ix[df['TRes_shift'].abs().idxmin()] )
    for i in range(1,5):
        print('MAXIMUM MPV'+str(i)+':')
        print( df.ix[df['MPV_'+str(i)].idxmax()] )

    print("CORRELATIONS:")
    print(df.corr())
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
