#!/usr/bin/env python3

# import sys
# sys.path.insert(0, '/home/iwsatlas1/mgabriel/.local/lib/python3.4/site-packages/')
from rootpy.io import root_open
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
%matplotlib inline

import matplotlib as mpl
mpl.font_manager.findSystemFonts()
# mpl.get_configdir()
# mpl.matplotlib_fname()
plt.colormaps()
cm.datad
plt.style.use('phd_thesis')
# plt.style.available

int_file = root_open(
    '/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/Run-401141-Int000.root', 'open')
ph_file = root_open(
    '/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/Data/Event-401141041.root', 'open')


fwd1_int = int_file.Get('FWD1-INT')
fwd1_ph = ph_file.Get('FWD1')

int_x = np.zeros((230,), dtype=np.double)
int_y = np.zeros((230,), dtype=np.double)

bins = fwd1_int.nbins()
bins

int_shift = 0
for i in range(int_shift, int_shift + 230):
    int_x[i - int_shift] = ( i - int_shift ) * 0.8
    int_y[i - int_shift] = (-1 * fwd1_int[i + 1].value) / (256**2) * 50

# ph_x = np.zeros((fwd1_ph.nbins(),), dtype=np.double)
# ph_y = np.zeros((fwd1_ph.nbins(),), dtype=np.double)

ph_x = np.zeros((320,), dtype=np.double)
ph_y = np.zeros((320,), dtype=np.double)

bins = fwd1_ph.nbins()
# ax1.errorbar(ph_x, ph_y, xerr=0.4, yerr=3.9, color=(65 / 255., 68 / 255., 81 / 255.),
#              label='Physics waveform')
# # 32512*0.9
#ax1.set_xlim(172000 * 0.8 - 8, (172000 + 230) * 0.8 + 8)
# for i in range(0, bins):
shift = 172025
for i in range(shift, shift + 320):
    ph_x[i - shift] = (i - shift) * 0.8
    ph_y[i - shift] = -1 * \
        ((fwd1_ph[i + 1].value - 32512 * 0.9) / (256**2) * 1000)
    #ph_y[i] = -1 * 5

# <codecell>
two_column_width = 183 / 25.4
two_column_width
# fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(
# two_column_width, two_column_width * 1 / 3), gridspec_kw={'width_ratios': [4, 3]})
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(
    two_column_width, two_column_width * 2 / 3))
# ax1.plot(ph_x, ph_y, aa=True, drawstyle='steps-post', color=(65 / 255.,
#                                                              68 / 255., 81 / 255.), linewidth=1, label='Physics waveform', fillstyle='full')
#
ax1.axhline(linestyle="--", lw=0.5, color=(154 / 255., 154 / 255., 154 / 255.))
ax1.axhline(100, linestyle="--", lw=0.5,
            color=(154 / 255., 154 / 255., 154 / 255.))
ax1.axhline(200, linestyle="--", lw=0.5,
            color=(154 / 255., 154 / 255., 154 / 255.))

ax1.plot(ph_x, ph_y, aa=True, drawstyle='steps-post', color=(0 / 255.,
                                                             0 / 255., 0 / 255.), linewidth=1, label='Physics waveform', fillstyle='full')
ax1.fill_between(ph_x, ph_y - 7.8125 / 2, ph_y + 7.8125 / 2, step='post',
                 facecolor=(154 / 255., 154 / 255., 154 / 255.), alpha=0.5, antialiased=True)

# Set the Axis limits
# ax1.set_xlim(172000 * 0.8 - 8, (172000 + 230) * 0.8 + 8)
ax1.set_ylim(-4, 230 * 0.8 + 4)
# ax1.set_xticks([0, 50, 100, 150, 200, 300])
# ax1.set_xticks([25, 75, 125, 175, 225], minor=True)

ax1.set_ylim(-5, 225)

ax1.set_yticks([0.0, 100, 200])
ax1.set_yticks([-50, 50, 150], minor=True)

