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
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Cryptography.X509Certificates;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers
{
    public static class LdapSecureConnectionCertificateFetcher
    {
        private const string LdapSecureProtocol = "ldaps://";
        private const int LdapSecurePort = 636;
        private static bool _allowSsl;
        public static X509Certificate FetchServerCertificate(string connection)
        {
            X509Certificate certificate = null;
            if (connection == null) return null;

            _allowSsl = connection.StartsWith(LdapSecureProtocol);
            if (Uri.IsWellFormedUriString(connection, UriKind.Absolute) && _allowSsl)
            {
                var hostnameWithPort = connection.Replace(LdapSecureProtocol, string.Empty);
                var hostname = hostnameWithPort.Split(':')[0];
                var client = new TcpClient(hostname, LdapSecurePort);
                var stream = client.GetStream();
                var sslStream = new SslStream(stream, false, new RemoteCertificateValidationCallback(ValidateRemoteCertificate), null);
                sslStream.AuthenticateAsClient(hostname);
                certificate = sslStream.RemoteCertificate;
            }
            return certificate;
        }
        private static bool ValidateRemoteCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors policyErrors)
        {
            return _allowSsl ? _allowSsl : policyErrors == SslPolicyErrors.None;
        }
    }
}
