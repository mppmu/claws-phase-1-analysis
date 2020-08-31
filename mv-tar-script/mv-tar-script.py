#!/usr/bin/python3

try:
    import os,shutil,zipfile,time,argparse
except ImportError:
    print ('Stuff not found in mv-tar-script.py')


defaultFolder         = '/remote/pcilc3/data/claws/phaseI'
destinationFolder    = '/remote/pcilc3/data/claws/phaseI'
destinationFolderZIP = '/remote/pcilc3/data/claws/phaseI'

def searchlist(files):
        """
        Takes a list of files as input searches it for .tdms and .tdms_index pairs.
        All valid pairs are return as a list of tuples. 
        """
        tdmsfiles=[]
        for file in files:
                if file.endswith('.tdms') and (file +'_index') in files:
                        tdmsfiles.append((file, file + '_index'))
        return tdmsfiles


def zipTDMSfiles(sourcedir,tdmsfiles, targetdir):
        """
        Takes the .tdms and .tdms_index in tdmsfiles and zips it to targetdir. If .zip already exists nothing is done
        """
        rarfile = tdmsfile[0][:-5]+'.zip'
        if not os.path.exists(os.path.join(targetdir,rarfile)):
                print ('Compressing %s and %s to % s' % (tdmsfiles[0],tdmsfiles[1], targetdir))
                zipf = zipfile.ZipFile(os.path.join(targetdir, rarfile),mode = 'w', compression = zipfile.ZIP_DEFLATED)
                zipf.write(os.path.join(sourcedir,tdmsfiles[0]), arcname = tdmsfiles[0])
                zipf.write(os.path.join(sourcedir,tdmsfiles[1]), acrname = tdmsfiles[1])
                zipf.close()
                print ('Done writing ' + rarfile)
        return 0




if __name__ == '__main__':

        parser = argparse.ArgumentParser(description='Script to zip CALWS data files. Searches the input directory and zips all .tdms and .tdms_index files inside..')
        parser.add_argument('-s', '--sourcedir', type = str, default = defaultFolder, help='Path to the TDMS files to zip, default is: ' + defaultFolder, required=False)
        args = parser.parse_args()
        parser.add_argument('-t','--targetdir', type = str, default = args.sourcedir, help = 'Target path for zip files. Default is source dir', required=False)
        args = parser.parse_args()
        
        print (type(os.listdir(args.sourcedir)))
        for root, dirs, files in os.walk(args.sourcedir):
                tdmsfiles=searchlist(files)
                for tdmsfile in tdmsfiles:
                    zipTDMSfiles(root,tdmsfile,args.targetdir)
