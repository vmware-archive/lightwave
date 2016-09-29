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
using System.Text;

namespace VMCA.Utilities
{
    public static class CertificateExtensions
    {
        public static string StripToBase64String (this string cert)
        {
            string START = "-----BEGIN CERTIFICATE-----";
            string END = "-----END CERTIFICATE-----";

            int begin = cert.IndexOf (START);
            if (begin == -1)
                return cert;
            begin = begin + START.Length;

            int end = cert.IndexOf (END);
            if (end == -1)
                return cert.Substring (begin);

            var result = cert.Substring (begin, end - begin);
            return result.Trim ();
        }

        public static string ExportToPem (this X509Certificate2 cert)
        {
            const int LINE_LENGTH = 65;

            byte[] certBytes = cert.Export (X509ContentType.Cert);
            var base64String = Convert.ToBase64String (
                                   certBytes,
                                   Base64FormattingOptions.None);
            var sb = new StringBuilder (base64String);
            for (
                int i = LINE_LENGTH - Environment.NewLine.Length;
                i < sb.Length;
                i += LINE_LENGTH) {
                sb.Insert (i, Environment.NewLine);
            }
            sb.Insert (0, "-----BEGIN CERTIFICATE-----" + Environment.NewLine);
            sb.Append (Environment.NewLine);
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
