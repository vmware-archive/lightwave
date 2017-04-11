using System;
using System.Runtime.InteropServices;

namespace VMDIR.Client
{
    internal class Adaptor
    {
        private const string LIBNAME = @"libvmdirclient.dll";

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDirGetComputers(
            string pszServerName,
            string pszUserName,
            string pszPassword,
            out IntPtr pComputers,
            out int numComputers
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDirGetDCInfo(
            string pszServerName,
            string pszUserName,
            string pszPassword,
            out IntPtr pDCInfos,
            out int numDCInfos
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDirFreeDCInfo(
            IntPtr pDCInfo
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDirFreeDCInfoArray(
            IntPtr pDCInfos,
            int numDCInfos
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDirFreeStringArray(
            IntPtr pStrs,
            int numStrs
            );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmDirGetErrorMessage(
            UInt32 dwError,
            out IntPtr pErrorMessage
            );
    }
}
