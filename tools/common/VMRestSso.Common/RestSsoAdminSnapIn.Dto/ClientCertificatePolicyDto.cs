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
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    public class ClientCertificatePolicyDto
    {
        [DataMember]
        private List<string> certPolicyOIDs;

        [DescriptionAttribute("Displays Certificate Policy OIDs")]
        public List<string> CertPolicyOIDs
        {
            get { return certPolicyOIDs; }
            set { certPolicyOIDs = value; }
        }
        [DataMember]
        private List<CertificateDto> trustedCACertificates;

        [DescriptionAttribute("Displays Trusted Certificate Autheority Certificates")]
        public List<CertificateDto> TrustedCACertificates
        {
            get { return trustedCACertificates; }
            set { trustedCACertificates = value; }
        }
        [DataMember]
        private bool revocationCheckEnabled;

        [DescriptionAttribute("Displays if the revocation check is enabled")]
        public bool RevocationCheckEnabled
        {
            get { return revocationCheckEnabled; }
            set { revocationCheckEnabled = value; }
        }
        [DataMember]
        private bool ocspEnabled;

        [DescriptionAttribute("Displays if the OCSP enabled")]
        public bool OcspEnabled
        {
            get { return ocspEnabled; }
            set { ocspEnabled = value; }
        }
        [DataMember]
        private bool failOverToCrlEnabled;

        [DescriptionAttribute("Displays if the failover to CRL is enabled")]
        public bool FailOverToCrlEnabled
        {
            get { return failOverToCrlEnabled; }
            set { failOverToCrlEnabled = value; }
        }
        [DataMember]
        private string ocspUrlOverride;

        [DescriptionAttribute("Displays OCSP URL override")]
        public string OcspUrlOverride
        {
            get { return ocspUrlOverride; }
            set { ocspUrlOverride = value; }
        }
        [DataMember]
        private bool crlDistributionPointUsageEnabled;

        [DescriptionAttribute("Displays if the CRL distribution policy usage is enabled")]
        public bool CrlDistributionPointUsageEnabled
        {
            get { return crlDistributionPointUsageEnabled; }
            set { crlDistributionPointUsageEnabled = value; }
        }
        [DataMember]
        private string crlDistributionPointOverride;

        [DescriptionAttribute("Displays CRL Distribution point override")]
        public string CrlDistributionPointOverride
        {
            get { return crlDistributionPointOverride; }
            set { crlDistributionPointOverride = value; }
        }
    }
}
