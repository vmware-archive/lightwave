using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    [TestClass]
    public class LdapModifyTest
    {

        static string myDN;
        static string hostName;
        static string password;
        static string dllPath;
        static int portNumber;
        static string upn;
        static string basedn;

        static string myDN_F;
        static string hostName_F;
        static string password_F;
        static int portNumber_F;
        static string upn_F;
        static string basedn_F;


        [ClassInitialize()]
        public static void MyClassInitialize(TestContext testContext)
        {
            var path = @"..\..\..\input.xml";
            Credentials cred = Input.ReadXML(path);
            myDN = cred.myDN;
            hostName = cred.hostName;
            upn = cred.upn;
            password = cred.password;
            portNumber = cred.portNumber;
            basedn = "cn=Cap15,cn=Users,dc=vsphere,dc=local";

            //False credentials
            myDN_F = cred.myDN_F;
            hostName_F = cred.hostName_F;
            upn_F = cred.upn_F;
            password_F = cred.password_F;
            portNumber_F = cred.portNumber_F;
            basedn_F = "cn=Cap150,cn=Users,dc=vsphere,dc=local";

            dllPath = @"C:\Users\aalokr\workspaces_new\aalokr_lotus_ws_2k8_dev_new\lotus\lotus-main\vmdir\interop\csharp\VmDirInterop\LdapTest\";        //put all the dll in the LdapTest Folder

            Dll.SetDllDirectory(dllPath);
        }


        [TestMethod]
        public void LdapModify_AddAttribute_Success()
        {

            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            string basedn = "cn=lModAdd,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "sn", new string[] { "Allen", null });


            try
            {
                ldapConnection.ModifyObject(basedn, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapModify_AddAttribute_Failure()
        {

            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            string basedn = "cn=lModAdd,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "Allen", null });


            try
            {
                ldapConnection.ModifyObject(basedn, lMods);
                Assert.Fail();
            }
            catch
            {
                //Expected Exception
            }

        }

        [TestMethod]
        public void LdapModify_ReplaceAttribute_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            string basedn = "cn=lModReplace,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, "sn", new string[] { "Barry", null });

            try
            {
                ldapConnection.ModifyObject(basedn, lMods);
            }
            catch
            {
                Assert.Fail();
            }

        }

        [TestMethod]
        public void LdapModify_DeleteAttribute_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            string basedn = "cn=lModAdd,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[1];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_DELETE, "sn", new string[] { "Allen", null });

            try
            {
                ldapConnection.ModifyObject(basedn, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }
    }
}
