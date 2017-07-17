/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.idm;

import java.util.Date;

/**
 * The Approval class indicates whether a client has been approved to operate on behalf of a user.
 */
public class Approval {

    public enum ApprovalStatus {
        APPROVED,
        DENIED
    }

    private String userId = "";

    private String clientId = "";

    private String scope = "";

    private ApprovalStatus status = ApprovalStatus.DENIED;

    private Date expiresAt;

    private Date lastUpdatedAt = new Date();

    public Approval(String userId, String clientId, String scope, ApprovalStatus status, Date expiresAt, Date lastUpdatedAt) {
        this.userId = userId == null ? "" : userId;
        this.clientId = clientId == null ? "" : clientId;
        this.scope = scope == null ? "" : scope;

        if (status == null) throw new IllegalArgumentException("status cannot be null");
        this.status = status;

        if (expiresAt == null) throw new IllegalArgumentException("expiresAt cannot be null");
        this.expiresAt = expiresAt;

        if (lastUpdatedAt == null) throw new IllegalArgumentException("lastUpdatedAt cannot be null");
        this.lastUpdatedAt = lastUpdatedAt;
    }

    public String getUserId() {
        return userId;
    }

    public String getClientId() {
        return clientId;
    }

    public String getScope() {
        return scope;
    }

    public ApprovalStatus getStatus() {
        return status;
    }

    public Date getExpiresAt() {
        return expiresAt;
    }

    public Date getLastUpdatedAt() {
        return lastUpdatedAt;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + userId.hashCode();
        result = prime * result + clientId.hashCode();
        result = prime * result + scope.hashCode();
        result = prime * result + status.hashCode();
        return result;
    }

    @Override
    public boolean equals(Object o) {
       if (o == null || !(o instanceof Approval)) {
           return false;
       }

       Approval other = (Approval) o;
       return userId.equals(other.userId) && clientId.equals(other.clientId) && scope.equals(other.scope)
               && status == other.status;
    }

    @Override
    public String toString() {
        return String.format("[%s, %s, %s, %s, %s, %s]", userId, scope, clientId, expiresAt, status.toString(),
                lastUpdatedAt);
    }

}
