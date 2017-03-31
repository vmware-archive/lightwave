/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.client;

import java.util.List;

/**
 * SsoRequestSettings is a structure holding the settings for the authentication
 * request. It contains a number of fields with getters and setters.
 *
 */
public class SsoRequestSettings {

    private String spAlias;

    private String idpAlias;

    private Boolean isSigned; // defaults to false

    private String nameIDFormat; // defaults to persistant

    private Boolean allowProxy; // defaults to true

    private Integer proxyCount; // maximum proxy delegation.

    private Boolean forceAuthn; // defaults to false

    private Boolean isPassive; // defaults to false

    // assertionConsumerServiceIndex and assertionConsumerServiceUrl are mutual
    // exclusive option. user should provide only one. If both are provided, url
    // is used.
    private Integer assertionConsumerServiceIndex;

    private String assertionConsumerServiceUrl;

    private String relayState; // default to null

    private Boolean isRenewable; // default to false

    private Boolean isDelegable; // default to false

    private Boolean allowScopingElement; // default to true

    private Boolean allowSPNameQualifierInNameIDPolicy; // default true

    private Boolean allowAllowCreateInNameIDPolicy; // default true

    /**
     * Currently the following are supported by VMWareIdentity
     *
     * "urn:oasis:names:tc:SAML:2.0:ac:classes:PasswordProtectedTransport"
     * "urn:federation:authentication:windows"
     * urn:oasis:names:tc:SAML:2.0:ac:classes:Kerberos
     * "urn:oasis:names:tc:SAML:2.0:ac:classes:TLSClient"
     * "urn:oasis:names:tc:SAML:2.0:ac:classes:TimeSyncToken"
     *
     *
     */
    private List<String> requestedAuthnContextClasses; // set of authentication
                                                       // type desired

    private Boolean allowRequestedAuthnContext; // default true;

    /**
     * SsoRequestSettings object. This object correspond to
     * samlp:AuthnRequestType.
     *
     * @param alias
     *            Required.
     * @param entityID
     *            Required.
     */
    public SsoRequestSettings(String spAlias, String idpAlias) {
        this(spAlias, idpAlias, false, SamlNames.PERSISTENT, true, false, false, 0, null, null);
        this.proxyCount = null; // match sementic of SAML Authnrequest "not set"
        this.isDelegable = false;
        this.isRenewable = false;
        this.allowScopingElement = true;
        this.allowSPNameQualifierInNameIDPolicy = true;
        this.allowAllowCreateInNameIDPolicy = true;
        this.allowAllowCreateInNameIDPolicy = true;
        this.allowRequestedAuthnContext = true;
    }

    /**
     * SsoRequestSettings object. This object correspond to
     * samlp:AuthnRequestType.
     *
     * @param spAlias
     *            Required. Need it set Issuer
     * @param idpAlias
     *            Required.
     * @param isSigned
     *            optional. if true, idp will expect the request to be signed.
     * @param nameIDFormat
     *            Optional. name id formats expected.
     * @param allowProxy
     *            Optional.
     * @param forceAuthn
     *            Optional.
     * @param isPassive
     *            optional.
     * @param assertionConsumerServiceIndex
     *            Optional.
     * @param assertionConsumerServiceUrl
     *            Optional.
     * @param relayState
     *            Optional.
     *
     */
    public SsoRequestSettings(String spAlias, String idpAlias, Boolean isSigned, String nameIDFormat,
            Boolean allowProxy, Boolean forceAuthn, Boolean isPassive, Integer assertionConsumerServiceIndex,
            String assertionConsumerServiceUrl, String relayState) {

        this.spAlias = spAlias;
        this.idpAlias = idpAlias;
        this.isSigned = isSigned;
        this.nameIDFormat = nameIDFormat;
        this.allowProxy = allowProxy;
        this.forceAuthn = forceAuthn;
        this.isPassive = isPassive;
        this.assertionConsumerServiceIndex = assertionConsumerServiceIndex;
        this.assertionConsumerServiceUrl = assertionConsumerServiceUrl;
        this.relayState = relayState;
        this.isDelegable = false;
        this.isRenewable = false;
        this.allowScopingElement = true;
        this.allowSPNameQualifierInNameIDPolicy = true;
        this.allowAllowCreateInNameIDPolicy = true;
        this.allowRequestedAuthnContext = true;
    }

    public void setSPAlias(String spAlias) {
        this.spAlias = spAlias;
    }

    public String getSPAlias() {
        return spAlias;
    }

    public void setIDPAlias(String idpAlias) {
        this.idpAlias = idpAlias;
    }

