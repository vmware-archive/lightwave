/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

package com.vmware.provider;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.security.Security;
import java.security.cert.CRL;
import java.security.cert.CRLException;
import java.security.cert.CertStore;
import java.security.cert.CertStoreException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509CRL;
import java.security.cert.X509CRLSelector;
import java.security.cert.X509Certificate;
import java.util.Collection;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.vecs.AlreadyExistsException;
import com.vmware.identity.vecs.VMwareEndpointCertificateStore;
import com.vmware.identity.vecs.VecsStoreFactory;

public class VecsCertStoreEngineTest {

   private static final String _storeName = "crl-store";
   private static CertStore _certStore;
   private static VecsStoreFactory _factory = VecsStoreFactory.getVecsStoreFactoryViaIPC();

   @BeforeClass
   static public void init() throws Exception {
      Security.addProvider(new VMwareSecurityProvider());
      try {
         _factory.createCertStore(_storeName);
      } catch (AlreadyExistsException e) {

      }
      _certStore = CertStore.getInstance("VCS", new VecsCertStoreParameters(_storeName));

   }

   @AfterClass
   static public void destroy() throws Exception {
      _factory.deleteCertStore(_storeName);
   }

   @Test
   public void test() throws CertificateException, CRLException, CertStoreException {
      VMwareEndpointCertificateStore vecs = _factory.getVecsStore(_storeName);
      vecs.openStore();
      String baseEntryAlias = "entryAlias";
      X509CRL inCRL = getCRL();
      X509Certificate inCert = getCertificate1();

      vecs.addCrlEntry(baseEntryAlias, inCRL);
      X509CRLSelector selector = new X509CRLSelector();
      selector.addIssuer(inCert.getIssuerX500Principal());
      selector.setCertificateChecking(inCert);
      Collection<CRL> coll = (Collection<CRL>) _certStore.getCRLs(selector);
      Assert.assertEquals(0, coll.size());

      vecs.deleteEntryByAlias(baseEntryAlias);
      vecs.closeStore();
   }

   private X509CRL getCRL() throws CertificateException, CRLException {
	      String crlString = "-----BEGIN X509 CRL-----\n"
	            + "MIIBqDCBkQIBATANBgkqhkiG9w0BAQUFADA8MQswCQYDVQQGEwJVUzELMAkGA1UE\n"
	            + "CBMCQ0ExDDAKBgNVBAcTA1JlZDESMBAGCSqGSIb3DQEJARYDYUBiFw0xNDA4MDYy\n"
	            + "MTI1MTdaFw0xNDA5MDUyMTI1MTdaoCEwHzAdBgNVHRQEFgIUEAAQABAAEAAQABAA\n"
	            + "EAAQABAAEAAwDQYJKoZIhvcNAQEFBQADggEBAJfgiM/8/zziCUB9/jnOP/P7V4tX\n"
	            + "zy1KLqfxgYEKhYF6DJ/iqzRjvF6x/GBJD18Gnm29dLiwMOcfqZsaRmX1wySjt/Jj\n"
	            + "+aMaNqPYcaUkRHy8eQIIIhqig76fX33pKy9E0/lzQE0qjuU5ius/b4PxCSecrRCg\n"
	            + "C3XSPjyDjL79bXksdFi6GysMNrUCltYXth4T3/biTNKwcn2laeyHyYZajyQT2lqa\n"
	            + "eO/i1TIkmSkOy0wFY/1TTfG65fx4lUmRHGvtFUCGkwSg/YKgzv3psPKchcYdo+WN\n"
	            + "/EsDuc4rwqaIJs/VZ61ub2E+NN3+HY3tLe1e2Cnq66/zGiMqL6UvNsxCYfE=\n"
	            + "-----END X509 CRL-----\n";
	      InputStream is;
	      X509CRL crl = null;
	      is = new ByteArrayInputStream(crlString.getBytes());
	      CertificateFactory cf = CertificateFactory.getInstance("X.509");
	      crl = (X509CRL) cf.generateCRL(is);
	      return crl;
	   }

