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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Security.Certificate
{
    public static class CertificateHelper
    {
        public static void ShowX509Certificate(string certString)
        {
            var bytes = Convert.FromBase64String(certString);
            var cert = new X509Certificate2(bytes);
            X509Certificate2UI.DisplayCertificate(cert);
        }
        public static string GetDisplayString(string certString)
        {
            var bytes = Convert.FromBase64String(certString);
            var cert = new X509Certificate2(bytes);
            if (cert != null)
            {
                return string.Format("Subject:{0}, Issuer:{1}, Expires:{2}",
                    cert.SubjectName.Name,
                    cert.IssuerName.Name,
                    cert.NotAfter.ToString("MM-dd-yyyy hh:mm:ss"));
            }
            return "Invalid certificate";
        }
        public static X509Certificate2 GetX509Certificate2FromString(string certString)
        {
            var bytes = Convert.FromBase64String(certString);
            return new X509Certificate2(bytes);
        }
        public static string PemToBase64EncodedString(string pem)
        {
            var pemStr = pem.Replace("-----BEGIN CERTIFICATE-----", string.Empty);
            pemStr = pemStr.Replace("-----END CERTIFICATE-----", string.Empty);
            pemStr = pemStr.Replace(Environment.NewLine, string.Empty);
            return pemStr;
        }
        public static string PemWithoutLineBreaks(string pem)
        {
            var sb = new StringBuilder();
            sb.AppendLine("-----BEGIN CERTIFICATE-----");
            var base64String = PemToBase64EncodedString(pem);
            sb.Append(base64String);
            sb.AppendLine("-----END CERTIFICATE-----");
            return sb.ToString();
        }
    }
}
