using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VMCertStore.Client
{
    public class VMCertStoreException : Exception
    {
        public UInt32 OriginalErrorCode { get; protected set; }
        public VMCertStoreErrorCode ErrorCode { get { return (VMCertStoreErrorCode)OriginalErrorCode; } }

        public override string Message
        {
            get
            {
                return string.Format("Error Number: {0}, Error Name: {1}, Error String: {2}", OriginalErrorCode, ErrorCode.ToString(), base.Message);
            }
        }

        public VMCertStoreException(UInt32 errorCode, string error):base(error)
        {

        }
    }
}
