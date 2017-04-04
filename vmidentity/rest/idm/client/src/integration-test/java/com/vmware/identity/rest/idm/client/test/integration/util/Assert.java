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
package com.vmware.identity.rest.idm.client.test.integration.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import java.util.List;

import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.UserDTO;

public class Assert {

    public static void assertClientsEqual(OIDCClientDTO expected, OIDCClientDTO actual) {
        assertEquals(expected.getClientId(), actual.getClientId());
        assertMetadataEqual(expected.getOIDCClientMetadataDTO(), actual.getOIDCClientMetadataDTO());
    }

    public static void assertContainsCertificates(List<CertificateDTO> expected, List<CertificateDTO> actual) {
        for (CertificateDTO expect : expected) {
            boolean found = false;
            for (CertificateDTO act : actual) {
                if (act.getEncoded().equals(expect.getEncoded())) {
                    found = true;
                }
            }

            if (!found) {
                fail("Unable to find the expected certificate");
            }
        }
    }

    public static void assertContainsClient(OIDCClientDTO expected, List<OIDCClientDTO> actual) {
        for (OIDCClientDTO c : actual) {
            if (expected.getClientId().equals(c.getClientId())) {
                assertMetadataEqual(expected.getOIDCClientMetadataDTO(), c.getOIDCClientMetadataDTO());
                return;
            }
        }

        fail("Unable to find the expected client");
    }

    public static void assertContainsResourceServer(ResourceServerDTO expected, List<ResourceServerDTO> actual) {
        for (ResourceServerDTO rs : actual) {
            if (expected.getName().equals(rs.getName())) {
                assertResourceServersEqual(expected, rs);
                return;
            }
        }

        fail("Unable to find the expected resource server");
    }

    public static void assertContainsGroup(GroupDTO expected, List<GroupDTO> actual) {
        for (GroupDTO g : actual) {
            if (expected.getName().equals(g.getName()) && expected.getDomain().equals(g.getDomain())) {
                assertGroupsEqual(expected, g);
                return;
            }
        }

        fail("Unable to find expected Group");
    }

    public static void assertContainsRP(RelyingPartyDTO expected, List<RelyingPartyDTO> actual) {
        for (RelyingPartyDTO r : actual) {
            if (expected.getName().equals(r.getName())) {
                assertRPsEqual(expected, r);
                return;
            }
        }

        fail("Unable to find expected Relying Party");
    }

    public static void assertContainsUser(UserDTO expected, List<UserDTO> actual) {
        for (UserDTO u : actual) {
            if (expected.getName().equals(u.getName()) && expected.getDomain().equals(u.getDomain())) {
                assertUsersEqual(expected, u);
                return;
            }
        }

        fail("Unable to find expected User");
    }

    public static void assertExternalIDPsEqual(ExternalIDPDTO expected, ExternalIDPDTO actual) {
        assertEquals(expected.getEntityID(), actual.getEntityID());
        assertEquals(expected.getNameIDFormats(), actual.getNameIDFormats());
        assertEquals(expected.getSubjectFormats(), actual.getSubjectFormats());
        assertEquals(expected.isJitEnabled(), actual.isJitEnabled());
        assertEquals(expected.getUpnSuffix(), actual.getUpnSuffix());
    }

    public static void assertGroupsEqual(GroupDTO expected, GroupDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDetails().getDescription(), actual.getDetails().getDescription());
    }

    public static void assertMetadataEqual(OIDCClientMetadataDTO expected, OIDCClientMetadataDTO actual) {
        assertEquals(expected.getRedirectUris(), actual.getRedirectUris());
        assertEquals(expected.getTokenEndpointAuthMethod(), actual.getTokenEndpointAuthMethod());
        assertEquals(expected.getPostLogoutRedirectUris(), actual.getPostLogoutRedirectUris());
        assertEquals(expected.getLogoutUri(), actual.getLogoutUri());
        assertEquals(expected.getCertSubjectDN(), actual.getCertSubjectDN());
        assertEquals(expected.getAuthnRequestClientAssertionLifetimeMS(), actual.getAuthnRequestClientAssertionLifetimeMS());
    }

    public static void assertResourceServersEqual(ResourceServerDTO expected, ResourceServerDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getGroupFilter(), actual.getGroupFilter());
    }

    public static void assertRPsEqual(RelyingPartyDTO expected, RelyingPartyDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getUrl(), actual.getUrl());
    }

    public static void assertSolutionUsersEqual(SolutionUserDTO expected, SolutionUserDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDescription(), actual.getDescription());
        assertEquals(expected.getCertificate().getEncoded(), actual.getCertificate().getEncoded());
    }

    public static void assertTenantEquals(TenantDTO expected, TenantDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getLongName(), expected.getLongName());
    }

    public static void assertUsersEqual(UserDTO expected, UserDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDetails().getDescription(), actual.getDetails().getDescription());
        assertEquals(expected.getDetails().getFirstName(), actual.getDetails().getFirstName());
        assertEquals(expected.getDetails().getLastName(), actual.getDetails().getLastName());
        assertEquals(expected.getDetails().getEmail(), actual.getDetails().getEmail());
    }

    public static void assertUsersEqual(com.vmware.directory.rest.common.data.UserDTO expected, UserDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDetails().getDescription(), actual.getDetails().getDescription());
        assertEquals(expected.getDetails().getFirstName(), actual.getDetails().getFirstName());
        assertEquals(expected.getDetails().getLastName(), actual.getDetails().getLastName());
        assertEquals(expected.getDetails().getEmail(), actual.getDetails().getEmail());
    }

    public static void assertVmdirUsersEqual(com.vmware.directory.rest.common.data.UserDTO expected, com.vmware.directory.rest.common.data.UserDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDetails().getDescription(), actual.getDetails().getDescription());
        assertEquals(expected.getDetails().getFirstName(), actual.getDetails().getFirstName());
        assertEquals(expected.getDetails().getLastName(), actual.getDetails().getLastName());
        assertEquals(expected.getDetails().getEmail(), actual.getDetails().getEmail());
    }

}
