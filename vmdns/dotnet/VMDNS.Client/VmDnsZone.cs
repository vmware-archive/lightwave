/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Runtime.InteropServices;

namespace VMDNS.Client
{
    public enum VmDnsZoneType
    {
        FORWARD,
        REVERSE
    }

    public class VmDnsZone
    {
        public VmDnsZone(
            VmDnsSession session,
            VMDNS_ZONE_INFO zoneInfo
        )
        {
            this.session = session;
            this.zoneInfo = zoneInfo;
        }

        public string Name
        {
            get
            {
                return this.zoneInfo.pszName;
            }
            set
            {
                this.zoneInfo.pszName = value;
            }
        }

        public VmDnsZoneType Type { get; set; }

        public uint Serial
        {
            get
            {
                return zoneInfo.serial;
            }
            set
            {
                this.zoneInfo.serial = value;
            }
        }

        public string DNSName
        {
            get
            {
                return this.zoneInfo.pszPrimaryDnsSrvName;
            }
            set
            {
                this.zoneInfo.pszPrimaryDnsSrvName = value;
            }
        }

        public string AdminEmail
        {
            get
            {
                return this.zoneInfo.pszRName;
            }
            set
            {
                this.zoneInfo.pszRName = value;
            }
        }

        public uint RefreshInterval
        {
            get
            {
                return this.zoneInfo.refreshInterval;
            }
            set
            {
                this.zoneInfo.refreshInterval = value;
            }
        }

        public uint RetryInterval
        {
            get
            {
                return this.zoneInfo.retryInterval;
            }
            set
            {
                this.zoneInfo.retryInterval = value;
            }
        }

        public uint MinimumTTL
        {
            get
            {
                return this.zoneInfo.minimum;
            }
            set
            {
                this.zoneInfo.minimum = value;
            }
        }

        public uint ExpiresAfter
        {
            get
            {
                return this.zoneInfo.expire;
            }
            set
            {
                this.zoneInfo.expire = value;
            }
        }

        private VmDnsSession session;

        public IList<VmDnsRecord>
        ListRecords()
        {
            IntPtr pRecordArrayPtr = IntPtr.Zero;
            IList<VmDnsRecord> records = null;

            UInt32 error = Adaptor.VmDnsListRecordsA(
                               session.ServerContext,
                               this.Name,
                               out pRecordArrayPtr);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }

            try
            {
                records = RecordArrayToList(pRecordArrayPtr);
            }
            finally
            {
                Adaptor.VmDnsFreeRecordArray(pRecordArrayPtr);
            }

            return records;
        }

        public IList<VmDnsRecord>
        QueryRecords(
            string name,
            RecordType type,
            UInt32 options
        )
        {
            IntPtr pRecordArrayPtr = IntPtr.Zero;
            IList<VmDnsRecord> records = null;
            UInt32 error = Adaptor.VmDnsQueryRecordsA(
                               session.ServerContext,
                               this.Name,
                               name,
                               (UInt16)type,
                               options,
                               out pRecordArrayPtr);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }

            try
            {
                records = RecordArrayToList(pRecordArrayPtr);
            }
            finally
            {
                Adaptor.VmDnsFreeRecordArray(pRecordArrayPtr);
            }

            return records;
        }

        public void
        AddRecord(
            VmDnsRecord record
        )
        {
            record.AddToZone(this.Name, session);
        }

        public void
        DeleteRecord(
            VmDnsRecord record
        )
        {
            record.DeleteFromZone(this.Name, session);
        }

        public VMDNS_ZONE_INFO ZoneInfo
        {
            get
            {
                return zoneInfo;
            }

            set
            {
                this.zoneInfo = value;
            }
        }

        protected VMDNS_ZONE_INFO zoneInfo;


        #region utils

        private static VmDnsRecord
        CreateRecordFromBuffer(IntPtr pRecord)
        {
            VmDnsRecord record = null;
            IntPtr pField = pRecord;
            string name = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(pField));
            pField = (IntPtr)((long)pField + Marshal.SizeOf(typeof(IntPtr)));
            RecordType type = (RecordType)Marshal.ReadInt16(pField);
            switch (type)
            {
                case RecordType.VMDNS_RR_TYPE_A:
                    record = new VmDnsRecordA(
                        (VMDNS_RECORD_A)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_A)));
                    break;

                case RecordType.VMDNS_RR_TYPE_AAAA:
                    record = new VmDnsRecordAAAA(
                        (VMDNS_RECORD_AAAA)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_AAAA)));
                    break;

                case RecordType.VMDNS_RR_TYPE_SRV:
                    record = new VmDnsRecordSRV(
                        (VMDNS_RECORD_SRV)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_SRV)));
                    break;

                case RecordType.VMDNS_RR_TYPE_NS:
                    record = new VmDnsRecordNS(
                        (VMDNS_RECORD_NS)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_NS)));
                    break;

                case RecordType.VMDNS_RR_TYPE_PTR:
                    record = new VmDnsRecordPTR(
                        (VMDNS_RECORD_PTR)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_PTR)));
                    break;

                case RecordType.VMDNS_RR_TYPE_CNAME:
                    record = new VmDnsRecordCNAME(
                        (VMDNS_RECORD_CNAME)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_CNAME)));
                    break;

                case RecordType.VMDNS_RR_TYPE_SOA:
                    record = new VmDnsRecordSOA(
                        (VMDNS_RECORD_SOA)
                                    Marshal.PtrToStructure(
                            pRecord,
                            typeof(VMDNS_RECORD_SOA)));
                    break;

                default:
                    throw new NotSupportedException(string.Format("Unknown type: {0}", type));
            }

            return record;
        }

        internal static IList<VmDnsRecord>
        RecordArrayToList(
            IntPtr pRecordArrayPtr
        )
        {
            var result = new List<VmDnsRecord>();
            VMDNS_RECORD_ARRAY pRecordArray = (VMDNS_RECORD_ARRAY)Marshal.PtrToStructure(
                                                  pRecordArrayPtr,
                                                  typeof(VMDNS_RECORD_ARRAY));
            IntPtr pRecord = pRecordArray.pRecords;
            for (int i = 0; i < pRecordArray.dwCount; ++i)
            {
                VmDnsRecord record = CreateRecordFromBuffer(pRecord);
                result.Add(record);
                pRecord = (IntPtr)((long)pRecord + Adaptor.VMDNS_RECORD_SIZE);
            }

            return result;
        }

        #endregion utils
    }
}