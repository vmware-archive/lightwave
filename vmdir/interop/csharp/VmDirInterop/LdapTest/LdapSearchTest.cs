using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    [TestClass]
    public class LdapSearchTest
    {
        static string myDN;
        static string hostName;
        static string password;
        static string attribute;
        static string value;
        static string dllPath;
        static string filter;
        static int portNumber;
        static string upn;

        static string myDN_F;
        static string hostName_F;
        static string password_F;
        static int portNumber_F;
        static string upn_F;
        static string filter_F;

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

            filter = "(&(objectClass=person)(cn=ws2k8r2-aalok-d.eng.vmware.com))";

            attribute = "cs";                               //same as what you are searching
            value = "ws2k8r2-aalok-d.eng.vmware.com";       //same as what you are searching

            filter_F = "(&(objectClass=person)(cn=ws2k8r2-aalok-f.eng.vmware.com))";
            dllPath = @"C:\Users\aalokr\workspaces_new\aalokr_lotus_ws_2k8_dev_new\lotus\lotus-main\vmdir\interop\csharp\VmDirInterop\LdapTest\";        //put all the dll in the LdapTest Folder

            Dll.SetDllDirectory(dllPath);
        }

        [TestMethod]
        public void LdapSearch_SimpleBind_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            ILdapMessage msg = ldapConnection.LdapSearchExtS("", 2, filter, null, 1, IntPtr.Zero, 0);
            Assert.IsNotNull(msg);

            List<ILdapEntry> entries = msg.GetEntries();
            Assert.IsNotNull(entries);

            foreach (LdapEntry entry in entries)
            {
                string dn = entry.getDN();
                Assert.IsNotNull(dn);

                List<string> attributeNames = entry.getAttributeNames();
                Assert.IsNotNull(attributeNames);

                foreach (string attributeName in attributeNames)
                {
                    List<LdapValue> attributeValues = entry.getAttributeValues(attributeName);
                    Assert.IsNotNull(attributeValues);

                    foreach (LdapValue attributeValue in attributeValues)
                    {
                        if(attributeName == attribute)
                            Assert.Equals(attributeValue.StringValue, value);
                    }
                }
            }
            ldapConnection.CleanSearch();
            //msg.FreeMessage();

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapSearch_SASLBind_Success()
        {
            ILdapMessage msg = null;

            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName, upn, password);
                msg = ldapConnection.LdapSearchExtS("", 2, filter, null, 1, IntPtr.Zero, 0);
            }
            catch
            {
                Assert.Fail();
            }

            List<ILdapEntry> entries = msg.GetEntries();

            foreach (LdapEntry entry in entries)
            {
                string dn = entry.getDN();
                Assert.IsNotNull(dn);

                List<string> attributeNames = entry.getAttributeNames();
                Assert.IsNotNull(attributeNames);
                foreach (string attributeName in attributeNames)
                {
                    List<LdapValue> attributeValues = entry.getAttributeValues(attributeName);
                    Assert.IsNotNull(attributeValues);
                    foreach (LdapValue attributeValue in attributeValues)
                    {
                        if (attributeName == attribute)
                            Assert.Equals(attributeValue.StringValue, value);
                    }
                }
            }

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapSearch_SimpleBind_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            ILdapMessage msg = ldapConnection.LdapSearchExtS("", 2, filter_F, null, 1, IntPtr.Zero, 0);
            Assert.IsNotNull(msg);

            List<ILdapEntry> entries = msg.GetEntries();

            if (entries.Count != 0)
                Assert.Fail();

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void LdapSearch_SASLBind_Failure()
        {
            ILdapMessage msg = null;

            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName, upn, password);
                msg = ldapConnection.LdapSearchExtS("", 2, filter_F, null, 1, IntPtr.Zero, 0);
            }
            catch
            {
                Assert.Fail();
            }

            List<ILdapEntry> entries = msg.GetEntries();
            if(entries.Count!=0)
                Assert.Fail();

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch
            {
                Assert.Fail();
            }
        }

        [TestMethod]
        public void MemoryLeak_NotPresent()
        {
            int i = 0;
            while (i < 10000)
            {
                LdapSearch_SimpleBind_Success();
                i++;
            }
        }
    }
}
