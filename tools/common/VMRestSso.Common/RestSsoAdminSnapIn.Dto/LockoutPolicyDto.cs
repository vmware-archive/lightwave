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
using System.Runtime.Serialization;
namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    public class LockoutPolicyDto : IDataContext
    {
        public LockoutPolicyDto() { }
        [DataMember]
        private string description;

        [DescriptionAttribute("Lockout policy description")]
        public string Description
        {
            get { return description; }
            set { description = value; }
        }
        [DataMember]
        private long failedAttemptIntervalSec;

        [DescriptionAttribute("Policy for failed attempt interval in seconds.")]
        public long FailedAttemptIntervalSec
        {
            get { return failedAttemptIntervalSec; }
            set { failedAttemptIntervalSec = value; }
        }
        [DataMember]
        private int maxFailedAttempts;

        [DescriptionAttribute("Policy for maximum attempts failed")]
        public int MaxFailedAttempts
        {
            get { return maxFailedAttempts; }
            set { maxFailedAttempts = value; }
        }
        [DataMember]
        private long autoUnlockIntervalSec;

        [DescriptionAttribute("Policy to auto-unlock in seconds")]
        public long AutoUnlockIntervalSec
        {
            get { return autoUnlockIntervalSec; }
            set { autoUnlockIntervalSec = value; }
        }
    }
}
