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
    public class TokenPolicyDto : IDataContext
    {
        public TokenPolicyDto() { }
        [DataMember]
        private long clockToleranceMillis;

        [DescriptionAttribute("Policy for clock tolerence in milliseconds")]
        public long ClockToleranceMillis
        {
            get { return clockToleranceMillis; }
            set { clockToleranceMillis = value; }
        }
        [DataMember]
        private int delegationCount;

        [DescriptionAttribute("Policy for delegation count")]
        public int DelegationCount
        {
            get { return delegationCount; }
            set { delegationCount = value; }
        }
        [DataMember]
        private long maxBearerTokenLifeTimeMillis;

        [DescriptionAttribute("Policy for lifetime of the bearer token in milliseconds")]
        public long MaxBearerTokenLifeTimeMillis
        {
            get { return maxBearerTokenLifeTimeMillis; }
            set { maxBearerTokenLifeTimeMillis = value; }
        }
        [DataMember]
        private long maxHOKTokenLifeTimeMillis;

        [DescriptionAttribute("Policy for lifetime of Holder token in milliseconds")]
        public long MaxHOKTokenLifeTimeMillis
        {
            get { return maxHOKTokenLifeTimeMillis; }
            set { maxHOKTokenLifeTimeMillis = value; }
        }
        [DataMember]
        private int renewCount;

        [DescriptionAttribute("Policy for renewal count")]
        public int RenewCount
        {
            get { return renewCount; }
            set { renewCount = value; }
        }
        [DataMember]
        private long maxBearerRefreshTokenLifeTimeMillis;

        public long MaxBearerRefreshTokenLifeTimeMillis
        {
            get { return maxBearerRefreshTokenLifeTimeMillis; }
            set { maxBearerRefreshTokenLifeTimeMillis = value; }
        }
        [DataMember]
        private long maxHOKRefreshTokenLifeTimeMillis;

        public long MaxHOKRefreshTokenLifeTimeMillis
        {
            get { return maxHOKRefreshTokenLifeTimeMillis; }
            set { maxHOKRefreshTokenLifeTimeMillis = value; }
        }
    }
}
