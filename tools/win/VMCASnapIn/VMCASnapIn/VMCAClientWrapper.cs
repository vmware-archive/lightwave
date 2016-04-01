using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace VMCASnapIn
{
    public class VMCAClientWrapper
    {
        public const int VMCA_CERTIFICATE_ACTIVE = 0;
        public const int VMCA_CERTIFICATE_REVOKED = 1;
        public const int VMCA_CERTIFICATE_EXPIRED = 2;
        public const int VMCA_CERTIFICATE_ALL = 4;

        public const int VMCA_ENUM_SUCCESS = 0;
        public const int VMCA_ENUM_END = 1;
        public const int VMCA_ENUM_ERROR = 2;

        public const UInt32 VMCA_SUCCESS = 0;

        [DllImport("C:\\Program Files\\VMware\\cis\\vmcad\\libvmcaclient.dll")]
        public static extern UInt32 VMCAGetRootCACertificate(string argServerName, out IntPtr pCert);

        [DllImport("C:\\Program Files\\VMware\\cis\\vmcad\\libvmcaclient.dll")]
        public static extern UInt32 VMCAOpenEnumContext(string argServerName, Int32 dwStatus, out IntPtr pContext);
        [DllImport("C:\\Program Files\\VMware\\cis\\vmcad\\libvmcaclient.dll")]
        public static extern UInt32 VMCAGetNextCertificate(IntPtr pContext, out IntPtr pCertificate, out Int32 currIndex, out Int32 enumStatus);
        [DllImport("C:\\Program Files\\VMware\\cis\\vmcad\\libvmcaclient.dll")]
        public static extern UInt32 VMCACloseEnumContext(IntPtr pContext);
        [DllImport("C:\\Program Files\\VMware\\cis\\vmcad\\libvmcaclient.dll")]
        public static extern UInt32 VMCARevokeCertificate(string argServerName, string cert);
    }
}
