using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMAFD.Client
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct CDC_DC_INFO
    {
        public string pszDCName;
        public string pszDCAddress;
        public CDC_ADDRESS_TYPE DcAddressType;
        public string pszDomainName;
        public string pszDcSiteName;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct CDC_DC_STATUS_INFO
    {
        public UInt32 dwLastPing;
        public UInt32 dwLastResponseTime;
        public UInt32 dwLastError;
        public UInt32 bIsAlive;
        public string pszSiteName;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMAFD_HB_INFO
    {
        public string pszServiceName;
        public UInt32 dwPort;
        public UInt32 dwLastHeartbeat;
        public UInt32 bIsAlive;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMAFD_HB_STATUS
    {
        public UInt32 bIsAlive;
        public UInt32 dwCount;
        public IntPtr pHeartbeatInfoArr;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMAFD_HEARTBEAT_STATUS
    {
        public UInt32 bIsAlive;
        public UInt32 dwCount;
        public IList<VMAFD_HB_INFO> info;
    };

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public struct VMAFD_DC_ENTRIES
    {
        public UInt32 dwCount;
        public IntPtr ppszEntries;
    };

}
