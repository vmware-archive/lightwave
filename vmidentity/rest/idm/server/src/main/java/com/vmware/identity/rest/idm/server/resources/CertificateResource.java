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
package com.vmware.identity.rest.idm.server.resources;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Locale;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CertificateInUseException;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.DuplicateCertificateException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchCertificateException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateGranularity;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;
import com.vmware.identity.rest.idm.server.mapper.CertificateMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Operations related to Single Sign On certificates
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class CertificateResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(CertificateResource.class);

    public CertificateResource(String tenant, Locale locale, String correlationId, SecurityContext securityContext) {
        super(tenant, locale, correlationId, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    public CertificateResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Retrieve certificate(Public key) chains associated with tenant.
     *
     * @return Collection of certificate chains associated with tenant (In PEM encoded format). It
     *         is expected to return active and older certificates chains if any
     * @see CertificateDTO, CertificateChainDTO
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    public Collection<CertificateChainDTO> getCertificates(@QueryParam("scope") String certificateScope, @DefaultValue("CHAIN") @QueryParam("granularity") String granularity) {
        CertificateScope scope = validateCertificateScope(certificateScope);
        CertificateGranularity certGranularity = validateCertificateGranularity(granularity);
        try {
            Collection<List<Certificate>> idmCertChains = new ArrayList<List<Certificate>>();
            if (scope == CertificateScope.TENANT) {
                    if(certGranularity == CertificateGranularity.LEAF) {
                        // Retrieve all leaf certificates associated with tenant
                        Collection<Certificate> stsIssuersCertificates = getIDMClient().getStsIssuersCertificates(tenant);
                        List<Certificate> stsTrustedCerts = new ArrayList<Certificate>(stsIssuersCertificates);
                        idmCertChains.add(stsTrustedCerts);
                    } else {
                    // default to return all certificates chains associated with a tenant
                    idmCertChains = getIDMClient().getTenantCertificates(tenant);
                }
                // Get certificate chains of system provider, localos, and other directly registered external IDPs (AD or OpenLDAP)
            } else if (scope == CertificateScope.EXTERNAL_IDP) {
                // Get certificate chains of externalIDPs (trusted) associated with tenant if any
                Collection<IDPConfig> idpConfigs = getIDMClient().getAllExternalIdpConfig(tenant); // Retrieve configuration of all trusted external IDPs
                if (idpConfigs != null) {
                    for (IDPConfig idpConfig : idpConfigs) {
                        List<Certificate> certificates = new ArrayList<Certificate>();
                        for (Certificate c : idpConfig.getSigningCertificateChain()) {
                            certificates.add(c);
                        }
                        idmCertChains.add(certificates);
                    }
                }
            }

            Collection<CertificateChainDTO> certChainsOfTenant = new ArrayList<CertificateChainDTO>();
            for (List<Certificate> idmChain : idmCertChains) {
                List<CertificateDTO> chainOfCerts = CertificateMapper.getCertificateDTOs(idmChain);
                CertificateChainDTO certChainDTO = CertificateChainDTO.builder()
                                                                   .withCertificates(chainOfCerts)
                                                                   .build();
                certChainsOfTenant.add(certChainDTO);
            }

            return certChainsOfTenant;
        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve certificates for tenant '{}'", tenant);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve certificates for tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.cert.get.public.cert.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve certificates for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Retrieve Signing Certificate(Private key) associated with tenant.
     *
     * @return details of private key associated with tenant (base-64 encoded)
     * @see PrivateKeyDTO
     */
    @GET @Path("/privatekey")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public PrivateKeyDTO getPrivateKey() {
        try {
            return new PrivateKeyDTO(getTenantPrivateKey());
        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve private key for tenant '{}'", tenant);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve private key for tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.cert.get.private.key.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve private key for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Set tenant private key along with its associated chain of certificates
     * @param tenantCredentialsDTO details of private key and its certificates
     * @see PrivateKeyDTO
     */
    @POST @Path("/privatekey")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void setTenantCredentials(TenantCredentialsDTO tenantCredentialsDTO) {
        try {
            PrivateKey privateKey = tenantCredentialsDTO.getPrivateKey().getPrivateKey();
            List<Certificate> certificates = CertificateMapper.getCertificates(tenantCredentialsDTO.getCertificates());
            getIDMClient().setTenantCredentials(tenant, certificates, privateKey);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to set private key for tenant '{}'", tenant);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to set private key for tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.cert.set.private.key.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to set private key for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Retrieve a signing cert of a given tenant
     */
    private PrivateKey getTenantPrivateKey() throws Exception {
        return getIDMClient().getTenantPrivateKey(tenant);
    }

    /**
     * Add a certificate to a tenant's certificate store.
     * @param certificate Details of certificate to be added (In PEM formatted)
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or certificate does not exist
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void addCertificate(CertificateDTO certificate) {
        try {
            Certificate cert = certificate.getX509Certificate();
            getIDMClient().addCertificate(tenant, cert, CertificateType.STS_TRUST_CERT);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to add certificate to tenant '{}'", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (CertificateException | InvalidArgumentException | DuplicateCertificateException e) {
            log.warn("Failed to add certificate to tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.cert.add.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to add certificate to tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Delete a certificate from the tenant's certificate store.
     *
     * @param fingerprint fingerprint of the certificate to be deleted.
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or certificate does not exist
     */
    @DELETE
    @Consumes(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@QueryParam("fingerprint") String fingerprint) {
        Validate.notEmpty(fingerprint, sm.getString("valid.not.empty", "fingerprint"));

        try {
            getIDMClient().deleteCertificate(tenant, fingerprint, CertificateType.STS_TRUST_CERT);
        } catch (NoSuchTenantException | NoSuchCertificateException e) {
            log.debug("Failed to delete certificate with fingerprint '{}' for tenant '{}' because the tenant does not exist", fingerprint, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | CertificateInUseException e) {
            log.warn("Failed to delete certificate with fingerprint '{}' from tenant '{}' due to a client side error", fingerprint, tenant, e);
            throw new BadRequestException(sm.getString("res.cert.delete.failed", fingerprint, tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete certificate with fingerprint '{}' from tenant '{}' due to a server side error", fingerprint, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    private CertificateScope validateCertificateScope(String certificateScope) throws BadRequestException {
        CertificateScope type = null;
        try {
            type = CertificateScope.valueOf(certificateScope.toUpperCase());
        } catch (IllegalArgumentException | NullPointerException e) {
            throw new BadRequestException(sm.getString("valid.invalid.type", "scope", Arrays.toString(CertificateScope.values())));
        }

        return type;
    }

    private CertificateGranularity validateCertificateGranularity(String granularity) throws BadRequestException {
        CertificateGranularity certGranularity = null;
        try{
            certGranularity = CertificateGranularity.valueOf(granularity.toUpperCase());
        } catch (IllegalArgumentException | NullPointerException e) {
            throw new BadRequestException(sm.getString("valid.invalid.type", "granularity", Arrays.toString(CertificateGranularity.values())));
        }
        return certGranularity;
    }
}
