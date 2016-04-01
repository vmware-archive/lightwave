using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace VMAFD.Client
{
    public enum WinError
    {
        ERROR_ACCESS_DENIED = 5,
        ERROR_INVALID_PARAMETER = 87,
        ERROR_NOT_FOUND = 1168,
    }

    class ErrorMessagePair
    {
        public WinError Error { get; protected set; }
        public string Message { get; protected set; }
        public ErrorMessagePair(WinError error, string message)
        {
            this.Error = error;
            this.Message = message;
        }
    }

    public class VmAfdException : Exception
    {
        ErrorMessagePair[] errorMap = new ErrorMessagePair[]
        {
            new ErrorMessagePair(WinError.ERROR_ACCESS_DENIED,   "Access denied."),
            new ErrorMessagePair(WinError.ERROR_INVALID_PARAMETER,   "An invalid parameter is given."),
            new ErrorMessagePair(WinError.ERROR_NOT_FOUND,   "Not found."),
        };

        public override string Message
        {
            get
            {
                return GetMessageForError(this.ErrorCode);
            }
        }

        public VmAfdException(UInt32 errorCode)
        {
            ErrorCode = errorCode;
        }

        public UInt32 ErrorCode { get; protected set; }

        public string GetMessageForError(UInt32 error)
        {
            for (int i = 0; i < errorMap.Length; ++i)
            {
                if (error == (UInt32)errorMap[i].Error)
                {
                    return errorMap[i].Message;
                }
            }

            return "Unknown error.";
        }
    }
}
