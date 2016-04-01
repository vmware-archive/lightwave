using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using VMDIR.Client;

namespace VmDirApiTest
{
    class Program
    {
        static string server = "1.2.3.4";
        static string user = "administrator";
        static string password = "CENCORED";
        static string domain = "vsphere.local";
        static string site = "Default-First-Site";

        static void Main(string[] args)
        {
            RunTests();
        }

        static void RunTests()
        {
            TestEnumComputers();
            TestEnumPSCs();
        }

        static void TestEnumComputers()
        {
            System.Console.WriteLine("Running test TestEnumComputers: ");
            IList<string> entries = Client.VmDirGetComputers(server, user, password);
            foreach (string entry in entries)
            {
                System.Console.WriteLine("Computer: " + entry);
            }
        }

        static void TestEnumPSCs()
        {
            System.Console.WriteLine("Running test TestEnumPSCs: ");
            IList<VmDirDCInfo> entries = Client.VmDirGetDCInfos(server, user, password);
            int i = 0;
            foreach (VmDirDCInfo entry in entries)
            {
                ++i;
                System.Console.WriteLine("PSC #" + i + ":");
                System.Console.WriteLine("Name: " + entry.pszDCName);
                System.Console.WriteLine("Site: " + entry.pszSiteName);
                System.Console.WriteLine("Partner count: " + entry.partners.Count);
                if (entry.partners.Count > 0)
                {
                    for (int j = 0; j < entry.partners.Count; ++j)
                    {
                        System.Console.WriteLine("    Partner: " + entry.partners[j]);
                    }
                }
                System.Console.WriteLine("");
            }
        }
    }
}
