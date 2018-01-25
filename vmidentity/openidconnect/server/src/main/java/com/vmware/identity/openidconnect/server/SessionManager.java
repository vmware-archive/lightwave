/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.openidconnect.server;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class SessionManager {
    private static final long LIFETIME_MS = 8 * 60 * 60 * 1000L; // 8 hours
    private final SlidingWindowMap<SessionID, Entry> map;

    public SessionManager() {
        this.map = new SlidingWindowMap<SessionID, Entry>(LIFETIME_MS);
    }

    public synchronized void add(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        this.map.add(sessionId, new Entry());
    }

    public synchronized void add(
            SessionID sessionId,
            PersonUser personUser,
            LoginMethod loginMethod,
            ClientInfo client) {
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(personUser, "personUser");
        Validate.notNull(loginMethod, "loginMethod");
        Validate.notNull(client, "client");

        this.map.add(sessionId, new Entry(personUser, loginMethod, client));
    }

    public synchronized Entry setExternalJWTContent(SessionID sessionId, String externalJWTContent) {
        Validate.notNull(sessionId, "sessionId");
        Validate.notEmpty(externalJWTContent, "external token");

        Entry entry = this.map.get(sessionId);
        if (entry != null) {
            entry.externalJWTContent = externalJWTContent;
        }
        return entry;
    }

    public synchronized Entry update(SessionID sessionId, PersonUser personUser, LoginMethod loginMethod, ClientInfo client) {
        Validate.notNull(sessionId, "sessionId");

        Entry entry = this.map.get(sessionId);
        if (entry != null) {
            if (personUser != null) {
                entry.personUser = personUser;
            }
            if (loginMethod != null) {
                entry.loginMethod = loginMethod;
            }
            if (client != null) {
                entry.add(client);
            }
        }
        return entry;
    }

    public synchronized Entry update(SessionID sessionId, ClientInfo client) {
        Validate.notNull(sessionId, "sessionId");
        Validate.notNull(client, "client");

        Entry entry = this.map.get(sessionId);
        if (entry != null) {
            entry.add(client);
        }
        return entry;
    }

    public synchronized Entry remove(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        return this.map.remove(sessionId);
    }

    public synchronized Entry get(SessionID sessionId) {
        Validate.notNull(sessionId, "sessionId");
        return this.map.get(sessionId);
    }

    public static String getSessionCookieName(String tenant) {
        Validate.notEmpty(tenant, "tenant");
        return String.format("oidc_session_id-%s", tenant);
    }

    public static String getExternalIdpIssuerCookieName(String tenant) {
        Validate.notEmpty(tenant, "tenant");
        return String.format("oidc_issuer-%s", tenant);
    }

    public static String getPersonUserCertificateLoggedOutCookieName(String tenant) {
        Validate.notEmpty(tenant, "tenant");
        return String.format("oidc_person_user_certificate_logged_out-%s", tenant);
    }

    public static class Entry {
        private PersonUser personUser;
        private LoginMethod loginMethod;
        private Set<ClientInfo> clients;
        private Set<ClientID> clientIds;
        private String externalJWTContent;

        private Entry(PersonUser personUser, LoginMethod loginMethod, ClientInfo client) {
            this.personUser = personUser;
            this.loginMethod = loginMethod;
            this.clients = new HashSet<ClientInfo>();
            this.clients.add(client);
            this.clientIds = new HashSet<ClientID>();
            this.clientIds.add(client.getID());
        }

        private Entry() {

        }

        public PersonUser getPersonUser() {
            Validate.notNull(this.personUser, "persion user");
            return this.personUser;
        }

        public LoginMethod getLoginMethod() {
            Validate.notNull(this.loginMethod, "login method");
            return this.loginMethod;
        }

        public Set<ClientInfo> getClients() {
            Validate.notNull(this.clients, "clients");
            return Collections.unmodifiableSet(this.clients);
        }

        public String getExternalJWTContent() {
            return this.externalJWTContent;
        }

        private void add(ClientInfo client) {
            if (this.clientIds == null) {
                this.clientIds = new HashSet<ClientID>();
            }
            if (this.clients == null) {
                this.clients = new HashSet<ClientInfo>();
            }
            if (!this.clientIds.contains(client.getID())) {
                this.clientIds.add(client.getID());
                this.clients.add(client);
            }
        }
    }
}