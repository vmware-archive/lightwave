using System;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    public class LdapAddTest
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

        public static void RunTests(Credentials cred)
        {
            System.Console.WriteLine("Running add tests ...");

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

            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            ldapConnection.LdapSimpleBindS(myDN, password);

            LdapUser user = LdapUser.CreateRandomUser(ldapConnection);

            ILdapConnection ldapConnectionSASL = LdapConnection.LdapInit(hostName, portNumber);
            ldapConnectionSASL.VmDirSafeLDAPBind(hostName, upn, password);

            string UserName = Guid.NewGuid().ToString("N");
            string UserNameSASL = Guid.NewGuid().ToString("N");
            Ldap_Add_Object_Success(ldapConnection, UserName);
            Ldap_Add_Object_SASL_Success(ldapConnectionSASL, UserNameSASL);
            Ldap_Add_Object_Failure(ldapConnection, UserName);
            Ldap_Add_Object_SASL_Failure(ldapConnectionSASL, UserNameSASL);
        }

        public static void Ldap_Add_Object_Success(ILdapConnection ldapConnection, string UserName)
        {
            string DN = String.Format("cn={0},cn=Users,dc=vsphere,dc=local", UserName);

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { UserName });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person" });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen" });

            try
            {
                ldapConnection.AddObject(DN, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }


        public static void Ldap_Add_Object_SASL_Success(ILdapConnection ldapConnection, string UserName)
        {
            string DN = String.Format("cn={0},cn=Users,dc=vsphere,dc=local", UserName);

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { UserName });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person" });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen" });

            try
            {
                ldapConnection.AddObject(DN, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }

        public static void Ldap_Add_Object_Failure(ILdapConnection ldapConnection, string UserName)
        {
            string DN = String.Format("cn={0},cn=Users,dc=vsphere,dc=local", UserName);

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "Cap2" });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person" });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen" });

            try
            {
                ldapConnection.AddObject(DN, lMods);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception Due to Invalid bind.
            }
        }

        public static void Ldap_Add_Object_SASL_Failure(ILdapConnection ldapConnection, string UserName)
        {
            string DN = String.Format("cn={0},cn=Users,dc=vsphere,dc=local", UserName);

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "Cap2" });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person" });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen" });

            try
            {
                ldapConnection.AddObject(DN, lMods);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception Due to Invalid bind.
            }
        }
    }
}
