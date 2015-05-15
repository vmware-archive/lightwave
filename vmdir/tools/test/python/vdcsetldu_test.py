import os;
import ldap;
from common import *

#test configuration


if(os.name == 'nt'):
    HOST_NAME = 'GARY-2003'
    HOST_PORT='389'
else:
    HOST_NAME = 'gary-sles'
    HOST_PORT = '11711'
DOMAIN_NAME = 'csp.com'

ADMIN_NAME = 'administrator'
ADMIN_PASSWORD = 'vmware'

reset_service()
vdc_promo(DOMAIN_NAME)

source_uri = 'ldap://%s:%s' % (HOST_NAME, HOST_PORT)

cmdline = '%s -h %s -d %s -u %s -w %s' % (vdcsetupldu,
        source_uri, DOMAIN_NAME, ADMIN_NAME, ADMIN_PASSWORD);
cmdline += ' -v' #verbose mode
print cmdline
os.system(cmdline)