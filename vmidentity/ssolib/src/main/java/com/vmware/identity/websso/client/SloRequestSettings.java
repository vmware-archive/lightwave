/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. 
 * ************************************************************************/
package com.vmware.identity.websso.client;

/**
 * SloRequestSettings is a structure holding the settings for the slo request.
 * It contains a number of fields with getters and setters.
 * 
 */
public class SloRequestSettings {

    private String spAlias;

    private String idpAlias;

    private Boolean isSigned; // defaults to false

    private String subject;

    private String nameIDFormat; // defaults to UPN

    private String sessionIndex;

    private String relayState;

    /**
     * SloRequestSettings object. Parameter to create samlp:LogoutRequest.
     * 
     * @param spAlias
     *            Required. Need it set Issuer
     * @param idpAlias
     *            Required.
     * @param isSigned
     *            optional. if true, idp will expect the request to be signed.
     * @param subject
     *            required
     * @param nameIDFormat
     *            Optional. name id formats used.
     * @param sessionIndex
     *            Optional.
     * @param relayState
     *            Optional.
     */
    public SloRequestSettings(String spAlias, String idpAlias, Boolean isSigned, String subject, String nameIDFormat,
            String sessionIndex, String relayState) {

        this.spAlias = spAlias;
        this.idpAlias = idpAlias;
        this.isSigned = isSigned;
        this.subject = subject;
        this.nameIDFormat = nameIDFormat;
        this.sessionIndex = sessionIndex;
        this.setRelayState(relayState);
    }

    public void setSPAlias(String spAlias) {
        this.spAlias = spAlias;
    }

    public String getSPAlias() {
        return this.spAlias;
    }

    public void setIDPAlias(String idpAlias) {
        this.idpAlias = idpAlias;
    }

    public String getIDPAlias() {
        return this.idpAlias;
    }

    public void setIsSigned(Boolean isSigned) {
        this.isSigned = isSigned;
    }

    public Boolean isSigned() {
        return this.isSigned;
    }

    public void setNameIDFormat(String nameIDFormat) {
        this.nameIDFormat = nameIDFormat;
    }

    public String getNameIDFormat() {
        return this.nameIDFormat;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    public String getSubject() {
        return this.subject;
    }

    public void setSessionIndex(String sessionIndex) {
        this.sessionIndex = sessionIndex;
    }

    public String getSessionIndex() {
        return this.sessionIndex;
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

}
