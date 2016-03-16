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

package com.vmware.vim.sso.admin;

import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * This class is used to define an IDP configuration. The elements of this class
 * are a similar to those defined in the XML document of the same purpose.
 */
public class IDPConfiguration
{
    private final String entityID;
    private final String entityName;
    private final Collection<String> nameIDFormats;
    private final Collection<ServiceEndpoint> ssoServices;
    private final Collection<ServiceEndpoint> sloServices;
    private final List<X509Certificate> signingCertificateChain;
    private AttributeConfig[] subjectFormatMappings;
    private Map<TokenClaimAttribute,List<String>> tokenClaimGroupMappings;
    private boolean isJitEnabled;
    private String upnSuffix;


    /**
     * Construct IDPConfiguration object supporting JIT configuration.
     *
     * @param entityID
     *          Entity ID for the external IDP. Doubles as the {@code entityName}.
     *          Required, non-null, non-empty.
     * @param nameIDFormats
     *          Zero or more elements that enumerate the name identifier formats
     *          supported by the IDP. Required, non-null.
     * @param ssoServices
     *          Zero or more elements that describe endpoints that support
     *          single sign on profiles. Required, non-null.
     * @param sloServices
     *          Zero or more elements that describe endpoints that support
     *          single logout profiles. Optional.
     * @param signingCertificateChain
     *          Sequence of elements that provide information about the
     *          cryptographic keys used by the IDP. Required, non-null, non-empty.
     * @param subjectFormatMappings
     *          Allows the SSO server to map the subject represented in an
     *          external token to a principal in the VMware Identity Stores.
     *          This is only necessary if the external token will use non-UPN
     *          as a subject nameID format. Optional.
     *          <p>
     *          <i>Ex:</i><br>
     *          "urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress" => "email"<br>
     *          "urn:oasis:names:tc:SAML:2.0:nameid-format:persistent" => "GID"
     * @param isJitEnabled
     *          True if enabling JIT in external IDP. Required.
     * @param tokenClaimGroupMappings
     *          Allows the SSO server to map the claims represented in an external token
     *          to a list of groups in the VMware Identity Store System Domain.
     *          This is to support authorization by groups.
     *          The list of groups cannot be null, or contain null elements. Optional.
     *          <p>
     *          <i>Ex:</i><br>
     *          Claim attribute in the SAML token:<br>
     *          <Attribute Name="http://schemas.microsoft.com/ws/2008/06/identity/claims/groups"><br>
     *          <AttributeValue>5581e43f-6096-41d4-8ffa-04e560bab39d</AttributeValue><br>
     *          <AttributeValue>07dd8a89-bf6d-4e81-8844-230b77145381</AttributeValue><br>
     *          </Attribute><br>
     *          The key of the map is the concatenation of attribute name and value, separated by delimiter '#'.
     *          The value the map is the list of group sids:<br>
     *          (http://schemas.microsoft.com/ws/2008/06/identity/claims/groups#5581e43f-6096-41d4-8ffa-04e560bab39d) ===> [S-1-7-21-614251302-352781979-2051859926-3081890169-16778276, S-1-7-21-614251302-352781979-2051859926-3081890169-16778275]><br>
     *          (http://schemas.microsoft.com/ws/2008/06/identity/claims/groups#07dd8a89-bf6d-4e81-8844-230b77145381) ===> [S-1-7-21-614251302-352781979-2051859926-3081890169-16778276]>
     * @param upnSuffix
     *          UPN suffix used to construct UPNs when provisioning JIT users
     *          if Subject NameId in external token is not in UPN format. Optional.
     *
     */
    public IDPConfiguration(String entityID, Collection<String> nameIDFormats,
            Collection<ServiceEndpoint> ssoServices, Collection<ServiceEndpoint> sloServices,
            List<X509Certificate> signingCertificateChain, AttributeConfig[] subjectFormatMappings,
            boolean isJitEnabled, Map<TokenClaimAttribute,List<String>> tokenClaimGroupMappings, String upnSuffix)
    {
        ValidateUtil.validateNotEmpty(entityID, "entityID");
        ValidateUtil.validateNotNull(nameIDFormats, "nameIDFormats");
        ValidateUtil.validateNotNull(ssoServices, "ssoServices");
        ValidateUtil.validateNotEmpty(signingCertificateChain, "signingCertificateChain");

        this.entityID = entityID;
        this.entityName = entityID; // Set as the ID for now..
        this.nameIDFormats = nameIDFormats;
        this.ssoServices = ssoServices;

        if (sloServices == null) {
            sloServices = Collections.<ServiceEndpoint>emptyList();
        }

        this.sloServices = sloServices;
        this.signingCertificateChain = signingCertificateChain;

        if (subjectFormatMappings == null) {
            subjectFormatMappings = new AttributeConfig[0];
        }
        if (tokenClaimGroupMappings == null) {
            tokenClaimGroupMappings = new HashMap<TokenClaimAttribute,List<String>>();
        }
        if (!tokenClaimGroupMappings.isEmpty()) {
            if (tokenClaimGroupMappings.values().contains(null)) {
                throw new IllegalArgumentException("One or more group lists are null in claim group mappings.");
            }
        }
        this.subjectFormatMappings = subjectFormatMappings;
        this.isJitEnabled = isJitEnabled;
        this.tokenClaimGroupMappings = tokenClaimGroupMappings;
        this.upnSuffix = upnSuffix;
    }

