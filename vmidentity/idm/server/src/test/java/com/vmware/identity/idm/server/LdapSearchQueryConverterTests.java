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
package com.vmware.identity.idm.server;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import com.unboundid.scim.sdk.SCIMFilterType;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import org.junit.Before;
import org.junit.Test;

import com.unboundid.scim.sdk.SCIMException;
import com.unboundid.scim.sdk.SCIMFilter;
import com.vmware.identity.idm.server.scim.AttributeUndefinedException;
import com.vmware.identity.idm.server.scim.SearchQueryConverter.ProcessedFilter;
import com.vmware.identity.idm.server.scim.ldap.LdapSearchQueryConverter;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

public class LdapSearchQueryConverterTests {

    private LdapSearchQueryConverter converter;

    @Before
    public void setup() {
        converter = new LdapSearchQueryConverter();
        converter.setMapping("id", "sAMAccountName");
        converter.setMapping("externalid", "cn");
        converter.setMapping("username", "cn");
        converter.setMapping("name.familyname", "sn");
        converter.setMapping("name.givenname", "givenName");
        converter.setMapping("displayname", "displayname");
    }

    @Test
    public void canConvertValidStringFilters() throws AttributeUndefinedException {
        validate(converter.convert("username pr"), "(cn=*)");
        validate(converter.convert("username eq \"james\""), "(cn=james)");
        validate(converter.convert("username eq \"\""), "(cn=)");
        validate(converter.convert("USERNAME eq \"james\""), "(cn=james)");
        validate(converter.convert("username eq \"james\" and name.familyname eq \"bond\""), "(&(cn=james)(sn=bond))");
        validate(converter.convert("username eq \"james\" or name.familyname eq \"bond\""), "(|(cn=james)(sn=bond))");
        validate(converter.convert("(username eq \"james\" and name.familyname eq \"bond\") or (username eq \"ernst\" and name.familyname eq \"blofeld\")"), "(|(&(cn=james)(sn=bond))(&(cn=ernst)(sn=blofeld)))");
        validate(converter.convert("username gt \"james\""), "(cn>james)");
        validate(converter.convert("username ge \"james\""), "(cn>=james)");
        validate(converter.convert("username lt \"james\""), "(cn<james)");
        validate(converter.convert("username le \"james\""), "(cn<=james)");
        validate(converter.convert("username sw \"james\""), "(cn=james*)");
        validate(converter.convert("username co \"james\""), "(cn=*james*)");
    }

    @Test
    public void canConvertValidSCIMFilters() throws SCIMException, AttributeUndefinedException {
        validate(converter.convert(toScim("username pr")), "(cn=*)");
        validate(converter.convert(toScim("username eq \"james\"")), "(cn=james)");
        validate(converter.convert(toScim("username eq \"\"")), "(cn=)");
        validate(converter.convert(toScim("USERNAME eq \"james\"")), "(cn=james)");
        validate(converter.convert(toScim("username eq \"james\" and name.familyname eq \"bond\"")), "(&(cn=james)(sn=bond))");
        validate(converter.convert(toScim("username eq \"james\" or name.familyname eq \"bond\"")), "(|(cn=james)(sn=bond))");
        validate(converter.convert(toScim("(username eq \"james\" and name.familyname eq \"bond\") or (username eq \"ernst\" and name.familyname eq \"blofeld\")")), "(|(&(cn=james)(sn=bond))(&(cn=ernst)(sn=blofeld)))");
        validate(converter.convert(toScim("username gt \"james\"")), "(cn>james)");
        validate(converter.convert(toScim("username ge \"james\"")), "(cn>=james)");
        validate(converter.convert(toScim("username lt \"james\"")), "(cn<james)");
        validate(converter.convert(toScim("username le \"james\"")), "(cn<=james)");
        validate(converter.convert(toScim("username sw \"james\"")), "(cn=james*)");
        validate(converter.convert(toScim("username co \"james\"")), "(cn=*james*)");
    }

    @Test(expected = AttributeUndefinedException.class)
    public void convertUndefinedAttribute() throws AttributeUndefinedException {
        converter.convert("members eq \"james\"");
    }

