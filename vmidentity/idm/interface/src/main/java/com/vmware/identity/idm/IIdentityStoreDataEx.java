/*
 *
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
 *
 */

package com.vmware.identity.idm;

import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Map;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/19/12
 * Time: 6:56 PM
 * To change this template use File | Settings | File Templates.
 */
public interface IIdentityStoreDataEx extends java.io.Serializable {

    /**
     * The type of the Identity Store.
     * See the {@link IdentityStoreType} enumeration.
     */
    public IdentityStoreType getProviderType();

    /**
     * The alternative name the identity store (domain) is referred by.
     * This Must be unique within all domain names&aliases for the tenant.
     */
    public String getAlias();

    /**
     * Gets the Authentication type to be used when connecting to this Identity Store.
     * See the {@link AuthenticationType} enumeration.
     */
    public AuthenticationType getAuthenticationType();

    /**
     * The username used for authentication in the case the {@code
     * authenticationType} is {@link AuthenticationType#PASSWORD} ; {@code
     * null} otherwise.
     *
     * @see AuthenticationType
     */
    public String getUserName();

    /**
     * The service principal name associate with the username used for authentication
     * This is the service account representing sso service
     *
     * The service principal name can be null, and is valid for Active Directory
     */
    public String getServicePrincipalName();

    /**
     * Whether the machine account should be used for authentication.
     * If this is true, a service principal name is not required
     *
     * @return true if the machine account is being used for authentication
     */
    public boolean useMachineAccount();

    /**
     * The password used for authentication in the case the {@code
     * authenticationType} is {@link AuthenticationType#PASSWORD} ; {@code
     * null} otherwise.
     */
    public String getPassword();

    public void setPassword(String passwordHash);

    /**
     * The human-friendly name of the ExternalDomain.
     */
    public String getFriendlyName();

    /**
     * The base Distinguished Name (DN) the Idm Server will use when
     * searching for a user in this Identity Store.
     */
    public String getUserBaseDn();

    /**
     * The base Distinguished Name (DN) the Idm Server will use when
     * searching for a group in this Identity Store.
     */
    public String getGroupBaseDn();

    /**
     * The list of addresses of external Identity Store(s) the Idm will conect to.
     * The values are in order of preference and non can be {@code null} or empty.
     */
    public Collection<String> getConnectionStrings();

    /**
     * The maximum number of seconds the Idm Server will wait for the
     * remote server to respond to a search query. Must be positive or
     * zero (unlimited).
     */
    public int getSearchTimeoutSeconds();

    /**
     * A SAML attribute mapping to use for this Identity Store.
     * Giving this attribute mapping allows the Idm server to map specified SAML attributes to
     * the attributes in the Identity Store.
     */
    public Map<String, String> getAttributeMap();

    /**
     * @return Returns a schema mapping if any for the identity store.
     *
     * @see IdentityStoreSchemaMapping
     */
    public IdentityStoreSchemaMapping getIdentityStoreSchemaMapping();

   /**
    * @return the collection of UPN suffixes for the identity store.
    */
   public Set<String> getUpnSuffixes();

   /**
    * returns bit flags controlling specific Identity Store behavior.
    * Opaque. Identity Store specific.
    */
   public int getFlags();

   /**
    * @return the certificate used for validation with ldaps connection
    */
   public Collection<X509Certificate> getCertificates();

   /**
    * @return the permissible authentication types on provider
    */
   public int[] getAuthnTypes();

   /**
    * @return if linking account using UPN for certificate authentication
    */
   public boolean getCertLinkingUseUPN();

   /**
    * @return attribute name for hint in certificate authentication
    */
   public String getCertUserHintAttributeName();

}
