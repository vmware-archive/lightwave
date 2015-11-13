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

import java.util.LinkedList;
import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

import com.vmware.identity.performanceSupport.IIdmAuthStat;

public class IdmAuthStatCache {
    private static IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmAuthStatCache.class);
    private LinkedList<IIdmAuthStat> _elements;
    private int _depth;

    public IdmAuthStatCache(int depth) {
        assert(depth >= 0);

        this._depth = depth;
        this._elements = new LinkedList<IIdmAuthStat>();
    }

    public void add(IIdmAuthStat stat) {
        assert(this._depth != 0);

        try {
            synchronized (this._elements) {
                if (this._elements.size() == this._depth) {
                    this._elements.removeFirst();
                }

                this._elements.addLast(stat);
            }
        } catch (Exception e) {
            logger.error("Failed to add IdmAuthStat, the error is Ignored.");
        }
    }

    public List<IIdmAuthStat> getIdmAuthStats() {
        synchronized(this._elements){
            List<IIdmAuthStat> result = new LinkedList<IIdmAuthStat>();
            result.addAll(this._elements);
            return result;
        }
    }
}
