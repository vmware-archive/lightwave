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
package com.vmware.identity.rest.afd.server.resources;

import java.io.IOException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.afd.server.util.Config;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.provider.VecsLoadStoreParameter;

/**
 * A resource for vecs operations
 */
@Path("/vecs")
public class VecsResource extends BaseResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(VecsResource.class);

    public VecsResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @Path("/ssl")
    public Collection<CertificateDTO> getSSLCertificates() {
        try {
            KeyStore keystore = KeyStore.getInstance("VKS");
            keystore.load(new VecsLoadStoreParameter("TRUSTED_ROOTS"));

            Enumeration<String> aliases = keystore.aliases();
            List<CertificateDTO> sslCerts = new ArrayList<CertificateDTO>();
            while (aliases.hasMoreElements()) {
                String alias = aliases.nextElement();
                Certificate cert = keystore.getCertificate(alias);

                CertificateDTO certDTO = new CertificateDTO((X509Certificate) cert);
                sslCerts.add(certDTO);
            }

            return sslCerts;
        } catch (CertificateException | IOException | KeyStoreException | NoSuchAlgorithmException e) {
            log.error("Error loading the VECS key store", e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

}
