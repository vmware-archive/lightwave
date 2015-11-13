using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    /// <summary>
    /// Summary description for UnitTest1
    /// </summary>
    [TestClass]
    public class LdapConnectionTest
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
        public void LdapConnect_Init_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN, password);
            }
            catch (Exception exp)
            {
                Assert.Fail();
            }

        }

        [TestMethod]
        public void LdapConnect_Init_Failure()
        {
            try
            {
                ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName_F, portNumber_F);
                ldapConnection.LdapSimpleBindS(myDN, password);
                Assert.Fail();
            }
            catch (Exception exp)
            {
            }
        }

        [TestMethod]
        public void LdapConnect_Simple_Bind_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN, password);
            }
            catch(Exception exp)
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapConnect_Simple_Bind_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN_F, password_F);
                Assert.Fail();
            }
            catch(Exception exp)
            {
            }
        }

        [TestMethod]
        public void LdapConnect_SASL_Bind_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName, upn, password);
            }
            catch(Exception exp)
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapConnect_SASL_Bind_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName_F, upn_F, password_F);
                Assert.Fail();
            }
            catch (Exception exp)
            {
            }
        }

        [TestMethod]
        public void LdapConnect_Unbind_S_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch (Exception exp)
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapConnect_Unbind_S_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName_F, portNumber_F);
            try
            {
                ldapConnection.LdapUnbindS();
                Assert.Fail();
            }
            catch (Exception exp)
            {
            }
        }
    }
}
