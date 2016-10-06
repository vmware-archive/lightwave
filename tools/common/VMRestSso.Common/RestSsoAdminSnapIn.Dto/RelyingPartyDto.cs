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


using System.Collections.Generic;
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class RelyingPartyDto : IDataContext
    {
		public RelyingPartyDto()
		{
			SignatureAlgorithms = new List<SignatureAlgorithmDto> ();
			AssertionConsumerServices = new List<AssertionConsumerServiceDto> ();
			AttributeConsumerServices = new List<AttributeConsumerServiceDto> ();
			SingleLogoutServices = new List<ServiceEndpointDto> ();
		}

        [DataMember(EmitDefaultValue = false)]
        private string name;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string url;

        public string Url
        {
            get { return url; }
            set { url = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private List<SignatureAlgorithmDto> signatureAlgorithms;

        public List<SignatureAlgorithmDto> SignatureAlgorithms
        {
            get { return signatureAlgorithms; }
            set { signatureAlgorithms = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private List<AssertionConsumerServiceDto> assertionConsumerServices;

        public List<AssertionConsumerServiceDto> AssertionConsumerServices
        {
            get { return assertionConsumerServices; }
            set { assertionConsumerServices = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private List<AttributeConsumerServiceDto> attributeConsumerServices;

        public List<AttributeConsumerServiceDto> AttributeConsumerServices
        {
            get { return attributeConsumerServices; }
            set { attributeConsumerServices = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private List<ServiceEndpointDto> singleLogoutServices;

        public List<ServiceEndpointDto> SingleLogoutServices
        {
            get { return singleLogoutServices; }
            set { singleLogoutServices = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private CertificateDto certificate;

        public CertificateDto Certificate
        {
            get { return certificate; }
            set { certificate = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string defaultAssertionConsumerService;

        public string DefaultAssertionConsumerService
        {
            get { return defaultAssertionConsumerService; }
            set { defaultAssertionConsumerService = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string defaultAttributeConsumerService;

        public string DefaultAttributeConsumerService
        {
            get { return defaultAttributeConsumerService; }
            set { defaultAttributeConsumerService = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private bool authnRequestsSigned;

        public bool AuthnRequestsSigned
        {
            get { return authnRequestsSigned; }
            set { authnRequestsSigned = value; }
        }
		public RelyingPartyDto DeepCopy()
		{
			return new RelyingPartyDto {
				Name = this.Name,
				Url = this.Url,
				Certificate = new CertificateDto (){ Encoded = this.Certificate.Encoded },
				SignatureAlgorithms = this.SignatureAlgorithms == null ? new List<SignatureAlgorithmDto>() : new List<SignatureAlgorithmDto> (this.SignatureAlgorithms),
				SingleLogoutServices = this.SingleLogoutServices == null ? new List<ServiceEndpointDto>() : new List<ServiceEndpointDto> (this.SingleLogoutServices),
				AssertionConsumerServices = this.AssertionConsumerServices == null ? new List<AssertionConsumerServiceDto>() : new List<AssertionConsumerServiceDto> (this.AssertionConsumerServices),
				AttributeConsumerServices = this.AttributeConsumerServices == null ? new List<AttributeConsumerServiceDto>() : new List<AttributeConsumerServiceDto> (this.AttributeConsumerServices),
			};
		}
    }
}
