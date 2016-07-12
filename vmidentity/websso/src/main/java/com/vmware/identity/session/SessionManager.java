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
package com.vmware.identity.session;

import java.util.Collection;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.samlservice.SamlServiceException;

/**
 * Session Manager interface
 *
 */
public interface SessionManager {

    /**
     * @param principal
     * @param authMethod
     * @param externalIDPSessionId optional. used for external authentication
     * @param idpEntId   optional. must used together with externalIDPSessionId for external authentication.
     * @return
     */
    public Session createSession(
            PrincipalId principal, AuthnMethod authMethod,
            String externalIDPSessionId, String idpEntId) throws SamlServiceException;

	/**
	 * Add a session to store
	 * @param session
	 */
	void add(Session session);

	/**
	 * Update session information in the store
	 * Important to call this if session has been changed
	 * @param session
	 */
	void update(Session session);

	/**
	 * Get (read-only copy of) all sessions
	 * @return
	 */
	Collection<Session> getAll();

	/**
	 * Get session by its id
	 * @param sessionId
	 * @return
	 */
	Session get(String sessionId);

	/**
	 * Get session by a participant id (typical operation for SP-initiated Logout)
	 * @param participantSessionId
	 * @return
	 */
	Session getByParticipant(String participantSessionId);

    /**
     * Get session by a logout request id (typical operation when we receive LogoutResponse)
     * @param logoutRequestId
     * @return
     */
    Session getByLogoutRequestId(String logoutRequestId);

    /**
	 * Remove session
	 * @param sessionId
	 */
	void remove(String sessionId);

	/**
	 * Clear the store
	 */
	void clear();
}
