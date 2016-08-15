using System;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    public class LdapSearchTest
    {
        static string myDN;
        static string hostName;
        static string password;
        static string attribute;
        static string value;
        static string filter;
        static int portNumber;
        static string upn;

        static string myDN_F;
        static string hostName_F;
        static string password_F;
        static int portNumber_F;
        static string upn_F;
        static string filter_F;

        public static void RunTests(Credentials cred)
        {
            System.Console.WriteLine("Running search tests ...");

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

            filter = "(&(objectClass=person)(cn=Administrator))";

            attribute = "cn";                               //same as what you are searching
            value = "Administrator";                        //same as what you are searching

            filter_F = "(&(objectClass=person)(cn=ws2k8r2-aalok-f.eng.vmware.com))";

            LdapSearch_SimpleBind_Success();
            LdapSearch_SASLBind_Success();
            LdapSearch_SimpleBind_Failure();
            LdapSearch_SASLBind_Failure();

            LdapPagedSearch();
        }

        public static void LdapSearch_SimpleBind_Success()
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
                        if (attributeName == attribute)
                        {
                            Assert.IsTrue(attributeValue.StringValue == value);
                        }
                    }
                }
            }
            ldapConnection.CleanSearch();

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch
            {
                Assert.Fail();
            }
        }

        public static void LdapSearch_SASLBind_Success()
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
                        {
                            Assert.IsTrue(attributeValue.StringValue == value);
                        }
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

        public static void LdapSearch_SimpleBind_Failure()
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

        public static void LdapSearch_SASLBind_Failure()
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

        public static void LdapPagedSearch()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);
            Assert.IsNotNull(ldapConnection);

            ldapConnection.LdapSimpleBindS(myDN, password);

            int totalCount = 0;
            int pageNumber = 1;
            bool morePages = false;
            int pageSize = 2;
            IntPtr cookie = IntPtr.Zero;
            IntPtr returnedControls = IntPtr.Zero;
            IntPtr[] serverControls = {IntPtr.Zero};
            ILdapMessage msg = null;
            string searchBase = "cn=users,dc=vsphere,dc=local";
            string filter = "(objectClass=user)";

            System.Console.WriteLine("The entries returned were:");

            do
            {
                serverControls[0] = ldapConnection.LdapCreatePageControl(pageSize, cookie, true);

                msg = ldapConnection.LdapSearchExtExS(searchBase, (int)LdapScope.SCOPE_SUBTREE, filter, null, 0, IntPtr.Zero, 0, serverControls);

                ldapConnection.LdapParseResult(msg, ref returnedControls, false);
                if (cookie != IntPtr.Zero)
                {
                    LdapClientLibrary.ber_bvfree(cookie);
                    cookie = IntPtr.Zero;
                }

                ldapConnection.LdapParsePageControl(returnedControls, ref cookie);

                morePages = ldapConnection.HasMorePages(cookie);

                if (returnedControls != IntPtr.Zero)
                {
                    LdapClientLibrary.ldap_controls_free(returnedControls);
                    returnedControls = IntPtr.Zero;
                }
                LdapClientLibrary.ldap_control_free(serverControls[0]);
                serverControls[0] = IntPtr.Zero;

                if (morePages)
                {
                    System.Console.WriteLine("===== Page : {0} =====", pageNumber);
                }

                totalCount += msg.GetEntriesCount();

                foreach (var entry in msg.GetEntries())
                {
                    System.Console.WriteLine("dn: {0}", entry.getDN());
                }

                ldapConnection.CleanSearch();
                pageNumber++;
            } while (morePages);

            System.Console.WriteLine("{0} entries found during the search", totalCount);
            LdapClientLibrary.ber_bvfree(cookie);
            cookie = IntPtr.Zero;

            ldapConnection.LdapUnbindS();
        }
    }
}