    /**
     * Create a new IDPConfiguration class.
     *
     * @param entityID
     *          Entity ID for the external IDP. Doubles as the {@code entityName}.
     * @param nameIDFormats
     *          Zero or more elements that enumerate the name identifier formats
     *          supported by the IDP.
     * @param ssoServices
     *          Zero or more elements that describe endpoints that support
     *          single sign on profiles.
     * @param sloServices
     *          Zero or more elements that describe endpoints that support
     *          single logout profiles.
     * @param signingCertificateChain
     *          Sequence of elements that provide information about the
     *          cryptographic keys used by the IDP.
     * @param subjectFormatMappings
     *          Allows the SSO server to map the subject represented in an
     *          external token to a principal in the VMware Identity Stores.
     *          This is only necessary if the external token will use non-UPN
     *          as a subject nameID format.
     *          <p>
     *          <i>Ex:</i><br>
     *          "urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress" => "email"<br>
     *          "urn:oasis:names:tc:SAML:2.0:nameid-format:persistent" => "GID"
     */
    public IDPConfiguration(String entityID, Collection<String> nameIDFormats,
            Collection<ServiceEndpoint> ssoServices, Collection<ServiceEndpoint> sloServices,
            List<X509Certificate> signingCertificateChain, AttributeConfig[] subjectFormatMappings)
    {
        this(entityID,
                nameIDFormats,
                ssoServices,
                sloServices,
                signingCertificateChain,
                subjectFormatMappings,
                false,
                null,
                null);
    }

    /**
     * Create a new IDPConfiguration class with only the required attributes.
     *
     * @param entityID
     *          Entity ID for the external IDP.
     * @param nameIDFormats
     *          Zero or more elements that enumerate the name identifier formats
     *          supported by the IDP.
     * @param ssoServices
     *          Zero or more elements that describe endpoints that support
     *          single sign on profiles.
     * @param signingCertificateChain
     *          Sequence of elements that provide information about the
     *          cryptographic keys used by the IDP.
     */
    public IDPConfiguration(String entityID,
            Collection<String> nameIDFormats,
            Collection<ServiceEndpoint> ssoServices,
            List<X509Certificate> signingCertificateChain)
    {
        this(entityID,
             nameIDFormats,
             ssoServices,
             Collections.<ServiceEndpoint>emptyList(),
             signingCertificateChain,
             new AttributeConfig[0],
             false,
             null,
             null);
    }

    public String getEntityID()
    {
        return entityID;
    }

    public String getEntityName()
    {
        return entityName;
    }

    public Collection<String> getNameIDFormats()
    {
        return this.nameIDFormats;
    }

    public boolean getJitAttribute() {
        return this.isJitEnabled;
    }

    public void setJitAttribute(boolean isJitEnabled) {
        this.isJitEnabled = isJitEnabled;
    }

    public String getUpnSuffix() {
        return this.upnSuffix;
    }

    public Collection<ServiceEndpoint> getSsoServices()
    {
        return this.ssoServices;
    }

    public Collection<ServiceEndpoint> getSloServices()
    {
        return this.sloServices;
    }

    public List<X509Certificate> getSigningCertificateChain()
    {
        return signingCertificateChain;
    }

    public AttributeConfig[] getSubjectFormatMappings()
    {
        return this.subjectFormatMappings;
    }

    public void setSubjectFormatMappings(AttributeConfig[] subjectFormatMappings)
    {
        this.subjectFormatMappings = subjectFormatMappings;
    }

    public Map<TokenClaimAttribute,List<String>> getTokenClaimGroupMappings()
    {
        return this.tokenClaimGroupMappings;
    }
}
