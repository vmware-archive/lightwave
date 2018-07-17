/*
 *
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
 *
 */

package com.vmware.identity.idm.server.utils;

import com.vmware.identity.idm.server.provider.IPooledConnectionProvider;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PooledLdapConnectionIdentity;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;

import org.easymock.EasyMock;

public class PooledConnectionProviderMock implements IPooledConnectionProvider, AutoCloseable {

    ILdapConnectionEx conn;

    public PooledConnectionProviderMock(IMockInitializer<ILdapConnectionEx> init) {
        this.conn = EasyMock.<ILdapConnectionEx>createMock(ILdapConnectionEx.class);
        if (init != null){
            init.initialize(this.conn);
        }
        EasyMock.replay(this.conn);
    }

	@Override
	public PooledLdapConnection borrowConnection(PooledLdapConnectionIdentity identity) throws Exception {
		return new PooledLdapConnection(this.conn, identity, this);
	}

	@Override
	public void returnConnection(PooledLdapConnection pooledConnection) {
        // noop
	}

	@Override
	public void close() throws Exception {
		EasyMock.verify(this.conn);
	}
}