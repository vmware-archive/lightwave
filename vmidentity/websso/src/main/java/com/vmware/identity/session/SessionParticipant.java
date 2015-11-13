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

import java.io.Serializable;
import java.security.NoSuchAlgorithmException;

import org.apache.commons.lang.Validate;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;

/**
 * Single Relying Party session info is stored here.
 * This is immutable data structure.
 *
 */
public final class SessionParticipant implements Serializable {

	private static final long serialVersionUID = 1L;

	private String sessionId;
	private String relyingPartyUrl;

	/**
	 * Construct the instance
	 * @param relyingPartyUrl
	 * @throws NoSuchAlgorithmException
	 */
	public SessionParticipant (String relyingPartyUrl) throws NoSuchAlgorithmException {
		Validate.notNull(relyingPartyUrl);

		SecureRandomIdentifierGenerator generator =
				new SecureRandomIdentifierGenerator();
		this.sessionId = generator.generateIdentifier();
		this.relyingPartyUrl = relyingPartyUrl;
	}

	/**
	 * @return the sessionId
	 */
	public String getSessionId() {
		return sessionId;
	}
	/**
	 * @return the relyingPartyUrl
	 */
	public String getRelyingPartyUrl() {
		return relyingPartyUrl;
	}

	@Override
	public String toString() {
		return String.format(
				"SessionParticipant [sessionId=%s, relyingPartyUrl=%s]",
				sessionId, relyingPartyUrl);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + sessionId.hashCode();
		result = prime * result + relyingPartyUrl.hashCode();
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null || this.getClass() != obj.getClass()) {
			return false;
		}

		SessionParticipant other = (SessionParticipant) obj;
		return sessionId.equals(other.sessionId) &&
				relyingPartyUrl.equals(other.relyingPartyUrl);
	}
}
