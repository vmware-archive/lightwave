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
using System.Runtime.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class EventLogDto : IDataContext
    {
        [DataMember(EmitDefaultValue = false)]
        string type;

        public string Type
        {
            get { return type; }
            set { type = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        string correlationId;

        public string CorrelationId
        {
            get { return correlationId; }
            set { correlationId = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        EventLevel level;

        public EventLevel Level
        {
            get { return level; }
            set { level = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        long start;

        public long Start
        {
            get { return start; }
            set { start = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        long elapsedMillis;        

        public long ElapsedMillis
        {
            get { return elapsedMillis; }
            set { elapsedMillis = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        Dictionary<string, object> metadata;

        public Dictionary<string,object> Metadata
        {
            get { return metadata; }
            set { metadata = value; }
        }

        string providerName;

        public string ProviderName
        {
            get {
                if (providerName == null)
                    providerName = GetProviderName();
                return providerName;
            }
            set
            {
                providerName = value;
            }
        }

        string accountName;
        public string AccountName
        {
            get {
                if (accountName == null)
                    accountName = GetAccountName();
                return accountName;
            }
            set
            {
                accountName = value;
            }
        }

        private string GetAccountName()
        {
            var username = string.Empty;
            object value;
            if (Metadata != null && Metadata.TryGetValue("username", out value) && value != null)
            {
                username = value.ToString();
            }
            return username;
        }
        private string GetProviderName()
        {
            var provider = string.Empty;
            object value;
            if (Metadata != null && Metadata.TryGetValue("provider", out value) && value != null)
            {
                var valueDict = value as Dictionary<string, object>;
                object providerValue;
                if (valueDict != null && valueDict.TryGetValue("type", out providerValue) && providerValue != null)
                {
                    provider = providerValue.ToString();
                }
            }
            return provider;
        }
    }

    public enum EventLevel
    {
        FATAL,
        ERROR,
        WARN,
        INFO,
        DEBUG,
        TRACE,
        ALL
    }
}