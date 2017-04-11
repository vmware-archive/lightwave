using System;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    public class LdapDeleteTest
    {
        static string myDN;
        static string hostName;
        static string password;
        static int portNumber;
        static string upn;

        static string myDN_F;
        static string hostName_F;
        static string password_F;
        static int portNumber_F;
        static string upn_F;
        static string basedn_F;

        public static void RunTests(Credentials cred)
        {
            System.Console.WriteLine("Running delete tests ...");

            myDN = cred.myDN;
            hostName = cred.hostName;
            upn = cred.upn;
            password = cred.password;
            portNumber = cred.portNumber;

            //False credentials
            myDN_F = cred.myDN_F;
            hostName_F = cred.hostName_F;
            upn_F = cred.upn_F;
            password_F = cred.password_F;
            portNumber_F = cred.portNumber_F;
            basedn_F = "cn=Cap150,cn=Users,dc=vsphere,dc=local";

            Ldap_Delete_Success();
            Ldap_Delete_SASL_Success();
            Ldap_Delete_Failure();
            Ldap_Delete_SASL_Failure();
        }

        public static void Ldap_Delete_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            ldapConnection.LdapSimpleBindS(myDN, password);

            LdapUser user = LdapUser.CreateRandomUser(ldapConnection);

            try
            {
                ldapConnection.DeleteObject(user.DN);
            }
            catch
            {
                Assert.Fail();
            }
        }

        public static void Ldap_Delete_SASL_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            ldapConnection.VmDirSafeLDAPBind(hostName, upn, password);

            LdapUser user = LdapUser.CreateRandomUser(ldapConnection);

            try
            {
                ldapConnection.DeleteObject(user.DN);
            }
            catch
            {
                Assert.Fail();
            }
        }

        public static void Ldap_Delete_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN, password_F);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception.Invalid Password
            }

            try
            {
                ldapConnection.DeleteObject(basedn_F);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception Due to Invalid Bind
            }

        }

        public static void Ldap_Delete_SASL_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName, upn, password_F);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception.Invalid Password
            }

            try
            {
                ldapConnection.DeleteObject(basedn_F);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception Due To Invalid SASL Bind.
            }

        }
    }
}
