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
using VMCA.Utilities;

namespace VMCA.Client
{
    public class VMCACertificate
    {
        VMCAClient _client;
        string _certificate;

        public VMCACertificate(VMCAClient client, string certificate)
        {
            _client = client;
            _certificate = certificate;
        }

        public VMCACertificate(VMCAClient client, X509Certificate2 certificate)
        {
            _client = client;
            _certificate = certificate.ExportToPem();
        }

        public void Revoke()
        {
            UInt32 result = VMCAAdaptor.VMCARevokeCertificate(_client.ServerName, _certificate);
            VMCAError.Check(result);
        }

        public X509Certificate2 GetX509Certificate2()
        {
            return VMCACertificate.GetX509Certificate(_certificate);
        }

        public static X509Certificate2 GetX509Certificate(string certificate)
        {
            var bytes = Convert.FromBase64String(certificate.StripToBase64String());
            return new X509Certificate2(bytes);
        }

        public static bool Validate(string certificate)
        {
            UInt32 result = VMCAAdaptor.VMCAValidateCACertificate(certificate);
            VMCAError.Check(result);
            return true;
        }

        public static string GetCertificateAsString(X509Certificate2 cert)
        {
            var certStringPtr = new IntPtr();
            UInt32 result = VMCAAdaptor.VMCAGetCertificateAsString(cert.ExportToPem(), out certStringPtr);
            VMCAError.Check(result);

            var certString = Marshal.PtrToStringAnsi(certStringPtr);
            VMCAAdaptor.VMCAFreeString(certStringPtr);

            return certString;
        }
    }
}
