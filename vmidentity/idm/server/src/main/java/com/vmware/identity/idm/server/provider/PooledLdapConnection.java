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
package com.vmware.identity.idm.server.provider;

import java.io.Closeable;

import com.vmware.identity.interop.ldap.ILdapConnectionEx;

public class PooledLdapConnection implements Closeable {
    private final ILdapConnectionEx connection;
    private final PooledLdapConnectionIdentity identity;
    private final LdapConnectionPool pool;

    public PooledLdapConnection(ILdapConnectionEx conn, PooledLdapConnectionIdentity identity, LdapConnectionPool pool) {
	this.connection = conn;
	this.identity = identity;
	this.pool = pool;
    }

    public LdapConnectionPool getPool() {
	return pool;
    }

    public ILdapConnectionEx getConnection() {
	return connection;
    }

    public PooledLdapConnectionIdentity getIdentity() {
	return identity;
    }

    @Override
    public void close() {
	pool.returnConnection(this);
    }
}
