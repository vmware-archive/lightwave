#!/usr/bin/env python

import os
import sys
import re
import subprocess
import ctypes
import shutil
import time
import traceback
import getopt
import logging

if not os.environ['VMWARE_PYTHON_PATH'] in sys.path:
    sys.path.append(os.environ['VMWARE_PYTHON_PATH'])

from cis.defaults import *
from cis.utils import *
from cis.tools import *
from cis.exceptions import *

logDir = os.path.join(os.environ['VMWARE_LOG_DIR'], 'firstboot')

SSO_PRODUCT_NAME = "VMware Identity Services"
if is_linux():
   installbin = os.path.normpath('/opt/vmware/lib64/')
else:
   installbin = os.path.normpath(os.path.join(get_cis_install_dir(), SSO_PRODUCT_NAME))
sys.path.append(installbin)

setupLogging("vmidentity-updateboot", logDir=logDir)

class VMwareIdentityUpdate:

    def __init__(self):

        self._component_name = "sso"
        self.__sts_instance_name = def_by_os( "vmware-sts", "VMwareSTSService" )
        self.__companyName = "VMware, Inc."

        self._home_dir = def_by_os("%s/vmware-%s" % (get_cis_install_dir(), self._component_name),
                                    os.path.join(get_cis_install_dir(), self._component_name))

        self.__tcinst_root = self.getTCInstanceRootPath()
        self.__tc_sts_base = os.path.join(self.__tcinst_root, self.__sts_instance_name)
        self.__warNames = ['ims', 'websso', 'sts', 'sso-adminserver', 'lookupservice']

    def getTCInstanceRootPath(self):
        return def_by_os(self._home_dir, '%s' % (os.environ['VMWARE_RUNTIME_DATA_DIR']) )

    #return where webapp files was installed by installer.
    def getWebAppFilePath(self):

        if is_windows():
            return os.path.join(self.getVMIdentityInstallPath(), "Tomcat", "webapps")
        else:
            return "/opt/vmware/webapps"

    def getVMIdentityInstallPath(self):

        import _winreg
        if is_windows():
            key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'Software\\%s\\%s' %(self.__companyName, SSO_PRODUCT_NAME))
            installPath = _winreg.QueryValueEx(key, "InstallPath")[0]
            return installPath
        else:
            return ""

    # Retryable shutil.rmtree function
    def retryableRmtree(self, path, nRetry = 100, secToSleep = 2):
        ok = False
        for i in range(1, nRetry + 1):
            try:
                shutil.rmtree(path)
                ok = True
                break
            except OSError, err:
                logging.critical("Failed to remove path %s with shutil.rmtree at attempt %d: %s" % (path, i, err))
                logging.critical("Files in path %s at attempt %d: %s" % (path, i, str(os.listdir(path))))
                time.sleep(secToSleep)

        if not ok:
            logging.critical("Failed to remove path %s with shutil.rmtree, even after %d attempts." % (path, nRetry))
        else:
            logging.info("Path %s successfully removed." % path)

    #files get installed to a different dir than webapps via msi/rpm
    #During firsttime install, firstboots will move these files.
    #During update, we need to do the same.
    def copyWarFilesFromSource(self):
        tc_sts_webapps_folder = os.path.join(self.__tc_sts_base, "webapps")
        source_folder = self.getWebAppFilePath()

        for file in self.__warNames:
            shutil.copy2(os.path.join(source_folder, file+'.war'), tc_sts_webapps_folder)

    #When updating war files, remove the expanded folder so when the service
    #starts again, tcserver will expand war files.
    def updateWarFiles(self):
        tc_sts_webapps_folder = os.path.join(self.__tc_sts_base, "webapps")

        for folder in self.__warNames:
            folderToRemove = os.path.join(tc_sts_webapps_folder, folder)
            if os.path.exists(folderToRemove):
                self.retryableRmtree(folderToRemove)

        self.copyWarFilesFromSource()

    def update(self):
        self.updateWarFiles()

    #rollback should be called after msi/rpm rollback
    #so previous files are restored at source location.
    def rollback(self):
        self.updateWarFiles()

#update boot calls this script with action=update
#rollback boot calls this script with action=rollback
def main (argv):

    try:

        logging.info("Begin VMware Identity Service Update")

        vmidentityUpdate = VMwareIdentityUpdate()

        action = "update"
        opts, args = getopt.getopt(sys.argv[1:], "", ["action="])
        for opt, arg in opts:
            if opt == "--action":
                action = arg
        logging.info("Processing VMware Identity Update for %s" % action)
        if action == 'update':
            vmidentityUpdate.update()
        elif action == 'rollback':
            vmidentityUpdate.rollback()

        logging.info("Successfully processed VMware Identity Update for %s" % action)
        sys.exit(0)

    except Exception, e:
        logging.critical("VMware Identity Service Update failed.")
        logging.critical("Exception: %s" % traceback.format_exc())

        sys.exit(1)

if __name__ == "__main__":

    main(sys.argv)
