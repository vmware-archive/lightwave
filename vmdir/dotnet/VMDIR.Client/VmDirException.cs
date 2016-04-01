using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMDIR.Client
{
    public class VmDirException : Exception
    {
        public override string Message
        {
            get
            {
                return GetMessageForError(this.ErrorCode);
            }
        }

        public VmDirException(UInt32 errorCode)
        {
            ErrorCode = errorCode;
        }

        public UInt32 ErrorCode { get; protected set; }

        public string GetMessageForError(UInt32 error)
        {
            string errorMessage = "Unknown error";
            IntPtr pErrorMessage = IntPtr.Zero;
            if (Adaptor.VmDirGetErrorMessage(error, out pErrorMessage) == 0)
            {
                errorMessage = Marshal.PtrToStringAnsi(pErrorMessage);
            }

            return errorMessage;
        }
    }
}
