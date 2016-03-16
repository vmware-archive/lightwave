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

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.performanceSupport.IIdmAuthStat;

public class PerformanceMonitorFactory {
	private static IPerformanceMonitor _perfMonitor = null;

	/**
	 * for unit-test to insert the mock IPerformanceMonitor.
	 */
	public static void setPerformanceMonitor(IPerformanceMonitor mon) {
		_perfMonitor = mon;
	}

	/**
	 * @return default PerformanceMonitor, the singleton.
	 */
	public static IPerformanceMonitor getPerformanceMonitor() {
		return (_perfMonitor != null) ? _perfMonitor : PerformanceMonitor
				.getInstance();
	}

	/**
	 * Create an IIdmAuthStatRecorder instance.
	 */
    public static IIdmAuthStatRecorder createIdmAuthStatRecorderInstance(
            String tenantName, String providerType, String providerId, int providerFlags,
            IIdmAuthStat.ActivityKind opType, IIdmAuthStat.EventLevel eventLevel, String userId) {
        if (PerformanceMonitorFactory.getPerformanceMonitor().getCache(tenantName).isEnabled()) {
            return new IdmAuthStatRecorder(
                    tenantName,
                    providerType,
                    providerId,
                    providerFlags,
                    opType,
                    eventLevel,
                    userId,
                    PerformanceMonitorFactory.getPerformanceMonitor().summarizeLdapQueries(),
                    DiagnosticsContextFactory.getCurrentDiagnosticsContext().getCorrelationId());
        } else {
            return NoopIdmAuthStatRecorder.getInstance();
        }
    }
}