# you have to run it with root permission
import getopt
from common import *

options = 'cd:'
cleanup = False
domain = DEFAULT_DOMAIN

opts, args = getopt.getopt(sys.argv[1:], options)
for o, p in opts:
    if o == '-d':
        domain = p
    else:
        cleanup = True

print "Domain:%s" % domain
print "Cleanup:%s" % cleanup

reset_rpms()
reset_service()

if(not cleanup):
    vdc_firstboot()




