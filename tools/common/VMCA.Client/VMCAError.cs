/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace VMCA.Client
{
    public enum VMCAErrorCode
    {
        RootCAMissing = 70000,
        RootCAAlreadyExists,
        InvalidTimeSpecified,
        ArgumentError,
        ErrorTimeOut,
        OutMemoryError,
        RequestError,
        KeyCreationFailure,
        CertDecodeFailure,
        KeyIOFailure,
        CertIOFailure,
        NotCACert,
        InvalidCSRField,
        SelfSignatureFailed,
        InitCAFailed,
        InvalidKeyLength,
        PKCS12CreateFailed,
        PKCS12IOFailed,
        Unknown
    }

    public static class VMCAError
    {
        public static void Check (UInt32 resultCode)
        {
            if (resultCode == VMCAAdaptor.VMCA_SUCCESS || resultCode == VMCAAdaptor.VMCA_ENUM_END)
                return;

            switch ((VMCAErrorCode)resultCode)
            {
                case VMCAErrorCode.KeyIOFailure:
                    throw new VMCAException(resultCode, "Private key is not valid");
                case VMCAErrorCode.CertIOFailure:
                    throw new VMCAException(resultCode, "Certificate is not valid");
                case VMCAErrorCode.NotCACert:
                    throw new VMCAException(resultCode, "Certificate is not a valid CA Certificate");
            }

            var errorStringPtr = new IntPtr ();
            UInt32 dwError = VMCAAdaptor.VMCAGetErrorString (
                                 resultCode, out errorStringPtr);
            string errorString = string.Empty;
            if (dwError == 0)
                errorString = Marshal.PtrToStringAnsi (errorStringPtr);
            else
                errorString = "Unknown Error";
            throw new VMCAException (resultCode, errorString);
        }
    }
}
