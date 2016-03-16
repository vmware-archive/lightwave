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

import java.io.Serializable;
import java.util.List;
import java.util.Map;

public interface IIdmAuthStat extends Serializable {

    public enum ActivityKind {
        AUTHENTICATE, GETATTRIBUTES,
    }

    public enum EventLevel {
        FATAL,
        ERROR,
        WARN,
        INFO,
        DEBUG,
        TRACE,
        ALL
    }

    public String getProviderType();

    public String getProviderName();

    public int getProviderFlag();

    ActivityKind getOpKind();

    EventLevel getEventLevel();

    String getUserName();

    long getStartTime();

    long getTimeTaken();

    String getCorrelationId();

    List<ILdapQueryStat> getLdapQueryStats();

    /**
     * Gets the extended authentication logging items, which is keyed by the extra item name,
     * for example: for CAC authentication, it has extensions: buildCertPath, validateCertPath, etc.
     */
    Map<String, String> getExtensions();
}
