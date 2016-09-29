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
using System.Runtime.InteropServices;

namespace Vecs
{
    public static class VecsError
    {
        public static void Check (UInt32 resultCode)
        {
            if (resultCode == VecsAdaptor.ERROR_NO_MORE_ITEMS)
                return;
            if (resultCode == VecsAdaptor.SUCCESS)
                return;
            if (resultCode == 183)
                throw new VecsException(resultCode, "Entry already exist!");

            string errorString = "Unknown Error";
            var errorStringPtr = new IntPtr ();
            UInt32 dwError = VecsAdaptor.VmAfdGetErrorMsgByCode (resultCode, out errorStringPtr);
            if (dwError == 0)
                errorString = Marshal.PtrToStringAnsi (errorStringPtr);
            throw new VecsException (resultCode, errorString);
        }
    }
}
