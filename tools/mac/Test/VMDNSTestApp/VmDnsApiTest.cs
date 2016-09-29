/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Runtime.InteropServices;
using VMDNS.Client;

/*
 * Mac Unit Test App for DNS nterop
 *  Needs Xamarin.Mac license to run in Xamarin Studio
 * 
 */

namespace VMDNSTestApp
{
    public static  class VmDnsApiTest
    {
        private static string hostname = "10.160.168.108";
        private static string username = "Administrator";
        private static string password = "Admin!23";
        private static string domain = "vsphere.local";

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
                zoneInfo.pszName = "TestZone98";
                zoneInfo.pszPrimaryDnsSrvName = "dns2.vsphere.local";
                zoneInfo.pszRName = "admin@vsphere.local";
                zoneInfo.refreshInterval = 3600;
                zoneInfo.retryInterval = 3600;
                zoneInfo.serial = 1;

                client.CreateZone(zoneInfo);
                zone = FindZone(client, zoneInfo.pszName);

                zoneInfo.pszRName = "administ@vmware.com";
                client.UpdateZone(zoneInfo);

                zone = FindZone(client, zoneInfo.pszName);
                Debug.Assert(zone.Serial == 2);
            }
        }

        private static void
        TestAddARecord()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "vsphere.local");

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
                var zone = FindZone(client, "vsphere.local");
                IList<VmDnsRecord> records;
                var data = new VMDNS_PTR_DATA();
                data.hostName = "dns1.TestZone";
                var record = new VMDNS_RECORD_PTR();
                record.data = data;
                record.common.iClass = 1;
                record.common.pszName = "11.1.168.192.in.arpa";
                record.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_PTR;

                zone.AddRecord(new VmDnsRecordPTR(record));
                var srvdata = new VMDNS_SRV_DATA();
                srvdata.pNameTarget = "dns1.TestZone";
                var srvrecord = new VMDNS_RECORD_SRV();
                srvrecord.data = srvdata;
                srvrecord.common.iClass = 1;
                srvrecord.common.pszName = "_kerberos._tcp";
                srvrecord.common.type = (UInt16)RecordType.VMDNS_RR_TYPE_SRV;
                records = zone.QueryRecords(
                    "_kerberos._tcp",
                    RecordType.VMDNS_RR_TYPE_SRV,
                    0);
                zone.DeleteRecord(records[0]);
            }
        }

        private static void
        TestListRecords()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "vsphere.local");
                var records = zone.ListRecords();
            }
        }

        private static void
        TestQueryRecords()
        {
            using (VmDnsClient client = new VmDnsClient(hostname, username, domain, password))
            {
                var zone = FindZone(client, "vsphere.local");
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
                var zone = FindZone(client, "vsphere.local");

                client.AddForwarder("fwd1.forwarders.com");
                client.AddForwarder("fwd2.forwarders.com");
                IList<string> forwarders = client.GetForwarders();
                client.DeleteForwarder("fwd2.forwarders.com");
                client.GetForwarders();
                client.DeleteForwarder("fwd1.forwarders.com");
                client.GetForwarders();
            }
        }


        public  static void
        RunTests()
        {
            TestZoneOperations();
            TestAddARecord();
            TestAddIpv6AddressRecord();
            TestAddSrvRecord();
            TestAddNSRecord();
            TestListRecords();
            TestQueryRecords();
            TestDeletePtrRecord();
            TestForwarders();
        }
    }
}