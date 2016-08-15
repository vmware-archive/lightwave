using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Runtime.InteropServices;
using VMDNS.Client;

namespace VmDnsApiTest
{
    internal class VmDnsApiTest
    {
        private static string hostname = "localhost";
        private static string username = "administrator";
        private static string password = "vmware";
        private static string domain = "vshere.local";

        private static VmDnsZone
        FindZone(
            VmDnsClient client,
            string zoneName
        )
        {
            VmDnsZone result = null;

            var zones = client.ListZones(VmDnsZoneType.FORWARD);
            foreach (VmDnsZone zone in zones)
            {
                if (zone.Name.Equals(zoneName, StringComparison.InvariantCultureIgnoreCase))
                {
                    result = zone;
                    break;
                }
            }

            return result;
        }

        private static void
        TestZoneOperations()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zoneInfo = new VMDNS_ZONE_INFO();
                VmDnsZone zone;
                zoneInfo.minimum = 3600;
                zoneInfo.pszName = "TestZone";
                zoneInfo.pszPrimaryDnsSrvName = "dns1.vsphere.local";
                zoneInfo.pszRName = "admin@vsphere.local";
                zoneInfo.refreshInterval = 3600;
                zoneInfo.retryInterval = 3600;
                zoneInfo.serial = 1;

                client.CreateZone(zoneInfo);

                zone = FindZone(client, zoneInfo.pszName);

                zoneInfo.serial = 2;
                /*client.UpdateZone(zoneInfo);

                zone = FindZone(client, zoneInfo.pszName);
                Debug.Assert(zone.Serial == 2);*/
            }
        }

        private static void
        TestAddARecord()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");

                var data = new VMDNS_A_DATA();
                data.IpAddress = (UInt32)IPAddress.NetworkToHostOrder(
                    (int)IPAddress.Parse("192.168.1.11").Address);
                VMDNS_RECORD_A record = new VMDNS_RECORD_A();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = "test1";
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_A;
                VmDnsRecordA addressRecord = new VmDnsRecordA(record);
                try
                {
                    zone.AddRecord(addressRecord);
                }
                catch (VmDnsException e)
                {
                    if (e.ErrorCode != 183)
                    {
                        throw;
                    }
                }

                IList<VmDnsRecord> records =
                    zone.QueryRecords(
                        "test1",
                        RecordType.VMDNS_RR_TYPE_A,
                        0);
                try
                {
                    foreach (VmDnsRecord entry in records)
                    {
                        Console.WriteLine("Record address {0}", ((VmDnsRecordA)entry).Address);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }
        }

        private static void
        TestAddIpv6AddressRecord()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");

                var data = new VMDNS_AAAA_DATA();
                var addr = IPAddress.Parse("fe80::8152:a429:635d:1284");
                data.Ip6Address.bytes = addr.GetAddressBytes();
                var record = new VMDNS_RECORD_AAAA();
                record.common.iClass = 1;
                record.common.pszName = "test2";
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_AAAA;
                record.data = data;
                zone.AddRecord(new VmDnsRecordAAAA(record));

                var records = zone.QueryRecords(
                                  "test2",
                                  RecordType.VMDNS_RR_TYPE_AAAA,
                                  0);
                try
                {
                    foreach (VmDnsRecord entry in records)
                    {
                        Console.WriteLine("Record address {0}", ((VmDnsRecordAAAA)entry).Address);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }
        }

        private static void
        TestAddSrvRecord()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");
                var data = new VMDNS_SRV_DATA();
                data.pNameTarget = "dns1.TestZone";
                var record = new VMDNS_RECORD_SRV();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = "_kerberos._tcp";
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_SRV;

                zone.AddRecord(new VmDnsRecordSRV(record));
            }
        }

        private static void
        TestAddNSRecord()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");
                var data = new VMDNS_PTR_DATA();
                data.hostName = "dns1.TestZone";
                var record = new VMDNS_RECORD_NS();
                record.data = data;
                record.common.pszName = "TestZone";
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_NS;

                zone.AddRecord(new VmDnsRecordNS(record));
            }
        }

        private static void
        TestDeletePtrRecord()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");
                IList<VmDnsRecord> records;
                var data = new VMDNS_PTR_DATA();
                data.hostName = "dns1.TestZone";
                var record = new VMDNS_RECORD_PTR();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = "11.1.168.192.in.arpa";
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_PTR;

                zone.AddRecord(new VmDnsRecordPTR(record));
                records = zone.QueryRecords(
                    record.common.pszName,
                    RecordType.VMDNS_RR_TYPE_PTR,
                    0);
                zone.DeleteRecord(new VmDnsRecordPTR(record));
            }
        }

        private static void
        TestListRecords()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");
                var records = zone.ListRecords();
            }
        }

        private static void
        TestQueryRecords()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");
                var records = zone.QueryRecords(
                                  "_kerberos._tcp",
                                  RecordType.VMDNS_RR_TYPE_SRV,
                                  0);
            }
        }

        private static void
        TestForwarders()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "TestZone");

                client.AddForwarder("fwd1.forwarders.com");
                client.AddForwarder("fwd2.forwarders.com");
                client.GetForwarders();
                client.DeleteForwarder("fwd2.forwarders.com");
                client.GetForwarders();
                client.DeleteForwarder("fwd1.forwarders.com");
                client.GetForwarders();
            }
        }

        private static void
        RunTests()
        {
            TestZoneOperations();
            TestAddARecord();
            TestAddIpv6AddressRecord();
            TestAddSrvRecord();
            TestAddNSRecord();
            TestListRecords();
            TestQueryRecords();
            TestForwarders();
            //TestDeletePtrRecord();
        }

        private static void Main(string[] args)
        {
            RunTests();
        }
    }
}