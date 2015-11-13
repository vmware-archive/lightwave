/*
 *
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
 *
 */

package com.vmware.identity.performanceSupport;

public class LdapQueryStat implements ILdapQueryStat {
    private static final long serialVersionUID = 1L;
    private long _timeTaken;
    private String _queryString;
    private String _baseDN;
    private String _connectionString;
    private int _count;

    public LdapQueryStat(String query, String baseDN, String conn, long time, int count) {
        this._queryString = query;
        this._baseDN = baseDN;
        this._connectionString = conn;
        this._timeTaken = time;
        this._count = count;
    }

    @Override
    public long getTimeTakenInMs() {
        return this._timeTaken;
    }

    @Override
    public String getQueryString() {
        return this._queryString;
    }

    @Override
    public String getBaseDN() {
        return this._baseDN;
    }

    @Override
    public String getConnectionString() {
        return this._connectionString;
    }

    @Override
    public int getCount() {
        return this._count;
    }
}
