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
package com.vmware.identity.idm.server.clientcert;

import java.io.IOException;
import java.net.URL;
import java.security.InvalidAlgorithmParameterException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.Security;
import java.security.cert.CertPath;
import java.security.cert.CertPathBuilder;
import java.security.cert.CertPathBuilderException;
import java.security.cert.CertPathBuilderResult;
import java.security.cert.CertPathValidator;
import java.security.cert.CertPathValidatorException;
import java.security.cert.CertPathValidatorResult;
import java.security.cert.CertStore;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.CollectionCertStoreParameters;
import java.security.cert.PKIXBuilderParameters;
import java.security.cert.PKIXParameters;
import java.security.cert.X509CRL;
import java.security.cert.X509CertSelector;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.Validate;

import sun.security.x509.CRLDistributionPointsExtension;
import sun.security.x509.DistributionPoint;
import sun.security.x509.GeneralName;
import sun.security.x509.X509CertImpl;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CertRevocationStatusUnknownException;
import com.vmware.identity.idm.CertificatePathBuildingException;
import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.CrlDownloadException;
import com.vmware.identity.idm.IdmCertificateRevokedException;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.server.ThreadLocalProperties;

/**
 * Class for client certificate validation
 * @author schai
 *
 */
public class IdmCertificatePathValidator {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmCertificatePathValidator.class);
    private static final String PREFIX_URI_NAME = "URIName: ";
    private final KeyStore trustStore;
    private CertificateFactory certFactory;
    private String tenantName;

    // collection containing certificates for certificate path building and CRLimpl for CRL checking
    // ivate final Collection<Object> crlCollection = new ArrayList<Object>();

    private final ClientCertPolicy certPolicy;

    /**
     * @param trustStore
     * @param certPolicy
     * @param tenantName
     * @throws CertificateRevocationCheckException
     */
    public IdmCertificatePathValidator(KeyStore trustStore, ClientCertPolicy certPolicy, String tenantName) throws CertificateRevocationCheckException{
        this.trustStore = trustStore;
        try {
            Validate.notNull(certPolicy, "Cert Policy");
            Validate.notNull(trustStore, "Trust Store");
            Validate.notEmpty(tenantName, "tenantName");

            this.tenantName = tenantName;
            this.certPolicy = certPolicy;
            this.certFactory = CertificateFactory.getInstance("X.509");
        } catch (CertificateException e) {
            throw new CertificateRevocationCheckException("Unable to initialize CertificateFactory: ", e);
        }
    }

    /**
     * Revocation check function 1. use ocsp first if it is enabled 2. fail if
     * the cert is revoked 3. Fall back to CRL if ocsp fails for reason other
     * then revoked 4. CRL validation using provided URL and in-cert URL
     *
     * Note:OCSP nonce extension appears not currently controllable in Java's
     * default OCSPChecker.
     *
     * @param certs
     *            Client cert chain. It could be a leaf certificate, a partial or a
     *            full chain including root CA.
     * Current implementation only relies on leaf certificate and use it to build certificate path then validate it.
     * @param authStatExt
     *            AuthStat extensions for profiling the detailed steps.
     * @throws CertificateRevocationCheckException unable to validate revocation status.
     * @throws IdmCertificateRevokedException  certificate revoked
     * @throws InvalidArgumentException
     * @throws CertificatePathBuildingException  cert path building error of any reasons: such as expired cert, etc.
     */
    public void validate(X509Certificate cert, Map<String, String> authStatExt)
            throws CertificateRevocationCheckException, IdmCertificateRevokedException, InvalidArgumentException, CertificatePathBuildingException {

        if (null == cert) {
            throw new InvalidArgumentException("No certs to validate.");
        }

        if (logger.isDebugEnabled()) {
            logger.debug("Certificate policy: " + this.certPolicy.toString());
            logger.debug("Checking revocation for certificate: "
                 + cert.getSubjectDN());
        }

        // Build the certpath
        long startTimeMs = System.currentTimeMillis();

        CertPath certPath = buildCertPath(cert);

        authStatExt.put("buildCertPath", String.format("%d Ms", System.currentTimeMillis() - startTimeMs));
        startTimeMs = System.currentTimeMillis();

        // Validate certpath
        validateCertPath(certPath);
        authStatExt.put("validateCertPath", String.format("%d Ms", System.currentTimeMillis() - startTimeMs));
        logger.info("Successfully validated client certificate : "
                 + cert.getSubjectDN());

    }

    /**
     * build and validate cert path from end certificate.
     *
     * Note: the certpath return seems only include intermediate CA unless there is none in
     * which case the end cert is returned.
     * @param endCert
     * @return CertPath  never null
     * @throws CertificatePathBuildingException
     */
    private CertPath buildCertPath(X509Certificate endCert) throws CertificatePathBuildingException
 {

        CertPathBuilder cpb = null;
        try {
            cpb = CertPathBuilder.getInstance("PKIX");
        } catch (NoSuchAlgorithmException e) {
            throw new CertificatePathBuildingException("Error building CertPathBuilder:"
                            + e.getMessage(), e);
        }

        PKIXBuilderParameters params = CreatePKIXBuilderParameters(endCert);
        CertPathBuilderResult cpbResult;
        try {
            cpbResult = cpb.build(params);
        } catch (CertPathBuilderException e) {
            throw new CertificatePathBuildingException(e.getMessage(), e.getCause());
        } catch (InvalidAlgorithmParameterException e) {
            throw new CertificatePathBuildingException(e.getMessage(), e);
        }

        CertPath cp = cpbResult.getCertPath();

        return cp;
    }

    /**
     * Create and init PKIXBuilderParameters for CertPathBuilder.
     *
     * @param endCert
     *            the target user certificate to use for building certificate
     *            path
     * @return
     * @throws CertificatePathBuildingException
     */
    private PKIXBuilderParameters CreatePKIXBuilderParameters(X509Certificate endCert)
                    throws CertificatePathBuildingException {
        X509CertSelector targetConstraints = new X509CertSelector();
        targetConstraints.setCertificate(endCert);
        PKIXBuilderParameters params;

        try {
            params = new PKIXBuilderParameters(trustStore, targetConstraints);

            // Do not validate the certificate at cert path building stage.
            // This would result in unknown failures.
            params.setRevocationEnabled(false);
        } catch (KeyStoreException e) {
            throw new CertificatePathBuildingException(
                            "Error creating PKIXBuilderParameters: Please check trust store"
                                            + e.getMessage(), e);
        } catch (InvalidAlgorithmParameterException e) {
            throw new CertificatePathBuildingException("Error creating PKIXBuilderParameters:"
                            + e.getMessage(), e);
        } catch (Throwable e) {
            // have this block in case a new type of error was thrown
            throw new CertificatePathBuildingException("Error creating PKIXBuilderParameters:"
                            + e.getMessage(), e);
        }

        Collection<Object> certCollection = new ArrayList<Object>();
        // add trusted CAs to the collection
        addCertificateCandidates(endCert, certCollection);

        if (!certCollection.isEmpty()) {
            try {
                CertStore certStore = CertStore.getInstance("Collection",
                                new CollectionCertStoreParameters(certCollection));
                params.addCertStore(certStore);
            } catch (InvalidAlgorithmParameterException e) {
                throw new CertificatePathBuildingException(
                                "Error creating CertStore for PKIXBuilderParameters:"
                                                + e.getMessage(), e);
            } catch (NoSuchAlgorithmException e) {
                throw new CertificatePathBuildingException(
                                "Error creating CertStore for  PKIXBuilderParameters:"
                                                + e.getMessage(), e);
            }
        } else {
            logger.debug("Revocation check: CRL list empty");
        }
        return params;

    }

    /**
     * Adding potential certificates to the collection to be used to build CertStore which
     * is used in building certificate path.
     *
     * @param userCert
     * @param certCollection  certificates to be used for cert path building.
     * @throws CertificateRevocationCheckException
     */
    private void addCertificateCandidates(X509Certificate userCert, Collection<Object> certCollection)
                    throws CertificatePathBuildingException {

        // adding end user cert provided
        certCollection.add(userCert);

        // adding trusted ca certificats in the trust store.
        try {
            Enumeration<String> certAliases = this.trustStore.aliases();
            while (certAliases.hasMoreElements()) {
                String alias = certAliases.nextElement();
                certCollection.add(this.trustStore.getCertificate(alias));
            }
        } catch (KeyStoreException e) {

            throw new CertificatePathBuildingException("Bad trustStore!", e);
        }

    }

    /**
     * Set a ThreadLocal property to System property.
     *
     * @param key
     * @param val
     * @throws CertificateRevocationCheckException
     */
    private void setThreadLocalSystemProperty(String key, String val)
                    throws CertificateRevocationCheckException {
        if (System.getProperties() instanceof ThreadLocalProperties) {
            ThreadLocalProperties properties = (ThreadLocalProperties) System
                            .getProperties();
            properties.setThreadLocalProperty(key, val);
            System.setProperties(properties);
        } else {
            throw new CertificateRevocationCheckException(
                            "System properties was not initialized to ThreadLocalProperties");
        }
    }

    /**
     * Perform certificate path validation for the given certificate path.
     *
     * @param certPath certificate path to be validated.
     * @param params PKIX Parameters for CertPathValidator
     * @throws CertificateRevocationCheckException error that prevent the function return the status
     *         of the cert
     * @throws IdmCertificateRevokedException CRL report the cert is revoked
     */
    private void validateCertPath(CertPath certPath)
                    throws CertificateRevocationCheckException, IdmCertificateRevokedException {

        Collection<Object> crlCollection = new ArrayList<Object>();
        setupValidateOptions(crlCollection, certPath);
        PKIXParameters params = createPKIXParameters(crlCollection);

        CertPathValidator certPathValidator;
        try {
            certPathValidator = CertPathValidator.getInstance("PKIX");
        } catch (NoSuchAlgorithmException e) {
            throw new CertificateRevocationCheckException(
                    "Error getting PKIX validator instance:" + e.getMessage(), e);
        }

        try {
            String pkiParam = params.toString();
            logger.trace("**Certificate Path Validation Parameters trust anchors **\n"
                            + params.getTrustAnchors().toString() + "\n");

            logger.trace("**Certificate Path Validation Parameters **\n" + pkiParam + "\n");

            CertPathValidatorResult result = certPathValidator.validate(certPath,
                    params);

            logger.trace("**Certificate Path Validation Result **\n"
                         + result.toString() + "\n");
        } catch (CertPathValidatorException e) {
            if (e.getReason() == CertPathValidatorException.BasicReason.REVOKED) {
                throw new IdmCertificateRevokedException(
                        "CRL shows certificate status as revoked");
            } else if (e.getReason() == CertPathValidatorException.BasicReason.UNDETERMINED_REVOCATION_STATUS) {
                throw new CertRevocationStatusUnknownException(
                                "CRL checking could not determine certificate status.");
            }
            throw new CertificateRevocationCheckException("Certificate path validation failed:"
                    + e.getMessage(), e);
        } catch (InvalidAlgorithmParameterException e) {
            throw new CertificateRevocationCheckException(
                   "Certificate validation parameters invalid, could not validate certificate path:"
                            + e.getMessage(), e);
        }

    }


    /**
     * Set up sun validator options for revocation checking.
     *
     * The following options will be set here:
     *  "com.sun.security.enableCRLDP"
     *  "ocsp.enable"
     *  "ocsp.responderURL"
     *  The ocsp controls currently are not thread-safe in the sense that multi-tenant usage of the feature could override
     *  each other. Note: DOD does not ask for multitenancy. PR 1417152.
     * @param certCollection   extra crl to be used in validating the status of the certificate.
     * @param CertPath  target certificate path for validation.
     * @throws CertificateRevocationCheckException
     */
    private void setupValidateOptions(Collection<Object> crlCollection, CertPath certPath)
                    throws CertificateRevocationCheckException {
        /**
         * Extract in-cert CRLs and adding them to working list of CRLimpl Setup
         * up revocation check related java property
         */

        if (this.certPolicy.revocationCheckEnabled()) {
            // setup ocsp
            boolean enableCRLChecking = false;

            if (this.certPolicy.useOCSP()) {
                Security.setProperty("ocsp.enable", "true");

                if (this.certPolicy.useCRLAsFailOver()) {
                    enableCRLChecking = true;
                }

                URL ocspURL = this.certPolicy.getOCSPUrl();
                if (ocspURL != null) {
                    Security.setProperty("ocsp.responderURL", ocspURL.toString());
                }
            } else {
                Security.setProperty("ocsp.enable", "false");
                enableCRLChecking = true;
            }

            //get custom CRL
            URL customCrlUri = this.certPolicy.getCRLUrl();
            if (enableCRLChecking == true && customCrlUri != null) {
                try {
                    addCRLToWorkingList(customCrlUri.toString(), crlCollection);
                } catch (CrlDownloadException e) {
                    throw new CertificateRevocationCheckException("Failed to download CRL from custom CRL URI: "+customCrlUri.toString(), e);
                }
            }

            //get CRLs from certpath CRLDP, use cache if available. Turn off sun security CRLDP checking since we extract them as override
            if (enableCRLChecking && this.certPolicy.useCertCRL()) {
               List<? extends java.security.cert.Certificate> certList = certPath.getCertificates();

               if (certList == null) {
                   return;
               }
               Iterator<? extends java.security.cert.Certificate> it = certList.iterator();

               while (it.hasNext()) {
                   try {
                       addCertCRLsToWorkingList((X509Certificate) it.next(), crlCollection);
                   }
                   catch (CertificateRevocationCheckException e) {
                       //Not able to get any of CRLDP from this certificate. Throw if no custom CRL and OCSP is not enabled
                       if (customCrlUri == null && this.certPolicy.useOCSP() == false) {
                           throw new CertificateRevocationCheckException("CRL download failure. ", e);
                       }
                   }
               }
            }

            setThreadLocalSystemProperty("com.sun.security.enableCRLDP", "false");

        } else {
            setThreadLocalSystemProperty("com.sun.security.enableCRLDP", "false");
        }

    }

    /**
     * Create parameters for CertPathValidator using PKIX algorithm.
     *
     * The parameter object was defined with given trustStore and CRL collection
     * @param trustStore2
     * @return non-null PKIXParameters
     * @throws CertificateRevocationCheckException
     */
    private PKIXParameters createPKIXParameters(Collection<Object> crlCollection)
                    throws CertificateRevocationCheckException {

        PKIXParameters params = null;
        try {
            Validate.notNull(trustStore, "TrustStore can not be null.");
            params = new PKIXParameters(trustStore);

            if (this.certPolicy.revocationCheckEnabled()) {
                params.setRevocationEnabled(true);
            } else {
                params.setRevocationEnabled(false);
            }
        } catch (KeyStoreException e) {
            throw new CertificateRevocationCheckException(
                    "Error creating validator parameters: Please check trust store"
                            + e.getMessage(), e);
        } catch (InvalidAlgorithmParameterException e) {
            throw new CertificateRevocationCheckException(
                  "Error creating validator parameters:" + e.getMessage(),e);
        } catch (Throwable e) {
            //have this block in case a new type of error was thrown
            throw new CertificateRevocationCheckException(
                    "Error creating validator parameters:" + e.getMessage(),e);
        }

        if (!crlCollection.isEmpty()) {
            try {
                 CertStore crlStore = CertStore.getInstance("Collection",
                         new CollectionCertStoreParameters(crlCollection));
                 params.addCertStore(crlStore);
            } catch (InvalidAlgorithmParameterException e) {
                 throw new CertificateRevocationCheckException(
                         "Error adding CRLs to validating parameters:"
                                 + e.getMessage(), e);
            } catch (NoSuchAlgorithmException e) {
                 throw new CertificateRevocationCheckException(
                         "Error adding CRLs to validating parameters:"
                                 + e.getMessage(), e);
            }
        } else {
            logger.debug("Revocation check: CRL list empty");
        }

        // setup certificate policy white list

        String[] oidWhiteList = this.certPolicy.getOIDs();

        if (oidWhiteList != null && oidWhiteList.length > 0) {
            Set<String> oidSet = new HashSet<String>();
            for (String oid : oidWhiteList) {
                oidSet.add(oid);
            }
            params.setInitialPolicies(oidSet);
            params.setExplicitPolicyRequired(true);
        }
        return params;

    }

    /**
     * Extract certificate CRLs and adding them to the working map of CRLImpl for cert path
     * validation. Use cached copy if available.
     *
     * @param leafCert
     * @param crlCollection crl collection that to contain any CRL found from the certificate or cached copy
     * @return void if at least one CRLDP URI result in accessible CRL.
     * @throws CertificateRevocationCheckException This exception is throw if one of the following occurs:
     *              a) Failure in retrieve all f non-LDAP CRLDP (no cached copy and unable to download in realtime).
     *              b) CRLDistributionPointsExtension.get fails. currently this happens only if we had passed a invalid
     *          access point constant. We propagate this error.
     */
    // @SuppressWarnings("unchecked")
    private void addCertCRLsToWorkingList(X509Certificate leafCert, Collection<Object> crlCollection)
                    throws CertificateRevocationCheckException {

        if (logger.isDebugEnabled()) {
            logger.debug("IdmCertificatePathValidator.addCertCRLsToWorkingList(): Adding CRLs from CRLDP");
        }

        String error = null;
        boolean atLeastOneCrlAdded = false;
        X509CertImpl certImpl = (X509CertImpl) leafCert;
        CRLDistributionPointsExtension crlDistributionPointsExt = certImpl
              .getCRLDistributionPointsExtension();
        if (null == crlDistributionPointsExt) {
            //no distribution points found
            return;
        }

        try {
            for (DistributionPoint distribPoint : (List<DistributionPoint>) crlDistributionPointsExt
                            .get(CRLDistributionPointsExtension.POINTS)) {
                for (GeneralName crlGeneralName : distribPoint.getFullName().names()) {
                    String crlGeneralNameString = crlGeneralName.toString();
                   if (crlGeneralNameString.startsWith(PREFIX_URI_NAME)) {
                        String crlURLString = crlGeneralNameString
                              .substring(PREFIX_URI_NAME.length());
                        try {
                            addCRLToWorkingList(crlURLString, crlCollection);
                            atLeastOneCrlAdded = true;
                        } catch (CrlDownloadException e) {
                            if (logger.isDebugEnabled()) {
                                logger.debug("No cached copy and failed to download CRL"
                                        + e.getMessage());
                            }
                            //continue fetching remaining crl in case of error
                            if (error == null) {
                                error = String.format("Unable to obtain CRL from certificate at following distribution points: %s", crlURLString);
                            } else {
                                error += String.format(error+", %s", crlURLString);
                            }

                        }
                   }

                }
            }
        } catch (IOException e) {
            logger.error("IOException in accessing CRLDP"
                        + e.getMessage());
            throw new CertificateRevocationCheckException("IOException in calling CRLDistributionPointsExtension.get()");
        }

        if (error != null) {
            logger.warn(error);
            if (!atLeastOneCrlAdded) {
                throw new CertificateRevocationCheckException(error);
            }
        }
    }

    /**
     *
     * Adding a CRL to the working list to be used for certificate path validation.
     *
     * Use the cached copy if available. Otherwise, download and add to CRL cache. LDAP URI is not supported.
     * Note: TenantCrlCache refresh cached CRL periodically. Authentication thread does not download CRL if there is a cached copy.
     *
     * @param crlURLString
     * @param crlCollection             Crl collection passed in to contains accumulative result.
     * @throws CrlDownloadException      if no crl was added due to downloading failure
     * @return void
     */
    private void addCRLToWorkingList(String crlURLString, Collection<Object> crlCollection)
                    throws CrlDownloadException {

        if (logger.isDebugEnabled()) {
             logger.debug("Adding CRL: " + crlURLString);
        }

        X509CRL crlImpl = null;

        IdmCrlCache crlCache = TenantCrlCache.get().get(this.tenantName);

        //add crl cache for the tenant if it does not exist.
        if (crlCache == null) {
            crlCache = new IdmCrlCache();
            TenantCrlCache.get().put(this.tenantName, crlCache);
        }

        crlImpl = crlCache.get(crlURLString);

        //If the crl is not cached, we download one and then add to the cache.
        if (crlImpl == null) {

            crlImpl = IdmCrlCache.downloadCrl(crlURLString);
            if (null != crlImpl) {
                crlCache.put(crlURLString, crlImpl);
            }
        }

        if (crlImpl != null) {
            crlCollection.add(crlImpl);
        }
    }

}
