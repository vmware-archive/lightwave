/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.certificate;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.NoSuchElementException;

import org.apache.commons.codec.binary.Base64;

import com.sun.jna.Pointer;

/**
 * @author Anu Engineer
 *
 */
public class VMCAClient implements Iterable<X509Certificate> {
    public static final String BEGIN_CERT = "-----BEGIN CERTIFICATE-----\n";
    public static final String END_CERT = "\n-----END CERTIFICATE-----";

    private String _username;
    private String _domain;
    private String _password;
    private String _serverName;

    public enum certFilters {
        ACTIVE_CERTIFICATES, REVOKED_CERTIFICATES, EXPIRED_CERTIFICATES, ALL_CERTIFICATES
    }

    /**
     * Creates a Client class that allows you to communicate to a VMCA Server
     *
     * @param ServerName
     *            - Name of the VMCA Server
     */
    public
    VMCAClient(
        String username,
        String domain,
        String password,
        String server_name
        )
    {
        _username = username;
        _domain = domain;
        _password = password;
        _serverName = server_name;
        setEnumFilter(certFilters.ACTIVE_CERTIFICATES);
    }

    /**
     * Creates a Certificate from a PEM encoded String
     *
     * @param certificateString
     * @return
     * @throws Exception
     */
    public static X509Certificate getCertificateFromString(
            String certificateString) throws Exception {
        InputStream is = new ByteArrayInputStream(certificateString.getBytes());
        CertificateFactory cf = CertificateFactory.getInstance("X509");
        X509Certificate c = (X509Certificate) cf.generateCertificate(is);
        return c;
    }

    /**
     * returns a PEM Encoded String from a X509Certificate
     *
     * @param certificate
     * @return
     * @throws Exception
     */
    private String getEncodedStringFromCertificate(X509Certificate certificate)
            throws Exception {
        if (certificate == null) {
            throw new IllegalStateException(
                    "Invalid Certificate, certificate cannot be null");
        }

        String encoded = new String(Base64.encodeBase64(certificate
                .getEncoded()));
        StringBuffer pemencode = new StringBuffer();
        for (int x = 0; x < encoded.length(); x++) {

            if ((x > 0) && (x % 64 == 0)) {
                pemencode.append("\n");
                pemencode.append(encoded.charAt(x));
            } else {
                pemencode.append(encoded.charAt(x));

            }
        }

        return BEGIN_CERT + pemencode.toString() + END_CERT;
    }

    private String encodeX509CertificatesToString(X509Certificate[] certs)
          throws Exception {
       if (certs == null || certs.length == 0) {
          return null;
       }

       int stringBuilderSize = certs.length * 1024; // approximate string builder
                                                    // size
       StringBuilder sb = new StringBuilder(stringBuilderSize);
       for (X509Certificate cert : certs) {
          String pem = getEncodedStringFromCertificate(cert);
          sb.append(pem);
          sb.append('\n');
       }
       if (sb.length() > 0) {
          sb.deleteCharAt(sb.length() - 1);
       }

       return sb.toString();
    }

    private String encodePrivateKeyToString(PrivateKey key) throws UnsupportedEncodingException {
       if (key == null) {
          return null;
       }
       byte[] privBytes = key.getEncoded();
       String encoded = new String (Base64.encodeBase64(privBytes));
       StringBuffer pemencode =  new StringBuffer();
       for ( int x =0; x< encoded.length(); x++)
       {

         if ((x > 0) && (x % 64 == 0)) {
             pemencode.append("\n");
             pemencode.append(encoded.charAt(x));
          } else  {
             pemencode.append(encoded.charAt(x));

          }
         }
       return  "-----BEGIN PRIVATE KEY-----\n" + pemencode.toString() + "\n" + "-----END PRIVATE KEY-----";
    }

    /**
     * Gets root CA Certificate Array from VMCA
     *
     * @return X509Certificate Root CA Certificate
     * @throws Exception
     */
    public X509Certificate[] getRootCertificates() throws Exception
    {
        try(VMCAServerContext context = getServerContext())
        {
            return this.getRootCertificates(context);
        }
    }