    @Test
    public void complexUserConversions() throws SCIMException, AttributeUndefinedException {
        String domain = "mi6.com";
        List<String> alternateDomains = Arrays.asList("cia.gov");
        LdapSearchQueryConverter converter = new LdapSearchQueryConverter();
        converter.setMapping("username", "cn");
        converter.setMapping("id", (sb, type, value) -> {
            String[] components = ServerUtils.splitNameAndDomain(value);

            ProviderLocation location = findProviderLocation(type, components[1], domain, alternateDomains);

            // If we should never match, we'll sabotage and bail out early
            if (location == ProviderLocation.NONE) {
                LdapSearchQueryConverter.constructPredicate(sb,
                        "dn",
                        SCIMFilterType.EQUALITY, "*", true);
                return;
            }

            if (location == ProviderLocation.DOMAIN) {
                sb.append("(|");
            }

            LdapSearchQueryConverter.constructPredicate(sb,
                    "upn",
                    type, value);

            if (location == ProviderLocation.DOMAIN) {
                LdapSearchQueryConverter.constructPredicate(sb,
                        "samAccountName",
                        type, components[0]);

                sb.append(")");
            }
        });

        validate(converter.convert(toScim("id eq \"james@mi6.com\"")), "(|(upn=james@mi6.com)(samAccountName=james))");
        validate(converter.convert(toScim("id eq \"james@cia.gov\"")), "(upn=james@cia.gov)");
        validate(converter.convert(toScim("id sw \"james\"")), "(|(upn=james*)(samAccountName=james*))");
        validate(converter.convert(toScim("id sw \"james@\"")), "(|(upn=james@*)(samAccountName=james*))");
        validate(converter.convert(toScim("id sw \"james@mi\"")), "(|(upn=james@mi*)(samAccountName=james*))");
        validate(converter.convert(toScim("id sw \"james@cia\"")), "(upn=james@cia*)");
        validate(converter.convert(toScim("id sw \"james@gibber\"")), "(!(dn=*))");
        validate(converter.convert(toScim("id co \"james\"")), "(|(upn=*james*)(samAccountName=*james*))");
        validate(converter.convert(toScim("id co \"james@\"")), "(|(upn=*james@*)(samAccountName=*james*))");
        validate(converter.convert(toScim("id co \"james@mi\"")), "(|(upn=*james@mi*)(samAccountName=*james*))");
        validate(converter.convert(toScim("id co \"james@cia\"")), "(upn=*james@cia*)");
        validate(converter.convert(toScim("id co \"james@gibber\"")), "(!(dn=*))");
        validate(converter.convert(toScim("id eq \"james\"")), "(!(dn=*))");
        validate(converter.convert(toScim("(id eq \"james\" or id eq \"james@mi6.com\")")), "(|(!(dn=*))(|(upn=james@mi6.com)(samAccountName=james)))");
        validate(converter.convert(toScim("(id eq \"james\" and username eq \"james\") or (id co \"james\")")), "(|(&(!(dn=*))(cn=james))(|(upn=*james*)(samAccountName=*james*)))");
    }

    @Test
    public void complexGroupConversions() throws SCIMException, AttributeUndefinedException {
        String domain = "mi6.com";
        List<String> alternateDomains = Arrays.asList("cia.gov");
        LdapSearchQueryConverter converter = new LdapSearchQueryConverter();
        converter.setMapping("displayname", "cn");
        converter.setMapping("id", (sb, type, value) -> {
            String[] components = ServerUtils.splitNameAndDomain(value);
            ProviderLocation location = findProviderLocation(type, components[1], domain, alternateDomains);

            // If we should never match, we'll sabotage and bail out early
            if (location == ProviderLocation.NONE) {
                LdapSearchQueryConverter.constructPredicate(sb,
                        "dn",
                        SCIMFilterType.EQUALITY, "*", true);
            } else {
                // Default to the regular value so that we handle 'sw' or 'co' queries that do not include a domain
                // otherwise use only the name component
                LdapSearchQueryConverter.constructPredicate(sb,
                        "samAccountName",
                        type, components[0]);
            }
        });

        validate(converter.convert(toScim("id eq \"james@mi6.com\"")), "(samAccountName=james)");
        validate(converter.convert(toScim("id eq \"james@cia.gov\"")), "(samAccountName=james)");
        validate(converter.convert(toScim("id sw \"james\"")), "(samAccountName=james*)");
        validate(converter.convert(toScim("id sw \"james@\"")), "(samAccountName=james*)");
        validate(converter.convert(toScim("id sw \"james@mi\"")), "(samAccountName=james*)");
        validate(converter.convert(toScim("id sw \"james@cia\"")), "(samAccountName=james*)");
        validate(converter.convert(toScim("id sw \"james@gibber\"")), "(!(dn=*))");
        validate(converter.convert(toScim("id co \"james\"")), "(samAccountName=*james*)");
        validate(converter.convert(toScim("id co \"james@\"")), "(samAccountName=*james*)");
        validate(converter.convert(toScim("id co \"james@mi\"")), "(samAccountName=*james*)");
        validate(converter.convert(toScim("id co \"james@cia\"")), "(samAccountName=*james*)");
        validate(converter.convert(toScim("id co \"james@gibber\"")), "(!(dn=*))");
        validate(converter.convert(toScim("id eq \"james\"")), "(!(dn=*))");
        validate(converter.convert(toScim("(id eq \"james\" or id eq \"james@mi6.com\")")), "(|(!(dn=*))(samAccountName=james))");
        validate(converter.convert(toScim("(id eq \"james\" and displayname eq \"james\") or (id co \"james\")")), "(|(&(!(dn=*))(cn=james))(samAccountName=*james*))");
    }

