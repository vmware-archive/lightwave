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

namespace VMCA.Client
{
    public class KeyPairData
    {
        public string PublicKey{ get; protected set; }

        public string PrivateKey{ get; protected set; }

        public KeyPairData (string publicKey, string privateKey)
        {
            PublicKey = publicKey;
            PrivateKey = privateKey;
        }
    }

    public class VMCAKeyPair
    {
        public static KeyPairData Create (UInt32 length)
        {
            IntPtr pk = new IntPtr ();
            IntPtr pbk = new IntPtr ();
            try {
                UInt32 result = VMCAAdaptor.VMCACreatePrivateKey (null, length, out pk, out pbk);
                VMCAError.Check (result);

                string pkString = Marshal.PtrToStringAnsi (pk);
                string pbkString = Marshal.PtrToStringAnsi (pbk);
                return new KeyPairData (pbkString, pkString);
            } finally {
                VMCAAdaptor.VMCAFreeString (pk);
                VMCAAdaptor.VMCAFreeString (pbk);
            }
        }
    }
}
