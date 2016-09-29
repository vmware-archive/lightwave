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
using System.Security.Cryptography.X509Certificates;
using VMAfd.Client;

namespace VMCertStore.Client
{
    public enum VMCertType
    {
        All,
        Personal,
        Trusted,
        Revoked,
        CARoot
    }

    public class CertDTO
    {
        public X509Certificate2 Cert{ get; set; }

        public bool AutoRefresh{ get; set; }

        public string Alias{ get; set; }

        public string PrivateKey { get; set; }
    }

    public class VMCertStoreClient
    {
        public string ServerName { get; protected set; }

        public VMCertStoreClient (string serverName)
        {
            ServerName = serverName;
        }

        public void OpenStore ()
        {
            UInt32 storeHandle = 0;
            var result = VMCertStoreAdaptor.VmAfdOpenCertStore (ServerName, 0, out storeHandle);
            VMAfdClientError.Check (result);
        }

        public void CloseStore ()
        {
            UInt32 storeHandle = 0;
            var result = VMCertStoreAdaptor.VmAfdCloseCertStore (ServerName, storeHandle);
            VMAfdClientError.Check (result);
        }

        public void VerifyCertificateTrust (string certificate)
        {
        }

        public void GetCertificateChain (string certificate)
        {
        }

        public void GetCertificateByAlias (VMCertType type, string alias)
        {
        }
    }
}