    public String getIDPAlias() {
        return idpAlias;
    }

    public void setIsSigned(Boolean isSigned) {
        this.isSigned = isSigned;
    }

    public Boolean isSigned() {
        return isSigned;
    }

    public void setNameIDFormat(String nameIDFormat) {
        this.nameIDFormat = nameIDFormat;
    }

    public String getNameIDFormat() {
        return nameIDFormat;
    }

    public void setAllowProxy(Boolean allowProxy) {
        this.allowProxy = allowProxy;
    }

    public Boolean isAllowProxy() {
        return allowProxy;
    }

    public void setForceAuthn(Boolean forceAuthn) {
        this.forceAuthn = forceAuthn;
    }

    public Boolean isForceAuthn() {
        return forceAuthn;

    }

    public void setIsPassive(Boolean isPassive) {
        this.isPassive = isPassive;
    }

    public Boolean isPassive() {
        return isPassive;
    }

    public void setAssertionConsumerServiceIndex(Integer index) {
        this.assertionConsumerServiceIndex = index;
    }

    public Integer getAssertionConsumerServiceIndex() {
        return assertionConsumerServiceIndex;
    }

    public void setAssertionConsumerServiceUrl(String url) {
        this.assertionConsumerServiceUrl = url;
    }

    public String getAssertionConsumerServiceUrl() {
        return this.assertionConsumerServiceUrl;
    }

    /**
     * @return the relayState
     */
    public String getRelayState() {
        return relayState;
    }

    /**
     * @param relayState
     *            the relayState to set
     */
    public void setRelayState(String relayState) {
        this.relayState = relayState;
    }

    /**
     * @return the isRenewable
     */
    public Boolean isRenewable() {
        return isRenewable;
    }

    /**
     * @param isRenewable
     *            the isRenewable to set
     */
    public void setIsRenewable(Boolean isRenewable) {
        this.isRenewable = isRenewable;
    }

    /**
     * @return the isDelegable
     */
    public Boolean isDelegable() {
        return isDelegable;
    }

    /**
     * @param isDelegable
     *            the isDelegable to set
     */
    public void setIsDelegable(Boolean isDelegable) {
        this.isDelegable = isDelegable;
    }

    /**
     * @return the proxyCount. Meaning of proxyCount value null - not set set
     *         (default) 0 - never use external IDP >0 - use external IDP with
     *         restricted number of delegation.
     */
    public Integer getProxyCount() {
        return proxyCount;
    }

    /**
     * set the proxyCount. Meaning of proxyCount value null - not set set
     * (default) 0 - never use external IDP >0 - use external IDP with
     * restricted number of delegation.
     */
    public void setProxyCount(Integer i) {
        this.proxyCount = i;
    }

    public Boolean getAllowScopingElement() {
        return allowScopingElement;
    }

    /**
     * Will set scoping only if this flag is true. Some IDP such ADFS can not
     * handle Scoping element in SAML authn request true - may emit Scoping
     * false - will not emit Scoping
     */
    public void setAllowScopingElement(Boolean allowScopingElement) {
        this.allowScopingElement = allowScopingElement;
    }

    public Boolean getAllowSPNameQualifierInNameIDPolicy() {
        return allowSPNameQualifierInNameIDPolicy;
    }

    public void setAllowSPNameQualifierInNameIDPolicy(Boolean allowSPNameQualifierInNameIDPolicy) {
        this.allowSPNameQualifierInNameIDPolicy = allowSPNameQualifierInNameIDPolicy;
    }

    public Boolean getAllowAllowCreateInNameIDPolicy() {
        return allowAllowCreateInNameIDPolicy;
    }

    public void setAllowAllowCreateInNameIDPolicy(Boolean allowAllowCreateInNameIDPolicy) {
        this.allowAllowCreateInNameIDPolicy = allowAllowCreateInNameIDPolicy;
    }

    public Boolean getAllowRequestAuthnContext() {
        return allowRequestedAuthnContext;
    }

    public void setAllowRequestAuthnContext(Boolean allowRequestAuthnContext) {
        this.allowRequestedAuthnContext = allowRequestAuthnContext;
    }

    /**
     * @return the requestedAuthnContextClasses
     */
    public List<String> getRequestedAuthnContextClasses() {
        return requestedAuthnContextClasses;
    }

    /**
     * @param requestedAuthnContextClasses
     *            the requestedAuthnContextClasses to set
     */
    public void setRequestedAuthnContextClasses(List<String> requestedAuthnContextClasses) {
        this.requestedAuthnContextClasses = requestedAuthnContextClasses;
    }

}
