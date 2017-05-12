/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.idm.server.scim.ldap;

import java.util.HashMap;

import com.unboundid.scim.sdk.SCIMFilterType;
import org.apache.commons.lang.StringUtils;

import com.unboundid.scim.sdk.SCIMException;
import com.unboundid.scim.sdk.SCIMFilter;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.server.scim.AttributeUndefinedException;
import com.vmware.identity.idm.server.scim.SearchQueryConverter;

public class LdapSearchQueryConverter implements SearchQueryConverter {

    private static IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(LdapSearchQueryConverter.class);
    private HashMap<String, LdapAttributeMapper> mappings = new HashMap<>();

    public static void constructPredicate(StringBuilder sb, String attribute, SCIMFilterType type, String value) {
        constructPredicate(sb, attribute, type, value, false);
    }

    public static void constructPredicate(StringBuilder sb, String attribute, SCIMFilterType type, String value, boolean invert) {
        if (invert) {
            sb.append("(!");
        }
        sb.append("(").append(attribute);
        constructComparison(sb, type, value);
        sb.append(")");
        if (invert) {
            sb.append(")");
        }
    }

    private static void constructComparison(StringBuilder sb, SCIMFilterType type, String value) {
        switch (type) {
            case EQUALITY:
                sb.append("=").append(value);
                return;
            case CONTAINS:
                sb.append("=*").append(value).append("*");
                return;
            case PRESENCE:
                sb.append("=*");
                return;
            case STARTS_WITH:
                sb.append("=").append(value).append("*");
                return;
            case GREATER_THAN:
                sb.append(">").append(value);
                return;
            case GREATER_OR_EQUAL:
                sb.append(">=").append(value);
                return;
            case LESS_THAN:
                sb.append("<").append(value);
                return;
            case LESS_OR_EQUAL:
                sb.append("<=").append(value);
        }
    }

    private static SCIMFilter parseFilter(String filter) {
        try {
            return SCIMFilter.parse(filter);
        } catch (SCIMException e) {
            logger.debug("Unable to parse " + filter, e);
            throw new IllegalArgumentException("Invalid SCIM filter: '" + filter + "'. Message: " + e.getMessage());
        }
    }

    private static String getAttributeName(SCIMFilter filter) throws AttributeUndefinedException {
        String name = filter.getFilterAttribute().getAttributeName();
        String subName = filter.getFilterAttribute().getSubAttributeName();
        if (StringUtils.isNotBlank(subName)) {
            name = name + "." + subName;
        }
       return name;
    }

    public void setMapping(String attribute, String replacement) {
        setMapping(attribute, ((sb, type, value) -> constructPredicate(sb, replacement, type, value)));
    }

    public void setMapping(String attribute, LdapAttributeMapper mapping) {
        mappings.put(attribute.toLowerCase(), mapping);
    }

    @Override
    public ProcessedFilter convert(String filter) throws AttributeUndefinedException {
        return convert(parseFilter(filter));
    }

    @Override
    public ProcessedFilter convert(SCIMFilter filter) throws AttributeUndefinedException {
        String ldap = filter != null ? createLdapQuery(filter) : null;
        return new LdapProcessedFilter(ldap);
    }

    private String createLdapQuery(SCIMFilter filter) throws AttributeUndefinedException {
        StringBuilder sb = new StringBuilder();
        createFilter(sb, filter);
        return sb.toString();
    }

    private void createFilter(StringBuilder sb, SCIMFilter filter) throws AttributeUndefinedException {
        SCIMFilterType type = filter.getFilterType();
        switch (type) {
            case AND:
                sb.append("(&"); createFilter(sb, filter.getFilterComponents().get(0)); createFilter(sb, filter.getFilterComponents().get(1)); sb.append(")");
                return;
            case OR:
                sb.append("(|"); createFilter(sb, filter.getFilterComponents().get(0)); createFilter(sb, filter.getFilterComponents().get(1)); sb.append(")");
                return;
            case PRESENCE:
            case EQUALITY:
            case CONTAINS:
            case STARTS_WITH:
            case GREATER_THAN:
            case GREATER_OR_EQUAL:
            case LESS_THAN:
            case LESS_OR_EQUAL:
                comparisonClause(sb, filter, type);
                return;
        }
    }

    private void comparisonClause(StringBuilder sb, SCIMFilter filter, SCIMFilterType type) throws AttributeUndefinedException {
        String attribute = getAttributeName(filter);
        LdapAttributeMapper mapping = mappings.get(attribute.toLowerCase());
        if (mapping != null) {
            mapping.map(sb, type, filter.getFilterValue());
        } else {
            throw new AttributeUndefinedException("No LDAP mapping defined for '" + attribute + "'");
        }
    }

    @FunctionalInterface
    public interface LdapAttributeMapper {
        void map(StringBuilder sb, SCIMFilterType type, String value);
    }

    public class LdapProcessedFilter implements ProcessedFilter {

        private final String ldap;

        private LdapProcessedFilter(String ldap) {
            this.ldap = ldap;
        }

        @Override
        public String getConvertedFilter() {
            return ldap;
        }

        @Override
        public String toString() {
            return String.format("ldap: '%s'", ldap);
        }

    }

}
