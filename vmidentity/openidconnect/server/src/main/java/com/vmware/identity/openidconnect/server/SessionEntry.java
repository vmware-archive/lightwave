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

package com.vmware.identity.openidconnect.server;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;

/**
 * @author Yehia Zayour
 */
public class SessionEntry {
    private final PersonUser personUser;
    private final Set<OIDCClientInformation> clients;
    private final Set<ClientID> clientIds;

    public SessionEntry(PersonUser personUser, OIDCClientInformation client) {
        Validate.notNull(personUser, "personUser");
        Validate.notNull(client, "client");

        this.personUser = personUser;
        this.clients = new HashSet<OIDCClientInformation>();
        this.clientIds = new HashSet<ClientID>();

        this.clients.add(client);
        this.clientIds.add(client.getID());
    }

    public PersonUser getPersonUser() {
        return this.personUser;
    }

    public Set<OIDCClientInformation> getClients() {
        return Collections.unmodifiableSet(this.clients);
    }

    public void add(OIDCClientInformation client) {
        Validate.notNull(client, "client");
        if (!this.clientIds.contains(client.getID())) {
            this.clientIds.add(client.getID());
            this.clients.add(client);
        }
    }
}