    /**
     * Gets root CA Certificate Array from VMCA
     *
     * @return X509Certificate Root CA Certificate
     * @throws Exception
     */
    protected X509Certificate[]
    getRootCertificates(VMCAServerContext context) throws Exception
    {
        ArrayList<X509Certificate> trustedRoots = new ArrayList<X509Certificate>();
        trustedRoots.add(VMCAClient.getCertificateFromString(VMCAAdapter2
                .GetRootCertificate(context)));
        return trustedRoots.toArray(new X509Certificate[trustedRoots.size()]);
    }

    /**
     * Adding new root CA Certificate chain to VMCA
     *
     * @param certificateChain
     *           Chain of certificates in case VMCA becomes a subordinate CA. Remember that the leaf certificate goes first (at index 0).
     * @param key
     * @throws Exception
     */
    public void addRootCertificate(X509Certificate[] certificateChain, PrivateKey key) throws Exception {
       try (VMCAServerContext context = getServerContext())
       {
          VMCAAdapter2.AddRootCertificate(context,
                encodeX509CertificatesToString(certificateChain),
                encodePrivateKeyToString(key)
                );
       }
    }

    /**
     * Gets root CA Certificate from VMCA
     *
     * @return X509Certificate Root CA Certificate
     * @throws Exception
     */
    public X509Certificate getRootCertificate() throws Exception {
        try (VMCAServerContext context = getServerContext())
        {
            return this.getRootCertificate(context);
        }
    }

    /**
     * Gets root CA Certificate from VMCA
     *
     * @return X509Certificate Root CA Certificate
     * @throws Exception
     */
    protected X509Certificate getRootCertificate(VMCAServerContext context) throws Exception {
        return VMCAClient.getCertificateFromString(
                                VMCAAdapter2.GetRootCertificate(context));
    }

    /*
     * Returns a Signed Certificate from the Server
     *
     * @param certificateRequest -- A PKCS 10 Certificate request
     *
     * @param notBefore - Start date of the Certificate
     *
     * @param notAfter - End Date of the Certificate
     *
     * @return X509Certificate
     *
     * @throws Exception
     */

    public X509Certificate getCertificate(String certificateRequest,
            Date notBefore, Date notAfter) throws Exception {

        try(VMCAServerContext context = getServerContext())
        {
            return this.getCertificate(context, certificateRequest, notBefore, notAfter);
        }
    }

    /*
     * Returns a Signed Certificate from the Server
     *
     * @param certificateRequest -- A PKCS 10 Certificate request
     *
     * @param notBefore - Start date of the Certificate
     *
     * @param notAfter - End Date of the Certificate
     *
     * @return X509Certificate
     *
     * @throws Exception
     */

    protected X509Certificate
    getCertificate(VMCAServerContext context, String certificateRequest,
            Date notBefore, Date notAfter) throws Exception {

        long epochNotBefore = notBefore.getTime();
        long epochNotAfter = notAfter.getTime();

        epochNotBefore = epochNotBefore / 1000;
        epochNotAfter = epochNotAfter / 1000;

        return getCertificateFromString(VMCAAdapter2
                .VMCAGetSignedCertificateFromCSR(context,
                        certificateRequest, epochNotBefore, epochNotAfter));
    }

    /*
     * Returns a Signed Host Certificate from the Server 
     *
     * @param hostName -- Host name, CSR will be validated against hostname
     *
     * @param ipAddress -- optional -- ipAddress to validate against CSR
     *
     * @param certificateRequest -- A PKCS 10 Certificate request
     *
     * @param notBefore - Start date of the Certificate
     *
     * @param notAfter - End Date of the Certificate
     *
     * @return X509Certificate
     *
     * @throws Exception
     */

    public X509Certificate
    getCertificateForHost(String hostName, String ipAddress,
            String certificateRequest,
            Date notBefore, Date notAfter) throws Exception {

        try(VMCAServerContext context = getServerContext())
        {
            return this.getCertificateForHost(context, hostName, ipAddress,
                                  certificateRequest, notBefore, notAfter);
        }
    }

