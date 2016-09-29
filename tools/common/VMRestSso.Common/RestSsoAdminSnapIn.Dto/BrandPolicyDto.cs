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
    public class BrandPolicyDto
    {
        [DataMember]
        private string name;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        [DataMember]
        private string logonBannerTitle;

        public string LogonBannerTitle
        {
            get { return logonBannerTitle; }
            set { logonBannerTitle = value; }
        }

        [DataMember]
        private string logonBannerContent;

        public string LogonBannerContent
        {
            get { return logonBannerContent; }
            set { logonBannerContent = value; }
        }

        [DataMember]
        private bool logonBannerCheckboxEnabled;

        public bool LogonBannerCheckboxEnabled
        {
            get { return logonBannerCheckboxEnabled; }
            set { logonBannerCheckboxEnabled = value; }
        }

        [DataMember]
        private bool logonBannerDisabled;

        public bool LogonBannerDisabled
        {
            get { return logonBannerDisabled; }
            set { logonBannerDisabled = value; }
        }
    }
}
