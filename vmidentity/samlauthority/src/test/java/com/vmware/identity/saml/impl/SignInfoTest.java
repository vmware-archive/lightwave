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
package com.vmware.identity.saml.impl;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import java.security.PrivateKey;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.junit.Test;

public class SignInfoTest {

   @Test
   public void testSignInfoValidateGetSigningCertEndTime()
      throws CertificateException {
      long now = new Date().getTime();
      long sixtyMinutes = 60 * 60 * 1000;
      Date signingCertEndTime = new Date(now + sixtyMinutes);
      X509Certificate cert = createMock(X509Certificate.class);
      expect(cert.getNotAfter()).andReturn(signingCertEndTime);
      PrivateKey authorityKey = createMock(PrivateKey.class);
      replay(cert, authorityKey);

      SignInfo signInfo = new SignInfo(authorityKey, createCertPath(cert), null);

      assertEquals(signingCertEndTime, signInfo.getSigningCertificate().getNotAfter());
      verify(cert, authorityKey);
   }

   private CertPath createCertPath(X509Certificate... certificates)
      throws CertificateException {
      List<Certificate> certPathList = new ArrayList<Certificate>();
      for (Certificate certificate : certificates) {
         certPathList.add(certificate);
      }
      CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
      CertPath certPath = certFactory.generateCertPath(certPathList);
      return certPath;
   }
}
