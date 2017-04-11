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

import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import org.joda.time.DateTime;
import org.w3c.dom.Document;

/**
 * AuthnData is an immutable structure of MessageData type. It holds information
 * about successful authentication. It has a number of properties with getters.
 * 
 */
public class AuthnData implements MessageData {

    private static final long serialVersionUID = 1L;

    private SubjectData subjectData;

    private Date expireDate;

    private String sessionIndex;

    private String authnContext;

    private List<Attribute> attributes;

    private TokenType tokenType;

    private Document token; // in case the token needs to be reused

    private DateTime sessionNotOnOrAfter; // optional field to tell the expiry
                                          // date of session.

    // required by subclass of serializable
    protected AuthnData() {
    }

    /**
     * AuthnData capture information on assertion in a SSO response received.
     * 
     * @param subjectData
     *            required
     * @param expDate
     *            optional
     * @param sessionIndex
     *            optional IDP server logon session identifier
     * @param authnContext
     *            optional from Assertion.AuthnticationStatements
     * @param attrs
     *            optional from Assertion.AttributeStatements.
     * @param tokenType
     *            required
     * @param token
     *            required
     */
    public AuthnData(SubjectData subjectData, Date expDate, String sessionIndex, String authnContext,
            List<Attribute> attrs, TokenType tokenType, Document token, DateTime sessionNotOnOrAfter) {
        this.subjectData = subjectData;
        this.expireDate = expDate;
        this.sessionIndex = sessionIndex;
        this.authnContext = authnContext;
        this.attributes = attrs;
        this.tokenType = tokenType;
        this.token = token;
        this.sessionNotOnOrAfter = sessionNotOnOrAfter;
    }

    public SubjectData getSubjectData() {
        return subjectData;
    }

    public Date getExpireDate() {
        return expireDate;
    }

    public String getAuthnContext() {
        return authnContext;
    }

    public Collection<Attribute> getAttributes() {
        if (attributes == null) {
            return null;
        }
        return Collections.unmodifiableCollection(attributes);
    }

    public TokenType getTokenType() {
        return tokenType;
    }

    public Document getToken() {
        return token;
    }

    /**
     * @return the sessionIndex
     */
    public String getSessionIndex() {
        return sessionIndex;
    }

    public DateTime getSessionNotOnOrAfter() {
        return sessionNotOnOrAfter;
    }
}
