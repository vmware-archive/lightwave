import os;
import ldap;
from common import *

#test configuration
HOST_NAME = 'gary-sles'
HOST_PORT = '11711'
DOMAIN_NAME = 'csp'

ADMIN_NAME = 'administrator'
ADMIN_PASSWORD = 'vmware'

USER_NAME = 'user1'
USER_OLD_PASSWORD = 'Vmware@ng12345'
USER_NEW_PASSWORD = 'Vmware@ng67890'

reset_service()
vdc_promo(DOMAIN_NAME)

source_uri = 'ldap://%s:%s' % (HOST_NAME, HOST_PORT)
admin_dn = 'cn=%s,cn=users,dc=%s,dc=com' % (ADMIN_NAME, DOMAIN_NAME)
user_dn= 'cn=%s,cn=users,dc=%s,dc=com' % (USER_NAME, DOMAIN_NAME)

s = ldap.initialize(source_uri)
s.simple_bind_s(admin_dn, ADMIN_PASSWORD)

add_user(s, user_dn)

print 'Set password'
set_password(source_uri, admin_dn, ADMIN_PASSWORD, user_dn, USER_OLD_PASSWORD)
s.unbind_s()

print 'Log in with new password'
s = ldap.initialize(source_uri)
s.simple_bind_s(user_dn, USER_OLD_PASSWORD)

print 'Chang password'
change_password(source_uri, user_dn, USER_OLD_PASSWORD, USER_NEW_PASSWORD)
s.unbind_s()

print 'Log in with changed password'
s = ldap.initialize(source_uri)
s.simple_bind_s(user_dn, USER_NEW_PASSWORD)

s.unbind_s()

print 'test completed'
