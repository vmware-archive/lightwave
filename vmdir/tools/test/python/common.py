import os
import sys
import re

DEFAULT_DOMAIN = 'vsphere.local'
DEFAULT_USER = 'administrator'
DEFAULT_PASSWORD = 'vmware'
DEFAULT_PORT = '389'

print 'running on %s' % (os.name)

if(os.name == 'nt'):
    build = 'c:\\PROGRA~1\\vmware\\cis\\vmdird\\'
    vdcpromo = build+'vdcpromo.exe'
    vdcmerge = build+ 'vdcmerge.exe'
    vdcsplit = build+ 'vdcsplit.exe'
    vdcsetupldu = build+'vdcsetupldu.exe'
    data_store = '"C:\\Documents and Settings\\All Users\\Application Data\\VMware\\cis\\data\\vmdird\\*"'
else:
    vdcfirstboot='/usr/lib/vmware-vmdir/bin/vmdir_fbcore.py'
    kdcfirstboot='/usr/lib/vmware-vmkdc/firstboot/vmkdc-firstboot.py'
    afdfirstboot='/usr/lib/vmware-vmafd/firstboot/vmafd-firstboot.py'
    vdcpromo = '/usr/lib/vmware-vmdir/bin/vdcpromo'
    vdcmerge = '/usr/lib/vmware-vmdir/bin/vdcmerge'
    vdcsplit = '/usr/lib/vmware-vmdir/bin/vdcsplit'
    #vdcmerge = '/home/zhaog/workspaces/lotus/main/vmdir/build/tools/vdcmerge/.libs/vdcmerge'
    #vdcsplit = '/home/zhaog/workspaces/lotus/main/vmdir/build/tools/vdcsplit/.libs/vdcsplit'
    vdcpass = '/home/zhaog/workspaces/lotus/main/vmdir/build/tools/vdcpass/.libs/vdcpass'
    vdcsetupldu = '/home/zhaog/workspaces/lotus/main/vmdir/build/tools/vdcsetupldu/vdcsetupldu'
    lwsm = '/opt/likewise/bin/lwsm'
    data_store = '/storage/db/vmware-vmdir/*'

def reset_rpms():
    print 'remove afd'
    os.system('rpm -e vmware-afd')
    print 'remove vmkdc'
    os.system('rpm -e vmware-kdc')
    print 'remove vmdir'
    os.system('rpm -e vmware-directory')
    print 'install vmdir'
    os.system('rpm --nodeps -i /mnt/hgfs/workspaces/lotus/main/vmdir/build/stage/RPMS/x86_64/vmware-directory-1.0.0-00000.x86_64.rpm ')
    print 'install vmkdc'
    os.system('rpm --nodeps -i /mnt/hgfs/workspaces/lotus/main/vmkdc/build/stage/RPMS/x86_64/vmware-kdc-1.0.0-00000.x86_64.rpm')
    print 'install vmafd'
    os.system('rpm --nodeps -i /mnt/hgfs/workspaces/lotus/main/vmafd/build/stage/RPMS/x86_64/vmware-afd-1.0.0-00000.x86_64.rpm')

def reset_service():
    print 'resetting service...'
    print 'cleaning up database...'

    if(os.name == 'nt'):
        print 'stopping service'
        os.system('net stop VMWareDirectoryService')
        os.system('%s %s' % ('del /Q ', data_store))
    else:
        os.system('%s %s' % ('rm', data_store))

    print 'restarting vmdir...'

    if(os.name == 'nt'):
        os.system('net start VMWareDirectoryService')
    else:
        os.system('%s %s %s' % (lwsm, 'restart', 'vmdir'))

def vdc_firstboot():
    cmd='python '+vdcfirstboot
    print cmd
    os.system(cmd)

    cmd='python '+kdcfirstboot
    print cmd
    os.system(cmd)

def init_source():
    reset_rpms()
    reset_service()
    vdc_firstboot()

def vdc_promo(domain):
    print 'vdcpromoing...'
    cmd='%s -d %s -u %s -w %s -i 1' % (vdcpromo, domain, DEFAULT_USER, DEFAULT_PASSWORD)
    print cmd
    os.system(cmd)

def find_rdn(dn):
    m = re.match(r'cn=(.+?),', dn)
    return m.group(1)

def add_user(l, dn):
    modlist = [('objectClass', 'vmIdentity-User'),
               ('vmIdentity-Account', find_rdn(dn)),
               ('cn',find_rdn(dn))]
    l.add_s(dn, modlist)

def add_userwithpassword(l, dn, password):
    modlist = [('objectClass', 'user'),
               ('sAMAccountName', find_rdn(dn)),
               ('cn',find_rdn(dn)),
               ('userpassword', password),
               ('userPrincipalName', find_rdn(dn)+'@vsphere.local')]

    l.add_s(dn, modlist)

def add_container(l, dn):
    modlist = [('objectClass', 'vmIdentity-Container'),
               ('cn', find_rdn(dn))]
    l.add_s(dn, modlist)

def set_password(source_uri, admin_dn, admin_password, user_dn, user_password ):
    cmdline= '%s -h %s -u %s -w %s -U %s -W %s' % (vdcpass, source_uri, admin_dn, admin_password, user_dn, user_password);
    print cmdline
    os.system(cmdline)

def change_password(source_uri, user_dn, old_password, new_password):
    cmdline= '%s -h %s -u %s -w %s -W %s' % (vdcpass, source_uri, user_dn, old_password, new_password);
    print cmdline
    os.system(cmdline)