/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.rest.idm.data;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.data.DTO;
import com.vmware.identity.rest.idm.data.attributes.AuthenticationType;
import com.vmware.identity.rest.idm.data.attributes.DomainType;
import com.vmware.identity.rest.idm.data.attributes.IdentityProviderType;
import com.vmware.identity.rest.idm.data.attributes.ObjectClass;

/**
 * The {@code IdentityProviderDTO} class contains the details of an Identity Provider.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonDeserialize(builder=IdentityProviderDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class IdentityProviderDTO extends DTO {

    private final String domainType;
    private final String name;

    // The following params are required only for external identity source. i.e If DomainType is DomainType#External.
    private final String alias;
    private final String type;
    private final String authenticationType;
    private final String friendlyName;
    private final Long searchTimeOutInSeconds;
    private final String username;
    private final String password;
    private final Boolean machineAccount;
    private final String servicePrincipalName;
    private final String userBaseDN;
    private final String groupBaseDN;
    private final Collection<String> connectionStrings;
    private final Map<String, String> attributesMap;
    private final Set<String> upnSuffixes;
    private final Map<String, SchemaObjectMappingDTO> schema;
    private final Boolean matchingRuleInChainEnabled;
    private final Boolean baseDnForNestedGroupsEnabled;
    private final Boolean directGroupsSearchEnabled;
    private final Boolean siteAffinityEnabled;
    private final Boolean linkAccountWithUPN;
    private final String hintAttributeName;
    private final Collection<CertificateDTO> certificates;

    /**
     * Construct an {@code IdentityProviderDTO} with its various details.
     * <p>
     * Note: arguments other than {@code domainType} and {@code name} are only
     * necessary for external domains.
     *
     * @param domainType the type of domain (e.g. system, local, external).
     *  See {@link DomainType}.
     * @param name the name of the identity provider. This is synonymous with the domain name.
     * @param alias the alias of the identity provider.
     * @param type the type of identity provider (e.g. AD, LDAP, etc.).
     *  See {@link IdentityProviderType}.
     * @param authenticationType the type of authentication to use with the identity provider.
     *  See {@link AuthenticationType}.
     * @param friendlyName the friendly name for the identity provider.
     * @param searchTimeOutInSeconds the maximum number of seconds to use when searching in the
     *  identity provider.
     * @param username the username used to authenticate against the identity provider when
     *  the authentication type is {@link AuthenticationType#PASSWORD}.
     * @param password the password used to authenticate against the identity provider when
     *  the authentication type is {@link AuthenticationType#PASSWORD}.
     * @param machineAccount indicate whether the machine account should be used for authentication.
     * @param servicePrincipalName the service principal name of the identity provider.
     * @param userBaseDN the base distinguished name where users are found in the
     *  identity provider.
     * @param groupBaseDN the base distinguished name where groups are found in the
     *  identity provder.
     * @param connectionStrings the collection of connection strings for the external
     *  identity provider.
     * @param attributesMap a map of attributes to attribute names.
     * @param upnSuffixes a set of eligible suffixes for the UPNs of the identity provider.
     * @param schema a mapping of object identifiers to schema object mappings.
     *  See {@link ObjectClass}.
     * @param matchingRuleInChainEnabled enable or disable matching-rule-in-chain for nested
     *  group searches.
     * @param baseDnForNestedGroupsEnabled enable or disable using the base distinguished name
     *  for token resolution of nested groups.
     * @param directGroupsSearchEnabled enable or disable direct group searching.
     * @param siteAffinityEnabled enable or disable site affinity.
     * @param linkAccountWithUPN for smartcard login.
     * @param hintAttributeName  Used for smartcard login, the account attribute that match to username hint. Default is "samAccountName"
     * @param certificates a collection of trusted certificates for LDAPS with the identity provider.
     */
    protected IdentityProviderDTO(String domainType, String name, String alias, String type, String authenticationType, String friendlyName, Long searchTimeOutInSeconds, String username, String password,
                    Boolean machineAccount, String servicePrincipalName, String userBaseDN, String groupBaseDN, Collection<String> connectionStrings, Map<String, String> attributesMap, Set<String> upnSuffixes,
                    Map<String, SchemaObjectMappingDTO> schema, Boolean matchingRuleInChainEnabled, Boolean baseDnForNestedGroupsEnabled, Boolean directGroupsSearchEnabled, Boolean siteAffinityEnabled,
                    Boolean linkAccountWithUPN, String hintAttributeName, Collection<CertificateDTO> certificates) {
        this.domainType = domainType;
        this.name = name;
        this.alias = alias;
        this.type = type;
        this.authenticationType = authenticationType;
        this.friendlyName = friendlyName;
        this.searchTimeOutInSeconds = searchTimeOutInSeconds;
        this.username = username;
        this.password = password;
        this.machineAccount = machineAccount;
        this.servicePrincipalName = servicePrincipalName;
        this.userBaseDN = userBaseDN;
        this.groupBaseDN = groupBaseDN;
        this.connectionStrings = connectionStrings;
        this.attributesMap = attributesMap;
        this.upnSuffixes = upnSuffixes;
        this.schema = schema;
        this.matchingRuleInChainEnabled = matchingRuleInChainEnabled;
        this.baseDnForNestedGroupsEnabled = baseDnForNestedGroupsEnabled;
        this.directGroupsSearchEnabled = directGroupsSearchEnabled;
        this.siteAffinityEnabled = siteAffinityEnabled;
        this.certificates = certificates;
        this.linkAccountWithUPN = linkAccountWithUPN;
        this.hintAttributeName = hintAttributeName;
    }

    /**
     * Get the domain type.
     *
     * @return the domain type.
     */
    public String getDomainType() {
        return domainType;
    }

    /**
     * Get the name of the identity provider
     *
     * @return the name of the identity provider.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the alias of the identity provider.
     *
     * @return the alias of the identity provider.
     */
    public String getAlias() {
        return alias;
    }

    /**
     * Get the type of the identity provider.
     *
     * @return the type of the identity provider.
     */
    public String getType() {
        return type;
    }

    /**
     * Get the authentication type used by the identity provider.
     *
     * @return the authentication type used by the identity provider.
     */
    public String getAuthenticationType() {
        return authenticationType;
    }

    /**
     * Get the friendly name of the identity provider.
     *
     * @return the friendly name of the identity provider.
     */
    public String getFriendlyName() {
        return friendlyName;
    }

    /**
     * Get the maximum number of seconds used when searching in the identity provider.
     *
     * @return the search timeout of the identity provider in seconds.
     */
    public Long getSearchTimeOutInSeconds() {
        return searchTimeOutInSeconds;
    }

    /**
     * Get the username used for authentication when the {@code authenticationType}
     * is {@link AuthenticationType#PASSWORD}.
     *
     * @return the username used for authentication.
     */
    public String getUsername() {
        return username;
    }

    /**
     * Get the password used for authentication when the {@code authenticationType}
     * is {@link AuthenticationType#PASSWORD}.
     *
     * @return the password used for authentication.
     */
    public String getPassword() {
        return password;
    }

    /**
     * Check if the machine account should be used for authentication.
     *
     * @return {@code true} if the machine account should be used for authentication,
     *  {@code false} otherwise.
     */
    public Boolean isMachineAccount() {
        return machineAccount;
    }

    /**
     * Get the service principal name associated with the {@code username} used for authentication.
     *
     * @return the service principal name used for authentication.
     */
    public String getServicePrincipalName() {
        return servicePrincipalName;
    }

    /**
     * Get the base distinguished name (DN) used when searching for a user.
     *
     * @return the base DN used when searching for a user.
     */
    public String getUserBaseDN() {
        return userBaseDN;
    }

    /**
     * Get the base distinguished name (DN) used when searching for a group.
     *
     * @return the base DN used when searching for a group.
     */
    public String getGroupBaseDN() {
        return groupBaseDN;
    }

    /**
     * Get the list of connection strings that will be used when connecting to the
     * external identity provider.
     *
     * @return the list of connection strings for the external identity provider.
     */
    public Collection<String> getConnectionStrings() {
        return connectionStrings;
    }

    /**
     * Get the SAML attribute mapping used by the identity provider.
     *
     * @return the map of SAML attributes used by the identity provider.
     */
    public Map<String, String> getAttributesMap() {
        return attributesMap;
    }

    /**
     * Get the set of eligible UPN suffixes for the identity provider.
     *
     * @return the set of eligible UPN suffixes for the identity provider.
     */
    public Set<String> getUpnSuffixes() {
        return upnSuffixes;
    }

    /**
     * Check whether to use matching-rule-in-chain for nested group searches.
     *
     * @return {@code true} if matching-rule-in-chain is enabled, {@code false} otherwise.
     */
    public Boolean isMatchingRuleInChainEnabled() {
        return matchingRuleInChainEnabled;
    }

    /**
     * Check whether to utilize the group base distinguished name during token
     * nested group resolution.
     *
     * @return {@code true} if base DN is enabled, {@code false} otherwise.
     */
    public Boolean isBaseDnForNestedGroupsEnabled() {
        return baseDnForNestedGroupsEnabled;
    }

    /**
     * Check whether to resolve only direct parent groups.
     *
     * @return {@code true} if it should search only for direct parent groups,
     *  {@code false} otherwise.
     */
    public Boolean isDirectGroupsSearchEnabled() {
        return directGroupsSearchEnabled;
    }

    /**
     * Check whether site-affinity is enabled on the identity provider.
     *
     * @return {@code true} if site-affinity is enabled, {@code false} otherwise.
     */
    public Boolean isSiteAffinityEnabled() {
        return siteAffinityEnabled;
    }

    /**
     * Get the schema mapping object identifiers to object mappings.
     *
     * @return a mapping of object identifiers to object mappings.
     */
    public Map<String, SchemaObjectMappingDTO> getSchema() {
        return schema;
    }

    /**
     * Get the identity provider's trusted certificates used with LDAPS.
     *
     * @return a collection of trusted certificates.
     */
    public Collection<CertificateDTO> getCertificates() {
        return certificates;
    }

    /**
     * Creates an instance of the {@link IdentityProviderDTO.Builder} class.
     *
     * @return a new {@code IdentityProviderDTO.Builder}.
     */
    public static Builder builder() {
        return new IdentityProviderDTO.Builder();
    }

    public Boolean getLinkAccountWithUPN() {
        return linkAccountWithUPN;
    }

    public String getHintAttributeName() {
        return hintAttributeName;
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for
     * usage when constructing the object from its JSON string and thus only accepts
     * content for the canonical fields of the JSON representation. Other constructors
     * may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown=true)
    @JsonPOJOBuilder
    public static class Builder {
        private String domainType;
        private String name;
        private String alias;
        private String type;
        private String authenticationType;
        private String friendlyName;
        private Long searchTimeOutInSeconds;
        private String username;
        private String password;
        private Boolean machineAccount;
        private String servicePrincipalName;
        private String userBaseDN;
        private String groupBaseDN;
        private Collection<String> connectionStrings;
        private Map<String, String> attributesMap;
        private Set<String> upnSuffixes;
        private Map<String, SchemaObjectMappingDTO> schema;
        private Boolean matchingRuleInChainEnabled;
        private Boolean baseDnForNestedGroupsEnabled;
        private Boolean directGroupsSearchEnabled;
        private Boolean siteAffinityEnabled;
        private Boolean linkAccountWithUPN = true;
        private String  hintAttributeName;

        private Collection<CertificateDTO> certificates;

        public Builder withDomainType(String domainType) {
            this.domainType = domainType;
            return this;
        }

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withAlias(String alias) {
            this.alias = alias;
            return this;
        }

        public Builder withType(String identityStoreType) {
            this.type = identityStoreType;
            return this;
        }

        public Builder withAuthenticationType(String authenticationType) {
            this.authenticationType = authenticationType;
            return this;
        }

        public Builder withFriendlyName(String friendlyName) {
            this.friendlyName = friendlyName;
            return this;
        }

        public Builder withSearchTimeOutInSeconds(Long searchTimeOutInSeconds) {
            this.searchTimeOutInSeconds = searchTimeOutInSeconds;
            return this;
        }

        public Builder withUsername(String username) {
            this.username = username;
            return this;
        }

        public Builder withPassword(String password) {
            this.password = password;
            return this;
        }

        public Builder withMachineAccount(Boolean machineAccount) {
            this.machineAccount = machineAccount;
            return this;
        }

        public Builder withServicePrincipalName(String servicePrincipalName) {
            this.servicePrincipalName = servicePrincipalName;
            return this;
        }

        public Builder withUserBaseDN(String userBaseDN) {
            this.userBaseDN = userBaseDN;
            return this;
        }

        public Builder withGroupBaseDN(String groupBaseDN) {
            this.groupBaseDN = groupBaseDN;
            return this;
        }

        public Builder withConnectionStrings(Collection<String> connectionStrings) {
            this.connectionStrings = connectionStrings;
            return this;
        }

        public Builder withAttributesMap(Map<String, String> attributesMap) {
            this.attributesMap = attributesMap;
            return this;
        }

        public Builder withUPNSuffixes(Set<String> upnSuffixes) {
            this.upnSuffixes = upnSuffixes;
            return this;
        }

        public Builder withSchema(Map<String, SchemaObjectMappingDTO> schema) {
            this.schema = schema;
            return this;
        }

        public Builder withMatchingRuleInChainEnabled(Boolean matchingRuleInChainEnabled) {
            this.matchingRuleInChainEnabled = matchingRuleInChainEnabled;
            return this;
        }

        public Builder withBaseDnForNestedGroupsEnabled(Boolean baseDnForNestedGroupsEnabled) {
            this.baseDnForNestedGroupsEnabled = baseDnForNestedGroupsEnabled;
            return this;
        }

        public Builder withDirectGroupsSearchEnabled(Boolean directGroupsSearchEnabled) {
            this.directGroupsSearchEnabled = directGroupsSearchEnabled;
            return this;
        }

        public Builder withCertificates(Collection<CertificateDTO> certificates) {
            this.certificates = certificates;
            return this;
        }

        public Builder withSiteAffinityEnabled(Boolean siteAffinityEnabled) {
            this.siteAffinityEnabled = siteAffinityEnabled;
            return this;
        }

        public Builder withLinkAccountWithUPN(Boolean linkAccountWithUPN) {
            this.linkAccountWithUPN = linkAccountWithUPN;
            return this;
        }

        public Builder withHintAttributeName(String hintAttrName) {
            this.hintAttributeName = hintAttrName;
            return this;
        }

        public IdentityProviderDTO build() {
             return new IdentityProviderDTO(domainType,
                                            name,
                                            alias,
                                            type,
                                            authenticationType,
                                            friendlyName,
                                            searchTimeOutInSeconds,
                                            username,
                                            password,
                                            machineAccount,
                                            servicePrincipalName,
                                            userBaseDN,
                                            groupBaseDN,
                                            connectionStrings,
                                            attributesMap,
                                            upnSuffixes,
                                            schema,
                                            matchingRuleInChainEnabled,
                                            baseDnForNestedGroupsEnabled,
                                            directGroupsSearchEnabled,
                                            siteAffinityEnabled,
                                            linkAccountWithUPN,
                                            hintAttributeName,
                                            certificates);
        }
    }

}
