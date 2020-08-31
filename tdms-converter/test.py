#!/usr/bin/env python3

try:
    import pyTDMS, argparse, os
except ImportError:
    print ("Couldn't import some shit in tdms-converter.py")




if __name__ == '__main__':
    print('Starting converter')
    
    tdms_path=os.getcwd() + '/file-in.tdms'
    root_path=os.getcwd() + '/file-out.root'
    parser = argparse.ArgumentParser(description='CLAWS converter from NI TDMS format to ROOT ntubples.')
    parser.add_argument('-i', '--filein', type = str, default = tdms_path, help='Path to the TDMS file to convert, default is: ' + tdms_path, required=True)
    parser.add_argument('-o','--fileout', type =str, default = root_path, help = 'Path and name of the final root file, default is: ' + root_path, required = False)
    args = parser.parse_args()

    try:
        (objects,rawdata)=pyTDMS.read(args.filein)
    except KeyboardInterrupt:
        print ('Keyboard interrupt in reading in the TDMS file.')
