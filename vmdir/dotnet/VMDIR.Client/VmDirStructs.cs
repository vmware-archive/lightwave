using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMDIR.Client
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VMDIR_DC_INFO
    {
        public string pszDCName;
        public string pszSiteName;
        public IntPtr pPartners;
        public UInt32 numPartners;
    };

    public struct VmDirDCInfo
    {
        public string pszDCName;
        public string pszSiteName;
        public IList<string> partners;
    };
}
