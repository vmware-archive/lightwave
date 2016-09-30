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
using System.Security.Cryptography.X509Certificates;
using VMAfd;
using VMCertStore.Client;

namespace Vecs
{
    public enum CertStoreType
    {
        All,
        Private,
        Trusted,
        Revoked,
        TrustedRoots,
        Password
    }

    public class CertDTO
    {
        public X509Certificate2 Cert{ get; set; }

        public bool AutoRefresh{ get; set; }

        public string Alias{ get; set; }

        public string PrivateKey { get; set; }

        public string Password { get; set; }
    }

    public class VecsClient
    {
        IntPtr _serverContext = IntPtr.Zero;

        public string ServerName { get; protected set; }

        public string UserName { get; set; }

        public string Password { get; set; }

        public string DomainName { get; set; }

        public IntPtr ServerContext {
            get {
                if (_serverContext == IntPtr.Zero) {
                    EnsureServerContext ();
                }
                return _serverContext;
            }
        }

        public bool NeedAuth {
            get {
                return string.IsNullOrEmpty (UserName) ||
                string.IsNullOrEmpty (Password);
            }
        }

        public VecsClient (string serverName, string userName, string password)
        {
            ServerName = serverName;
            UserName = userName;
            Password = password;
        }

        void EnsureServerContext ()
        {
            try {
                if (_serverContext != IntPtr.Zero)
                    return;

                UInt32 result = VMAfdAdaptor.VmAfdOpenServerA (
                                    ServerName,
                                    UserName,
                                    Password,
                                    out _serverContext);
                VecsError.Check (result);
            } catch (Exception e) {
                UserName = "";
                Password = "";
                throw e;
            }
        }

        public void CloseServer()
        {
             ErrorHelper.CatchAndThrow (delegate() {
                 if (_serverContext != IntPtr.Zero)
                     VMAfdAdaptor.VmAfdCloseServer(_serverContext);
             });
        }

        public void RefreshServerContext (string user, string pass)
        {
            ErrorHelper.CatchAndThrow (delegate() {
                _serverContext = IntPtr.Zero;
                UserName = user;
                Password = pass;

                EnsureServerContext ();
            });
        }

        public string[] GetStores ()
        {
            try {
                string[] storeNamesOut = null;
                UInt32 numberOfStores = 0;
                IntPtr storeNames = IntPtr.Zero;

                var result = VecsAdaptor.VecsEnumCertStoreHA (ServerContext, out storeNames, out numberOfStores);
                VecsError.Check (result);

                MarshalArrayOfStrings (storeNames, (int)numberOfStores, out storeNamesOut);

                return storeNamesOut;
            } catch (Exception e) {
                throw e;
            }
        }

        public IntPtr CreateStore (string serverName, string password)
        {
            try {
                IntPtr storeHandle = IntPtr.Zero;
                var result = VecsAdaptor.VecsCreateCertStoreHA (ServerContext, serverName, password, out storeHandle);
                VecsError.Check (result);
                return storeHandle;
            } catch (Exception e) {
                throw e;
            }
        }

        public void DeleteStore (string storeName)
        {
            ErrorHelper.CatchAndThrow (delegate() {
                var result = VecsAdaptor.VecsDeleteCertStoreHA (ServerContext, storeName);
                VecsError.Check (result);
            });
        }

        static void MarshalArrayOfStrings (IntPtr pIntPtr, int count, out string[] stringsOut)
        {
            IntPtr[] pIntPtrArray = new IntPtr[count];
            stringsOut = new string[count];

            Marshal.Copy (pIntPtr, pIntPtrArray, 0, count);

            for (int i = 0; i < count; i++) {
                stringsOut [i] = Marshal.PtrToStringAnsi (pIntPtrArray [i]);
            }
        }
    }
}
