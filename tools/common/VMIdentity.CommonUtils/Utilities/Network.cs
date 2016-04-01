using System.Linq;
using System.Net;

namespace VMIdentity.CommonUtils.Utilities
{
    public class Network
    {
        public static string GetIpAddress(string hostname)
        {
            var ips = Dns.GetHostAddresses(hostname);
            return ips != null ? ips.First().ToString() : string.Empty;
        }
    }
}
