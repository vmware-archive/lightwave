import sys;
import re;
import os;
import ldap;
import time;
from common import *

#test configuration
SOURCE_HOST = 'gary-sles'
TARGET_HOST = 'gary-testvm'

SOURCE_DOMAIN = 'vsphere'
TARGET_DOMAIN = 'vsphere'

SOURCE_USERNAME = 'administrator'
TARGET_USERNAME = 'administrator'
USER_NAME = 'user1'

SOURCE_PASSWORD = 'vmware'
TARGET_PASSWORD = 'vmware'
USER_PASSWORD = 'Vmware@ng12345'

SOURCE_BASE = 'cn=ComponentManager,dc=vsphere,dc=local'
TARGET_BASE = 'cn=ComponentManager,dc=vsphere,dc=local'

def init_source():
    reset_rpms()
    reset_service()
    vdc_firstboot()
#main function
init_source()

source_uri = 'ldap://%s' % (SOURCE_HOST)
target_uri = 'ldap://%s' % (TARGET_HOST)
source_dn = 'cn=%s,cn=users,dc=%s,dc=local' % (SOURCE_USERNAME, SOURCE_DOMAIN)
target_dn = 'cn=%s,cn=users,dc=%s,dc=local' % (TARGET_USERNAME, TARGET_DOMAIN)
user_dn = 'cn=%s,%s' % (USER_NAME, SOURCE_BASE)

#initialize and log in ldap servers
s = ldap.initialize(source_uri)
s.simple_bind_s(source_dn, SOURCE_PASSWORD)

add_userwithpassword(s, user_dn, USER_PASSWORD)

#r = s.search_s(user_dn, ldap.SCOPE_BASE, 'cn=*', ['*','-'])
#for i in r:
#    print i

#modlist = []
#for i in r[0][1]:
#    if(i=='pwdLastSet'):
#        continue
#    modlist.append((i, r[0][1][i][0]))
#modlist.append(('passwordHashScheme','DEFAULT-vmdird-v1'))
#s.add_s('cn=user2,cn=ComponentManager,dc=csp,dc=com', modlist)

s.unbind_s()

print 'login user1 on source'
s = ldap.initialize(source_uri)
s.simple_bind_s(user_dn, USER_PASSWORD)
s.unbind_s()

#call vdcmerge
cmdline = '%s -u %s -w %s -H %s -U %s -W %s' % (vdcmerge,
        SOURCE_USERNAME, SOURCE_PASSWORD, TARGET_HOST, TARGET_USERNAME, TARGET_PASSWORD);
print cmdline
os.system(cmdline)

print 'login user1 on target'
s = ldap.initialize(target_uri)
s.simple_bind_s(user_dn, USER_PASSWORD)
s.unbind_s()

cmdline = '%s -u %s -w %s -D %s -U %s -W %s' % (vdcsplit, SOURCE_USERNAME, SOURCE_PASSWORD, 'vsphere.local', TARGET_USERNAME, TARGET_PASSWORD)
print cmdline
os.system(cmdline)


