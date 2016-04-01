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
package com.vmware.identity.rest.idm.server.mapper;

import static com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper.isBaseDnForNestedGroupsEnabled;
import static com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper.isDirectGroupsSearchEnabled;
import static com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper.isMatchingRuleInChainEnabled;
import static com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper.isSiteAffinityEnabled;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.identity.performanceSupport.IIdmAuthStat;
import com.vmware.identity.performanceSupport.IIdmAuthStatus;
import com.vmware.identity.performanceSupport.ILdapQueryStat;
import com.vmware.identity.rest.idm.data.EventLogDTO;
import com.vmware.identity.rest.idm.data.EventLogStatusDTO;

/**
 * Mapper utility to map objects into a {@link EventLogDTO}.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class EventLogMapper {

    public static EventLogDTO getEventLog(IIdmAuthStat stat) {
        EventLogDTO.Builder builder = EventLogDTO.builder();

        Map<String, Object> metadata = getMetadata(stat);

        return builder.withType(stat.getOpKind().toString())
               .withLevel(stat.getEventLevel().toString())
               .withCorrelationId(stat.getCorrelationId())
               .withStart(stat.getStartTime())
               .withElapsedMillis(stat.getTimeTaken())
               .withMetadata(metadata)
               .build();
    }

    public static List<EventLogDTO> getEventLogs(List<IIdmAuthStat> stats) {
        List<EventLogDTO> logs = new ArrayList<EventLogDTO>(stats.size());
        for (IIdmAuthStat stat : stats) {
            logs.add(getEventLog(stat));
        }
        return logs;
    }

    public static EventLogStatusDTO getEventLogStatus(IIdmAuthStatus status) {
        EventLogStatusDTO.Builder builder = EventLogStatusDTO.builder();

        return builder.withEnabled(status.isEnabled())
                .withSize((long) status.getSize())
                .build();
    }

    private static Map<String, Object> getMetadata(IIdmAuthStat stat) {
        Map<String, Object> metadata = new HashMap<String, Object>();

        metadata.put("username", stat.getUserName());
        metadata.put("provider", getProviderMetadata(stat));
        if (stat.getLdapQueryStats() != null && !stat.getLdapQueryStats().isEmpty()) {
            metadata.put("ldapQueryStats", getLdapQueryStats(stat.getLdapQueryStats()));
        }
        if (stat.getExtensions() != null && !stat.getExtensions().isEmpty()) {
            metadata.put("extensions", stat.getExtensions());
        }

        return metadata;
    }

    private static Map<String, Object> getProviderMetadata(IIdmAuthStat stat) {
        Map<String, Object> metadata = new HashMap<String, Object>();
        metadata.put("name", stat.getProviderName());
        metadata.put("type", stat.getProviderType());
        metadata.put("matchingRuleInChainEnabled", isMatchingRuleInChainEnabled(stat.getProviderFlag()));
        metadata.put("baseDnForNestedGroupsEnabled", isBaseDnForNestedGroupsEnabled(stat.getProviderFlag()));
        metadata.put("directGroupsSearchEnabled", isDirectGroupsSearchEnabled(stat.getProviderFlag()));
        metadata.put("siteAffinityEnabled", isSiteAffinityEnabled(stat.getProviderFlag()));
        return metadata;
    }

    private static List<Map<String, Object>> getLdapQueryStats(List<ILdapQueryStat> queryStats) {
        if (queryStats == null || queryStats.isEmpty()) {
            return null;
        }

        List<Map<String, Object>> stats = new ArrayList<Map<String, Object>>(queryStats.size());
        for (ILdapQueryStat queryStat : queryStats) {
            Map<String, Object> stat = new HashMap<String, Object>();
            stat.put("baseDN", queryStat.getBaseDN());
            stat.put("query", queryStat.getQueryString());
            stat.put("connection", queryStat.getConnectionString());
            stat.put("elapsedMillis", queryStat.getTimeTakenInMs());
            stat.put("count", queryStat.getCount());

            stats.add(stat);
        }

        return stats;
    }

}
