using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VMAFD.Client
{
    public class Session : IDisposable
    {
        public
        Session(
            string pszNetworkAddress,
            string pszUserName,
            string pszPassword
            )
        {
            UInt32 dwError = 0;
            IntPtr pServerContext = IntPtr.Zero;
            dwError = Adaptor.VmAfdOpenServerA(
                                    pszNetworkAddress,
                                    pszUserName,
                                    pszPassword,
                                    out pServerContext
                                    );
            if (dwError != 0)
            {
                throw new VmAfdException(dwError);
            }
            this.ServerContext = pServerContext;
        }

        ~Session()
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
            if (this.ServerContext != IntPtr.Zero)
            {
                Adaptor.VmAfdCloseServer(this.ServerContext);
                this.ServerContext = IntPtr.Zero;
            }
        }

        public IntPtr ServerContext
        {
            get;
            private set;
        }
    }
}
