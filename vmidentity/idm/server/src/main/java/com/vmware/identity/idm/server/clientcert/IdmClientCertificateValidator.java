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
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateParsingException;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import javax.security.auth.x500.X500Principal;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.lang.Validate;
import org.bouncycastle.asn1.ASN1Encodable;
import org.bouncycastle.asn1.ASN1OctetString;
import org.bouncycastle.asn1.ASN1Primitive;
import org.bouncycastle.asn1.ASN1StreamParser;
import org.bouncycastle.asn1.ASN1String;
import org.bouncycastle.asn1.DERObjectIdentifier;
import org.bouncycastle.asn1.DERSequence;
import org.bouncycastle.asn1.DERTaggedObject;
import org.bouncycastle.cert.jcajce.JcaX509ExtensionUtils;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CertificatePathBuildingException;
import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.ExplicitCertificateMappingException;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IdmCertificateRevokedException;
import com.vmware.identity.idm.IdmClientCertificateParsingException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.TenantInformation;
import com.vmware.identity.idm.server.config.directory.DirectoryConfigStore;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.UserSet;

public class IdmClientCertificateValidator {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmClientCertificateValidator.class);

    private static final int SUBALTNAME_TYPE_OTHERNAME = 0;
    private static final int SUBALTNAME_TYPE_RFC822NAME = 1;

    //explicit mapping constants
    private static final String CONST_X509 = "X509:";
    private static final String CONST_ISSUER = "<I>";
    private static final String CONST_SUBJECT = "<S>";
    private static final String CONST_SERIAL_NUMBER = "<SR>";
    private static final String CONST_SKI = "<SKI>";
    private static final String CONST_SHA1_PUKEY = "<SHA1-PUKEY>";
    private static final String CONST_RFC822 = "<RFC822>";
    private static final String x509LdapAttributeName = "altSecurityIdentities";// not
                                                                                // configurable
                                                                                // per
                                                                                // spec.
    private static final String ATTRIBUTE_USER_PRINCIPAL_NAME_FRIENDLY_NAME = "userPrincipalName";

    private final ClientCertPolicy certPolicy;
    private final KeyStore trustStore;
    private final String tenantName;
    private final TenantInformation tenantInfo;

    public IdmClientCertificateValidator(ClientCertPolicy certPolicy, TenantInformation tenantInfo)
                    throws InvalidArgumentException {
        Validate.notNull(certPolicy, "certPolicy");
        Validate.notNull(tenantInfo, "tenantInfo");

        this.tenantInfo = tenantInfo;
        this.certPolicy = certPolicy;
        this.tenantName = tenantInfo.getTenant().getName();

        trustStore = getTrustedClientCaStore();
    }

    /**
     * Validate certificate path governed by the cert (validation) policy.
     *
     * @param x509Certificate
     *            Client end certificate .
     * @param authStatExt
     *            AuthStat extensions for profiling the detailed steps.
     * @throws CertificateRevocationCheckException
     * @throws InvalidArgumentException
     *             ocsp url is missing when ocsp is enabled. This condition will
     *             be allowed if we support in-cert ocsp.
     * @throws IdmCertificateRevokedException
     * @throws CertificatePathBuildingException
     */
    public void validateCertificatePath(X509Certificate x509Certificate, String siteID, Map<String, String> authStatExt)
            throws CertificateRevocationCheckException,
 InvalidArgumentException, IdmCertificateRevokedException, CertificatePathBuildingException {
        IdmCertificatePathValidator checker = new IdmCertificatePathValidator(
                trustStore, certPolicy, this.tenantName, siteID);
        checker.validate(x509Certificate, authStatExt);
    }

    /**
     * Extract principal name from SAN extension of the certificate
     *
     * @param data
     *            DER encoded data
     * @return String UPN of the subject that the cert was issued to or throw
     *         exception.
     * @throws IdmClientCertificateParsingException
     *             Not able to extract UPN from the certificate.
     */

    public static String extractUPN(X509Certificate clientCert)
            throws IdmClientCertificateParsingException, IDMException {

        String upn = null;
        logger.trace("Extract SAN from client certificate");

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
                if (logger.isDebugEnabled()) {
                    logger.debug("Successfully extracted UPN from SAN entry:" + upn);
                }
                break;
            }
        }

        /*
         * if no UPN found in SAN. Next, TBD: should we try the Subject DN's
         * X500Principal Note: if UPN is found in SAN but matching to user
         * failed, we do not look farther.
         */
        if (upn == null) {
            throw new IdmClientCertificateParsingException("No UPN entry in Subject Alternative Names extension");
        }

        return upn;
    }

    /**
     * Extract RFC822 name from SAN extension of the certificate
     *
     * @param data
     *            DER encoded data
     * @return String RFC822 name of the subject that the cert was issued to or null
     * @throws IdmClientCertificateParsingException
     *             Not able to extract RFC822 from the certificate.
     */

    public static String extractEmail(X509Certificate clientCert) throws IdmClientCertificateParsingException
    {

        String rfc822 = null;
        logger.debug("Extract RFC822 name from client certificate");

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

        for (List<?> altName : altNames) {
            Validate.isTrue(altName.size() > 1, "Invalid certicate SAN entry");

            if (Integer.valueOf(IdmClientCertificateValidator.SUBALTNAME_TYPE_RFC822NAME).equals(
                    altName.get(0))) {

				rfc822 = (String) altName.get(1);
            }

            if (rfc822 != null) {
                if (logger.isDebugEnabled()) {
                    logger.debug("Successfully extracted RFC822 name from SAN entry:" + rfc822);
                }
                break;
            }
        }

        return rfc822;
    }

    /**
     * Parse DER-encoded bytes to locate a String object
     *
     * @param alterNameValue
     *            DER encoded data
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

    /**
     * Find identity provider using certificate SAN
     *
     * @param targetCert
     * @return null value possible.
     * @throws IDMException
     */
    private IIdentityProvider findProviderFromCertPrincipalName(X509Certificate targetCert) throws IDMException {
        ValidateUtil.validateNotNull(targetCert, "targetCert");

        String upn = extractUPN(targetCert);

        // Validate allowed authentication type on provider
        PrincipalId userPrincipal;

        IIdentityProvider provider;
        try {
            userPrincipal = ServerUtils.getUserPrincipal(this.tenantInfo, upn);
            provider = this.tenantInfo.findProviderADAsFallBack(userPrincipal.getDomain());
        } catch (Exception e) {
            throw new IDMException("Failed to retrieve details of identity provider with domain from UPN:" + upn);
        }

        return provider;
    }

    /**
     * Account search using UPN with optional AltSecurityIdentities retrieval.
     *
     * @param X509Certificate
     *            non null.
     * @param IIdentityProvider
     *            non null.
     * @return non null
     * @throws IdmClientCertificateParsingException
     *             unable to read certificate SAN
     * @throws IDMException
     *             other types of error.
     *
     */
    private UserSet accountLinkingWithUPN(X509Certificate targetCert, IIdentityProvider provider, boolean retrieveAltSec)
            throws IdmClientCertificateParsingException, IDMException {

        ValidateUtil.validateNotNull(targetCert, "targetCert");
        ValidateUtil.validateNotNull(provider, "provider");

        logger.info("Searching user with certificate SAN.");
        String upn = extractUPN(targetCert);
        PrincipalId userPrincipal;

        try {
            userPrincipal = ServerUtils.getUserPrincipal(this.tenantInfo, upn);

            logger.debug("Successfully in account linking using certificate SAN: "
                    + userPrincipal.getUPN());

            return provider.findActiveUsersInDomain(ATTRIBUTE_USER_PRINCIPAL_NAME_FRIENDLY_NAME,
                    upn, userPrincipal.getDomain(), retrieveAltSec ? x509LdapAttributeName : null);

        } catch (Exception e) {
            throw new IDMException("Failed in account linking using certificate SAN: " + upn, e);
        }

    }


    /**
     * Validates the certificate to account via altSecurityIdentities attribute.
     *
     * @param principal
     * @param targetCert
     * @return PrincipalId non-null value user that passed validation.
     * @throws IDMException
     *             None of provided principal validates successfully.
     */
    public PrincipalId accountValidationWithExplicitX509(UserSet principals, X509Certificate targetCert)
            throws IDMException {

        logger.info("Validating user account altSecurityIdentities attribute.");

        ValidateUtil.validateNotNull(targetCert, "targetCert");
        ValidateUtil.validateNotNull(principals, "principals");

        IDMException interim_error = null;
        PrincipalId result_principal = null;

        Iterator<Map.Entry<PrincipalId, Collection<String>>> it = principals.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry<PrincipalId, Collection<String>> pair = it.next();
            PrincipalId principal = pair.getKey();
            try {
                Collection<String> altSecIdentities = pair.getValue();
                if (altSecIdentities == null || altSecIdentities.size() == 0) {
                    throw new IDMException(String.format(
                            "Unable to authenticate with the credential. Failed retrieving altSecurityIdentities attribute for linked account: %s ",
                            principal.getUPN()));
                }

                // Validate altSecurityIdentities
                ValidateAltSec(targetCert, altSecIdentities);
                logger.info("Successfully validated an user account using explicit certificate mapping: " + principal.getUPN());
                if (result_principal != null) {
                    throw new IDMException(String.format(
                            "Unable to uniquely determine an account. Found at least two hinted accounts: %s and %s map to the target certificate.",
                            result_principal.getUPN(), principal.getUPN()));

                } else {
                    result_principal = principal;
                }
            } catch (IDMException e) {
                // cache it then check the rest
                interim_error = e;
            }
        }

        if (null != result_principal) {
            return result_principal;
        } else {
            // above logic implies interim_error is non-null.
            throw interim_error;
        }
    }

    /**
	 * Validate the certificate by explicit mapping altSecurityIdentities value
	 * of the account. This implementation follows
	 * https://blogs.msdn.microsoft.com
	 * /spatdsg/2010/06/18/howto-map-a-user-to-a-
	 * certificate-via-all-the-methods-
	 * available-in-the-altsecurityidentities-attribute/
	 *
	 * @param targetCert
	 *            The user certificate mapping to the account.
	 * @param altSecIdentities
	 *            altSecurityIdentities values extracted from the account to be
	 *            validated.
	 * @throws ExplicitCertificateMappingException
	 *             no matching found in the provided altSecIdentities
	 * @throws IDMException
	 *             other unexpected error during validation
	 * @throws IOException
	 *             IO error during validation
	 * @return void at successful validation.
	 */
    public static void ValidateAltSec(X509Certificate targetCert, Collection<String> altSecIdentities) throws IDMException {

        if (ValidateIssuerAndSubject(targetCert, altSecIdentities)) {
            return;
        }

        if (ValidateSubject(targetCert, altSecIdentities)) {
            return;
        }

        if (ValidateIssuerAndSerialNumber(targetCert, altSecIdentities)) {
            return;
        }

        if (ValidateSubjectKeyIdentifier(targetCert, altSecIdentities)) {
            return;
        }
        if (ValidateHashOfPubKey(targetCert, altSecIdentities)) {
            return;
        }
        if (ValidateRFC822(targetCert, altSecIdentities)) {
            return;
        }

        String error = "The certifcate failed to match alSecurityIdentities attribute value:  [";
        for (String val : altSecIdentities) {
            error += val;
        }
        error +="]";
        throw new ExplicitCertificateMappingException(error);
    }

    /**
     * Explicit validate with RC822 Name
     *
     * @param targetCert
     * @param altSecIdentities
     * @return true/false
     */
    private static boolean ValidateRFC822(X509Certificate targetCert, Collection<String> altSecIdentities) {
        String certVal = CONST_X509;

        try {
            String rfc822Name = IdmClientCertificateValidator.extractEmail(targetCert);

            if (rfc822Name == null || rfc822Name.isEmpty()) {
                return false;
            }
            certVal += CONST_RFC822 + rfc822Name;
        } catch (IdmClientCertificateParsingException e) {
            return false;
        }
        for (String val : altSecIdentities) {
            if (val.equalsIgnoreCase(certVal)) {
                logger.info("Certificate mapping succeeded via RFC822 name identifier.");
                return true;
            }
        }
        return false;
    }

    /**
	 * Explicit mapping with Hash of subject key This method of cert mapping is
	 * not generally recommended by GSA; may be difficult to manage
	 *
	 * @param targetCert
	 * @param altSecIdentities
	 * @return
	 */
    private static boolean ValidateHashOfPubKey(X509Certificate targetCert, Collection<String> altSecIdentities) {
        String certVal = CONST_X509;

        try {
            certVal += CONST_SHA1_PUKEY + calculateSHA1(targetCert);
		} catch (NoSuchAlgorithmException e) {
			logger.error("Failed to calculate sha-1 hash. ", e.getMessage());
			return false;
		}
        for (String val : altSecIdentities) {
            if (val.equalsIgnoreCase(certVal)) {
                logger.info("Certificate mapping succeeded via SHA1 hash of pub key.");
                return true;
            }
        }
        return false;
    }

    /**
	 * calculate Sha1 hash of the cert.
	 *
	 * @param targetCert
	 * @return
	 * @throws NoSuchAlgorithmException
	 */
    private static String calculateSHA1(X509Certificate targetCert) throws NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-1");
        byte[] hashbytes = digest
				.digest(targetCert.getPublicKey().getEncoded());
        final int len = hashbytes.length * 2 ;
        // Typically SHA-1 algorithm produces 20 bytes, i.e. len should be 40
        StringBuilder sha1String = new StringBuilder(len);

        for (int i = 0; i < hashbytes.length; i++) {
            // unsigned byte
            hashbytes[i] &= 0xff;
            // get the hex chars
            sha1String.append(String.format("%02x", hashbytes[i]));
        }

        return sha1String.toString();
    }

    /**
     * Explicit validate with SKI
     *
     * @param targetCert
     * @param altSecIdentities
     * @return true/false
     * @throws IOException
     */
    private static boolean ValidateSubjectKeyIdentifier(X509Certificate targetCert, Collection<String> altSecIdentities) throws IDMException {

        String certVal = CONST_X509;

        byte[] extValueInBytes = targetCert.getExtensionValue("2.5.29.14");

        // We need to unwrap extension encoding, then octet encoding.
        ASN1Primitive skiPrimitive;
        try {
            skiPrimitive = JcaX509ExtensionUtils.parseExtensionValue(extValueInBytes);
        } catch (IOException e) {
            throw new IDMException("Failed to parse subject key identifier extension.", e);
        }

        // Unwrap second 'layer'
        byte[] skiBytes;
        try {
            skiBytes = ASN1OctetString.getInstance(skiPrimitive.getEncoded()).getOctets();
        } catch (IOException e) {
            throw new IDMException("Failed in octet-decoding of subject key identifier extension.", e);
        }

        // Use keyIdentifier in e.g. CMS SignerInfo
        // SignerInfoGenerator signerInfoGenerator = jcaSignerInfoGeneratorBuilder.build(sha1Signer, skiBytes);

        certVal += CONST_SKI + Hex.encodeHexString(skiBytes);
        for (String val : altSecIdentities) {
            if (val.equalsIgnoreCase(certVal)) {
                logger.info("Certificate mapping succeeded via subject key identifier.");
                return true;
            }
        }
        return false;
    }

    /**
     * Explicit validate with issuer and serial number.
     *
     * @param targetCert
     * @param altSecIdentities
     * @return true/false
     */
    private static boolean ValidateIssuerAndSerialNumber(X509Certificate targetCert, Collection<String> altSecIdentities) {
        String certVal = CONST_X509;

        certVal += CONST_ISSUER + getReversedDN(targetCert.getIssuerX500Principal());
        certVal += CONST_SERIAL_NUMBER + targetCert.getSerialNumber();
        for (String val : altSecIdentities) {
            if (val.equalsIgnoreCase(certVal)) {
                logger.info("Certificate mapping succeeded via issuer and serial number.");
                return true;
            }
        }
        return false;
    }

    /**
     * Explicit validate with subject
     *
     * @param targetCert
     * @param altSecIdentities
     * @return true/false
     */
    private static boolean ValidateSubject(X509Certificate targetCert, Collection<String> altSecIdentities) {
        String certVal = CONST_X509;

        certVal += CONST_SUBJECT + getReversedDN(targetCert.getSubjectX500Principal());
        for (String val : altSecIdentities) {
            if (val.equalsIgnoreCase(certVal)) {
                logger.info("Certificate mapping succeeded via subject.");
                return true;
            }
        }
        return false;
    }

    /**
     * Explicit validation with issuer and subject.
     *
     * @param targetCert
     * @param altSecIdentities
     * @return true/false
     */
    private static boolean ValidateIssuerAndSubject(X509Certificate targetCert, Collection<String> altSecIdentities) {

        String certVal = CONST_X509;

        certVal += CONST_ISSUER + getReversedDN(targetCert.getIssuerX500Principal());
        certVal += CONST_SUBJECT + getReversedDN(targetCert.getSubjectX500Principal());
        for (String val : altSecIdentities) {
            if (val.equalsIgnoreCase(certVal)) {
                logger.info("Certificate mapping succeeded via issuer and subject.");
                return true;
            }
        }
        return false;
    }

    /**
     * Return DN of the principal name that match altSecurityIdentity convention
     * which list CN at the end, with no spaces in between components.
     *
     * @param p
     * @return
     */
    static String getReversedDN(X500Principal p) {
        if (p == null) {
            return null;
        }

        String pRFC1779 = p.getName(X500Principal.RFC2253);
        if (pRFC1779 == null) {
            return null;
        }

        String[] pComponents = pRFC1779.split(",");
        String reversedDN = null;
        for (int i = pComponents.length - 1; i >= 0; i--) {
            if (reversedDN == null) {
                reversedDN = pComponents[i];
            } else {
                reversedDN += "," + pComponents[i];
            }

        }
        return reversedDN;
    }

    /**
     * Initial search for principal with username hint value.
     *
     * @provider identity provider.
     * @param userDomain
     *            the actual domain that user is provisioned.
     * @param usernameHint
     *            default to "sAMAccountName" if not provided.
     * @return
     * @throws IDMException
     * @throws Exception
     */
    private UserSet accountLinkingWithHint(IIdentityProvider provider, String userDomain
            , String usernameHint) throws IDMException {
        ValidateUtil.validateNotEmpty(usernameHint, "usernameHint");
        Validate.notNull(provider, "provider");
        Validate.notNull(userDomain, "userDomain");

        logger.info("Searching users with hint.");

        String hintAttributeName = provider.getStoreUserHintAttributeName();


        if (null == hintAttributeName) {
            hintAttributeName = "sAMAccountName";
        }
        try {
            logger.info("username");
            return provider.findActiveUsersInDomain(hintAttributeName, usernameHint, userDomain,
                    x509LdapAttributeName);
        } catch (Exception e) {
            throw new IDMException("Failed to find user: "+usernameHint+"using "+hintAttributeName, e);
        }
    }

    /**
     * Scenarioes:
     * A:  One to one mapping using UPN
     * B: One to many mapping using hint and explicit certificate validation
     * C: One to one mapping using UPN and explicit certificate validation.
     *
     *
     * @param targetCert  required
     * @param hint    optional
     * @return   non null validated principalId that the certificate maps to.
     * @throws IDMException
     * @throws InvalidPrincipalException
     * @throws IdmClientCertificateParsingException
     */
    public PrincipalId certificateAccountMapping(X509Certificate targetCert, String hint)
            throws IdmClientCertificateParsingException,
            InvalidPrincipalException,
            IDMException {

        Validate.notNull(targetCert, "targetCert");

        IIdentityProvider provider = null;
        UserSet candicateUsers = null;

        // First, find a set of candidate user using hint or UPN.
        if (null == hint) {
            //Scenario A & C
            try {
                provider = findProviderFromCertPrincipalName(targetCert);
                if (null == provider) {
                    throw new IDMException("Unable to locate an identity provider.");
                }
            } catch (Exception e) {
                throw new IDMException("User hint was not provided. Unable to match an account with certficate SAN extension!", e);
            }

            // Validate allowed authentication type on provider
            IdentityManager.validateProviderAllowedAuthnTypes(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE, provider.getName(),
                    this.tenantInfo);
            candicateUsers = accountLinkingWithUPN(targetCert, provider, !provider.getCertificateMappingUseUPN());
        } else {
            String[] userInfo = ServerUtils.separateUserIDAndDomain(hint);
            String userDomainName = userInfo[1];

            // try get provider from domain in hint
            try {
                provider = this.tenantInfo.findProviderADAsFallBack(userInfo[1]);
            } catch (Exception e) {
                throw new IDMException("Failed to find provider from domain in hint:  " + userDomainName, e);
            }

            if (provider == null) {
                throw new IDMException("Failed to find provider from domain in hint:  " + userDomainName);
            }

            // Validate allowed authentication type on provider
            IdentityManager.validateProviderAllowedAuthnTypes(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE, provider.getName(),
                    this.tenantInfo);

            if (provider.getCertificateMappingUseUPN()) {
                //Scenario A
                candicateUsers = accountLinkingWithUPN(targetCert, provider, false);
            } else {
                //Scenarion B
                candicateUsers = accountLinkingWithHint(provider, userDomainName, userInfo[0]);
            }
        }

        // Next, validate using AltSecurityIdentities if required for the
        // domain.
        PrincipalId result_principal;
        if (false == provider.getCertificateMappingUseUPN()) {
           //Needed only for scenarion B & C
            result_principal = accountValidationWithExplicitX509(candicateUsers, targetCert);
        } else {
            Validate.isTrue(candicateUsers.size() == 1, "Unexpected intermediate result");
            result_principal = candicateUsers.entrySet().iterator().next().getKey();
        }
        return result_principal;
    }

}
