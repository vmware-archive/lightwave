using System;

namespace VMDNS.Client
{
    public class VmDnsSession : IDisposable
    {
        public
        VmDnsSession(
            string pszNetworkAddress,
            string pszUserName,
            string pszDomain,
            string pszPassword
        )
        {
            try
            {
                EnsureServerContext(pszNetworkAddress, pszUserName, pszDomain, pszPassword);
            }
            catch (Exception e)
            {
                throw e;
            }

        }

        public void EnsureServerContext(string pszNetworkAddress,
                                        string pszUserName,
                                        string pszDomain,
                                        string pszPassword)
        {
            try
            {
                UInt32 dwError = 0;
                IntPtr pServerContext = IntPtr.Zero;

                if (this.ServerContext != IntPtr.Zero)
                    return;
                dwError = Adaptor.VmDnsOpenServerA(
                    pszNetworkAddress, pszUserName, pszDomain, pszPassword,
                    0,
                    IntPtr.Zero,
                    out pServerContext
                );
                if (dwError != 0)
                {
                    throw new VmDnsException(dwError);
                }
                this.ServerContext = pServerContext;

            }
            catch (Exception e)
            {
                throw e;
            }
        }


        ~VmDnsSession ()
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
            DisposeConnection();
        }

        public void DisposeConnection()
        {
            if (this.ServerContext != IntPtr.Zero)
            {
                Adaptor.VmDnsCloseServer(this.ServerContext);
                this.ServerContext = IntPtr.Zero;
            }
        }

        public IntPtr ServerContext
        {
            get;
            set;
        }

    }
}