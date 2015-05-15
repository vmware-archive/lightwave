import sys;
import re;
import os;
import ldap;
import time;
from common import *

#test configuration
TARGET_HOST = '192.168.103.131'

SOURCE_USERNAME = DEFAULT_USER
TARGET_USERNAME = DEFAULT_USER

SOURCE_PASSWORD = DEFAULT_PASSWORD
TARGET_PASSWORD = DEFAULT_PASSWORD
TARGET_DOMAIN = DEFAULT_DOMAIN

def init_source():
    reset_rpms()
    reset_service()
    vdc_firstboot()

#main function
init_source()

#call vdcmerge
for i in xrange(2):
    cmdline = '%s -u %s -w %s -H %s -U %s -W %s' % (vdcmerge,
            SOURCE_USERNAME, SOURCE_PASSWORD,
            TARGET_HOST, TARGET_USERNAME, TARGET_PASSWORD);
    print cmdline
    os.system(cmdline)

    cmdline = '%s -u %s -w %s' % (vdcsplit, SOURCE_USERNAME, SOURCE_PASSWORD)
    print cmdline
    os.system(cmdline)



