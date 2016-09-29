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
using VMCA.Client;
using VMCA.Utilities;

namespace VMCA
{
    public class VMCARequest : IDisposable
    {
        public string Name{ get; set; }

        public string Country{ get; set; }

        public string Locality{ get; set; }

        public string State{ get; set; }

        public string Organization{ get; set; }

        public string OU{ get; set; }

        public string DNSName{ get; set; }

        public string URIName{ get; set; }

        public string Email{ get; set; }

        public string IPAddress{ get; set; }

        public uint KeyUsageConstraints{ get; set; }

        public string DomainName { get; set; }

        public VMCAClient Client{ get; protected set; }

        public VMCARequest(VMCAClient client)
        {
            Client = Client;
            DomainName = client.DomainName;
        }

        #region IDisposable Members

        public void Dispose()
        {
        }

        #endregion

        public VMCACertificate GetSelfSignedCertificate(string privateKey, DateTime notBefore, DateTime notAfter)
        {
            IntPtr pCert = new IntPtr();

            var requestData = GetRequestData();
            long nBefore = notBefore.ToTime_t();
            long nAfter = notAfter.ToTime_t();
            UInt32 result = VMCAAdaptor.VMCACreateSelfSignedCertificate(requestData, privateKey, "", nBefore, nAfter, out pCert);
            VMCAError.Check(result);

            string cert = Marshal.PtrToStringAnsi(pCert);
            VMCAAdaptor.VMCAFreeCertificate(pCert);

            return new VMCACertificate(Client, cert);
        }

        public VMCAAdaptor.VMCA_PKCS_10_REQ_DATA GetRequestData()
        {
            var req = new VMCAAdaptor.VMCA_PKCS_10_REQ_DATA();
            req.pszName = Name;
            req.pszDomainName = DomainName;
            req.pszCountry = Country;
            req.pszLocality = Locality;
            req.pszState = State;
            req.pszOrganization = Organization;
            req.pszOU = OU;
            req.pszDNSName = DNSName;
            req.pszURIName = URIName;
            req.pszEmail = Email;
            req.pszIPAddress = IPAddress;
            req.dwKeyUsageConstraints = KeyUsageConstraints;

            return req;
        }

        public string GetCSR(string privateKey)
        {
            IntPtr pCSR = new IntPtr();

            var requestData = GetRequestData();
            UInt32 result = VMCAAdaptor.VMCACreateSigningRequest(requestData, privateKey, "", out pCSR);
            VMCAError.Check(result);

            string csr = Marshal.PtrToStringAnsi(pCSR);
            VMCAAdaptor.VMCAFreeCSR(pCSR);
            return csr;
        }
    }
}
