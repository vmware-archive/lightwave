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
package com.vmware.identity.saml.config;

import static com.vmware.identity.saml.TestUtil.TOKEN_DELEGATION_COUNT;
import static com.vmware.identity.saml.TestUtil.TOKEN_RENEW_COUNT;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import org.easymock.EasyMock;
import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.saml.SignatureAlgorithm;

public class ConfigTest {

   public static final String ISSUER_NAME = "issuer";
   public static final PrivateKey AUTHORITY_KEY = createMock(PrivateKey.class);
   public static final X509Certificate DUMMY_CERT1 = createMock(X509Certificate.class);
   public static final X509Certificate DUMMY_CERT2 = createMock(X509Certificate.class);
   public static final X509Certificate DUMMY_CERT3 = createMock(X509Certificate.class);
   public static final List<Certificate> AUTHORITY_CHAIN = new ArrayList<Certificate>();
   public static final List<Certificate> AUTHORITY_CERTS3 = new ArrayList<Certificate>();
   public static final Collection<List<Certificate>> VALID_CERTS = new ArrayList<List<Certificate>>();
   public static final Collection<List<Certificate>> DUMMY_VALID_CERTS3 = new ArrayList<List<Certificate>>();

   public static final long DEFAULT_CLOCK_TOLERANCE = 1000;
   public static final int DEFAULT_MAXIMUM_TOKEN_LIFETIME = 10000;
   public static final int DEFAULT_COUNT = 0;
   public static final String TENANT_NAME = "tenant";

   static {
      AUTHORITY_CHAIN.add(DUMMY_CERT1);
      AUTHORITY_CHAIN.add(DUMMY_CERT2);

      List<Certificate> chain2 = new ArrayList<Certificate>();
      chain2.add(DUMMY_CERT1);
      chain2.add(DUMMY_CERT2);
      VALID_CERTS.add(chain2);

      AUTHORITY_CERTS3.add(DUMMY_CERT1);
      AUTHORITY_CERTS3.add(DUMMY_CERT2);
      AUTHORITY_CERTS3.add(DUMMY_CERT3);

      List<Certificate> chain3 = new ArrayList<Certificate>();
      chain3.add(DUMMY_CERT1);
      chain3.add(DUMMY_CERT2);
      chain3.add(DUMMY_CERT3);
      DUMMY_VALID_CERTS3.add(chain3);
   }

