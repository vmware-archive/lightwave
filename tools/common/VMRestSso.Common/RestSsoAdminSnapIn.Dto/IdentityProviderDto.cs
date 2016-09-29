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
    public class IdentityProviderDto : IDataContext
    {
        [DataMember]
        private string domainType;

        public string DomainType
        {
            get { return domainType; }
            set { domainType = value; }
        }

        [DataMember]
        private string name;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        [DataMember]
        private string alias;

        public string Alias
        {
            get { return alias; }
            set { alias = value; }
        }

        [DataMember]
        private string type;

        public string Type
        {
            get { return type; }
            set { type = value; }
        }

        [DataMember]
        private string authenticationType;

        public string AuthenticationType
        {
            get { return authenticationType; }
            set { authenticationType = value; }
        }

        [DataMember]
        private string friendlyName;

        public string FriendlyName
        {
            get { return friendlyName; }
            set { friendlyName = value; }
        }

        [DataMember]
        private long searchTimeOutInSeconds;

        public long SearchTimeOutInSeconds
        {
            get { return searchTimeOutInSeconds; }
            set { searchTimeOutInSeconds = value; }
        }

        [DataMember]
        private string username;

        public string Username
        {
            get { return username; }
            set { username = value; }
        }

        [DataMember]
        private string password;

        public string Password
        {
            get { return password; }
            set { password = value; }
        }

        [DataMember]
        private bool machineAccount;

        public bool UserMachineAccount
        {
            get { return machineAccount; }
            set { machineAccount = value; }
        }

        [DataMember]
        private string servicePrincipalName;

        public string ServicePrincipalName
        {
            get { return servicePrincipalName; }
            set { servicePrincipalName = value; }
        }

        [DataMember]
        private string userBaseDN;

        public string UserBaseDN
        {
            get { return userBaseDN; }
            set { userBaseDN = value; }
        }

        [DataMember]
        private string groupBaseDN;

        public string GroupBaseDN
        {
            get { return groupBaseDN; }
            set { groupBaseDN = value; }
        }

        [DataMember]
        private IList<string> connectionStrings;

        public IList<string> ConnectionStrings
        {
            get { return connectionStrings; }
            set { connectionStrings = value; }
        }

        [DataMember]
        private Dictionary<string, string> attributesMap;

        public Dictionary<string, string> AttributesMap
        {
            get { return attributesMap; }
            set { attributesMap = value; }
        }

        [DataMember]
        private Dictionary<string, SchemaObjectMappingDto> schema;

        public Dictionary<string, SchemaObjectMappingDto> Schema
        {
            get { return schema; }
            set { schema = value; }
        }

        [DataMember]
        private bool matchingRuleInChainEnabled;
        public bool MatchingRuleInChainEnabled
        {
            get { return matchingRuleInChainEnabled; }
            set { matchingRuleInChainEnabled = value; }
        }
        [DataMember]
        private bool baseDnForNestedGroupsEnabled;
        public bool BaseDnForNestedGroupsEnabled
        {
            get { return baseDnForNestedGroupsEnabled; }
            set { baseDnForNestedGroupsEnabled = value; }
        }
        [DataMember]
        private bool directGroupsSearchEnabled;
        public bool DirectGroupsSearchEnabled
        {
            get { return directGroupsSearchEnabled; }
            set { directGroupsSearchEnabled = value; }
        }
        [DataMember]
        private bool siteAffinityEnabled;
        public bool SiteAffinityEnabled
        {
            get { return siteAffinityEnabled; }
            set { siteAffinityEnabled = value; }
        }
        [DataMember]
        private List<CertificateDto> certificates;
        public List<CertificateDto> Certificates
        {
            get { return certificates; }
            set { certificates = value; }
        }
        public IdentityProviderDto DeepCopy()
        {
            return new IdentityProviderDto
            {
                DomainType = this.DomainType,
                Name = this.Name,
                Alias = this.Alias,
                Type = this.Type,
                AuthenticationType = this.AuthenticationType,
                FriendlyName = this.FriendlyName,
                SearchTimeOutInSeconds = this.SearchTimeOutInSeconds,
                Username = this.Username,
                Password = this.Password,
                UserMachineAccount = this.UserMachineAccount,
                UserBaseDN = this.UserBaseDN,
                GroupBaseDN = this.GroupBaseDN,
                ConnectionStrings = this.ConnectionStrings,
                AttributesMap = this.AttributesMap,
                Schema = this.Schema,
                ServicePrincipalName = this.ServicePrincipalName,
                SiteAffinityEnabled = this.SiteAffinityEnabled,
                BaseDnForNestedGroupsEnabled = this.BaseDnForNestedGroupsEnabled,
                DirectGroupsSearchEnabled = this.DirectGroupsSearchEnabled,
                MatchingRuleInChainEnabled = this.MatchingRuleInChainEnabled,
                Certificates = Certificates == null ? null : new List<CertificateDto>(Certificates)
            };
        }

        public bool IsSameAs(IdentityProviderDto _provider)
        {
            return DomainType == this.DomainType &&
               Name == this.Name &&
               Alias == this.Alias &&
               Type == this.Type &&
               AuthenticationType == this.AuthenticationType &&
               FriendlyName == this.FriendlyName &&
               SearchTimeOutInSeconds == this.SearchTimeOutInSeconds &&
               Username == this.Username &&
               Password == this.Password &&
               UserMachineAccount == this.UserMachineAccount &&
               UserBaseDN == this.UserBaseDN &&
               GroupBaseDN == this.GroupBaseDN &&
               ConnectionStrings[0] == this.ConnectionStrings[0] &&
               IsSame(AttributesMap, this.AttributesMap) &&
               IsSame(Schema, this.Schema) &&
               ServicePrincipalName == this.ServicePrincipalName &&
               SiteAffinityEnabled == this.SiteAffinityEnabled &&
               BaseDnForNestedGroupsEnabled == this.BaseDnForNestedGroupsEnabled &&
               DirectGroupsSearchEnabled == this.DirectGroupsSearchEnabled &&
               MatchingRuleInChainEnabled == this.MatchingRuleInChainEnabled &&
               IsSame(Certificates, this.Certificates);
        }

        private bool IsSame(Dictionary<string, string> left, Dictionary<string, string> right)
        {
            if (left == null && right == null) return true;
            if ((left == null && right != null) || (left != null && right == null)) return false;
            if (left.Count == right.Count)
            {
                foreach (var leftItem in left)
                {
                    var found = (right.ContainsKey(leftItem.Key) && right[leftItem.Key] == leftItem.Value);
                    if (!found)
                        return false;
                }
                return true;
            }
            return false;
        }
        private bool IsSame(Dictionary<string, SchemaObjectMappingDto> left, Dictionary<string, SchemaObjectMappingDto> right)
        {
            if (left == null && right == null) return true;
            if ((left == null && right != null) || (left != null && right == null)) return false;
            if (left.Count == right.Count)
            {
                foreach (var leftItem in left)
                {
                    var found = (right.ContainsKey(leftItem.Key) && right[leftItem.Key] == leftItem.Value);
                    if (!found)
                        return false;
                    else
                    {
                        var leftSchemaItem = leftItem.Value;
                        SchemaObjectMappingDto rightSchemaItem = right[leftItem.Key];
                        if (leftSchemaItem.ObjectClass == rightSchemaItem.ObjectClass)
                        {
                            foreach (var attributue in leftSchemaItem.AttributeMappings)
                            {
                                found = (rightSchemaItem.AttributeMappings.ContainsKey(attributue.Key) && rightSchemaItem.AttributeMappings[attributue.Key] == attributue.Value);
                                if (!found)
                                    return false;
                            }
                        }
                        else
                            return false;

                    }
                }
                return true;
            }
            return false;
        }

        private bool IsSame(List<CertificateDto> left, List<CertificateDto> right)
        {
            if (left == null && right == null) return true;
            if ((left == null && right != null) || (left != null && right == null)) return false;
            if (left.Count == right.Count)
            {
                foreach (var leftItem in left)
                {
                    var found = (right.Exists(x => x.Encoded == leftItem.Encoded));
                    if (!found)
                        return false;
                }
            }
            else
                return false;
            return true;
        }
    }
}