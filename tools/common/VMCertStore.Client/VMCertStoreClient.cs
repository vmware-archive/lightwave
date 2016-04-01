using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
namespace VMCertStore.Client
{    public class VMCertStoreClient
    {
        public string ServerName { get; protected set; }
        public VMCertStoreClient(string serverName)
        {
            ServerName = serverName;
        }		
        public string GetVersion()
        {            return "1.0.0";
        }
    }
}
