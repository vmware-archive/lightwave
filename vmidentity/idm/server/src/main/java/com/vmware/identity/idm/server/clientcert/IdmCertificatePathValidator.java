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
import java.io.InputStream;
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
import java.security.cert.X509CertSelector;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.Validate;

import sun.security.x509.CRLDistributionPointsExtension;
import sun.security.x509.DistributionPoint;
import sun.security.x509.GeneralName;
import sun.security.x509.X509CRLImpl;
import sun.security.x509.X509CertImpl;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CertRevocationStatusUnknownException;
import com.vmware.identity.idm.CertificatePathBuildingException;
import com.vmware.identity.idm.CertificateRevocationCheckException;
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

    // CRLs location map of <CRL_URI, X509CRLImpl>
    private final Map<String, X509CRLImpl> crlCheckMap = new HashMap<String, X509CRLImpl>();

    // collection containing certificates for certificate path building and CRLimpl for CRL checking
    // ivate final Collection<Object> crlCollection = new ArrayList<Object>();

    private final ClientCertPolicy certPolicy;

    /**
     * @param trustStore
     * @param certPolicy
     * @throws CertificateRevocationCheckException
     */
    public IdmCertificatePathValidator(KeyStore trustStore, ClientCertPolicy certPolicy) throws CertificateRevocationCheckException{
        this.trustStore = trustStore;
        try {
            Validate.notNull(certPolicy, "Cert Policy");
            Validate.notNull(trustStore, "Trust Store");

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
     *
     * @throws CertificateRevocationCheckException
     * @throws IdmCertificateRevokedException
     * @throws InvalidArgumentException
     */
    public void validate(X509Certificate cert)
            throws CertificateRevocationCheckException,
            IdmCertificateRevokedException, InvalidArgumentException {

        if (null == cert) {
            throw new CertificateRevocationCheckException("No certs to validate.");
        }

        logger.debug("Certificate policy: " + this.certPolicy.toString());
        logger.debug("Checking revocation for certificate: "
                 + cert.getSubjectDN());

        // Build the certpath
        CertPath certPath;
        try {
            certPath = buildCertPath(cert);
        } catch (CertificatePathBuildingException e) {
            throw new CertificateRevocationCheckException("CertPath building failed. "
                            + e.getMessage(), e);
        }

        // Validate certpath
        validateCertPath(certPath);

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
        setupValidateOptions(crlCollection);
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
     * @throws CertificateRevocationCheckException
     */
    private void setupValidateOptions(Collection<Object> crlCollection)
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

            if (enableCRLChecking == true && this.certPolicy.getCRLUrl() != null) {
                addCRLToWorkingList(this.certPolicy.getCRLUrl().toString(), crlCollection);
            }

            // setup crl checking with crlDP
            setThreadLocalSystemProperty("com.sun.security.enableCRLDP",
                            (enableCRLChecking && this.certPolicy.useCertCRL()) ? "true" : "false");
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
     * validation.
     *
     * @param leafCert
     * @param certFactory2
     * @throws CertificateRevocationCheckException
     */
    // @SuppressWarnings("unchecked")
    private void addCertCRLsToWorkingList(X509Certificate leafCert,
                    CertificateFactory certFactory2, Collection<Object> crlCollection)
                    throws CertificateRevocationCheckException {

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
                    try {
                       if (crlGeneralNameString.startsWith(PREFIX_URI_NAME)) {
                            String crlURLString = crlGeneralNameString
                                  .substring(PREFIX_URI_NAME.length());
                            addCRLToWorkingList(crlURLString, crlCollection);
                       }
                     } catch (CertificateRevocationCheckException e) {
                         logger.warn("Problem importing CRL: "
                                     + e.getMessage());
                         if (logger.isDebugEnabled()) {
                             logger.debug("Problem importing CRL: "
                                   + e.getMessage(), e);
                         }
                         throw e;
                    }
                }
            }
       } catch (CertificateRevocationCheckException ex) {
           throw ex;
        } catch (IOException e) {
            logger.error("Problem in accessing CRL: " + e.getMessage());
            throw new CertificateRevocationCheckException(
                    "Error reading CRL for smart-card or client certificate: "
                            + "(" + e.getMessage() + ")", e);
       }

    }

    /**
     * Adding a CRL to the working list to be used for certificate path validation
     * @param crlURLString
     * @param crlCollection
     * @throws CertificateRevocationCheckException
     * @return  if succeeded
     */
    private void addCRLToWorkingList(String crlURLString, Collection<Object> crlCollection)
                    throws CertificateRevocationCheckException {

        if (logger.isDebugEnabled()) {
             logger.debug("Adding CRL: " + crlURLString);
        }

        X509CRLImpl crlImpl = null;

        crlImpl = crlCheckMap.get(crlURLString);

        /*
         * If CRL has been updated since the last check, clear it for refresh
         */
        if (crlImpl != null) {
            Date update = crlImpl.getNextUpdate();
            if (update.before(new Date())) {
                crlCheckMap.remove(crlURLString);
                crlImpl = null;
            }
        }

        try {
            if (crlImpl == null) {
                if (crlURLString.startsWith("ldap:///")) {
                    throw new CertificateRevocationCheckException(
                            "LDAP CRL stores is not supported");
                } else {

                    InputStream crlInputStream = new URL(crlURLString)
                            .openConnection().getInputStream();
                    try {
                        crlImpl = (X509CRLImpl) this.certFactory
                                .generateCRL(crlInputStream);
                    } finally {
                        crlInputStream.close();
                    }

                }

                crlCheckMap.put(crlURLString, crlImpl);
            }
        } catch (Exception e) {
            logger.error("Error reading CRL: " + crlURLString + e.getMessage());
            throw new CertificateRevocationCheckException("Error reading CRL: "
                    + crlURLString + "(" + e.getMessage() + ")", e);
        }
        if (crlImpl != null) {
            crlCollection.add(crlImpl);
        }
    }

}
