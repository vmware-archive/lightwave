import os;
from common import *

#main function
init_source()

#call vdcmerge
for i in xrange(1024):
    cmdline = '%s -u %s -w %s -d %s%d -t' % (vdcpromo,
            "administrator", "vmware", "tenant", i);
    print cmdline
    os.system(cmdline)




