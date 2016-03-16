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

import java.util.ArrayList;
import java.util.Collection;

import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;

/**
 * Mapper utility to map objects from {@link OIDCClient} to {@link OIDCClientDTO} and vice-versa.
 */
public class OIDCClientMapper {

    public static OIDCClient getOIDCClient(OIDCClientDTO oidcClientDTO) {

        OIDCClient oidcClient = null;
        try {
            Long lifetimeMS = oidcClientDTO.getOIDCClientMetadataDTO().getAuthnRequestClientAssertionLifetimeMS();
            oidcClient = new OIDCClient.Builder(oidcClientDTO.getClientId()).
                             redirectUris(oidcClientDTO.getOIDCClientMetadataDTO().getRedirectUris()).
                             tokenEndpointAuthMethod(oidcClientDTO.getOIDCClientMetadataDTO().getTokenEndpointAuthMethod()).
                             postLogoutRedirectUris(oidcClientDTO.getOIDCClientMetadataDTO().getPostLogoutRedirectUris()).
                             logoutUri(oidcClientDTO.getOIDCClientMetadataDTO().getLogoutUri()).
                             certSubjectDN(oidcClientDTO.getOIDCClientMetadataDTO().getCertSubjectDN()).
                             authnRequestClientAssertionLifetimeMS((lifetimeMS == null) ? 0L : lifetimeMS.longValue()).build();
        } catch(Exception e) {
            throw new DTOMapperException("Failed to map OIDCClientDTO to OIDCClient", e);
        }
        return oidcClient;
    }

    public static OIDCClient getOIDCClient(OIDCClientMetadataDTO oidcClientMetadataDTO) {
        OIDCClient oidcClient  = null;
        try {
            Long lifetimeMS = oidcClientMetadataDTO.getAuthnRequestClientAssertionLifetimeMS();
            oidcClient = new OIDCClient.Builder(null).
                         redirectUris(oidcClientMetadataDTO.getRedirectUris()).
                         tokenEndpointAuthMethod(oidcClientMetadataDTO.getTokenEndpointAuthMethod()).
                         postLogoutRedirectUris(oidcClientMetadataDTO.getPostLogoutRedirectUris()).
                         logoutUri(oidcClientMetadataDTO.getLogoutUri()).
                         certSubjectDN(oidcClientMetadataDTO.getCertSubjectDN()).
                         authnRequestClientAssertionLifetimeMS((lifetimeMS == null) ? 0L : lifetimeMS.longValue()).build();
        } catch(Exception e) {
            throw new DTOMapperException("Failed to map OIDCClientMetadataDTO to OIDCClient", e);
        }
        return oidcClient;
    }

    public static OIDCClientDTO getOIDCClientDTO(OIDCClient oidcClient) {
        OIDCClientDTO oidcClientDTO = null;
        try {
            oidcClientDTO = new OIDCClientDTO.Builder().
                            withClientId(oidcClient.getClientId()).
                            withOIDCClientMetadataDTO(new OIDCClientMetadataDTO.Builder().
                            withRedirectUris(oidcClient.getRedirectUris()).
                            withTokenEndpointAuthMethod(oidcClient.getTokenEndpointAuthMethod()).
                            withPostLogoutRedirectUris(oidcClient.getPostLogoutRedirectUris()).
                            withLogoutUri(oidcClient.getLogoutUri()).
                            withCertSubjectDN(oidcClient.getCertSubjectDN()).
                            withAuthnRequestClientAssertionLifetimeMS(oidcClient.getAuthnRequestClientAssertionLifetimeMS()).build()).
                            build();
        } catch(Exception e) {
            throw new DTOMapperException("Failed to map OIDCClient to OIDCClientDTO", e);
        }
        return oidcClientDTO;
    }

    public static Collection<OIDCClientDTO> getOIDCClientDTOs(Collection<OIDCClient> oidcClients) {
        Collection<OIDCClientDTO> oidcClientDTOs = new ArrayList<OIDCClientDTO>();
        for (OIDCClient oidcClient : oidcClients) {
            oidcClientDTOs.add(getOIDCClientDTO(oidcClient));
        }
        return oidcClientDTOs;
    }
}
