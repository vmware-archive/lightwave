using System;
using System.Runtime.InteropServices;

namespace VMAFD.Client
{
    internal class Adaptor
    {
        private const string LIBNAME = @"libvmafdclient.dll";

        #region connection methods

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmAfdOpenServerA(
            string pszServerName,
            string pszUserName,
            string pszPassword,
            out IntPtr pServerContext
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmAfdCloseServer(
            IntPtr pServerContext
            );

        #endregion connection methods

        #region CDC API

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcEnableClientAffinity(
            IntPtr pServerContext
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcDisableClientAffinity(
            IntPtr pServerContext
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcGetDCNameA(
            IntPtr pServerContext,
            string pszDomainName,
            string pszDomainGuid,
            string pszSiteName,
            UInt32 dwFlags,
            out IntPtr pDomainControllerInfo
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcEnumDCEntriesA(
            IntPtr pServerContext,
            out IntPtr pDCEntries,
            out UInt32 dwCount
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcGetDCStatusInfoA(
            IntPtr pServerContext,
            string pwszDCName,
            string pwszDomainName,
            out IntPtr ppDCStatusInfo,
            out IntPtr ppHbStatus
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcGetCurrentState(
            IntPtr pServerContext,
            ref CDC_DC_STATE cdcState
            );

        [DllImport(LIBNAME, CharSet=CharSet.Ansi)]
        public static extern UInt32
        CdcFreeStringArrayA(
            IntPtr pStrings,
            UInt32 dwCount
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcFreeDomainControllerInfoA(
            IntPtr pDomainControllerInfo
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcFreeDCEntriesA(
            IntPtr pDCEntries
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        CdcFreeDCStatusInfoA(
            IntPtr pDCInfo
            );

        #endregion CDC API

        #region AFD API

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmAfdGetSiteNameHA(
            IntPtr pServerContext,
            out IntPtr pszSiteName
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmAfdGetHeartbeatStatusA(
            IntPtr pServerContext,
            out IntPtr pStatus
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmAfdFreeHeartbeatStatusA(
            IntPtr pStatus
            );

        #endregion AFD API
    }
}
