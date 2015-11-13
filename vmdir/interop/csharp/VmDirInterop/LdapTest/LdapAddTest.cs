using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    [TestClass]
    public class LdapAddTest
    {

        static string myDN;
        static string hostName;
        static string password;
        static string dllPath;
        static int portNumber;
        static string upn;

        static string myDN_F;
        static string hostName_F;
        static string password_F;
        static int portNumber_F;
        static string upn_F;

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

            //False credentials
            myDN_F = cred.myDN_F;
            hostName_F = cred.hostName_F;
            upn_F = cred.upn_F;
            password_F = cred.password_F;
            portNumber_F = cred.portNumber_F;

            dllPath = @"C:\Users\aalokr\workspaces_new\aalokr_lotus_ws_2k8_dev_new\lotus\lotus-main\vmdir\interop\csharp\VmDirInterop\LdapTest\";        //put all the dll in the LdapTest Folder

            Dll.SetDllDirectory(dllPath);
        }



        [TestMethod]
        public void Ldap_Add_Object_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            string basedn = "cn=ExampleAdd,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "ExampleAdd", null });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person", null });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen", null });

            try
            {
                ldapConnection.AddObject(basedn, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }


        [TestMethod]
        public void Ldap_Add_Object_SASL_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.VmDirSafeLDAPBind(hostName, upn, password);

            string basedn = "cn=ExampleAddSasl,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "ExampleAddSasl", null });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person", null });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen", null });

            try
            {
                ldapConnection.AddObject(basedn, lMods);
            }
            catch
            {
                Assert.Fail();
            }
        }


        [TestMethod]
        public void Ldap_Add_Object_Failure()
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
                //Expected Exception
            }

            string basedn = "cn=CapNew,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "Cap2", null });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person", null });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen", null });

            try
            {
                ldapConnection.AddObject(basedn, lMods);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception Due to Invalid bind.
            }

        }

         [TestMethod]
        public void Ldap_Add_Object_SASL_Failure()
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
                //Expected Exception
            }

            string basedn = "cn=Cap2,cn=Users,dc=vsphere,dc=local";

            LdapMod[] lMods = new LdapMod[3];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { "Cap2", null });

            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person", null });

            lMods[2] = new LdapMod(0, "sn", new string[] { "Allen", null });

            try
            {
                ldapConnection.AddObject(basedn, lMods);
                Assert.Fail();
            }
            catch
            {
                // Expected Exception Due to Invalid bind.
            }

        }

    }
}
