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

package com.vmware.vim.sso.client;

import java.io.IOException;
import java.net.URL;
import java.security.cert.X509Certificate;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.token.impl.Constants;
import com.vmware.identity.token.impl.SamlTokenImpl;
import com.vmware.vim.sso.client.exception.InvalidTimingException;
import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;

public class ADFSTokenTest
{
    private static final String ADFS_CERT_ALIAS = "ADFS";
    private static final String ADFS_CERT_PWD = "adfskeystore";
    private static final String ADFS_CERT_JKS = "ADFSKeyStore.jks";
    private static final String SAML_TOKEN_DIR = "/saml_token/";
    private static final String ADFS_SAML_TOKEN_FILE = "adfs_token.xml";
    private static JAXBContext _JAXBContext;

    // saml token is a saved afdfs xml; it's expiration is in the past -
    // we still want to validate token for test purposes
    private static int CLOCK_TOLERANCE_10YRS = 60*60*24*365*10;

    @BeforeClass
    public static void prep() throws JAXBException
    {
        _JAXBContext = JAXBContext.newInstance(Constants.ASSERTION_JAXB_PACKAGE);
    }

    @Test
    public void test()
    {
        URL keyStoreResource = ADFSTokenTest.class.getResource(
                "/" + ADFSTokenTest.ADFS_CERT_JKS);
        Assert.assertNotNull("Should be able to get resource stream from " + ADFS_CERT_JKS, keyStoreResource);

        KeyStoreData keyStore = null;
        try {
            keyStore = new KeyStoreData(keyStoreResource.getFile(),
                    ADFSTokenTest.ADFS_CERT_PWD.toCharArray(),
                    ADFSTokenTest.ADFS_CERT_ALIAS);
        } catch (SsoKeyStoreOperationException e1) {
            e1.printStackTrace();
            Assert.fail("Should be able to get adfs keystore. Error: " + e1.getMessage());
        }

        String adfsTokenXml = null;
        try {
            adfsTokenXml = TestTokenUtil.loadStreamContent(this.getClass()
               .getResourceAsStream(SAML_TOKEN_DIR + ADFS_SAML_TOKEN_FILE));
        } catch (IOException e1) {
            e1.printStackTrace();
            Assert.fail("Should be able to get adfs token from file. Error: " + e1.getMessage());
        }
        Assert.assertNotNull("Should be able to get resource stream from " + ADFS_SAML_TOKEN_FILE, adfsTokenXml);

        try
        {
            SamlTokenImpl samlTokenImpl = new SamlTokenImpl(adfsTokenXml, _JAXBContext, true);
            try
            {
                samlTokenImpl.validate(new X509Certificate[] { keyStore.getCertificate() }, CLOCK_TOLERANCE_10YRS);
            }
            catch(InvalidTimingException ex)
            {
                // expected as this token can be expired - it is a saved token ...
            }
            Assert.assertNotNull("Token subject should be non-null", samlTokenImpl.getSubjectNameId());
            Assert.assertEquals("Token subject name format should be http://schemas.xmlsoap.org/claims/UPN", samlTokenImpl.getSubjectNameId().getFormat(), "http://schemas.xmlsoap.org/claims/UPN");
            Assert.assertEquals("Token subject should be ExternalIdpTest@acme.vmware.com", samlTokenImpl.getSubjectNameId().getValue(), "ExternalIdpTest@acme.vmware.com");
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            Assert.fail(ex.getClass().getName() + ": " + ex.getMessage());
        }
     }
}
