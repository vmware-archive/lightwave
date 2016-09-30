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

using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class SolutionUserDto : IDataContext
    {
        [DataMember]
        private string name;

        [DataMember]
        private string domain;

        [DataMember]
        private string description;

        [DataMember]
        private PrincipalDto alias;

        [DataMember]
        private bool disabled;

        [DataMember]
        private CertificateDto certificate;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        public string Domain
        {
            get { return domain; }
            set { domain = value; }
        }

        public PrincipalDto Alias
        {
            get { return alias; }
            set { alias = value; }
        }

        public bool Disabled
        {
            get { return disabled; }
            set { disabled = value; }
        }

        public string Description
        {
            get { return description; }
            set { description = value; }
        }

        public CertificateDto Certificate
        {
            get { return certificate; }
            set { certificate = value; }
        }

        public State State { get; set; }

        public SolutionUserDto()
        {
            State = State.UnChanged;
        }

		public SolutionUserDto DeepCopy()
        {
            return new SolutionUserDto()
            {
                Name = this.Name,
                Domain = this.Domain,
                Description = this.Description,
                Disabled = this.Disabled,
                Certificate = new CertificateDto { Encoded = this.Certificate.Encoded },
                Alias = new PrincipalDto
                {
                    Name = this.Name,
                    Domain = this.Domain
                }
            };
        }
    }
}