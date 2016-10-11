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
    public class PersonUserDto : IDataContext
    {
        [DataMember(EmitDefaultValue = false)]
        string firstName;

        [DataMember(EmitDefaultValue = false)]
        string lastName;

        [DataMember(EmitDefaultValue = false)]
        string email;

        [DataMember(EmitDefaultValue = false)]
        string description;

        [DataMember(EmitDefaultValue = false)]
        string upn;


        public string LastName
        {
            // Implement get and set.
            get { return lastName; }
            set { lastName = value; }

        }

        public string FirstName
        {
            // Implement get and set.
            get { return firstName; }
            set { firstName = value; }
        }

        public string EmailAddress
        {
            // Implement get and set.
            get { return email; }
            set { email = value; }

        }

        public string Description
        {
            // Implement get and set.
            get { return description; }
            set { description = value; }
        }

        public string UserPrincipalName
        {
            // Implement get and set.
            get { return upn; }
            set { upn = value; }

        }

        public PersonUserDto()
        {
        }
    }
}