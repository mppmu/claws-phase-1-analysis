#!/usr/bin/python

import os
import shutil
import time
import zipfile

sourceFolder = "ssd_folder"
destinationFolder = "hdd_folder"
destinationFolderZIP = "zip_folder"

def zipdir(path, ziph):
    for root, dirs, files in os.walk(path):
        for file in files:
            ziph.write(os.path.join(root, file))


if __name__ == '__main__':
	while(True):
		for file in os.listdir(sourceFolder):
			if file.endswith(".ready"):
				print("Moving %s to %s" % (file, destinationFolder))
				try: 
					shutil.move(os.path.join(sourceFolder, file), destinationFolder)
				except IOError, e:
					print "Unable to move file. %s" % e
				print("Compressing %s" % (file))
				zipf = zipfile.ZipFile(os.path.join(destinationFolderZIP, file) + '.zip', 'w')
				zipdir(os.path.join(destinationFolder, file), zipf)
				zipf.close()
				print("Done.")

		time.sleep(10)
	
