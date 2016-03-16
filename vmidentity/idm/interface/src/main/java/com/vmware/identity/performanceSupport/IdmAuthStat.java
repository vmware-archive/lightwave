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

import java.util.List;
import java.util.Map;

public class IdmAuthStat implements IIdmAuthStat {
    private static final long serialVersionUID = 1L;
    private String _providerType;
    private String _providerName;
    private int _providerFlag;
    private ActivityKind _opKind;
    private EventLevel _eventLevel;
    private String _userName;
    private long _startTime;
    private long _timeTaken;
    private String _correlationId;
    private List<ILdapQueryStat> _ldapStats;
    private Map<String, String> _extensions;

    public IdmAuthStat(String providerType, String providerName,
            int providerFlag, ActivityKind opKind, EventLevel eventLevel, String userName,
            long startTime, long timeTaken, String correlationId, List<ILdapQueryStat> ldapStats, Map<String, String> ext) {

        this._providerType = providerType;

        this._providerName = providerName;

        this._providerFlag = providerFlag;

        this._opKind = opKind;

        this._eventLevel = eventLevel;

        this._userName = userName;

        this._startTime = startTime;

        this._timeTaken = timeTaken;

        this._correlationId = correlationId;

        this._ldapStats = ldapStats;

        this._extensions = ext;

    }

    @Override
    public String getProviderType() {
        return this._providerType;
    }

    @Override
    public String getProviderName() {
        return this._providerName;
    }

    @Override
    public int getProviderFlag() {
        return this._providerFlag;
    }

    @Override
    public long getTimeTaken() {
        return this._timeTaken;
    }

    @Override
    public ActivityKind getOpKind() {
        return this._opKind;
    }

    @Override
    public EventLevel getEventLevel() {
        return this._eventLevel;
    }

    @Override
    public String getUserName() {
        return this._userName;
    }

    @Override
    public long getStartTime() {
        return this._startTime;
    }

    @Override
    public String getCorrelationId() {
        return this._correlationId;
    }

    @Override
    public List<ILdapQueryStat> getLdapQueryStats() {
        return this._ldapStats;
    }

    @Override
    public Map<String, String> getExtensions() {
        return this._extensions;
    }

}
