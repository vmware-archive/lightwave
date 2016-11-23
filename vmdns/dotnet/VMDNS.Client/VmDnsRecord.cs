using System;
using System.Net;
using System.Runtime.InteropServices;

namespace VMDNS.Client
{
    public enum RecordType
    {
        VMDNS_RR_TYPE_A = 0x0001,
        VMDNS_RR_TYPE_NS = 0x0002,
        VMDNS_RR_TYPE_MD = 0x0003,
        // obsoleted by MX
        VMDNS_RR_TYPE_MF = 0x0004,
        // obsoleted by MX
        VMDNS_RR_TYPE_CNAME = 0x0005,
        VMDNS_RR_TYPE_SOA = 0x0006,
        VMDNS_RR_TYPE_MB = 0x0007,
        // experimental
        VMDNS_RR_TYPE_MG = 0x0008,
        // experimental
        VMDNS_RR_TYPE_MR = 0x0009,
        // experimental
        VMDNS_RR_TYPE_NULL = 0x000A,
        // experimental
        VMDNS_RR_TYPE_WKS = 0x000B,
        VMDNS_RR_TYPE_PTR = 0x000C,
        VMDNS_RR_TYPE_HINFO = 0x000D,
        VMDNS_RR_TYPE_MINFO = 0x000E,
        VMDNS_RR_TYPE_MX = 0x000F,
        VMDNS_RR_TYPE_TXT = 0x0010,
        VMDNS_RR_TYPE_RP = 0x0011,
        VMDNS_RR_TYPE_AFSDB = 0x0012,
        VMDNS_RR_TYPE_SIG = 0x0018,
        VMDNS_RR_TYPE_AAAA = 0x001C,
        VMDNS_RR_TYPE_LOC = 0x001D,
        VMDNS_RR_TYPE_SRV = 0x0021,
        VMDNS_RR_TYPE_CERT = 0x0025,
        VMDNS_RR_TYPE_DS = 0x002B,
        VMDNS_RR_TYPE_SSHFP = 0x002C,
        VMDNS_RR_TYPE_IPSEC = 0x002D,
        VMDNS_RR_TYPE_RRSIG = 0x002E,
        VMDNS_RR_TYPE_DNSKEY = 0x0030,
        VMDNS_RR_TYPE_TKEY = 0x00F9,
        VMDNS_RR_TYPE_TSIG = 0x00FA
    }

