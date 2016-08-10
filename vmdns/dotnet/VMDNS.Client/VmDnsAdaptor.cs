using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace VMDNS.Client
{
    public class Adaptor
    {
        static Adaptor()
        {
            VMDNS_RECORD_SIZE = 0;
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_A)));
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_AAAA)));
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_SOA)));
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_SRV)));
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_NS)));
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_PTR)));
            VMDNS_RECORD_SIZE = Math.Max(VMDNS_RECORD_SIZE, Marshal.SizeOf(typeof(VMDNS_RECORD_CNAME)));
        }

        private const string LIBNAME = @"libvmdnsclient.dll";

        internal static int VMDNS_RECORD_SIZE;

        #region connection methods

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsOpenServerA(
            string pszNetworkAddress,
            string pszUserName,
            string pszDomain,
            string pszPassword,
            UInt32 dwFlags,
            IntPtr pReserved,
            out IntPtr pServerContext
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsCloseServer(
            IntPtr pServerContext
        );

        #endregion connection methods

        #region zone methods

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsListZoneA(
            IntPtr pServerContext,
            out IntPtr pZoneInfoArray
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsCreateZoneA(
            IntPtr pServerContext,
            ref VMDNS_ZONE_INFO zoneInfo
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsUpdateZoneA(
            IntPtr pServerContext,
            ref VMDNS_ZONE_INFO zoneInfo
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsDeleteZoneA(
            IntPtr pServerContext,
            string name
        );

        #endregion zone methods

        #region AddRecord methods

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordA(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_A pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordAAAA(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_AAAA pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordSRV(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_SRV pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordNS(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_NS pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordPTR(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_PTR pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordCNAME(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_CNAME pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsAddRecordA")]
        public static extern UInt32
        VmDnsAddRecordSOA(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_SOA pRecord
        );

        #endregion AddRecord methods

        #region DeleteRecord methods

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordA(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_A pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordAAAA(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_AAAA pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordSRV(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_SRV pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordNS(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_NS pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordPTR(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_PTR pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordCNAME(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_CNAME pRecord
        );

        [DllImport(LIBNAME, EntryPoint = "VmDnsDeleteRecordA")]
        public static extern UInt32
        VmDnsDeleteRecordSOA(
            IntPtr pServerContext,
            string pszZone,
            ref VMDNS_RECORD_SOA pRecord
        );

        #endregion DeleteRecord methods

        #region misc methods

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsListRecordsA(
            IntPtr pServerContext,
            string pszZone,
            out IntPtr pRecordArray
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsQueryRecordsA(
            IntPtr pServerContext,
            string pszZone,
            string pszName,
            UInt16 type,
            UInt32 dwOptions,
            out IntPtr pRecordArray
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsGetForwardersA(
            IntPtr pServerContext,
            out IntPtr pForwarders
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsAddForwarderA(
            IntPtr pServerContext,
            string forwarder
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDnsDeleteForwarderA(
            IntPtr pServerContext,
            string forwarder
        );

        [DllImport(LIBNAME)]
        public static extern void
        VmDnsFreeZoneInfoArray(
            IntPtr pZoneInfoArray
        );

        [DllImport(LIBNAME)]
        public static extern void
        VmDnsFreeRecordArray(
            IntPtr pRecordArray
        );

        [DllImport(LIBNAME)]
        public static extern void
        VmDnsFreeForwarders(
            IntPtr pForwarders
        );

        #endregion misc methods

        #region utils

        private static VmDnsRecord
        VmDnsCreateRecordFromBuffer(IntPtr pRecord)
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

        private static IList<VmDnsRecord>
        VmDnsRecordArrayToList(
            IntPtr pRecordArrayPtr
        )
        {
            List<VmDnsRecord> result = new List<VmDnsRecord>();
            VMDNS_RECORD_ARRAY pRecordArray = (VMDNS_RECORD_ARRAY)
                Marshal.PtrToStructure(pRecordArrayPtr, typeof(VMDNS_RECORD_ARRAY));
            IntPtr pRecord = pRecordArray.pRecords;
            for (int i = 0; i < pRecordArray.dwCount; ++i)
            {
                VmDnsRecord record = VmDnsCreateRecordFromBuffer(pRecord);
                result.Add(record);
                pRecord = (IntPtr)((long)pRecord + VMDNS_RECORD_SIZE);
            }

            return result;
        }

        #endregion utils
    }
}