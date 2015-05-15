import sys;
import re;
import os;
import ldap;
import time;
from common import *

#test configuration
source_host = 'gary-sles'
target_host = 'gary-testvm'

source_port = '11711'
target_port = '11711'

source_domain = 'bellevue'
target_domain = 'csp'

source_username = 'administrator'
target_username = 'administrator'

source_password = 'vmware'
target_password = 'vmware'

source_base = 'cn=ComponentManager,dc=bellevue'
target_base = 'cn=ComponentManager,dc=csp'

server_id = '2'

CHILDREN_NUM = 10
LAYER_NUM = 3
#test data
container_list = [('objectClass', 'vmIdentity-Container')]
user_list = [('objectClass', 'vmIdentity-User')]

def find_rdn(dn):
    m = re.match(r'cn=(.+?),', dn)
    return m.group(1)

def delete_dn(l, dn):
    try:
        l.delete_s(dn)
    except ldap.NO_SUCH_OBJECT:
        pass

def dfs_clean(l, dn):
    try:
        r = l.search_s(dn, ldap.SCOPE_BASE, 'cn=*')
    except:
        return

    r = l.search_s(dn, ldap.SCOPE_ONELEVEL, 'cn=*')
    for i in r:
        dfs_clean(l, i[0])
    delete_dn(l, dn)

def add_entry(l, entry):
    modlist = []
    modlist.append(('cn', find_rdn(entry[0])))
    if(entry[1] == 'user'):
        modlist.append(('vmIdentity-Account', find_rdn(entry[0])))
        modlist.extend(user_list)
    else:
        modlist.extend(container_list)
    l.add_s(entry[0], modlist)

def dfs_add(l, dn, layer):
    if layer == 0:
        add_entry(l, (dn, 'user'))
    else:
        add_entry(l, (dn, 'container'))
        for i in xrange(CHILDREN_NUM):
            newdn = None
            if layer == 1:
                newdn = 'cn=user%d,%s' % (i, dn)
            else:
                newdn = 'cn=team%d,%s' % (i, dn)
            dfs_add(l, newdn, layer - 1)

def dfs_check(l, dn, layer):
    try:
        r = l.search_s(dn, ldap.SCOPE_ONELEVEL, 'cn=*')
        expectednum = 0 if layer == 0 else CHILDREN_NUM
        if(len(r) != expectednum):
            return False
        for i in r:
            dfs_check(l, i[0], layer - 1)
        return True
    except:
        return False

def init_source():
    reset_service()
    vdc_promo(source_domain)

def init_target():
    os.system('ssh root@gary-testvm python /mnt/hgfs/workspaces/lotus/main/vmdir/tools/vdcmerge/test/test_config.py')

#main function
init_source()
#init_target()

source_uri = 'ldap://%s:%s' % (source_host, source_port)
target_uri = 'ldap://%s:%s' % (target_host, target_port)
source_dn = 'cn=%s,cn=users,dc=%s' % (source_username, source_domain)
target_dn = 'cn=%s,cn=users,dc=%s' % (target_username, target_domain)
#initialize and log in ldap servers
s = ldap.initialize(source_uri)
s.simple_bind_s(source_dn, source_password)

t = ldap.initialize(target_uri)
t.simple_bind_s(target_dn, target_password)

dfs_clean(t, target_base)

#add test data
dfs_add(s, source_base, LAYER_NUM)
dfs_add(t, target_base, 0)

#call vdcmerge
cmdline = '%s -p %s -d %s -u %s -w %s -b %s -i %s -H %s -P %s -D %s -U %s -W %s -B %s' % (vdcmerge,
        source_port, source_domain, source_username, source_password, source_base, server_id,
        target_host, target_port, target_domain, target_username, target_password, target_base);
cmdline += ' -v' #verbose mode
print '%s\n' % (cmdline)
os.system(cmdline)

#validate results
if dfs_check(t, target_base, LAYER_NUM):
    print "Test passed."
else:
    print "Test failed."

time.sleep(5)

s = ldap.initialize(source_uri)
s.simple_bind_s(target_dn, target_password)

if dfs_check(s, target_base, LAYER_NUM):
    print "Test passed."
else:
    print "Test failed."

#unbind
s.unbind_s()
t.unbind_s()


