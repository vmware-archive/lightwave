using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using VMDirInterop;

namespace LdapTest
{
    class Program
    {
        static void Usage()
        {
            System.Console.WriteLine("Invalid number of arguments!");
            System.Console.WriteLine("Usage: LdapTest.exe <user_dn> <hostname> <user_upn> <password> <port number>\n");
            Environment.Exit(1);
        }

        static void Main(string[] args)
        {
            if (args.Length != 5)
            {
                Usage();
            }

            Credentials cred = new Credentials();
            cred.myDN = args[0];
            cred.hostName = args[1];
            cred.upn = args[2];
            cred.password = args[3];
            cred.portNumber = Int32.Parse(args[4]);

            cred.myDN_F = "dn=NoAdmin,cn=users,dc=vsphere,dc=local";
            cred.hostName_F = "badhostname";
            cred.upn_F = "noadmin@vsphere.local";
            cred.password_F = "@#Rkladfj";
            cred.portNumber_F = 33;

            try
            {
                LdapAddTest.RunTests(cred);
                LdapConnectionTest.RunTests(cred);
                LdapDeleteTest.RunTests(cred);
                LdapModifyTest.RunTests(cred);
                LdapSearchTest.RunTests(cred);
            }
            catch (Exception e)
            {
                System.Console.WriteLine("Caught exception while running tests ==> " + e.ToString());
            }
        }
    }
}
