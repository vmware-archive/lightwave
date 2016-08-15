using System;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    /// <summary>
    /// Summary description for UnitTest1
    /// </summary>
    public class LdapConnectionTest
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
            System.Console.WriteLine("Running connection tests ...");

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

            LdapConnect_Init_Success();
            LdapConnect_Init_Failure();
            LdapConnect_Simple_Bind_Success();
            LdapConnect_Simple_Bind_Failure();
            LdapConnect_SASL_Bind_Success();
            LdapConnect_SASL_Bind_Failure();
            LdapConnect_Unbind_S_Success();
        }

        public static void LdapConnect_Init_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN, password);
            }
            catch (Exception)
            {
                Assert.Fail();
            }

        }

        public static void LdapConnect_Init_Failure()
        {
            try
            {
                ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName_F, portNumber_F);
                ldapConnection.LdapSimpleBindS(myDN, password);
                Assert.Fail();
            }
            catch (Exception)
            {
            }
        }

        public static void LdapConnect_Simple_Bind_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN, password);
            }
            catch(Exception)
            {
                Assert.Fail();
            }
        }

        public static void LdapConnect_Simple_Bind_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapSimpleBindS(myDN_F, password_F);
                Assert.Fail();
            }
            catch(Exception)
            {
            }
        }

        public static void LdapConnect_SASL_Bind_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName, upn, password);
            }
            catch(Exception)
            {
                Assert.Fail();
            }
        }

        public static void LdapConnect_SASL_Bind_Failure()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.VmDirSafeLDAPBind(hostName_F, upn_F, password_F);
                Assert.Fail();
            }
            catch (Exception)
            {
            }
        }

        public static void LdapConnect_Unbind_S_Success()
        {
            ILdapConnection ldapConnection = LdapConnection.LdapInit(hostName, portNumber);

            try
            {
                ldapConnection.LdapUnbindS();
            }
            catch (Exception)
            {
                Assert.Fail();
            }
        }
    }
}
