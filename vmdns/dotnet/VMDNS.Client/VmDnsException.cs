using System;
using System.Runtime.InteropServices;

namespace VMDNS.Client
{
    public enum WinError
    {
        ERROR_ACCESS_DENIED = 5,
        ERROR_INVALID_PARAMETER = 87,
        ERROR_NOT_FOUND = 1168,
    }

    public class VmDnsException : Exception
    {
        public override string Message
        {
            get
            {
                return "ErrorCode : " + ErrorCode.ToString();
                //TODO - Hu to fix this and return a platform dependent Error Message
                //return GetSystemMessage(this.ErrorCode);
            }
        }

        public VmDnsException(UInt32 errorCode)
        {
            ErrorCode = errorCode;
        }

        public UInt32 ErrorCode { get; protected set; }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr LocalFree(IntPtr pMemory);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern int FormatMessage(
            FormatMessageFlags flags,
            IntPtr source,
            uint messageId,
            uint languageId,
            ref IntPtr buffer,
            uint size,
            IntPtr arguments);

        [Flags]
        private enum FormatMessageFlags : uint
        {
            FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100,
            FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200,
            FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000
        }

        public static string GetSystemMessage(UInt32 errorCode)
        {
            string message;
            try
            {
                IntPtr messageBuffer = IntPtr.Zero;

                if (FormatMessage(
                        FormatMessageFlags.FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FormatMessageFlags.FORMAT_MESSAGE_FROM_SYSTEM |
                        FormatMessageFlags.FORMAT_MESSAGE_IGNORE_INSERTS,
                        IntPtr.Zero,
                        (uint)errorCode,
                        0,
                        ref messageBuffer,
                        0,
                        IntPtr.Zero) > 0)
                {
                    message = Marshal.PtrToStringAnsi(messageBuffer);
                    messageBuffer = LocalFree(messageBuffer);
                }
                else
                {
                    message = "Failed to get system error message.";
                }

                return message;
            }
            catch (Exception e)
            {
                return "Failed to get system error message. " + e.ToString();
            }
        }
    }
}