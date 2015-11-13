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

import org.apache.commons.lang.ObjectUtils;

/**
 * Structure to hold LogoutRequest-related data. Initiator - Relying party URL
 * for the entity that sent initial logout request InitiatorRequestId - original
 * LogoutRequest id Current - current 'intermediate' relying party which is
 * being logged out CurrentRequestId - current LogoutRequest id
 *
 */
public final class LogoutRequestData implements Serializable {

    /**
     *
     */
    private static final long serialVersionUID = 1L;

    private String initiator;
    private String initiatorRequestId;
    private String current;
    private String currentRequestId;

    /**
     * Construct the instance
     *
     * @param initiator
     * @param initiatorRequestId
     * @throws NoSuchAlgorithmException
     */
    public LogoutRequestData(String initiator, String initiatorRequestId) {
        this.initiator = initiator;
        this.initiatorRequestId = initiatorRequestId;
    }

    /**
     * @return the initiator
     */
    public String getInitiator() {
        return initiator;
    }

    /**
     * @return the initiatorRequestId
     */
    public String getInitiatorRequestId() {
        return initiatorRequestId;
    }

    @Override
    public String toString() {
        return String
                .format("LogoutRequestData [initiator=%s, initiatorRequestId=%s, current=%s, currentRequestId=%s]",
                        initiator, initiatorRequestId, current,
                        currentRequestId);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ObjectUtils.hashCode(initiator);
        result = prime * result + ObjectUtils.hashCode(initiatorRequestId);
        result = prime * result + ObjectUtils.hashCode(current);
        result = prime * result + ObjectUtils.hashCode(currentRequestId);
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

        LogoutRequestData other = (LogoutRequestData) obj;
        return ObjectUtils.equals(initiator, other.initiator)
                && ObjectUtils.equals(initiatorRequestId,
                        other.initiatorRequestId)
                && ObjectUtils.equals(current, other.current)
                && ObjectUtils.equals(currentRequestId, other.currentRequestId);
    }

    /**
     * @return the current
     */
    public String getCurrent() {
        return current;
    }

    /**
     * @param current
     *            the current to set
     */
    public void setCurrent(String current) {
        this.current = current;
    }

    /**
     * @return the currentRequestId
     */
    public String getCurrentRequestId() {
        return currentRequestId;
    }

    /**
     * @param currentRequestId
     *            the currentRequestId to set
     */
    public void setCurrentRequestId(String currentRequestId) {
        this.currentRequestId = currentRequestId;
    }
}