	   private X509Certificate getCertificate1() throws CertificateException {
	      String certificateString = "-----BEGIN CERTIFICATE-----\n"
	            + "MIIDkDCCAnigAwIBAgIRAN+NbA6d3U+aKpXIRKKhYfowDQYJKoZIhvcNAQELBQAw\n"
	            + "TzELMAkGA1UEBhMCVVMxDzANBgNVBAoTBlZNd2FyZTEbMBkGA1UECxMSVk13YXJl\n"
	            + "IEVuZ2luZWVyaW5nMRIwEAYDVQQDEwlDZXJUb29sQ0EwHhcNMTIxMTIxMDAzNjE3\n"
	            + "WhcNMjIxMTE5MDAzNjE3WjBPMQswCQYDVQQGEwJVUzEPMA0GA1UEChMGVk13YXJl\n"
	            + "MRswGQYDVQQLExJWTXdhcmUgRW5naW5lZXJpbmcxEjAQBgNVBAMTCUNlclRvb2xD\n"
	            + "QTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALqfVt5lOc1gPIgy27kl\n"
	            + "PvbAS+sLWpReg4NbfUx3gOMA5Ga18nBfy4pbmIv5tZIg95TFresULi83lZwZOJ6M\n"
	            + "svOW7Lu1UQPDofBEiqZ0j7GFj0qbW4oUP3ePqTwBIRr2QXhgRPHhR9wxKfc5Yvl7\n"
	            + "trmkaQvSbit+7OtHoirSaBCyLnU1D35UbSzT1qINkrgzS/aCHk4r2wuQDaIWCi4O\n"
	            + "FdaSgMldEoFD6lhxnDXjZpXP3Cdkd20aIP7dDNj2EXJhgxtGih9HFd9tEEB6wOem\n"
	            + "VRLchBLno9RF98P4tvjlfbuIQImrA4WoWp7VlcFbc/RlPOe0ubOHZ/RXDLO90KRl\n"
	            + "UmsCAwEAAaNnMGUwEgYDVR0TAQH/BAgwBgEB/wIBATAOBgNVHQ8BAf8EBAMCAQYw\n"
	            + "HQYDVR0OBBYEFB4QOUMmKZ3oVHst0ghXZTB+xwgWMCAGA1UdEQQZMBeBD3ZtY2FA\n"
	            + "dm13YXJlLmNvbYcEfwAAATANBgkqhkiG9w0BAQsFAAOCAQEAVREDPIqZI90veiXV\n"
	            + "hrwXuay9HQpyFOfPq8wXQlrAXsO83toWsDK8bFhiRBwS4qmlI3kIhu25hKUBdyJG\n"
	            + "KSAoSojJkMtHhZV4pHWG6h3lUElZ/qSwfgZHfougaN/2MYmx+KL4hjqvXeJhD6ps\n"
	            + "zHeNAk2az4LI1u2Xt2CBNKxOLYOgjInVNlF9qlF+EcZgr9xKtXnKcBK3c7ErWLtX\n"
	            + "6oM7ZMbGvHd49+sKS0cy9RWomemhS6+LtvBb1Bk9gafmRR7nMfqHBWM0OKg0Wtfj\n"
	            + "w6v8QfJWLI4MeBexS5VV2zLAOH3FD6GMJSmICkRKsVuBd7aqBEn2RMbzyW0bIvHr\n"
	            + "8vVU/A==\n" + "-----END CERTIFICATE-----\n";

	      InputStream is;
	      X509Certificate cert = null;
	      is = new ByteArrayInputStream(certificateString.getBytes());
	      CertificateFactory cf = CertificateFactory.getInstance("X.509");
	      cert = (X509Certificate) cf.generateCertificate(is);
	      return cert;
	   }
}
