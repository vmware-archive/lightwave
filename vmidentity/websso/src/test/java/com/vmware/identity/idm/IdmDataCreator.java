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
package com.vmware.identity.idm;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * Create some demo data in IDM
 *
 */
public final class IdmDataCreator {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(IdmDataCreator.class);

    private static final String CONFIG_FILE_DIRECTORY = "/tmp"; // used on Linux
    private static final String CONFIG_FILE_DIRECTORY_ENV_VAR = "TMP"; // used
                                                                       // on
                                                                       // Windows
    private static final String CONFIG_FILE = "config.properties";

    private static boolean forceCleanup = false;

    // open input stream
    private static InputStream getInputStream(String filename) {
        InputStream retval = null;

        try {
            // get env var
            String envvar = System.getenv(CONFIG_FILE_DIRECTORY_ENV_VAR);
            String path;
            if (envvar != null) {
                // assume Windows
                path = envvar + "\\";
            } else {
                // assume Linux
                path = CONFIG_FILE_DIRECTORY + "/";
            }
            // try filesystem first
            String filePath = path + filename;
            File file = new File(filePath);
            if (file.exists()) {
                retval = new FileInputStream(filePath);
            }
        } catch (Exception e) {
        }
        if (retval == null) {
            // fallback to resource file
            retval = IdmDataCreator.class.getResourceAsStream("/" + filename);
        }
        return retval;
    }

    /**
     * Load IDM data
     *
     * @throws Exception
     */
    public static void loadData() throws Exception {
        logger.debug("IdmDataCreator.loadData called");

        InputStream is = getInputStream(CONFIG_FILE);
        Validate.notNull(is);

        try {
            ServerConfig.initialize(is);
        } finally {
            is.close();
        }
    }

    // read key store data
    private static KeyPair readKeyStore(CredentialDescriptor cd)
            throws IOException {
        KeyPair kp = null;
        InputStream is = null;

        try {
            KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
            char[] stsKeystorePassword = cd.getPassword().toCharArray();
            is = getInputStream(cd.getFilename());
            ks.load(is, stsKeystorePassword);

            kp = new KeyPair();
            kp.setCertificateChain(Arrays.asList(ks.getCertificateChain(cd
                    .getAlias())));
            kp.setPrivateKey((PrivateKey) ks.getKey(cd.getAlias(),
                    stsKeystorePassword));
        } catch (Exception e) {
            logger.debug("Caught exception while reading keystore {}", e.toString());
        } finally {
            if (is != null) {
                is.close();
            }
        }

        return kp;
    }

    /**
     * Create data (ServerConfig loaded at this point)
     *
     * @param idmClient
     * @throws Exception
     */
    public static void createData(CasIdmClient idmClient) throws Exception {
        logger.debug("IdmDataCreator.createData called");

        Validate.notNull(idmClient);

        if (forceCleanup) {
            // delete tenants
            int i = 0;
            String tenantName = ServerConfig.getTenant(i);
            while (tenantName != null) {
                IdmDataRemover.addTenant(tenantName);
                i++;
                tenantName = ServerConfig.getTenant(i);
            }
            try {
                IdmDataRemover.removeData(idmClient);
            } catch (Exception e) {
                logger.debug("Caught exception while removing data {}", e.toString());
            }
            forceCleanup = false;
        }

        // create tenants
        int i = 0;
        String tenantName = ServerConfig.getTenant(i);
        while (tenantName != null) {
            processTenant(idmClient, tenantName);
            i++;
            tenantName = ServerConfig.getTenant(i);
        }
        // process default tenant
        String defaultTenant = ServerConfig.getDefaultTenant();
        idmClient.setDefaultTenant(defaultTenant);
    }

