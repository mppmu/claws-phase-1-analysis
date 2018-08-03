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

    df = pd.read_csv('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results_Run-900122_third_run.csv')
    print(df.tail(10))

    quant = 0.9
    quant2 = 0.5
    low = 0.6
    up = 1.05

    # df = df[df['WaveformDecomposition.stop_region'] == 3]
    # df.drop(['WaveformDecomposition.stop_region'], axis=1)



    #df = df[df['TRes_shift'].abs()<0.6]
    #df = df[df['TRes']<1.25]

    #df = df[(df['TRes']<1.75)&(df['MPV_1']>= low)&(df['MPV_1']<= up)&(df['MPV_2']>= low)&(df['MPV_2']<= up)&(df['MPV_3']>= low)&(df['MPV_3']<= up)&(df['MPV_4']>= low)& (df['MPV_4'] <= up)]

    df = df[(df['MPV_1']>= low)&(df['MPV_1']<= up)&(df['MPV_2']>= low)&(df['MPV_2']<= up)&(df['MPV_3']>= low)&(df['MPV_3']<= up)&(df['MPV_4']>= low)& (df['MPV_4'] <= up)]
    #df=df[(df['MPV_4']>= low)&(df['MPV_4']<= up)]
    #df = df[(df['TRes']<df['TRes'].quantile(0.8))]
    #df = df[(df['TRes']<df['TRes'].quantile(quant))&(df['MPV_1']>= low)&(df['MPV_1']<= up)&(df['MPV_2']>= low)&(df['MPV_2']<= up)&(df['MPV_3']>= low)&(df['MPV_3']<= up)&(df['MPV_4']>= low)& (df['MPV_4'] <= up)]

    #df = df[(df['TRes']<1.5)&(df['MPV_1']>= low)&(df['MPV_1']<= up)&(df['MPV_2']>= low)&(df['MPV_2']<= up)&(df['MPV_3']>= low)&(df['MPV_3']<= up)&(df['MPV_4']>= low)& (df['MPV_4'] <= up)]

    #df = df[(df['WaveformDecomposition.threshold']>= 1.0)& (df['WaveformDecomposition.threshold'] <= 4.0)]
#    df = df[(df['MPV_1']>= low)&(df['MPV_1']<= up)&(df['MPV_2']>= low)&(df['MPV_2']<= up)&(df['MPV_3']>= low)&(df['MPV_3']<= up)&(df['MPV_4']>= low)& (df['MPV_4'] <= up)]

    #df = df[df['MipTimeRetrieval.window_length']>10.0]
#    df = df[df['MipTimeRetrieval.window_length']>10.0]
    #df = df[(df['TRes']<df['TRes'].quantile(quant))&(df['MPV_1']>= df['MPV_1'].quantile(quant2))&(df['MPV_2']>= df['MPV_2'].quantile(quant2))&(df['MPV_3']>= df['MPV_3'].quantile(quant2))&(df['MPV_4']>= df['MPV_4'].quantile(quant2))]


    #df = df[(df['SystematicsStudy.threshold_tres']  < 0.65 )&(df['TRes']<df['TRes'].quantile(quant))&(df['MPV_1']>= df['MPV_1'].quantile(quant2))&(df['MPV_2']>= df['MPV_2'].quantile(quant2))&(df['MPV_3']>= df['MPV_3'].quantile(quant2))&(df['MPV_4']>= df['MPV_4'].quantile(quant2))]
    #df = df[(df['SystematicsStudy.threshold_tres']  < 0.65 )&(df['TRes']<df['TRes'].quantile(quant))&(df['MPV_1']>= df['MPV_1'].quantile(quant2))&(df['MPV_2']>= df['MPV_2'].quantile(quant2))&(df['MPV_3']>= df['MPV_3'].quantile(quant2))&(df['MPV_4']>= df['MPV_4'].quantile(quant2))]
    #df = df.drop(['Unnamed: 0'], axis=1)

    # print("TRes Mean PE: " + str(df[df['MipTimeRetrieval.timing_type'] == 'pe hit time'].loc[:,'TRes'].mean()) )
    # print("TRes Min PE: " + str(df[df['MipTimeRetrieval.timing_type'] == 'pe hit time'].min() ))
    # print("TRes Mean CF: " + str(df[df['MipTimeRetrieval.timing_type'] == 'constant fraction'].loc[:,'TRes'].mean()) )
    # print("TRes Min CF: " + str(df[df['MipTimeRetrieval.timing_type'] == 'constant fraction'].min() ))

    from matplotlib.ticker import ScalarFormatter, FormatStrFormatter


    #bins = 20
    fig =plt.figure(figsize=(24,8))
    ax = fig.add_subplot(141)
    print(df['MPV_4'].unique())
    print(df['MPV_4'].count())
    #ax.set_xlim(left=0.8,right=0.9)
    bins=20
    df['MPV_1'].hist(ax = ax, label='MPV_1', alpha=.5, range=(low, up), bins=bins)
    df['MPV_2'].hist(ax = ax, label='MPV_2', alpha=.5, range=(low, up), bins=bins)
    df['MPV_3'].hist(ax = ax, label='MPV_3', alpha=.5, range=(low, up), bins=bins)
    df['MPV_4'].hist(ax = ax, label='MPV_4', alpha=.5, range=(low, up), bins=bins)

    # ax.get_xaxis().get_major_formatter().set_useOffset(False)
    # ax.get_xaxis().get_major_formatter().set_scientific(False)
    ax.xaxis.set_major_formatter(FormatStrFormatter('%.3f'))
    #df.plot(x='TRes',y='MPV_4', style='o',ax = ax, label='MPV_4', alpha=.5)

    #ax.set_xlim(left=low,right=up)