   @Test
   public void testNewInstanceOK() {
      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoIssuer() {
      createConfig(null, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceEmptyIssuer() {
      createConfig("", AUTHORITY_CHAIN, AUTHORITY_KEY, TOKEN_DELEGATION_COUNT,
         TOKEN_RENEW_COUNT, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoCertificate1() {
      createConfig(ISSUER_NAME, null, AUTHORITY_KEY, TOKEN_DELEGATION_COUNT,
         TOKEN_RENEW_COUNT, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoCertificate2() {
      createConfig(ISSUER_NAME, new ArrayList<Certificate>(), AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoCertificate3() {
      List<Certificate> authorityCertChain = new ArrayList<Certificate>();
      authorityCertChain.add(EasyMock.createMock(Certificate.class));
      createConfig(ISSUER_NAME, authorityCertChain, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoKey() {
      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, null, TOKEN_DELEGATION_COUNT,
         TOKEN_RENEW_COUNT, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoValidCertificates1() {
      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME, null,
         DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoValidCertificates2() {
      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         new HashSet<List<Certificate>>(), DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoValidCertificates3() {
      List<Certificate> authorityCertChain = new ArrayList<Certificate>();
      Collection<List<Certificate>> validCertChains = new HashSet<List<Certificate>>();
      validCertChains.add(authorityCertChain);

      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         validCertChains, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoValidCertificates4() {
      List<Certificate> authorityCertChain = new ArrayList<Certificate>();
      authorityCertChain.add(EasyMock.createMock(Certificate.class));
      Collection<List<Certificate>> validCertChains = new HashSet<List<Certificate>>();
      validCertChains.add(authorityCertChain);

      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         validCertChains, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoValidCertificates5() {
      List<Certificate> authorityCertChain = new ArrayList<Certificate>();
      Collection<List<Certificate>> validCertChains = new HashSet<List<Certificate>>();
      validCertChains.add(authorityCertChain);
      validCertChains.add(AUTHORITY_CHAIN);

      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         validCertChains, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testNewInstanceNoValidCertificates6() {
      List<Certificate> authorityCertChain = new ArrayList<Certificate>();
      authorityCertChain.add(null);
      Collection<List<Certificate>> validCertChains = new HashSet<List<Certificate>>();
      validCertChains.add(AUTHORITY_CHAIN);
      validCertChains.add(authorityCertChain);

      createConfig(ISSUER_NAME, AUTHORITY_CHAIN, AUTHORITY_KEY,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         validCertChains, DEFAULT_CLOCK_TOLERANCE);
   }

   @Test
   public void testEquals_NoSignatureAlgorithm() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      Config instance2 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      Assert.assertEquals(instance1, instance2);
   }

   @Test
   public void testEquals_WithSignatureAlgorithm() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE,
         SignatureAlgorithm.RSA_SHA256.toString());
      Config instance2 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE,
         SignatureAlgorithm.RSA_SHA256.toString());
      Assert.assertEquals(instance1, instance2);
   }

   @Test
   public void testEqualsFail1() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      Config instance2 = createConfig(ISSUER_NAME + ISSUER_NAME,
         AUTHORITY_CHAIN, AUTHORITY_KEY, TOKEN_DELEGATION_COUNT,
         TOKEN_RENEW_COUNT, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      Assert.assertFalse(instance1.equals(instance2));
   }

   @Test
   public void testEqualsFail2() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      Certificate certificate1 = EasyMock.createMock(Certificate.class);
      Certificate certificate2 = EasyMock.createMock(Certificate.class);
      List<Certificate> otherAuthCert = new ArrayList<Certificate>();
      otherAuthCert.add(certificate1);
      otherAuthCert.add(certificate2);

      List<Certificate> authorityCertChain = new ArrayList<Certificate>();
      authorityCertChain.add(certificate1);
      authorityCertChain.add(certificate2);
      Collection<List<Certificate>> validCertChains = new HashSet<List<Certificate>>();
      validCertChains.add(authorityCertChain);

      Config instance2 = createConfig(ISSUER_NAME, otherAuthCert,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         validCertChains, DEFAULT_CLOCK_TOLERANCE);
      replay(certificate1, certificate2);
      Assert.assertFalse(instance1.equals(instance2));
      verify(certificate1, certificate2);
   }

   @Test
   public void testEqualsFail3() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      PrivateKey otherKey = EasyMock.createMock(PrivateKey.class);
      Config instance2 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN, otherKey,
         TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      replay(otherKey);
      Assert.assertFalse(instance1.equals(instance2));
      verify(otherKey);
   }

   @Test
   public void testEqualsFail4() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE);
      Config instance2 = createConfig(ISSUER_NAME, AUTHORITY_CERTS3,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DUMMY_VALID_CERTS3, DEFAULT_CLOCK_TOLERANCE);
      Assert.assertFalse(instance1.equals(instance2));
   }

   @Test
   public void testEqualsFail5() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE,
         SignatureAlgorithm.RSA_SHA256.toString());
      Config instance2 = createConfig(ISSUER_NAME, AUTHORITY_CERTS3,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DUMMY_VALID_CERTS3, DEFAULT_CLOCK_TOLERANCE,
         SignatureAlgorithm.RSA_SHA512.toString());
      Assert.assertFalse(instance1.equals(instance2));
   }

   @Test
   public void testEqualsFail6() {
      Config instance1 = createConfig(ISSUER_NAME, AUTHORITY_CHAIN,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         VALID_CERTS, DEFAULT_CLOCK_TOLERANCE,
         SignatureAlgorithm.RSA_SHA256.toString());
      Config instance2 = createConfig(ISSUER_NAME, AUTHORITY_CERTS3,
         AUTHORITY_KEY, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DUMMY_VALID_CERTS3, DEFAULT_CLOCK_TOLERANCE, null);
      Assert.assertFalse(instance1.equals(instance2));
   }

   public static Config createConfig(String issuer,
      List<Certificate> authorityCert, PrivateKey authorityKey,
      int delegationCount, int renewCount, long maximumBearerTokenLifetime,
      long maximumHoKTokenLifetime, Collection<List<Certificate>> validCerts,
      long clockTolerance) {
      return createConfig(issuer, authorityCert, authorityKey, delegationCount,
         renewCount, maximumBearerTokenLifetime, maximumHoKTokenLifetime,
         validCerts, clockTolerance, null);
   }

   public static Config createConfig(String issuer,
      List<Certificate> authorityCert, PrivateKey authorityKey,
      int delegationCount, int renewCount, long maximumBearerTokenLifetime,
      long maximumHoKTokenLifetime, Collection<List<Certificate>> validCerts,
      long clockTolerance, String signatureAlgorithm) {
      return new Config(new Config.SamlAuthorityConfiguration(issuer,
         authorityCert, authorityKey, signatureAlgorithm),
         new TokenRestrictions(maximumBearerTokenLifetime,
            maximumHoKTokenLifetime, delegationCount, renewCount), validCerts,
         clockTolerance, Collections.<IDPConfig> emptyList());
   }

}
