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
package com.vmware.identity.rest.core.server.test.util;

import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.KeyStore;
import java.security.KeyStore.Entry;
import java.security.KeyStore.PrivateKeyEntry;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableEntryException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import com.vmware.identity.token.impl.Constants;
import com.vmware.identity.token.impl.SamlTokenImpl;
import com.vmware.vim.sso.client.exception.InvalidTokenException;

public class SAMLTokenTestUtil {

    private static final String TEST_SAML_LOC = "src/test/resources/saml/";
    private static final String TEST_SAML_KEYSTORE_LOC = TEST_SAML_LOC + "sso_test.jks";
    private static final String TEST_SAML_KEYSTORE_PASS = "vmware";
    private static final String TEST_SAML_KEYSTORE_ALIAS = "vmware";

    private static KeyStore store;

    /**
     * Use Java reflection to set the token as having been validated, since it is unusable
     * for comparisons otherwise.
     *
     * NOTE: This breaks if the SamlTokenImpl changes the field name.
     *
     * @param token SAML token to set as validated
     * @throws SecurityException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public static void setValidated(SamlTokenImpl token) throws NoSuchFieldException, SecurityException, IllegalArgumentException, IllegalAccessException {
        Field field = token.getClass().getDeclaredField("_tokenValidated");
        field.setAccessible(true);
        AtomicBoolean _tokenValidated = (AtomicBoolean) field.get(token);
        _tokenValidated.set(true);
    }

    /**
     * Use Java reflection to set the token's confirmation certificate, since we have no builder class
     * and generating customized SAML tokens is painful with the current library.
     *
     * NOTE: This breaks if the SamlTokenImpl changes the field name.
     *
     * @param token SAML token to set the certificate on
     * @param certificate the certificate to set on the token
     * @throws NoSuchFieldException
     * @throws SecurityException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public static void setConfirmationCertificate(SamlTokenImpl token, X509Certificate certificate) throws NoSuchFieldException, SecurityException, IllegalArgumentException, IllegalAccessException {
        Field field = token.getClass().getDeclaredField("_confirmationCertificate");
        field.setAccessible(true);
        field.set(token, certificate);
    }

    /**
     * Get the SAML token POJO parsed from an XML file
     *
     * @param filename name of the file to parse
     * @return a SamlTokenImpl representing the parsed XML
     * @throws InvalidTokenException
     * @throws IOException
     * @throws JAXBException
     * @throws NoSuchFieldException
     * @throws SecurityException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public static SamlTokenImpl getSAMLToken(String filename) throws InvalidTokenException, IOException, JAXBException, NoSuchFieldException, SecurityException, IllegalArgumentException, IllegalAccessException {
        return new SamlTokenImpl(getSAMLFile(filename), createJAXBContext());
    }

    /**
     * Get the XML from a SAML file
     *
     * @param filename name of the file to parse
     * @return the XML from the file
     * @throws IOException
     */
    public static String getSAMLFile(String filename) throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(TEST_SAML_LOC + filename));
        return new String(encoded, Charset.defaultCharset());
    }

    public static KeyStore getKeystore() {
        if (store == null) {
            try {
                store = KeyStore.getInstance("JKS");
                FileInputStream stream = new FileInputStream(TEST_SAML_KEYSTORE_LOC);
                store.load(stream, TEST_SAML_KEYSTORE_PASS.toCharArray());
            } catch (NoSuchAlgorithmException | CertificateException | IOException | KeyStoreException e) {
                throw new IllegalStateException("Error loading the keystore", e);
            }
        }

        return store;
    }

    public static X509Certificate getCertificate() {
        try {
            return (X509Certificate) getKeystore().getCertificate(TEST_SAML_KEYSTORE_ALIAS);
        } catch (KeyStoreException e) {
            throw new IllegalStateException("Error fetching the certificate from the keystore", e);
        }
    }

    public static PrivateKey getPrivateKey() {
        try {
            Entry entry = getKeystore().getEntry(TEST_SAML_KEYSTORE_ALIAS, new KeyStore.PasswordProtection(TEST_SAML_KEYSTORE_PASS.toCharArray()));
            return ((PrivateKeyEntry) entry).getPrivateKey();
        } catch (KeyStoreException | NoSuchAlgorithmException | UnrecoverableEntryException e) {
            throw new IllegalStateException("Error fetching the private key from the keystore", e);
        }
    }

    /**
     * @return {@link JAXBContext} for {@link Constants#ASSERTION_JAXB_PACKAGE}
     * @throws JAXBException
     */
    private static JAXBContext createJAXBContext() throws JAXBException {
      return JAXBContext.newInstance(Constants.ASSERTION_JAXB_PACKAGE);
    }

}
