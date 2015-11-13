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
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.Validate;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;

/**
 * WebSSO session object
 *
 */
public final class Session implements Serializable {

    private static final long serialVersionUID = 1L;

    private final String id;
    private final PrincipalId principalId;
    private final Date expireDate;
    private final AuthnMethod authnMethod;
    private final Map<String, SessionParticipant> participants;
    private final Map<String, SessionParticipant> participantsByUrl;
    private LogoutRequestData logoutRequestData;
    private final Lock lock;
    private boolean usingExtIDP;
    private IDPConfig extIDPUsed;  //external IDP used if usingExtIDP is true
    private String extIDPSessionID;

    /**
     * Construct the object
     *
     * @param principalId
     * @param expireDate
     * @param authnMethod
     * @throws NoSuchAlgorithmException
     */
    public Session(PrincipalId principalId, Date expireDate, AuthnMethod authnMethod)
            throws NoSuchAlgorithmException {
        Validate.notNull(principalId);
        Validate.notNull(expireDate);

        SecureRandomIdentifierGenerator generator = new SecureRandomIdentifierGenerator();
        this.id = generator.generateIdentifier();
        this.principalId = principalId;
        this.expireDate = expireDate;
        this.authnMethod = authnMethod;
        this.participants = new HashMap<String, SessionParticipant>();
        this.participantsByUrl = new HashMap<String, SessionParticipant>();
        this.lock = new ReentrantLock();
        this.usingExtIDP = false;
        this.extIDPUsed = null;
        this.extIDPSessionID = null;
    }

    /**
     *
     * @return the lock
     */
    public Lock getLock() {
        return lock;
    }

    /**
     * @return the principalId
     */
    public PrincipalId getPrincipalId() {
        return principalId;
    }

    /**
     * @return the expireDate
     */
    public Date getExpireDate() {
        return expireDate;
    }

    /**
     * @return check if the is expired
     */
    public boolean isValid() {
        Calendar calendar = new GregorianCalendar();
        Date currentTime = calendar.getTime();
        if (currentTime.after(expireDate)) {
            return false;
        }
        return true;
    }

    /**
     * @return the sessionParticipants
     */
    public Collection<SessionParticipant> getSessionParticipants() {
        return Collections.unmodifiableCollection(participants.values());
    }

    /**
     * Add new session participant
     *
     * @param sessionParticipant
     */
    public void addSessionParticipant(SessionParticipant sessionParticipant) {
        Validate.notNull(sessionParticipant);
        this.participants.put(sessionParticipant.getSessionId(),
                sessionParticipant);
        this.participantsByUrl.put(sessionParticipant.getRelyingPartyUrl(),
                sessionParticipant);
    }

    /**
     * Remove session participant
     *
     * @param sessionParticipantIndex
     */
    public void removeSessionParticipant(String sessionParticipantIndex) {
        Validate.notNull(sessionParticipantIndex);
        if (participants.containsKey(sessionParticipantIndex)) {
            String relyingParty = this.participants.get(sessionParticipantIndex).getRelyingPartyUrl();
            this.participants.remove(sessionParticipantIndex);
            this.participantsByUrl.remove(relyingParty);
        }
    }

    /**
     * Remove session participant by its URL
     *
     * @param sessionParticipantUrl
     */
    public void removeSessionParticipantByUrl(String sessionParticipantUrl) {
        Validate.notNull(sessionParticipantUrl);
        if (participantsByUrl.containsKey(sessionParticipantUrl)) {
            String participant = this.participantsByUrl.get(sessionParticipantUrl).getSessionId();
            this.participants.remove(participant);
            this.participantsByUrl.remove(sessionParticipantUrl);
        }
    }

    /**
     * Check existence of the session participant
     *
     * @param sessionParticipantIndex
     * @return
     */
    public boolean containsSessionParticipant(String sessionParticipantIndex) {
        Validate.notNull(sessionParticipantIndex);
        return this.participants.containsKey(sessionParticipantIndex);
    }

