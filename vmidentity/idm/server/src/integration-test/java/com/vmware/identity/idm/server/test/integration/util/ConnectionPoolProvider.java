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

package com.vmware.identity.idm.server.test.integration.util;

import java.util.ArrayList;
import java.util.Collection;

import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.provider.IPooledConnectionProvider;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PooledLdapConnectionIdentity;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapException;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.ServerDownLdapException;
import com.vmware.identity.interop.ldap.TimeoutLdapException;
import com.vmware.identity.interop.ldap.UnavailableLdapException;

public class ConnectionPoolProvider implements IPooledConnectionProvider, AutoCloseable {

    PooledLdapConnection pooled = null;

    @Override
    public PooledLdapConnection borrowConnection(PooledLdapConnectionIdentity identity) throws Exception {

        if ( this.connectionOK() == false ) {
            this.pooled.close();
            this.pooled = null;
        }

        if ( this.pooled == null ) {
            Collection<String> connectionStrings = new ArrayList<>();
            connectionStrings.add(identity.getConnectionString());
            ILdapConnectionEx connection = ServerUtils.getLdapConnectionByURIs(
                    ServerUtils.toURIObjects(connectionStrings),
                    identity.getUsername(),
                    identity.getPassword(),
                    identity.getAuthType(),
                    identity.isUseGCPort(),
                    null);

            this.pooled = new PooledLdapConnection(connection, identity, this);
        }
        return this.pooled;
    }

    @Override
    public void returnConnection(PooledLdapConnection pooledConnection) {
        // no op
    }

    @Override
    public void close() throws Exception {
        if (this.pooled != null) {
            this.pooled.close();
            this.pooled = null;
        }
    }

    private boolean connectionOK() {
        if (this.pooled != null) {
            ILdapConnectionEx ldapConnection = this.pooled.getConnection();
            if (ldapConnection == null) {
                return false;
            }

            String[] attributes = new String[0];
            try (ILdapMessage message = ldapConnection.search("", LdapScope.SCOPE_BASE, "objectclass=*", attributes, false)){
                // Not doing anything, just making sure the search worked
            } catch (ServerDownLdapException | UnavailableLdapException | TimeoutLdapException e) {
                return false;
            } catch (LdapException e) {
                // Ignore these since the connection is at least active
            } catch (Exception e) {
                return false;
            }
        }

        return true;
    }
}