    #region record structs

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_RECORD_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_A
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_A_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_AAAA
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_AAAA_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_ARRAY
    {
        public UInt32 dwCount;
        public IntPtr pRecords;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_CNAME
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_PTR_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_COMMON
    {
        public string pszName;
        public UInt16 type;
        public UInt16 iClass;
        public Int32 ttl;
    }

    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_DATA
    {
        [FieldOffset(0)]
        private VMDNS_PTR_DATA NS;

        [FieldOffset(0)]
        private VMDNS_PTR_DATA PTR;

        [FieldOffset(0)]
        private VMDNS_PTR_DATA CNAME;

        [FieldOffset(0)]
        private VMDNS_A_DATA A;

        [FieldOffset(0)]
        private VMDNS_AAAA_DATA AAAA;

        [FieldOffset(0)]
        private VMDNS_SRV_DATA SRV;

        [FieldOffset(0)]
        private VMDNS_SOA_DATA SOA;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_NS
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_PTR_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_PTR
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_PTR_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_SOA
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_SOA_DATA data;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_RECORD_SRV
    {
        public VMDNS_RECORD_COMMON common;
        public VMDNS_SRV_DATA data;
    }

    #endregion record structs

    #region record classes

    public abstract class VmDnsRecord
    {
        public UInt16 IClass { get { return this.Common.iClass; } }

        public string Name
        {
            get
            {
                return this.Common.pszName;
            }
        }

        public Int32 TTL
        {
            get
            {
                return this.Common.ttl;
            }
        }

        public UInt16 Type
        {
            get
            {
                return this.Common.type;
            }
        }

        protected abstract VMDNS_RECORD_COMMON Common { get; }

        public abstract void AddToZone(
            string zoneName,
            VmDnsSession session
        );

        public abstract void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        );
    }

    public class VmDnsRecordA : VmDnsRecord
    {
        public VmDnsRecordA(VMDNS_RECORD_A record)
        {
            this.RECORD = record;
        }

        public string Address
        {
            get
            {
                return IPAddress.Parse(this.RECORD.data.IpAddress.ToString()).ToString();
            }
        }

        public VMDNS_RECORD_A RECORD { get; protected set; }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_A record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordA(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_A record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordA(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    public class VmDnsRecordAAAA : VmDnsRecord
    {
        public VmDnsRecordAAAA(VMDNS_RECORD_AAAA record)
        {
            this.RECORD = record;
        }

        public string Address
        {
            get
            {
                var ipv6 = new IPAddress(this.RECORD.data.Ip6Address.bytes);
                return ipv6.ToString();
            }
        }

        public VMDNS_RECORD_AAAA RECORD { get; protected set; }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_AAAA record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordAAAA(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_AAAA record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordAAAA(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    public class VmDnsRecordCNAME : VmDnsRecord
    {
        public VmDnsRecordCNAME(VMDNS_RECORD_CNAME record)
        {
            this.RECORD = record;
        }

        public string HostName
        {
            get
            {
                return this.RECORD.data.hostName;
            }
        }

        public VMDNS_RECORD_CNAME RECORD { get; protected set; }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_CNAME record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordCNAME(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_CNAME record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordCNAME(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    public class VmDnsRecordNS : VmDnsRecord
    {
        public VmDnsRecordNS(VMDNS_RECORD_NS record)
        {
            this.RECORD = record;
        }

        public VMDNS_RECORD_NS RECORD { get; protected set; }

        public string Target
        {
            get
            {
                return this.RECORD.data.hostName;
            }
        }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_NS record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordNS(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_NS record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordNS(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    public class VmDnsRecordPTR : VmDnsRecord
    {
        public VmDnsRecordPTR(VMDNS_RECORD_PTR record)
        {
            this.RECORD = record;
        }

        public string HostName
        {
            get
            {
                return this.RECORD.data.hostName;
            }
        }

        public VMDNS_RECORD_PTR RECORD { get; protected set; }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_PTR record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordPTR(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_PTR record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordPTR(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    public class VmDnsRecordSOA : VmDnsRecord
    {
        public VmDnsRecordSOA(VMDNS_RECORD_SOA record)
        {
            this.RECORD = record;
        }

        public VMDNS_RECORD_SOA RECORD { get; protected set; }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_SOA record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordSOA(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_SOA record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordSOA(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    public class VmDnsRecordSRV : VmDnsRecord
    {
        public VmDnsRecordSRV(VMDNS_RECORD_SRV record)
        {
            this.RECORD = record;
        }

        public UInt16 Port
        {
            get
            {
                return this.RECORD.data.wPort;
            }
        }

        public UInt16 Priority
        {
            get
            {
                return this.RECORD.data.wPriority;
            }
        }

        public VMDNS_RECORD_SRV RECORD { get; protected set; }

        public string Target
        {
            get
            {
                return this.RECORD.data.pNameTarget;
            }
        }

        public UInt16 Weight
        {
            get
            {
                return this.RECORD.data.wWeight;
            }
        }

        protected override VMDNS_RECORD_COMMON Common { get { return this.RECORD.common; } }

        public override void AddToZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_SRV record = this.RECORD;
            UInt32 error = Adaptor.VmDnsAddRecordSRV(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public override void DeleteFromZone(
            string zoneName,
            VmDnsSession session
        )
        {
            VMDNS_RECORD_SRV record = this.RECORD;
            UInt32 error = Adaptor.VmDnsDeleteRecordSRV(
                               session.ServerContext,
                               zoneName,
                               ref record);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }
    }

    #endregion record classes
}