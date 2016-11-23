using System;
using System.Collections.Generic;
using System.Net;
using System.Runtime.InteropServices;

namespace VMDNS.Client
{
    public class VmDnsClient : IDisposable
    {
        public VmDnsClient(
            string pszNetworkAddress,
            string pszUserName,
            string pszDomain,
            string pszPassword
        )
        {
            if (string.IsNullOrEmpty(pszNetworkAddress) ||
                string.IsNullOrEmpty(pszUserName) ||
                string.IsNullOrEmpty(pszDomain) ||
                string.IsNullOrEmpty(pszPassword))
            {
                throw new VmDnsException((uint)WinError.ERROR_INVALID_PARAMETER);
            }

            this.session = new VmDnsSession(
                pszNetworkAddress,
                pszUserName,
                pszDomain,
                pszPassword);
        }

        public IList<VmDnsZone>
        ListZones(VmDnsZoneType type)
        {
            VMDNS_ZONE_INFO_ARRAY pZoneInfoArray;
            VMDNS_ZONE_INFO zoneInfo;
            var pZoneInfoArrayPtr = IntPtr.Zero;
            var zoneList = new List<VmDnsZone>();

            UInt32 errorCode = Adaptor.VmDnsListZoneA(
                                   session.ServerContext,
                                   out pZoneInfoArrayPtr);
            if (errorCode != 0)
            {
                throw new VmDnsException(errorCode);
            }

            try
            {
                pZoneInfoArray = (VMDNS_ZONE_INFO_ARRAY)
                                    Marshal.PtrToStructure(
                    pZoneInfoArrayPtr,
                    typeof(VMDNS_ZONE_INFO_ARRAY));
                var zoneInfoSize = Marshal.SizeOf(typeof(VMDNS_ZONE_INFO));
                for (int i = 0; i < pZoneInfoArray.dwNumInfos; ++i)
                {
                    var pZoneInfoPtr = IntPtr.Zero;
                    pZoneInfoPtr = (IntPtr)((long)pZoneInfoArray.zoneInfos + i * zoneInfoSize);
                    zoneInfo = (VMDNS_ZONE_INFO)
                                Marshal.PtrToStructure(
                        pZoneInfoPtr,
                        typeof(VMDNS_ZONE_INFO));
                    if (zoneInfo.dwZoneType == (UInt32)type)
                    {
                        zoneList.Add(new VmDnsZone(this.session, zoneInfo));
                    }
                }
            }
            finally
            {
                if (pZoneInfoArrayPtr != IntPtr.Zero)
                {
                    Adaptor.VmDnsFreeZoneInfoArray(pZoneInfoArrayPtr);
                }
            }

            return zoneList;
        }

        public void
        CreateZone(
            VMDNS_ZONE_INFO zoneInfo
        )
        {
            UInt32 error = Adaptor.VmDnsCreateZoneA(
                               this.session.ServerContext,
                               ref zoneInfo);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public void
        UpdateZone(
            VMDNS_ZONE_INFO zoneInfo
        )
        {
            UInt32 error = Adaptor.VmDnsUpdateZoneA(
                               this.session.ServerContext,
                               ref zoneInfo);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public void
        DeleteZone(
            string name
        )
        {
            UInt32 error = Adaptor.VmDnsDeleteZoneA(
                               this.session.ServerContext,
                               name);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public IList<string>
        GetForwarders()
        {
            var forwarderList = new List<string>();
            var forwarderArrayPtr = IntPtr.Zero;
            VMDNS_FORWARDERS pForwarders;

            UInt32 error = Adaptor.VmDnsGetForwardersA(
                               this.session.ServerContext,
                               out forwarderArrayPtr);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }

            try
            {
                pForwarders = (VMDNS_FORWARDERS)Marshal.PtrToStructure(
                    forwarderArrayPtr,
                    typeof(VMDNS_FORWARDERS));
                for (int i = 0; i < pForwarders.dwCount; ++i)
                {
                    var ptrSize = Marshal.SizeOf(typeof(IntPtr));
                    var pForwarder = Marshal.ReadIntPtr(
                                         (IntPtr)((long)pForwarders.pszForwarders + i * ptrSize));
                    string forwarder = Marshal.PtrToStringAnsi(pForwarder);
                    forwarderList.Add(forwarder);
                }
            }
            finally
            {
                if (forwarderArrayPtr != IntPtr.Zero)
                {
                    Adaptor.VmDnsFreeForwarders(forwarderArrayPtr);
                }
            }

            return forwarderList;
        }

        public void
        AddForwarder(
            string forwarder
        )
        {
            UInt32 error = Adaptor.VmDnsAddForwarderA(
                               this.session.ServerContext,
                               forwarder);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public void
        DeleteForwarder(
            string forwarder
        )
        {
            UInt32 error = Adaptor.VmDnsDeleteForwarderA(
                               this.session.ServerContext,
                               forwarder);
            if (error != 0)
            {
                throw new VmDnsException(error);
            }
        }

        public void CloseConnection()
        {
            this.session.DisposeConnection();
        }

        #region IDisposable

        ~VmDnsClient ()
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

        private VmDnsSession session;
    }
}