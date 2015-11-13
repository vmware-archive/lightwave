/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Text;

namespace VmIdentity.CommonUtils.Utilities
{
    public static class CertificateExtensions
    {
        public static string ExportToPem (this X509Certificate2 cert)
        {
            var sb = new StringBuilder ();
            sb.AppendLine ("-----BEGIN CERTIFICATE-----");

            byte[] certBytes = cert.Export (X509ContentType.Cert);
            sb.Append (Convert.ToBase64String (certBytes));
            sb.AppendLine ("-----END CERTIFICATE-----");

            return sb.ToString ();
        }

        public static string GetKeyUsage (this X509Certificate2 cert)
        {
            string result = "";
            foreach (var ext in cert.Extensions) {
                if (ext is X509KeyUsageExtension) {
                    var keyUsageExtension = ext as X509KeyUsageExtension;
                    result = keyUsageExtension.KeyUsages.ToString ();

                    break;
                }
            }
            return result;
        }

        public static X509Certificate2 GetX509Certificate2FromString (this string certString)
        {
            var bytes = Convert.FromBase64String (certString);
            return new X509Certificate2 (bytes);
        }
    }
}
