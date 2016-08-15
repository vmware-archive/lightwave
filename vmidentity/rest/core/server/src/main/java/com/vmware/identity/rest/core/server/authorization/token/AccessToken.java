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
package com.vmware.identity.rest.core.server.authorization.token;

import java.util.Date;
import java.util.List;

import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerToken;

/**
 * An object that represents an access token.
 * <p>
 * The access token is the entity used to grant or deny access to any
 * resource or resource method.
 * <p>
 * Some implementations may contain supplemental data such as the underlying
 * token representation (<i>e.g.</i> {@link JWTBearerToken}).
 */
public interface AccessToken {

    /**
     * Retrieve the audience list from the access token.
     *
     * @return the list of audiences associated with this token
     */
    public List<String> getAudience();

    /**
     * Retrieve the issuer from the access token.
     *
     * @return the issuer for this token
     */
    public String getIssuer();

    /**
     * Retrieve the role name from the access token.
     *
     * @return the role for this token or <tt>null</tt> if it is undefined.
     */
    public String getRole();

    /**
     * Retrieve the group list from the access token.
     *
     * @return the list of groups for this token or <tt>null</tt> if it is undefined.
     */
    public List<String> getGroupList();

    /**
     * Retrieve the time at which the token was issued.
     *
     * @return the issue time for this token
     */
    public Date getIssueTime();

    /**
     * Retrieve the time at which the token expires.
     *
     * @return the expiration time for this token
     */
    public Date getExpirationTime();

    /**
     * Retrieve the subject for which the token was issued.
     *
     * @return the subject of this token
     */
    public String getSubject();

    /**
     * Retrieve the type from the access token.
     *
     * @return the type of the token.
     */
    public String getTokenType();

}
