﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
    public class AuthErrorDto
    {
        [DataMember(EmitDefaultValue = false)]
        private string error;

        public string Error
        {
            get { return error; }
            set { error = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string details;

        public string Details
        {
            get { return details; }
            set { details = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string error_description;

        public string Description
        {
            get { return error_description; }
            set { error_description = value; }
        }
		[DataMember(EmitDefaultValue = false)]
		private string cause;

		public string Cause
		{
			get { return cause; }
			set { cause = value; }
		}
    }
}
