using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMAFD.Client
{
    public class Client : IDisposable
    {
        public Client(
            string pszNetworkAddress,
            string pszUserName,
            string pszPassword
            )
        {
            if (string.IsNullOrEmpty(pszNetworkAddress) ||
                string.IsNullOrEmpty(pszUserName) ||
                string.IsNullOrEmpty(pszPassword))
            {
                throw new VmAfdException((uint)WinError.ERROR_INVALID_PARAMETER);
            }

            this.session = new Session(
                                pszNetworkAddress,
                                pszUserName,
                                pszPassword);
        }

        #region CDC API

        public void
        CdcEnableClientAffinity()
        {
            UInt32 error = Adaptor.CdcEnableClientAffinity(this.session.ServerContext);
            if (error != 0)
            {
                throw new VmAfdException(error);
            }
        }

        public void
        CdcDisableClientAffinity()
        {
            UInt32 error = Adaptor.CdcDisableClientAffinity(this.session.ServerContext);
            if (error != 0)
            {
                throw new VmAfdException(error);
            }
        }

        public CDC_DC_INFO
        CdcGetDCName(
            string pszDomainName,
            string pszSiteName,
            UInt32 dwFlags
            )
        {
            CDC_DC_INFO dcInfo;
            IntPtr pDomainControllerInfo = IntPtr.Zero;
            UInt32 error = Adaptor.CdcGetDCNameA(
                            this.session.ServerContext,
                            pszDomainName,
                            "",
                            pszSiteName,
                            dwFlags,
                            out pDomainControllerInfo);
            if (error != 0)
            {
                throw new VmAfdException(error);
            }

            try
            {
                dcInfo = (CDC_DC_INFO)Marshal.PtrToStructure(
                                pDomainControllerInfo,
                                typeof(CDC_DC_INFO));
            }
            finally
            {
                Adaptor.CdcFreeDomainControllerInfoA(pDomainControllerInfo);
            }

            return dcInfo;
        }

        public IList<string>
        CdcEnumDCEntries()
        {
            IntPtr pDCEntries = IntPtr.Zero;
            uint count = 0;
            var entries = new List<string>();

            UInt32 error = Adaptor.CdcEnumDCEntriesA(
                            this.session.ServerContext,
                            out pDCEntries,
                            out count);
            if (error != 0)
            {
                throw new VmAfdException(error);
            }

            try
            {
                IntPtr pDC = pDCEntries; 
                var ptrSize = Marshal.SizeOf(typeof(IntPtr));
                for (int i = 0; i < count; ++i)
                {
                    var pEntry = Marshal.ReadIntPtr(pDC);
                    string entry = Marshal.PtrToStringAnsi(pEntry);
                    entries.Add(entry);
                    pDC = (IntPtr)((long)pDC + ptrSize);
                }
            }
            finally
            {
                if (pDCEntries != IntPtr.Zero)
                {
                    Adaptor.CdcFreeStringArrayA(pDCEntries, count);
                }
            }

            return entries;
        }

        public CDC_DC_STATE
        CdcGetCurrentState()
        {
            CDC_DC_STATE cdcState = CDC_DC_STATE.CDC_DC_STATE_UNDEFINED;
            UInt32 error = Adaptor.CdcGetCurrentState(
                                this.session.ServerContext,
                                ref cdcState);

            if (error != 0)
            {
                throw new VmAfdException(error);
            }

            return cdcState;
        }
        
        public void CdcGetDCStatus(
            string pwszDCName,
            string pwszDomainName,
            out CDC_DC_STATUS_INFO dcStatus,
            out VMAFD_HEARTBEAT_STATUS hbStatus
        )
        {
            string siteName = string.Empty;
            IntPtr dcNativeStatus = IntPtr.Zero;
            IntPtr hbHbStatus = IntPtr.Zero;

            try
            {
                UInt32 error = Adaptor.CdcGetDCStatusInfoA(
                                    this.session.ServerContext,
                                    pwszDCName,
                                    pwszDomainName,
                                    out dcNativeStatus,
                                    out hbHbStatus);
                if (error != 0)
                {
                    throw new VmAfdException(error);
                }

                dcStatus = MarshalCdcDcStatusInfo(dcNativeStatus);
                hbStatus = MarshalHeartbeatStatus(hbHbStatus);
            }
            finally
            {
                if (dcNativeStatus != IntPtr.Zero)
                {
                    Adaptor.CdcFreeDCStatusInfoA(dcNativeStatus);
                }

                if (hbHbStatus != IntPtr.Zero)
                {
                    Adaptor.VmAfdFreeHeartbeatStatusA(hbHbStatus);
                }


            }
        }

        private static CDC_DC_STATUS_INFO MarshalCdcDcStatusInfo(IntPtr dcNativeStatus)
        {
            CDC_DC_STATUS_INFO result = (CDC_DC_STATUS_INFO)Marshal.PtrToStructure(
                                                dcNativeStatus,
                                                typeof(CDC_DC_STATUS_INFO));
            return result;
        }

        #endregion CDC API

        #region Heartbeat API

        public VMAFD_HEARTBEAT_STATUS
        VmAfdGetHeartbeatStatus()
        {
            string siteName = string.Empty;
            IntPtr pStatus = IntPtr.Zero;
            VMAFD_HEARTBEAT_STATUS result = new VMAFD_HEARTBEAT_STATUS();
            try
            {
                UInt32 error = Adaptor.VmAfdGetHeartbeatStatusA(
                                    this.session.ServerContext,
                                    out pStatus);
                if (error != 0)
                {
                    throw new VmAfdException(error);
                }

                result = MarshalHeartbeatStatus(pStatus);
            }
            finally
            {
                if (pStatus != IntPtr.Zero)
                {
                    Adaptor.VmAfdFreeHeartbeatStatusA(pStatus);
                }
            }

            return result;
        }

        private static VMAFD_HEARTBEAT_STATUS MarshalHeartbeatStatus(IntPtr pStatus)
        {
            VMAFD_HB_STATUS status;
            VMAFD_HEARTBEAT_STATUS result = new VMAFD_HEARTBEAT_STATUS();

            status = (VMAFD_HB_STATUS)Marshal.PtrToStructure(
                        pStatus,
                        typeof(VMAFD_HB_STATUS));
            result.dwCount = status.dwCount;
            result.bIsAlive = status.bIsAlive;
            if (result.dwCount > 0)
            {
                result.info = new List<VMAFD_HB_INFO>();
            }
            for (int i = 0; i < status.dwCount; ++i)
            {
                var sizeOfStruct = Marshal.SizeOf(typeof(VMAFD_HB_INFO));
                IntPtr pStatusEntry = (IntPtr)(
                                        (long)status.pHeartbeatInfoArr +
                                            i * sizeOfStruct);
                var infoEntry = (VMAFD_HB_INFO)Marshal.PtrToStructure(
                                    pStatusEntry,
                                    typeof(VMAFD_HB_INFO));
                result.info.Add(infoEntry);
            }
            return result;
        }

        #endregion Heartbeat API

        #region Misc API

        public string
        VmAfdGetSiteName()
        {
            IntPtr pSiteName = IntPtr.Zero;
            UInt32 error = Adaptor.VmAfdGetSiteNameHA(
                                this.session.ServerContext,
                                out pSiteName);
            if (error != 0)
            {
                throw new VmAfdException(error);
            }

            return Marshal.PtrToStringAnsi(pSiteName);
        }

        #endregion Misc API

        #region IDisposable

        ~Client()
        {
            this.Dispose(false);
        }

        public void
        Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        public void
        Dispose(bool dispose)
        {
            if (this.session != null)
            {
                this.session.Dispose();
                this.session = null;
            }
        }

        #endregion IDisposable

        private Session session;
    }
}