    // create tenant from configuration properties. We do not overwrite existing
    // tenants.
    private static void processTenant(CasIdmClient idmClient, String tenantName)
            throws Exception {
        // create tenant
        Tenant tenantToCreate = new Tenant(tenantName);
        tenantToCreate._issuerName = ServerConfig.getTenantEntityId(tenantName);
        IdmDataRemover.addTenant(tenantName);

        Tenant existingTenant = null;
        try
        {
            existingTenant = idmClient.getTenant(tenantName);
            assert (existingTenant != null);
            return;
        }
        catch(NoSuchTenantException ex)
        {
            idmClient.addTenant(tenantToCreate, ServerConfig.getTenantAdminUsername(), ServerConfig.getTenantAdminPassword().toCharArray());
        }

        existingTenant = idmClient.getTenant(tenantName);
        assert (existingTenant != null);

        // add entity ID, clock tolerance, certificates and keys
        try {
            idmClient.setEntityID(tenantName,
                    ServerConfig.getTenantEntityId(tenantName));
            idmClient.setClockTolerance(tenantName,
                    ServerConfig.getTenantClockTolerance(tenantName));
            idmClient.setDelegationCount(tenantName,
                    ServerConfig.getTenantDelegationCount(tenantName));
            idmClient.setBrandName(tenantName,
                    ServerConfig.getTenantBrandName(tenantName));
            idmClient.setRenewCount(tenantName,
                    ServerConfig.getTenantRenewCount(tenantName));
            idmClient.setMaximumBearerTokenLifetime(tenantName, ServerConfig
                    .getTenantMaximumBearerTokenLifetime(tenantName));
            idmClient.setMaximumHoKTokenLifetime(tenantName,
                    ServerConfig.getTenantMaximumHokTokenLifetime(tenantName));
            KeyPair kp = readKeyStore(ServerConfig
                    .getTenantCredentialDescriptor(tenantName));
            idmClient.setTenantCredentials(tenantName,
                    kp.getCertificateChain(), kp.getPrivateKey());
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }

        // create relying parties
        int i = 0;
        String rpName = ServerConfig.getRelyingParty(tenantName, i);
        while (rpName != null) {
            RelyingParty rp = processRelyingParty(idmClient, rpName);
            // add relying party info
            idmClient.addRelyingParty(tenantName, rp);
            i++;
            rpName = ServerConfig.getRelyingParty(tenantName, i);
        }

        // create identity stores
        int j = 0;
        String adProviderName = ServerConfig.getADProvider(tenantName, j);
        while (adProviderName != null) {
            IdentityStoreData adStore = processADProvider(idmClient,
                    adProviderName);
            // add store info
            idmClient.addProvider(tenantName, adStore);
            j++;
            adProviderName = ServerConfig.getADProvider(tenantName, j);
        }
    }

    // create relying party from configuration properties
    private static RelyingParty processRelyingParty(CasIdmClient idmClient,
            String rpName) throws Exception {
        // create relying party
        RelyingParty rp = new RelyingParty(rpName);

        // add certificate and other parameters
        try {
            rp.setUrl(ServerConfig.getRelyingPartyUrl(rpName));
            rp.setAuthnRequestsSigned(ServerConfig
                    .getRelyingPartyAuthnRequestsSigned(rpName));

            KeyPair kp = readKeyStore(ServerConfig
                    .getRelyingPartyCredentialDescriptor(rpName));
            List<Certificate> certificateChain = kp.getCertificateChain();
            // TODO change the Relaying Party to get a chain not only leaf
            // certificate
            assert certificateChain != null && certificateChain.size() > 0;
            rp.setCertificate(certificateChain.get(0));
        } catch (Exception e) {
            logger.debug("processRelyingParty: Caught exception {}", e.toString());
            throw new IllegalStateException(e);
        }

        // add Assertion Consumer Services
        int i = 0;
        ArrayList<AssertionConsumerService> services = new ArrayList<AssertionConsumerService>();
        String acsName = ServerConfig.getAssertionConsumerService(rpName, i);
        while (acsName != null) {
            AssertionConsumerService acs = processAssertionConsumerService(
                    idmClient, acsName);
            // store service data
            services.add(acs);
            i++;
            acsName = ServerConfig.getAssertionConsumerService(rpName, i);
        }

        // commit Assertion Consumer Services
        rp.setAssertionConsumerServices(services);

        // get default assertion consumer service
        String defaultService = ServerConfig
                .getDefaultAssertionConsumerService(rpName);
        rp.setDefaultAssertionConsumerService(defaultService);

        // add Single Logout Services
        i = 0;
        ArrayList<ServiceEndpoint> sloServices = new ArrayList<ServiceEndpoint>();
        String sloName = ServerConfig.getSingleLogoutService(rpName, i);
        while (sloName != null) {
            ServiceEndpoint slo = processSingleLogoutService(
                    idmClient, sloName);
            // store service data
            sloServices.add(slo);
            i++;
            sloName = ServerConfig.getSingleLogoutService(rpName, i);
        }

        // commit Single Logout Services
        rp.setSingleLogoutServices(sloServices);

        return rp;
    }

