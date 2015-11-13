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
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateParsingException;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;

import org.apache.commons.lang.Validate;
import org.bouncycastle.asn1.ASN1Encodable;
import org.bouncycastle.asn1.ASN1Primitive;
import org.bouncycastle.asn1.ASN1StreamParser;
import org.bouncycastle.asn1.ASN1String;
import org.bouncycastle.asn1.DERObjectIdentifier;
import org.bouncycastle.asn1.DERSequence;
import org.bouncycastle.asn1.DERTaggedObject;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IdmCertificateRevokedException;
import com.vmware.identity.idm.IdmClientCertificateParsingException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;

public class IdmClientCertificateValidator {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmClientCertificateValidator.class);

    private static final int SUBALTNAME_TYPE_OTHERNAME = 0;

    private final ClientCertPolicy certPolicy;
    private final KeyStore trustStore;

    public IdmClientCertificateValidator(ClientCertPolicy certPolicy)
                    throws InvalidArgumentException {
        this.certPolicy = certPolicy;
        trustStore = getTrustedClientCaStore();
    }

    /**
     * Validate certificate path governed by the cert (validation) policy.
     *
     * @param x509Certificate
     *            Client end certificate .
     * @throws CertificateRevocationCheckException
     * @throws InvalidArgumentException
     *             ocsp url is missing when ocsp is enabled. This condition will
     *             be allowed if we support in-cert ocsp.
     * @throws IdmCertificateRevokedException
     */
    public void validateCertificatePath(X509Certificate x509Certificate)
            throws CertificateRevocationCheckException,
 InvalidArgumentException, IdmCertificateRevokedException {

        IdmCertificatePathValidator checker = new IdmCertificatePathValidator(
                trustStore, certPolicy);
        checker.validate(x509Certificate);
    }

    /**
     * validate the subject represent in the given client certificate. The
     * process uses following information in this order: 1. UPN in SAN,KeyStore
     * 2. Subject DN 3. Other alternative names in the SAN if enabled
     *
     * @param data
     *            DER encoded data
     * @return String UPN of the subject that the cert was issued to or throw
     *         exception.
     * @throws IdmClientCertificateParsingException
     *             Not able to find principal in SSO with subject information in
     *             the certificate.
     * @throws InvalidPrincipalException
     *             Subject name in certificate exist but does not match to valid
     *             user in SSO.
     */

    public String extractUPN(X509Certificate clientCert)
            throws IdmClientCertificateParsingException, InvalidPrincipalException, IDMException {

        String upn = null;
        /**
         * UPN matching
         */
        logger.info("Extract and validating subject in client certificate");

        Collection<List<?>> altNames;
        try {
            altNames = clientCert.getSubjectAlternativeNames();
        } catch (CertificateParsingException e) {
            logger.error("No subject alternative name found in the cert.");
            throw new IdmClientCertificateParsingException("Error in finding cert SAN", e);
        }

        if (altNames == null) {
            logger.error("No subject alternative name found in the cert.");
            throw new IdmClientCertificateParsingException("Empty Subject Alternative Names");
        }

        // Examine each SAN entry for UPN that map to a registered principal
        for (List<?> altName : altNames) {
            Validate.isTrue(altName.size() > 1, "Invalid certicate SAN entry");
            Object altNameVal = altName.get(1);
            /*
             * Step 1. Get candidate UPN string
             * Expect UPN defined as "OtherName" type of SAN. Only upn will be returned as a byte
             * array.
             */
            if (Integer.valueOf(IdmClientCertificateValidator.SUBALTNAME_TYPE_OTHERNAME).equals(
                            altName.get(0))
                            && altNameVal instanceof byte[]) {

                byte[] altNameValByte = (byte[]) altNameVal;

                try {
                    upn = parseDERString(altNameValByte);
                } catch (Throwable t) {
                    throw new IdmClientCertificateParsingException(
                                    "Failed to parse SAN entry with \'OtherName\' type.", t);
                }
            } else {
                /*
                 * Unknown type. we are not parsing this SAN entry
                 */
                String skippedAltName = null;
                if (altNameVal instanceof String) {
                    skippedAltName = (String) altNameVal;
                } else if (altNameVal instanceof byte[]) {
                    skippedAltName = new String((byte[]) altNameVal);
                }
                logger.debug("Skipping SAN entry of type " + altName.get(0) + " with value: "
                                + skippedAltName);
            }

            if (upn != null) {
                logger.info("Successfully extracted UPN from SAN entry:" + upn);
                break;
            }

        }

        /*
         * if no UPN found in SAN. Next, TBD: should we try the Subject DN's
         * X500Principal Note: if UPN is found in SAN but matching to user
         * failed, we do not look farther.
         */
        return upn;
    }

    /**
     * Parse DER-encoded bytes to locate a String object
     *
     * @param alterNameValue DER encoded data
     * @return First string found
     * @throws Throwable
     */
    private static String parseDERString(byte[] alterNameValue) throws Throwable {
        try {
            ASN1StreamParser p = new ASN1StreamParser(alterNameValue);
            ASN1Encodable d = p.readObject();
            ASN1Primitive der = d.toASN1Primitive();

            return getStringFromObject(der);
        } catch (Throwable e) {
              // Exception indicates parsing failed, skip this
              // value (most likely not UPN format)
              logger.error("Unable to extract User Principal Name: "
                          + e.getMessage());
              throw e;
        }
    }
    /**
    * Find any DER-encoded String inside a DER Object of unknown type
    *
    * @param derObj
    * @return The string value inside this object, or null if none is found
    */
    private static String getStringFromObject(ASN1Primitive derObj) {
        if (derObj instanceof DERSequence) {
            return getStringFromSequence((DERSequence) derObj);
        } else if (derObj instanceof DERObjectIdentifier) {
            return null;
        } else if (derObj instanceof DERTaggedObject) {
            return getStringFromTaggedObject((DERTaggedObject) derObj);
        } else if (derObj instanceof ASN1String) {
            logger.trace("String of type " + derObj.getClass().getName());
            return ((ASN1String) derObj).getString();
        } else {
            logger.warn("Unexpected DER type, ignoring ("
                  + derObj.getClass().getName() + "): " + derObj.toString());
        }
        return null;
    }

    /**
     * Find any DER-encoded String inside a Tagged Object
     *
     * @param taggedObj
     * @return The string value inside this sequence, or null if none is found
     */
    private static String getStringFromTaggedObject(DERTaggedObject taggedObj) {
        if (null == taggedObj)
            return null;

        return getStringFromObject(taggedObj.getObject());
    }

    /**
     * Find any DER-encoded String inside a DER sequence
     *
     * @param derSeq
     * @return The first string value inside this sequence, or null if none is found
     */
    private static String getStringFromSequence(DERSequence derSeq) {
        if (null == derSeq)
            return null;

        Enumeration<?> objects = derSeq.getObjects();
        while (objects.hasMoreElements()) {
            ASN1Primitive o = (ASN1Primitive) objects.nextElement();
            String retVal = getStringFromObject(o);
            if (null != retVal) {
                return retVal;
            }
        }
        return null;
    }

    /**
     *
     * @return keyStore representing that containing the trust CA certificates of the tenant
     * @throws InvalidArgumentException
     */
    private KeyStore getTrustedClientCaStore() throws InvalidArgumentException {
        KeyStore trustedClientCaStore;

        if (certPolicy == null || certPolicy.getTrustedCAs() == null) {
            throw new InvalidArgumentException(
                            "Null client certificate policy or trust ca certficagtes.");
        }
        try {
            trustedClientCaStore = KeyStore.getInstance(KeyStore.getDefaultType());
        } catch (KeyStoreException e1) {
            throw new InvalidArgumentException("Failed in creating a keyStore instance: ", e1);
        }
        try {
            trustedClientCaStore.load(null, null);
        } catch (NoSuchAlgorithmException | CertificateException | IOException e1) {
            throw new InvalidArgumentException("Failed in initializing a keyStore instance: "
                            + e1.getMessage(), e1);
        }
        for (Certificate trustCa : certPolicy.getTrustedCAs()) {
            X509Certificate x509Cert = (X509Certificate) trustCa;
            try {
                trustedClientCaStore.setCertificateEntry(x509Cert.getSubjectX500Principal()
                                .getName(), trustCa);
            } catch (KeyStoreException e) {
                throw new InvalidArgumentException("Failed in storing a ca cert to keyStore: "
                                + e.getMessage(), e);
            }
        }
        return trustedClientCaStore;
    }

}
