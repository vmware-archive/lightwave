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
    /// <summary>
    /// User details serializable service entity
    /// </summary>
    [DataContract]
    public class UserDto : IDataContext
    {
        /// <summary>
        /// Name of the use
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string name;

        /// <summary>
        /// Domain for the user
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string domain { get; set; }

        /// <summary>
        /// Alias name for the user
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        PrincipalDto alias { get; set; }

        /// <summary>
        /// Person details for the user
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        private PersonUserDto details = new PersonUserDto();

        /// <summary>
        /// Is the user account disabled
        /// </summary>
        [DataMember]
        bool disabled { get; set; }

        /// <summary>
        /// Is the user account locked?
        /// </summary>
        [DataMember]
        bool locked { get; set; }

        /// <summary>
        /// Password for the user
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        PasswordDetailsDto passwordDetails;

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
        public PersonUserDto PersonDetails
        {
            get { return details; }
            set { details = value; }
        }

        public bool Disabled
        {
            get { return disabled; }
            set { disabled = value; }
        }
        public bool Locked
        {
            get { return locked; }
            set { locked = value; }
        }
        public PasswordDetailsDto PasswordDetails
        {
            get { return passwordDetails; }
            set { passwordDetails = value; }
        }
        public State State { get; set; }
        public bool ActAsUsers { get; set; }
        public bool IsIdpAdmin { get; set; }
        public UserRole Role { get; set; }

        /// <summary>
        /// Ctor
        /// </summary>
        public UserDto()
        {
            State = State.UnChanged;
        }

        public bool IsDifferentThan(UserDto other)
        {
            return (this.Name != other.Name || this.PasswordDetails != other.PasswordDetails || this.Locked != other.Locked || this.Disabled != other.Disabled
                || this.Domain != other.Domain || this.PersonDetails.Description != other.PersonDetails.Description ||
                this.PersonDetails.EmailAddress != other.PersonDetails.EmailAddress ||
                this.PersonDetails.FirstName != other.PersonDetails.FirstName ||
                this.PersonDetails.LastName != other.PersonDetails.LastName ||
                this.PersonDetails.UserPrincipalName != other.PersonDetails.UserPrincipalName ||
                this.Role != other.Role ||
                this.IsIdpAdmin != other.IsIdpAdmin ||
                this.ActAsUsers != other.ActAsUsers);
        }

        public UserDto DeepCopy()
        {
            return new UserDto()
            {
                Name = this.Name,
                Domain = this.Domain,
                PasswordDetails = this.PasswordDetails,
                Locked = this.Locked,
                Disabled = this.Disabled,
                Role = this.Role,
                IsIdpAdmin = this.IsIdpAdmin,
                ActAsUsers = this.ActAsUsers,
                PersonDetails = new PersonUserDto
                                {
                                    FirstName = this.PersonDetails.FirstName,
                                    LastName = this.PersonDetails.LastName,
                                    Description = this.PersonDetails.Description,
                                    EmailAddress = this.PersonDetails.EmailAddress,
                                    UserPrincipalName = this.PersonDetails.UserPrincipalName
                                }
            };
        }
    }
}