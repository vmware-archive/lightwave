using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMDNS.Client
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_ZONE_INFO
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string pszName;              // Max length is 255 octets

        [MarshalAs(UnmanagedType.LPStr)]
        public string pszPrimaryDnsSrvName; // Primary DNS server name

        [MarshalAs(UnmanagedType.LPStr)]
        public string pszRName;             // Mailbox of responsible person

        public UInt32 serial;               // version of zone data.
        public UInt32 refreshInterval;      // Refresh interval
        public UInt32 retryInterval;        // Retry of refresh interval
        public UInt32 expire;               // upper limit of being authoritative.
        public UInt32 minimum;              // Minimum TTL
        public UInt32 dwFlags;              // Flags - reserved
        public UInt32 dwZoneType;           // Type
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct VMDNS_ZONE_INFO_ARRAY
    {
        public UInt32 dwNumInfos;       // Count of zone info
        public IntPtr zoneInfos;        // Zone info array
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_IP6_ADDRESS
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public Byte[] bytes;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_IP4_ADDRESS_ARRAY
    {
        public UInt32 dwCount;
        public UInt32[] addrs;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_IP6_ADDRESS_ARRAY
    {
        public UInt32 dwCount;
        public VMDNS_IP6_ADDRESS[] addrs;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_A_DATA
    {
        public UInt32 IpAddress;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_AAAA_DATA
    {
        public VMDNS_IP6_ADDRESS Ip6Address;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_PTR_DATA
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string hostName;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_SOA_DATA
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string pNamePrimaryServer;

        [MarshalAs(UnmanagedType.LPStr)]
        public string pNameAdministrator;

        public UInt32 dwSerialNo;
        public UInt32 dwRefresh;
        public UInt32 dwRetry;
        public UInt32 dwExpire;
        public UInt32 dwDefaultTtl;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_SRV_DATA
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string pNameTarget;

        public UInt16 wPriority;
        public UInt16 wWeight;
        public UInt16 wPort;
        public UInt16 Pad;            // keep ptrs DWORD aligned
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDNS_FORWARDERS
    {
        public UInt32 dwCount;
        public IntPtr pszForwarders;
    }
}
