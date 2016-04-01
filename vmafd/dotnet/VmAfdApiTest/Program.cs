using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using VMAFD.Client;

namespace VmAfdApiTest
{
    class Program
    {
        static string server = "10.161.244.112";
        static string user = "Administrator@vsphere.local";
        static string password = "Admin!23";
        static string domain = "vsphere.local";
        static string site = "Default-First-Site";

        static void Main(string[] args)
        {
            RunTests();
        }

        static void RunTests()
        {
            TestDcInfo(); 
        }

        static void TestStates()
        {
            using (Client client = new Client(server, user, password))
            {
                VMAFD.Client.CDC_DC_STATE state = CDC_DC_STATE.CDC_DC_STATE_UNDEFINED;

                client.CdcDisableClientAffinity();
                state = client.CdcGetCurrentState();
                Debug.Assert(state == CDC_DC_STATE.CDC_DC_STATE_LEGACY);

                client.CdcEnableClientAffinity();
                state = client.CdcGetCurrentState();
                Debug.Assert(state != CDC_DC_STATE.CDC_DC_STATE_LEGACY);
            }
        }

        static void TestGetDCName()
        {
            using (Client client = new Client(server, user, password))
            {
                CDC_DC_INFO dcInfo = client.CdcGetDCName(domain, site, 0);
            }
        }

        static void TestGetHeartBeatStatus()
        {
            using (Client client = new Client(server, user, password))
            {
                VMAFD_HEARTBEAT_STATUS hbStatus = client.VmAfdGetHeartbeatStatus();
            }
        }

        static void TestGetSiteName()
        {
            using (Client client = new Client(server, user, password))
            {
                string siteName = client.VmAfdGetSiteName();
            }
        }

        static void TestEnumEntries()
        {
            using (Client client = new Client(server, user, password))
            {
                IList<string> entries = client.CdcEnumDCEntries();
            }
        }

        static void TestDcInfo()
        {
            using (Client client = new Client(server, user, password))
            {
                IList<string> entries = client.CdcEnumDCEntries();

                foreach(var entry in entries)
                {
                    CDC_DC_STATUS_INFO info;
                    VMAFD_HEARTBEAT_STATUS hbStatus;

                    client.CdcGetDCStatus(entry, string.Empty, out info, out hbStatus);

                    Console.WriteLine("============DC Status=====================");
                    PrintCdcStatus(entry, info);
                    Console.WriteLine("============Service Status=====================");
                    foreach (var serviceInfo in hbStatus.info)
                        PrintHbStatus(serviceInfo);
                }
            }
        }

        static void PrintCdcStatus(string dcName, CDC_DC_STATUS_INFO info)
        {
            Console.WriteLine("DcName              : {0}", dcName);
            Console.WriteLine("SiteName            : {0}", info.pszSiteName);
            Console.WriteLine("IsActive            : {0}", info.bIsAlive);
            Console.WriteLine("Last Ping Time      : {0}", DateTime.FromFileTime(info.dwLastPing).ToLongDateString());
        }

        static void PrintHbStatus(VMAFD_HB_INFO status)
        {
            Console.WriteLine("     Service Name        : {0}", status.pszServiceName);
            Console.WriteLine("     IsActive            : {0}", status.bIsAlive);
            Console.WriteLine("     Last Heartbeat Time : {0}", DateTime.FromFileTime(status.dwLastHeartbeat).ToLongDateString());
        }
    }
}