ax1.tick_params(direction='in', which='both', labelsize=8)
# Visibility:
ax1.spines["top"].set_visible(False)
# ax1.spines["bottom"].set_visible(False)
ax1.spines["right"].set_visible(False)
# ax1.spines["left"].set_visible(False)
#plt.setp(ax1.spines.values(), linewidth=0.8,)

ax1.tick_params(direction='in', labelsize=8,
                )
# Labels
ax1.set_ylabel('Voltage [mV]', horizontalalignment='right',
               y=1.0, weight='normal', labelpad=4)
ax1.set_xlabel('Time [ns]', horizontalalignment='right', x=1.0)
# ax1.xaxis.label.set_visible(False)
#ax1.set_xlabel('Time [ns]', horizontalalignment='right', x=1.0)
#ax1.set_xlabel(labelpad, horizontalalignment='right', x=1.0)

# lg1 = ax1.legend(frameon=False, numpoints=2, loc=(0.68, 0.8))
# plt.setp(lg1.get_lines(), linewidth=1.5)
# Calibration waveform
tab20 = mpl.cm.get_cmap('tab20')
# ax2.plot(int_x, int_y, aa=True, drawstyle='steps-post', linewidth=1,
#          label='Calibration waveform', color=tab20(6))
ax2.axhline(linestyle="--", lw=0.5, color=(154 / 255.,
                                           154 / 255., 154 / 255.))
ax2.axhline(1, linestyle="--", lw=0.5, color=(154 / 255.,
                                              154 / 255., 154 / 255.))
ax2.axhline(2, linestyle="--", lw=0.5,
            color=(154 / 255., 154 / 255., 154 / 255.))
# ax2.plot(int_x, int_y, aa=True, drawstyle='steps-post', linewidth=1,
#          label='Calibration waveform',  color=(205 / 255., 41 / 255., 15 / 255.))
# ax2.fill_between(int_x, int_y - 0.390625, int_y + 0.390625,
#                  step='post', facecolor=(154 / 255., 154 / 255., 154 / 255.), alpha=0.5, antialiased=True)

ax2.fill_between(int_x, int_y - 0.390625 / 2, int_y + 0.390625 / 2,
                 step='post', facecolor=tab20(7), alpha=0.75, antialiased=True)

# ax2.plot(int_x, int_y, aa=True, drawstyle='steps-post', linewidth=1,
#          label='Calibration waveform',  color=(205 / 255., 41 / 255., 15 / 255.))
ax2.plot(int_x, int_y, aa=True, drawstyle='steps-post', linewidth=1,
         label='Calibration waveform',  color=(205 / 255., 41 / 255., 15 / 255.))
# Set the Axis limits
#ax2.set_xlim(172000 * 0.8 - 8, (172000 + 230) * 0.8 + 8)
ax2.set_xlim(-4, 230 * 0.8 + 4)
#ax2.set_xticks([0, 50, 100, 150, 200])
#ax2.set_xticks([-25, 25, 75, 125, 175, 225], minor=True)
# ax2.set_xticklabels(['',0,'',50,'',100,'',150,'',200,''])

ax2.set_ylim(-0.5, 2.25)

ax2.set_yticks([0.0, 1, 2])
ax2.set_yticks([-0.5, 0.5, 1.5], minor=True)
ax2.tick_params(direction='in', which='both', labelsize=8)


# ax.set_xticks([137600, 137650, 137700, 137750, 137800])
# ax.set_yticks([0.0, 100, 200])
#
# # Visibility:
ax2.spines["top"].set_visible(False)
ax2.spines["right"].set_visible(False)
#plt.setp(ax2.spines.values(), linewidth=0.8)
# # Labels

ax2.set_ylabel('Voltage [mV]', horizontalalignment='right', y=1.0, labelpad=12)
ax2.set_xlabel('Time [ns]', horizontalalignment='right', x=1.0)

