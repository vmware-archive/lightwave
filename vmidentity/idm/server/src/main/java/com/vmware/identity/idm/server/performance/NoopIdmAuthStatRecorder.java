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

package com.vmware.identity.idm.server.performance;

import java.util.List;
import java.util.Map;

import com.vmware.identity.performanceSupport.ILdapQueryStat;

public class NoopIdmAuthStatRecorder implements IIdmAuthStatRecorder {

    private static NoopIdmAuthStatRecorder _instance = new NoopIdmAuthStatRecorder();

    private NoopIdmAuthStatRecorder() {
    }

    public static NoopIdmAuthStatRecorder getInstance() {
        return _instance;
    }

    @Override
    public void start() {
    }

    @Override
    public void add(List<ILdapQueryStat> ldapQueries) {
    }

    @Override
    public void add(ILdapQueryStat ldapQuery) {
    }

    @java.lang.Override
    public void add(Map<String, String> ext) {
    }

    @Override
    public void end() {
    }

    @Override
    public boolean summarizeLdapQueries() {
        return false;
    }
}