    // create assertion consumer service from configuration properties
    private static AssertionConsumerService processAssertionConsumerService(
            CasIdmClient idmClient, String acsName) throws Exception {

        AssertionConsumerService assertSvc = new AssertionConsumerService(
                acsName);
        assertSvc.setBinding(ServerConfig
                .getServiceBinding(acsName));
        assertSvc.setEndpoint(ServerConfig
                .getServiceEndpoint(acsName));

        return assertSvc;
    }

    // create single logout service from configuration properties
    private static ServiceEndpoint processSingleLogoutService(
            CasIdmClient idmClient, String sloName) throws Exception {

        ServiceEndpoint sloSvc = new ServiceEndpoint(
                sloName);
        sloSvc.setBinding(ServerConfig
                .getServiceBinding(sloName));
        sloSvc.setEndpoint(ServerConfig
                .getServiceEndpoint(sloName));

        return sloSvc;
    }

    // create AD provider from configuration properties
    private static IdentityStoreData processADProvider(CasIdmClient idmClient,
            String adProviderName) throws Exception {

        final ArrayList<String> kdcList = new ArrayList<String>();
        int i = 0;
        String kdc = ServerConfig.getKdc(adProviderName, i);
        while (kdc != null) {
            kdcList.add(kdc);
            i++;
            kdc = ServerConfig.getKdc(adProviderName, i);
        }

        final Map<String, String> attrMap = new HashMap<String, String>();
        attrMap.put(
                "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname",
                "givenName");
        attrMap.put(
                "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname",
                "sn");
        attrMap.put("http://rsa.com/schemas/attr-names/2009/01/GroupIdentity",
                "memberof");
        attrMap.put(
                "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress",
                "mail");
        attrMap.put("http://vmware.com/schemas/attr-names/2011/07/isSolution",
                "subjectType");
        attrMap.put("http://schemas.xmlsoap.org/claims/UPN",
                "userPrincipalName");

        IdentityStoreData adStore = IdentityStoreData
                .CreateExternalIdentityStoreData(adProviderName, null,
                        // bugzilla#1173915
                        IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                        AuthenticationType.PASSWORD, null, 0,
                        ServerConfig.getADProviderUserName(adProviderName),
                        ServerConfig.getADProviderUserPassword(adProviderName),
                        ServerConfig.getADProviderSearchBaseDN(adProviderName),
                        ServerConfig.getADProviderSearchBaseDN(adProviderName),
                        kdcList, attrMap);

        return adStore;
    }

    /**
     * @return the forceCleanup
     */
    public static boolean isForceCleanup() {
        return forceCleanup;
    }

    /**
     * @param forceCleanup
     *            the forceCleanup to set
     */
    public static void setForceCleanup(boolean forceCleanup) {
        IdmDataCreator.forceCleanup = forceCleanup;
    }
}
