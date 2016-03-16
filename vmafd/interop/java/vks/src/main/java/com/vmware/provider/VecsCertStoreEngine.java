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

import java.security.InvalidAlgorithmParameterException;
import java.security.cert.CRL;
import java.security.cert.CRLSelector;
import java.security.cert.CertSelector;
import java.security.cert.CertStoreException;
import java.security.cert.CertStoreParameters;
import java.security.cert.CertStoreSpi;
import java.security.cert.Certificate;
import java.security.cert.X509CRLSelector;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.security.cert.X509CRL;

import javax.security.auth.x500.X500Principal;

import com.vmware.identity.vecs.VMwareEndpointCertificateStore;
import com.vmware.identity.vecs.VecsEntry;
import com.vmware.identity.vecs.VecsEntryInfoLevel;
import com.vmware.identity.vecs.VecsStoreFactory;

final public class VecsCertStoreEngine extends CertStoreSpi{
   private final VMwareEndpointCertificateStore _vecs;
   private String _storeName = null;
   private final Object _monitor = new Object();

   public VecsCertStoreEngine(CertStoreParameters params) throws InvalidAlgorithmParameterException {
      super(params);

      if (params == null || !(params instanceof VecsCertStoreParameters)) {
         throw new IllegalArgumentException("params is expected to be of type VecsCertStoreParameters");
      }
      _storeName = ((VecsCertStoreParameters) params).getStoreName();
      _vecs = VecsStoreFactory.getVecsStoreFactoryViaIPC().getVecsStore(_storeName);
   }

   @Override
   public Collection<? extends Certificate> engineGetCertificates(
         CertSelector selector) throws CertStoreException {
      throw new UnsupportedOperationException("This is not implemented yet. Use VecsKeyStore API.");
   }

   @Override
   public Collection<? extends CRL> engineGetCRLs(CRLSelector selector)
         throws CertStoreException {
      ArrayList<X509CRL> resList = new ArrayList<X509CRL>();

      X500Principal targetCertIssuer = null;

      if(selector == null) {
          throw new CertStoreException("CRLSelector argument is null");
      }

      if (selector instanceof X509CRLSelector) {
         X509Certificate targetCert = ((X509CRLSelector)selector).getCertificateChecking();
         targetCertIssuer = targetCert.getIssuerX500Principal();
      }
      synchronized(_monitor) {
         try {
            _vecs.openStore();
            Enumeration<VecsEntry> entryEnum = _vecs.enumerateEntries(VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);
            while (entryEnum.hasMoreElements()) {
               VecsEntry entry = entryEnum.nextElement();
               X500Principal crlIssuer = entry.crl.getIssuerX500Principal();
               if ((targetCertIssuer == null || targetCertIssuer.equals(crlIssuer)) && selector.match(entry.crl)) {
                  resList.add(entry.crl);
               }
            }
         } finally {
         _vecs.closeStore();
         }
      }
      return resList;
   }
}
