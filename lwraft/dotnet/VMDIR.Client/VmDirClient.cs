using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMDIR.Client
{
    public class Client
    {
        #region Topology API

        public static IList<string>
        VmDirGetComputers(
            string server,
            string user,
            string password
            )
        {
            IntPtr pDCEntries = IntPtr.Zero;
            var entryList = new List<string>();
            int numEntries = 0;
            UInt32 error = Adaptor.VmDirGetComputers(
                            server,
                            user,
                            password,
                            out pDCEntries,
                            out numEntries);
            if (error != 0)
            {
                throw new VmDirException(error);
            }

            try
            {
                var ptrSize = Marshal.SizeOf(typeof(IntPtr));
                for (int i = 0; i < numEntries; ++i)
                {
                    var pEntry = Marshal.ReadIntPtr((IntPtr)(
                                    (long)pDCEntries + i * ptrSize));
                    string entry = Marshal.PtrToStringAnsi(pEntry);
                    entryList.Add(entry);
                }
            }
            finally
            {
                if (pDCEntries != IntPtr.Zero)
                {
                    Adaptor.VmDirFreeStringArray(pDCEntries, numEntries);
                }
            }

            return entryList;
        }

        public static IList<VmDirDCInfo>
        VmDirGetDCInfos(
            string server,
            string user,
            string password
            )
        {
            IntPtr pDCEntries = IntPtr.Zero;
            var entryList = new List<VmDirDCInfo>();
            int numEntries = 0;
            UInt32 error = Adaptor.VmDirGetDCInfo(
                            server,
                            user,
                            password,
                            out pDCEntries,
                            out numEntries);
            if (error != 0)
            {
                throw new VmDirException(error);
            }

            try
            {
                var ptrSize = Marshal.SizeOf(typeof(IntPtr));
                for (int i = 0; i < numEntries; ++i)
                {
                    var pEntry = Marshal.ReadIntPtr((IntPtr)(
                                    (long)pDCEntries + i * ptrSize));
                    var entry = (VMDIR_DC_INFO)Marshal.PtrToStructure(
                                                pEntry,
                                                typeof(VMDIR_DC_INFO));
                    var dc = new VmDirDCInfo();
                    dc.pszDCName = entry.pszDCName;
                    dc.pszSiteName = entry.pszSiteName;
                    dc.partners = new List<string>();
                    for (int j = 0; j < entry.numPartners; ++j)
                    {
                        var pPartner = Marshal.ReadIntPtr((IntPtr)(
                                        (long)entry.pPartners + j * ptrSize));
                        dc.partners.Add(Marshal.PtrToStringAnsi(pPartner));
                    }
                    entryList.Add(dc);
                }
            }
            finally
            {
                if (pDCEntries != IntPtr.Zero)
                {
                    Adaptor.VmDirFreeDCInfoArray(pDCEntries, numEntries);
                }
            }

            return entryList;
        }

        #endregion Topology API
    }
}
