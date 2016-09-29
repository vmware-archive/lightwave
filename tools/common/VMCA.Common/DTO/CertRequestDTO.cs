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
using System.ComponentModel;
using VMCA;

namespace VMCASnapIn.DTO
{
    public interface IPrivateKeyProvider
    {
        PrivateKeyDTO PrivateKey { get; set; }
    }

    [TypeConverter (typeof(ExpandableObjectConverter))]
    public class CertRequestDTO : IPrivateKeyProvider
    {
        //[Category ("General")]
        public string Name { get; set; }

        //[Category ("General")]
        public string Country { get; set; }

        //[Category ("General")]
        public string Locality { get; set; }

        //[Category ("General")]
        public string State { get; set; }

        //[Category ("General")]
        public string Organization { get; set; }

        //[Category ("General")]
        public string OU { get; set; }

        //[Category ("General")]
        public string DNSName { get; set; }

        //[Category ("General")]
        public string URIName { get; set; }

        //[Category ("General")]
        public string Email { get; set; }

        //[Category ("General")]
        public string IPAddress { get; set; }

        //[Category ("General")]
        public UInt32 KeyUsageConstraints { get; set; }

        //[Category ("Security")]
        [ReadOnly (true)]
        public PrivateKeyDTO PrivateKey { get; set; }

        //[Category ("Security")]
        [Description("Time in GMT")]
        public DateTime NotBefore { get; set; }

        //[Category ("Security")]
        [Description ("Time in GMT")]
        public DateTime NotAfter { get; set; }

        public CertRequestDTO ()
        {
            Country = "US";
            NotBefore = DateTime.UtcNow;
            NotAfter = NotBefore.AddYears (10);
            PrivateKey = new PrivateKeyDTO ();
        }

        public VMCAAdaptor.VMCA_PKCS_10_REQ_DATA GetRequestData ()
        {
            var req = new VMCAAdaptor.VMCA_PKCS_10_REQ_DATA ();
            req.pszName = Name;
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

        public void FillRequest (VMCARequest request)
        {
            request.Country = Country;
            request.DNSName = DNSName;
            request.Email = Email;
            request.IPAddress = IPAddress;
            request.KeyUsageConstraints = KeyUsageConstraints;
            request.Locality = Locality;
            request.Name = Name;
            request.Organization = Organization;
            request.OU = OU;
            request.State = State;
            request.URIName = URIName;
        }
    }
}