# lg2 = ax2.legend(frameon=False, numpoints=2, loc=(0.68, 0.8))
# plt.setp(lg2.get_lines(), linewidth=1.5)

# for legobj in lg2.legendHandles:
#      legobj.set_linewidth(2.0)

#mpl.rcParams['axes.linewidth'] = 1
# ax1.xaxis.linewidth = 3
#rc('axes', linewidth=0.5)

plt.tight_layout()
# fig.savefig("/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/raw_waveforms.pdf",
#             dpi='figure', bbox_inches='tight')
# fig.savefig("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Calibration/Images/raw_waveforms.pdf",
#             dpi='figure', bbox_inches='tight')
fig.savefig(
    "/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/raw_waveforms.pdf")
fig.savefig(
    "/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Calibration/Images/raw_waveforms.pdf")
plt.show()

# # <codecell>
# fig, ax = plt.subplots(figsize=(7.204724409, 7.204724409 * 1 / 3))
#
# ax.plot(ph_x, ph_y, aa=True, drawstyle='steps-post', color=(65 / 255.,
#                                                             68 / 255., 81 / 255.), linewidth=0.8, label='Physics waveform')
# # Set the Axis limits
# ax.set_xlim(172000 * 0.8 - 8, (172000 + 230) * 0.8 + 8)
# ax.set_ylim(-50, 250)
#
# ax.axhline(linestyle="--", lw=0.8, color="black", alpha=0.3)
#
# ax.set_xticks([137600, 137650, 137700, 137750, 137800])
# ax.set_yticks([0.0, 100, 200])
#
# # Visibility:
# ax.spines["top"].set_visible(False)
# # # ax1.spines["bottom"].set_visible(False)
# ax.spines["right"].set_visible(False)
# # # ax1.spines["left"].set_visible(False)
# ax.tick_params(direction='out', labelsize=8)
# # Labels
# ax.set_ylabel('Voltage [mV]', horizontalalignment='right', y=1.0)
# ax.set_xlabel('Time [ns]', horizontalalignment='right', x=1.0)
#
#
# fig.savefig("/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/raw_physics.pdf",
#             dpi='figure', bbox_inches='tight')
# fig.savefig("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Calibration/Images/raw_physics.pdf",
#             dpi='figure', bbox_inches='tight')
# plt.show()
#
# # <codecell>
# fig, ax = plt.subplots(figsize=(7.204724409, 7.204724409 * 1 / 3))
#
# ax.plot(int_x, int_y, aa=True, drawstyle='steps-post', linewidth=0.8, label='Physics waveform', color = cm.get_cmap('tab10',1))
# # Set the Axis limits
# ax.set_xlim(172000 * 0.8 - 8, (172000 + 230) * 0.8 + 8)
# ax.set_ylim(-0.5, 2.5)
#
# ax.axhline(linestyle="--", lw=0.8, color="black", alpha=0.3)
#
# ax.set_xticks([137600, 137650, 137700, 137750, 137800])
# ax.set_yticks([0.0, 100, 200])
#
# # Visibility:
# ax.spines["top"].set_visible(False)
# # # ax1.spines["bottom"].set_visible(False)
# ax.spines["right"].set_visible(False)
# # # ax1.spines["left"].set_visible(False)
# ax.tick_params(direction='out', labelsize=8)
# # Labels
# ax.set_ylabel('Voltage [mV]', horizontalalignment='right', y=1.0)
# ax.set_xlabel('Time [ns]', horizontalalignment='right', x=1.0)
#
#
# fig.savefig("/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/raw_calibration.pdf",
#             dpi='figure', bbox_inches='tight')
# fig.savefig("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Calibration/Images/raw_calibration.pdf",
#             dpi='figure', bbox_inches='tight')
# plt.show()

