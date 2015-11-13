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
package com.vmware.identity.sts;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.easymock.EasyMock;

public class TestUtil {

   public static final String ISSUER_NAME = "issuer";
   public static final PrivateKey AUTHORITY_KEY = EasyMock
      .createMock(PrivateKey.class);
   public static final X509Certificate DUMMY_CERT1 = EasyMock
      .createMock(X509Certificate.class);
   public static final X509Certificate DUMMY_CERT2 = EasyMock
      .createMock(X509Certificate.class);
   public static final X509Certificate DUMMY_CERT3 = EasyMock
      .createMock(X509Certificate.class);
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

}