#    ax.set_ylim(bottom=0.,top=200)
    #ax.semilogx()
    plt.legend()

    ax2 = fig.add_subplot(142)

    # df['MPV_1'].hist(ax = ax, label='MPV_1', alpha=.5)
    # df['MPV_2'].hist(ax = ax, label='MPV_2', alpha=.5)
    # df['MPV_3'].hist(ax = ax, label='MPV_3', alpha=.5)
    # df['MPV_4'].hist(ax = ax, label='MPV_4', alpha=.5)
    # ax.set_xlim(left=0,right=2.0)
    df.plot(x='TRes',y='MPV_1', style='o',ax = ax2, label='MPV_1', alpha=.5)
    #df.plot(x='TRes',y='MPV_4', style='o',ax = ax2, label='MPV_4', alpha=.5)
    ax2.get_yaxis().get_major_formatter().set_useOffset(False)
    ax2.yaxis.set_major_formatter(FormatStrFormatter('%.3f'))
    #ax2.set_ylim(left=bottom,top=up)

#    ax.set_ylim(bottom=0.,top=200)
    #ax.semilogx()
    plt.legend()


    ax3 = fig.add_subplot(143)
    bins = 25
    df[df['MipTimeRetrieval.timing_type'] == 'constant fraction']['TRes'].hist(ax = ax3, label='constant fractrion', color='red', alpha=.5, bins=bins)
    df[df['MipTimeRetrieval.timing_type'] == 'pe hit time']['TRes'].hist(ax = ax3, label='pe hit time', color='blue', alpha=.5,bins=bins)
    ax3.set_xlim(left=0,right=2.5)
    plt.legend()

    ax4 = fig.add_subplot(144)
    bins = 20
    df[df['MipTimeRetrieval.timing_type'] == 'constant fraction']['TRes_shift'].hist(ax = ax4, label='constant fractrion', color='red', alpha=.5, bins=bins)
    df[df['MipTimeRetrieval.timing_type'] == 'pe hit time']['TRes_shift'].hist(ax = ax4, label='pe hit time', color='blue', alpha=.5,bins=bins)
    ax4.set_xlim(left=-1,right=1)
    plt.legend()
    #ax = ax.hist(df[df['MipTimeRetrieval.timing_type'] == 'constant fraction']['TRes'].values, label='constant fractrion', color='red', alpha=.5)
    #ax = ax.hist(df[df['MipTimeRetrieval.timing_type'] == 'constant fraction'].values, label='constant fractrion', color='red', alpha=.5)
    #ax = ax.hist(df[df['MipTimeRetrieval.timing_type'] == 'pe hit time']['TRes'].values, label='pe hit time', color='blue', alpha=.5)

    #df = df[df['MipTimeRetrieval.timing_type'] == 'constant fraction']
    plt.savefig('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results_Run-900122_third_run_tres.png')
    #plt.show()
    #df[df['MipTimeRetrieval.timing_type'] == 'pe hit time']

    plt.figure()
    # sns.pairplot(data=df,hue="Survived", dropna=True)
    sns.pairplot(data=df)
    plt.savefig('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results_Run-900122_third_run.png')


    print('MINIMUM TIME RESOLUTION:')
    print( df.ix[df['TRes'].idxmin()] )
    print('MINIMUM TIME RESOLUTION CF:')
    print( df.ix[df[df['MipTimeRetrieval.timing_type'] == 'constant fraction']['TRes'].idxmin()] )

    print('MINIMUM TIME SHIFT:')

    print( df.ix[df['TRes_shift'].abs().idxmin()] )
    for i in range(1,5):
        print('MAXIMUM MPV'+str(i)+':')
        #print( df.ix[df['MPV_'+str(i)].idxmax()] )
        print( df.ix[(df['MPV_'+str(i)]-1).abs().idxmin()] )
        #print()
        #print( df[(df['MPV_'+str(i)]-1).abs().idxmin()] )

    print("CORRELATIONS:")
    print(df.corr())

    plt.figure(figsize=(16,16))
    sns.heatmap(df.corr(), annot=True, fmt=".3f")
    plt.savefig('/home/iwsatlas1/mgabriel/workspace/claws_phaseI/claws_calibration/SystematicsStudy/systematics_results_Run-900122_third_run_heat_map.png')