    /**
     * Check existence of the session participant by its Url
     *
     * @param sessionParticipantIndex
     * @return
     */
    public boolean containsSessionParticipantUrl(String sessionParticipantUrl) {
        Validate.notNull(sessionParticipantUrl);
        return this.participantsByUrl.containsKey(sessionParticipantUrl);
    }

    /**
     * Return session participant or null
     *
     * @param sessionParticipantIndex
     * @return
     */
    public SessionParticipant getSessionParticipant(
            String sessionParticipantIndex) {
        Validate.notNull(sessionParticipantIndex);
        if (this.participants.containsKey(sessionParticipantIndex)) {
            return this.participants.get(sessionParticipantIndex);
        } else {
            return null;
        }
    }

    /**
     * Ensure that Session contains a participant with specified URL Return
     * session participant index
     *
     * @param sessionParticipantUrl
     * @return
     */
    public String ensureSessionParticipant(String sessionParticipantUrl) {
        Validate.notNull(sessionParticipantUrl);

        String retval = null;
        SessionParticipant participant = null;
        for (SessionParticipant sp : getSessionParticipants()) {
            if (sp.getRelyingPartyUrl().equals(sessionParticipantUrl)) {
                participant = sp;
                break;
            }
        }

        if (participant == null) {
            // need to create an entry for this relying party
            try {
                participant = new SessionParticipant(sessionParticipantUrl);
                addSessionParticipant(participant);
            } catch (NoSuchAlgorithmException e) {
                participant = null; // reset participant value
            }
        }

        if (participant != null) {
            retval = participant.getSessionId();
        }

        return retval;
    }

    @Override
    public String toString() {
        return String.format("Session [id=%s, principalId=%s, expireDate=%s, authnMethod=%s, logoutRequestData=%s, extIDPSessionID=%s, participants=[%s]]",
                id, principalId, expireDate, authnMethod, logoutRequestData,extIDPSessionID, participantsToString());
    }

    /**
     * return string representation of participants
     * @return
     */
    private String participantsToString() {
        StringBuilder sb = new StringBuilder();
        for (String participantByUrl : participantsByUrl.keySet()) {
            SessionParticipant sp = participantsByUrl.get(participantByUrl);
            if (sb.length() > 0) {
                sb.append(" , ");
            }
            sb.append(sp.toString());
        }
        return sb.toString();
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + id.hashCode();
        result = prime * result + principalId.hashCode();
        result = prime * result + expireDate.hashCode();
        result = prime * result + participants.hashCode();
        result = prime * result + participantsByUrl.hashCode();
        result = prime * result + authnMethod.hashCode();
        result = prime * result + ObjectUtils.hashCode(logoutRequestData);
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

        Session other = (Session) obj;
        return id.equals(other.id) && principalId.equals(other.principalId)
                && expireDate.equals(other.expireDate)
                && authnMethod.equals(other.authnMethod)
                && participants.equals(other.participants)
                && participantsByUrl.equals(other.participantsByUrl)
                && ObjectUtils.equals(logoutRequestData, other.logoutRequestData);
    }

    /**
     * @return the id
     */
    public String getId() {
        return id;
    }

    /**
     * @return the authnMethod
     */
    public AuthnMethod getAuthnMethod() {
        return authnMethod;
    }

    /**
     * @return the logoutRequestData
     */
    public LogoutRequestData getLogoutRequestData() {
        return logoutRequestData;
    }

    /**
     * @param logoutRequestData the logoutRequestData to set
     */
    public void setLogoutRequestData(LogoutRequestData logoutRequestData) {
        this.logoutRequestData = logoutRequestData;
    }

	public boolean isUsingExtIDP() {
		return usingExtIDP;
	}

	public void setUsingExtIDP(boolean usingExtIDP) {
		this.usingExtIDP = usingExtIDP;
	}

	public IDPConfig getExtIDPUsed() {
		return extIDPUsed;
	}

	public void setExtIDPToUsed(IDPConfig extIDPUsed) {
		this.extIDPUsed = extIDPUsed;
	}

	public String getExtIDPSessionID() {
		return extIDPSessionID;
	}

	public void setExtIDPSessionID(String extIDPSessionID) {
		this.extIDPSessionID = extIDPSessionID;
	}
}