    /*
     * Returns a Signed Host Certificate from the Server
     *
     * @param certificateRequest -- A PKCS 10 Certificate request
     *
     * @param hostName -- Host name, CSR will be validated against hostname
     *
     * @param ipAddress -- optional -- ipAddress to validate against CSR
     *
     * @param notBefore - Start date of the Certificate
     *
     * @param notAfter - End Date of the Certificate
     *
     * @return X509Certificate
     *
     * @throws Exception
     */
    protected X509Certificate
    getCertificateForHost(VMCAServerContext context, String hostName,
            String ipAddress, String certificateRequest,
            Date notBefore, Date notAfter) throws Exception {

        long epochNotBefore = notBefore.getTime();
        long epochNotAfter = notAfter.getTime();

        epochNotBefore = epochNotBefore / 1000;
        epochNotAfter = epochNotAfter / 1000;

        return getCertificateFromString(VMCAAdapter2
                .VMCAGetSignedCertificateForHost(context,
                        hostName, ipAddress, certificateRequest,
                        epochNotBefore, epochNotAfter));
    }

    private String getPEMEncodedKey(KeyPair Keys) {
        byte[] privBytes = Keys.getPrivate().getEncoded();
        String encoded = new String(Base64.encodeBase64(privBytes));
        StringBuffer pemencode = new StringBuffer();
        for (int x = 0; x < encoded.length(); x++) {

            if ((x > 0) && (x % 64 == 0)) {
                pemencode.append("\n");
                pemencode.append(encoded.charAt(x));
            } else {
                pemencode.append(encoded.charAt(x));

            }
        }
        return "-----BEGIN PRIVATE KEY-----\n" + pemencode.toString() + "\n"
                + "-----END PRIVATE KEY-----";
    }

    /**
     * Returns a Signed Certificate from the Server
     *
     * @param Req
     *            -- A Request Object
     * @param Keys
     *            - A Java Key Pair Object
     * @param notBefore
     *            - Start Date for the Certificate
     * @param notAfter
     *            - End Date for the validity of the Certificate
     * @return X509Certificate that is signed by VMCA
     */
    public X509Certificate getCertificate(Request req, KeyPair Keys,
            Date notBefore, Date notAfter) throws Exception {

        try (VMCAServerContext context = getServerContext())
        {
            return this.getCertificate(context, req, Keys, notBefore, notAfter);
        }
    }

    /**
     * Returns a Signed Certificate from the Server
     *
     * @param Req
     *            -- A Request Object
     * @param Keys
     *            - A Java Key Pair Object
     * @param notBefore
     *            - Start Date for the Certificate
     * @param notAfter
     *            - End Date for the validity of the Certificate
     * @return X509Certificate that is signed by VMCA
     */
    protected X509Certificate getCertificate(VMCAServerContext context, Request req, KeyPair Keys,
            Date notBefore, Date notAfter) throws Exception {

        long epochNotBefore = notBefore.getTime();
        long epochNotAfter = notAfter.getTime();

        epochNotBefore = epochNotBefore / 1000;
        epochNotAfter = epochNotAfter / 1000;
        String certString = VMCAAdapter2.VMCAJavaGenCert(context,
                req.getName(), req.getCountry(), req.getLocality(),
                req.getState(), req.getOrganization(), req.getOrgunit(),
                req.getDnsname(), req.getUri(), req.getEmail(),
                req.getIpaddress(), req.getKeyusage(), 0,
                getPEMEncodedKey(Keys), epochNotBefore, epochNotAfter);

        return getCertificateFromString(certString);

    }

    /**
     * Revokes a Certificate
     *
     * @param certificate
     * @throws Exception
     */
    public void revokeCertificate(X509Certificate certificate) throws Exception {
        try (VMCAServerContext context = getServerContext())
        {
            this.revokeCertificate(context, certificate);
        }
    }

    /**
     * Revokes a Certificate
     *
     * @param certificate
     * @throws Exception
     */
    protected void revokeCertificate(VMCAServerContext context, X509Certificate certificate) throws Exception {
        VMCAAdapter2.RevokeCertificate(context,
                getEncodedStringFromCertificate(certificate));
    }

    /**
     * Gets the VMCA Server Version
     *
     * @return Version String
     * @throws Exception
     */
    public String getServerVersion() throws Exception {
        try (VMCAServerContext context = getServerContext())
        {
            return this.getServerVersion(context);
        }
    }