    @Test
    public void emptyConversion() throws SCIMException, AttributeUndefinedException {
        LdapSearchQueryConverter converter = new LdapSearchQueryConverter();
        converter.setMapping("username", (sb, type, value) -> {});
        converter.setMapping("id", "upn");

        validate(converter.convert(toScim("username eq \"james\"")), "");
        validate(converter.convert(toScim("id eq \"james@mi6.com\" and username eq \"james\"")), "(&(upn=james@mi6.com))");
    }

    private static SCIMFilter toScim(String filter) throws SCIMException {
        return SCIMFilter.parse(filter);
    }

    private static void validate(ProcessedFilter filter, String expected) {
        assertNotNull(filter);
        assertEquals(expected, filter.getConvertedFilter());
    }

    private enum ProviderLocation {
        DOMAIN,
        PROVIDER,
        NONE
    }

    private static ProviderLocation findProviderLocation(SCIMFilterType type, String sequence, String actualDomain, Collection<String> upnSuffixes) {
        ProviderLocation location = ProviderLocation.NONE;
        switch (type) {
            case EQUALITY:
                location = findProviderLocationEquals(sequence, actualDomain, upnSuffixes);
                break;
            case CONTAINS:
                location = findProviderLocationContains(sequence, actualDomain, upnSuffixes);
                break;
            case STARTS_WITH:
                location = findProviderLocationStartsWith(sequence, actualDomain, upnSuffixes);
                break;
        }

        return location;
    }

    private static ProviderLocation findProviderLocationContains(String sequence, String actualDomain, Collection<String> upnSuffixes) {
        final String downCasedSequence = sequence.toLowerCase(); // Downcase for case-insensitive comparisons
        if (actualDomain.toLowerCase().contains(downCasedSequence)) {
            return ProviderLocation.DOMAIN;
        }

        if (upnSuffixes != null) {
            if (upnSuffixes.stream().anyMatch(s -> s.toLowerCase().contains(downCasedSequence))) {
                return ProviderLocation.PROVIDER;
            }
        }

        return ProviderLocation.NONE;
    }

    private static ProviderLocation findProviderLocationEquals(String domain, String actualDomain, Collection<String> upnSuffixes) {
        if (actualDomain.equalsIgnoreCase(domain)) {
            return ProviderLocation.DOMAIN;
        }

        if (upnSuffixes != null) {
            if (upnSuffixes.stream().anyMatch(s -> s.equalsIgnoreCase(domain))) {
                return ProviderLocation.PROVIDER;
            }
        }

        return ProviderLocation.NONE;
    }

    private static ProviderLocation findProviderLocationStartsWith(String prefix, String actualDomain, Collection<String> upnSuffixes) {
        final String downCasedPrefix = prefix.toLowerCase(); // Downcase for case-insensitive comparisons
        if (actualDomain.toLowerCase().startsWith(downCasedPrefix)) {
            return ProviderLocation.DOMAIN;
        }

        if (upnSuffixes != null) {
            if (upnSuffixes.stream().anyMatch(s -> s.toLowerCase().startsWith(downCasedPrefix))) {
                return ProviderLocation.PROVIDER;
            }
        }

        return ProviderLocation.NONE;
    }

}