# #fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(7.204724409, 3.503937008))
# fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(7.204724409, 7.204724409 * 2 / 3))
# #fig.set_size_inches(1400 / fig.dpi, 600 / fig.dpi)
# # fig.set_size_inches(6.063, 6.063 * 2 / 3)
#
# ax1.plot(ph_x, ph_y, aa=True, drawstyle='steps-post', color=(65 / 255.,
#                                                              68 / 255., 81 / 255.), linewidth=0.8, label='Physics waveform')
#
# # ax1.errorbar(ph_x, ph_y, xerr=0.4, yerr=3.9, color=(65 / 255., 68 / 255., 81 / 255.),
# #              label='Physics waveform')
# ax2.plot(int_x, int_y, aa=True, drawstyle='steps-post',
#          linewidth=0.8, color='red')
#
# #######################
# # Axis
# #######################
#
# # Visibility:
# ax1.spines["top"].set_visible(False)
# # ax1.spines["bottom"].set_visible(False)
# ax1.spines["right"].set_visible(False)
# # ax1.spines["left"].set_visible(False)
#
# ax2.spines["top"].set_visible(False)
# # ax2.spines["bottom"].set_visible(False)
# ax2.spines["right"].set_visible(False)
# # ax2.spines["left"].set_visible(False)
#
# # Ensure that the axis ticks only show up on the bottom and left of the plot.
# # Ticks on the right and top of the plot are generally unnecessary chartjunk.
# ax1.get_xaxis().tick_bottom()
# ax1.get_yaxis().tick_left()
# ax2.get_xaxis().tick_bottom()
# ax2.get_yaxis().tick_left()
#
# # Set the Axis limits
# ax1.set_xlim(172000 * 0.8, (171950 + 300) * 0.8)
#
# ax1.set_ylim(-50, 250)
# ax2.set_xlim(-8, 230 * 0.8 + 8)
# ax2.set_ylim(-0.5, 2.5)
#
# # Ticks
# # ax1.tick_params(axis="both", which="both", bottom="off", top="off",
# #                labelbottom="on", left="off", right="off", labelleft="on")
#
# # ax2.tick_params(axis="both", which="both", bottom="on", top="off",
# #                labelbottom="on", left="o", right="off", labelleft="on")
# ax2.set_xticks([0, 50, 100, 150, 200])
# ax2.set_yticks([-0.5, 0.0, 0.5, 1.0, 1.5, 2.0])
# # ax2.locator_params(axis='x', nbins=5)
# # ax2.locator_params(axis='y', nbins=5)
#
# # Labels
# # hfont = {'fontname':'Helvetica'}
#
# ax1.set_xlabel('')
# ax1.set_ylabel('Voltage [mV]', horizontalalignment='right',
#                y=1.0, fontsize=11, family="Helvetica")
# ax2.set_xlabel('Time [ns]', horizontalalignment='right',
#                x=1.0, fontsize=11, family='Helvetica', weight='medium')
# ax2.set_ylabel('')
#
# # # Make sure your axis ticks are large enough to be easily read.
# # # You don't want your viewers squinting to read your plot.
# # ax2.yticks(fontsize=14)
# # ax2.xticks(fontsize=14)
#
# # Provide tick lines across the plot to help your viewers trace along
# # the axis ticks. Make sure that the lines are light and small so they
# # don't obscure the primary data lines.
#
# ax1.axhline(linestyle="--", lw=1, color="black", alpha=0.3)
# ax2.axhline(linestyle="--", lw=1, color="black", alpha=0.3)
#
# myfontsize = 16
# myfontname = ''
# myfontweight = 'bold'
#
#
# fig.savefig("/home/iwsatlas1/mgabriel/workspace/Ploting/PhD_Thesis/Calibration/raw_waveforms.pdf",
#             dpi='figure', bbox_inches='tight')
# fig.savefig("/home/iwsatlas1/mgabriel/Documents/PhD_Thesis/Calibration/Images/raw_waveforms.pdf",
#             dpi='figure', bbox_inches='tight')
# plt.show()