    /**
     * Gets the VMCA Server Version
     *
     * @return Version String
     * @throws Exception
     */
    protected String getServerVersion(VMCAServerContext context) throws Exception {
        return VMCAAdapter2.getServerVersion(context);
    }

    /*
     * (non-Javadoc)
     *
     * @see java.lang.Iterable#iterator()
     */
    @Override
    public Iterator<X509Certificate> iterator() {
        try
        {
            VMCAServerContext context = getServerContext();
            return new VMCACertIterator(context,
                    filterToInteger(getEnumFilter()));
        } catch (VMCAException e) {
            throw new RuntimeException(e.getMessage() +
                                       " Error Code: [" +
                                        e.getErrorCode() + "]");
        }

    }

    /**
     * @return the enumFilter
     */
    public certFilters getEnumFilter() {
        return enumFilter;
    }

    /**
     * @param enumFilter
     *            the enumFilter to set
     */
    public void setEnumFilter(certFilters enumFilter) {
        this.enumFilter = enumFilter;
    }

    private certFilters enumFilter;

    private VMCAServerContext getServerContext() throws VMCAException
    {
        return VMCAAdapter2.getServerContext(
                                _serverName,
                                _username,
                                _domain,
                                _password);
    }

    /**
     * filter to Integer, since I am using ENUM java Enum to Integer is not
     * direct
     *
     * @param filter
     * @return
     */
    private static int filterToInteger(final certFilters filter) {
        switch (filter) {
        case ACTIVE_CERTIFICATES:
            return 0;
        case REVOKED_CERTIFICATES:
            return 1;
        case EXPIRED_CERTIFICATES:
            return 2;
        case ALL_CERTIFICATES:
            return 4;
        }
        return 0;
    }

    private class VMCACertIterator implements Iterator<X509Certificate>, AutoCloseable  {
        private VMCAServerContext serverContext;
        private Pointer enumContext;
        private String nextCert;
        private int certFilter;

        /**
         * Ctor for Iterator
         *
         * @param serverName
         * @param certFilter
         */
        public VMCACertIterator(VMCAServerContext context, int certFilter) throws VMCAException {
            this.certFilter = certFilter;
            this.serverContext = context;
            enumContext = VMCAAdapter2.VMCAOpenEnumContext(context,
                    this.certFilter);
        }

        @Override
        public boolean hasNext() {
            try {
                if (enumContext != null) {
                    nextCert = VMCAAdapter2.VMCAGetNextCertificate(enumContext);
                    if (nextCert != null) {
                        return true;
                    } else {
                        close();
                    }
                }
            } catch (Exception e) {
                close();
            }
            return false;
        }

        @Override
        public X509Certificate next() {
            try {
                return VMCAClient.getCertificateFromString(nextCert);
            } catch (Exception e) {
                throw new NoSuchElementException(e.getMessage());
            }
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException(
                    "To Remove or Revoke a Certificate, Please use Revoke Certificate");

        }

        @Override
        public void close() {
            dispose();
        }

        @Override
        protected void finalize() throws Throwable {
            try {
                dispose();
            }
            finally {
                super.finalize();
            }
        }

        public void dispose() {
            if (enumContext != null) {
                VMCAAdapter2.VMCACloseEnumContext(enumContext);
                enumContext = null;
            }

            if (serverContext != null) {
                serverContext.dispose();
                serverContext = null;
            }
        }
    }

    public void getCRL(String existingCRL, String value) throws VMCAException, IOException {
        try (VMCAServerContext context = getServerContext())
        {
            this.getCRL(context,  existingCRL, value);
        }
    }

    protected void getCRL(VMCAServerContext context, String existingCRL, String value) throws VMCAException {
        @SuppressWarnings("unused")
        String returnedCRL = VMCAAdapter2.VMCAGetCRL(context,
                existingCRL, value);
    }

    public void publishRoots() throws VMCAException, Exception {
        try (VMCAServerContext context = getServerContext())
        {
            this.publishRoots(context);
        }
    }

    protected void publishRoots(VMCAServerContext context) throws VMCAException, Exception {
        VMCAAdapter2.VMCAPublishRoots(context);
    }

}
