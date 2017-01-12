using System;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    public class LdapModifyTest
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
        static ILdapConnection ldapConnection;

        public static void RunTests(Credentials cred)
        {
            System.Console.WriteLine("Running modify tests ...");

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

            ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);
            LdapUser user = LdapUser.CreateRandomUser(ldapConnection);

            LdapModify_AddAttribute_Success(ldapConnection, user);
            LdapModify_AddAttribute_Failure(ldapConnection, user);
            LdapModify_ReplaceAttribute_Success(ldapConnection, user);
            LdapModify_DeleteAttribute_Success(ldapConnection, user);
        }

        public static void LdapModify_AddAttribute_Success(ILdapConnection ldapConnection, LdapUser user)
        {
            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "sn", new string[] { "Allen" });

            try
            {
                ldapConnection.ModifyObject(user.DN, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }

        public static void LdapModify_AddAttribute_Failure(ILdapConnection ldapConnection, LdapUser user)
        {
            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "Allen" });

            try
            {
                ldapConnection.ModifyObject(user.DN, lMods);
                Assert.Fail();
            }
            catch
            {
                //Expected Exception
            }
        }

        public static void LdapModify_ReplaceAttribute_Success(ILdapConnection ldapConnection, LdapUser user)
        {
            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, "sn", new string[] { "Barry" });

            try
            {
                ldapConnection.ModifyObject(user.DN, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }

        public static void LdapModify_DeleteAttribute_Success(ILdapConnection ldapConnection, LdapUser user)
        {
            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_DELETE, "sn", new string[] { "Barry" });

            try
            {
                ldapConnection.ModifyObject(user.DN, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }
    }
}
