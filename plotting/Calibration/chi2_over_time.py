#!/usr/bin/env python3

import os

if __name__ == '__main__':

     #  print(bcolors.OKGREEN + "Running '{}'".format(__file__) + bcolors.ENDC)

    # parser = argparse.ArgumentParser(
    #     description='CLAWS converter from LabVIEW Lab DAQ for myon measurements to phase I root format.')
    #
    # parser.add_argument('-i', '--input', type=str, default='/home/iwsatlas1/mgabriel/workspace/claws_phaseI/MyonDAQToPhaseI/Berta_ladder7.1_56.6-mV_BC408_2x2_final_2x2-02',
    #                     help='Path to the TDMS file to convert, default is: ', required=True)
    #
    # parser.add_argument('-o', '--output', type=str, default='/home/iwsatlas1/mgabriel/workspace/claws_phaseI/MyonDAQToPhaseI/output',
    #                     help='Folder name where the out put should go to. For exampel foo/bar/Run-900000', required=True)
    #
    # parser.add_argument('-n', '--run_number', type=int, default=900000,
    #                     help='Run number needed', required=True)
    #
    # parser.add_argument('-t', '--conversion_table', type=str, default='/remote/ceph/group/ilc/claws/data/Myon/conversion_table.ini',
    #                     help='Absolut path for file to store the conversion_table.', required=False)
    #
    # parser.add_argument('-v', '--verbose', help=', default is: ',
    #                     action='store_true', required=False)

    # myargs = namedtuple('myargs', ['verbose', 'input', 'output'])
    # args = myargs(True, '/home/iwsatlas1/mgabriel/workspace/claws_phaseI/MyonDAQToPhaseI/Berta_ladder7.1_56.6-mV_BC408_2x2_final_2x2-02',
    #               '/home/iwsatlas1/mgabriel/workspace/claws_phaseI/MyonDAQToPhaseI/output/Run-900100')
    #
    # args

    args = parser.parse_args()

    logger = logging.getLogger(__name__)

    if args.verbose:
        logger.setLevel(logging.DEBUG)
        print('Logging DEBUG')
    else:
        logger.setLevel(logging.INFO)
        print('Logging INFO')

    (root, dirs, files) = next(os.walk(data_folder))

    for filename in files:
        if filename.startswith('int-wf') and filename.endswith('.dat') and (int(filename[17:-4]) < 1000):
import nptdms

tdmsfile = nptdms.TdmsFile(
    '/remote/ceph/group/ilc/claws/data/RAW/connecticut/2016-05-26/Run-401161/Run-401161-Intermediate.tdms')

properties = tdmsfile.object().properties

import pandas as pd
import datetime
import pytz
from time import mktime
pytz.utc.localize(
mktime(properties['TimeStamp'].timetuple())
properties['TimeStamp'].timestamp()
datetime.datetime(1970, 1, 1, tzinfo= < UTC > )

pd.to_datetime(1464190755.587041)


self._name=properties['name']
self._nr=properties['name'][4:-13]
    self._timestamp=properties['TimeStamp']
    self._unixtime=properties['TimeStamp'].timestamp()

    self._logger.debug('Running intermediate conversion for Run-' + str(self._nr) +
                       ' at time: ' + str(self._timestamp) + '/' + str(self._unixtime))

    self._events={}
    for i in range(len(tdms_file.group_channels('FWD1-INT'))):
        self._events[self._name[:-12] + 'Int' +
                     '{0:03d}'.format(i)]=IntermediateEvent(tdms_file, self._nr, i)
