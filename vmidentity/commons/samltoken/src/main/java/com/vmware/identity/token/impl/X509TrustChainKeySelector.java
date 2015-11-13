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
package com.vmware.identity.token.impl;

import java.security.GeneralSecurityException;
import java.security.Key;
import java.security.cert.CertPathBuilder;
import java.security.cert.CertPathBuilderException;
import java.security.cert.CertPathBuilderResult;
import java.security.cert.CertStore;
import java.security.cert.Certificate;
import java.security.cert.CollectionCertStoreParameters;
import java.security.cert.PKIXBuilderParameters;
import java.security.cert.PKIXCertPathBuilderResult;
import java.security.cert.TrustAnchor;
import java.security.cert.X509CertSelector;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.crypto.AlgorithmMethod;
import javax.xml.crypto.KeySelector;
import javax.xml.crypto.KeySelectorException;
import javax.xml.crypto.KeySelectorResult;
import javax.xml.crypto.XMLCryptoContext;
import javax.xml.crypto.dsig.keyinfo.KeyInfo;
import javax.xml.crypto.dsig.keyinfo.X509Data;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A {@link KeySelector} implementation which selects a Key from a X.509
 * certificate chain embedded in the Signature's KeyInfo element. The chain is
 * validated to anchor on any of the client's provided most trusted root
 * certificates and, if it doesn't, no key is returned.
 * <p>
 * The certificate chain must be encoded in one or more
 * &lt;ds:X509Certificate&gt; elements within a single &lt;ds:X509Data&gt;
 * element residing in the Signature's &lt;ds:KeyInfo&gt; element. If the
 * KeyInfo has more than one child or it's child is not X509Data, the
 * KeySelector fails. The non-X509Certificate children of the X509Data are
 * ignored.
 */
public class X509TrustChainKeySelector extends KeySelector {

   private static final String BUILDER_PROVIDER_PKIX = "PKIX";
   private static final String CERTSTORE_PROVIDER_COLLECTION = "Collection";
   private static final String UNRECOGNIZED_DS_KEYINFO = "Unrecognized <ds:KeyInfo> element";

   private final Logger _log;
   private final Set<TrustAnchor> _trustAnchors;

   /**
    * Create an X509TrustChainKeySelector instance which will accept a chain
    * rooted at any of the provided certificates.
    *
    * @param trustedRoots
    */
   public X509TrustChainKeySelector(X509Certificate... trustedRoots) {
      _log = LoggerFactory.getLogger(X509TrustChainKeySelector.class);

      checkCtorArgsNotNull(trustedRoots);

      _trustAnchors = new HashSet<TrustAnchor>();
      for (X509Certificate cert : trustedRoots) {
         _trustAnchors.add(new TrustAnchor(cert, null));
      }
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public KeySelectorResult select(KeyInfo keyInfo, Purpose purpose,
      AlgorithmMethod method, XMLCryptoContext context)
      throws KeySelectorException {

      if (purpose != Purpose.VERIFY) {
         _log
            .warn("Incorrect usage: this selector only returns verification keys");

         return fixedKeyResult(null);
      }

      List<X509Certificate> keyInfoCertificates = extractCertificateList(keyInfo);
      if (keyInfoCertificates.isEmpty()) {

         // If we were unable to find any certificates AND the user have
         // provided just single trusted key, return that key.
         if (_trustAnchors.size() == 1) {
            return fixedKeyResult(_trustAnchors.iterator().next()
               .getTrustedCert().getPublicKey());
         }

         return fixedKeyResult(null);
      }

      final X509Certificate signingCertificate = keyInfoCertificates.get(0);
      if (verifyTrustedPathExists(signingCertificate, keyInfoCertificates)) {
         return fixedKeyResult(signingCertificate.getPublicKey());
      }

      return fixedKeyResult(null);
   }

   /**
    * Helper: extracts the certificate chain from the KeyInfo. If no KeyInfo is
    * provided (i.e. {@code keyInfo} is null) or the XML sturcture under the
    * KeyInfo is not recognized, an empty list is returned.
    */
   private List<X509Certificate> extractCertificateList(KeyInfo keyInfo) {

      if (keyInfo == null) {
         return Collections.emptyList();
      }

      final X509Data x509Data;
      if (keyInfo.getContent().size() == 1
         && keyInfo.getContent().get(0) instanceof X509Data) {

         x509Data = (X509Data) keyInfo.getContent().get(0);

      } else {
         _log.info(UNRECOGNIZED_DS_KEYINFO
            + ": should have just one child of type <ds:X509Data>");
         return Collections.emptyList();
      }

      final List<?> x509Certificates = x509Data.getContent();
      List<X509Certificate> extracted = new ArrayList<X509Certificate>();
      for (Object cert : x509Certificates) {
         if (cert instanceof X509Certificate) {
            extracted.add((X509Certificate) cert);
         }
      }

      return extracted;
   }

   /**
    * Returns true if (and only if) there exists a certificate path (chain) to
    * the given {@code target} certificate, anchored at any of the certificate,
    * provided in the constructor, and containing only certificates from the
    * provided {@code nodes} set.
    */
   private boolean verifyTrustedPathExists(X509Certificate target,
      Collection<X509Certificate> nodes) throws KeySelectorException {

      try {
         CertPathBuilder pathBuilder = CertPathBuilder
            .getInstance(BUILDER_PROVIDER_PKIX);

         CertStore builderStore = CertStore.getInstance(
            CERTSTORE_PROVIDER_COLLECTION, new CollectionCertStoreParameters(
               nodes));

         X509CertSelector certSelector = new X509CertSelector();
         certSelector.setCertificate(target);

         PKIXBuilderParameters buildParams = new PKIXBuilderParameters(
            _trustAnchors, certSelector);
         buildParams.addCertStore(builderStore);
         buildParams.setRevocationEnabled(false);

         CertPathBuilderResult builderResult = pathBuilder.build(buildParams);

         if (_log.isDebugEnabled()) {
            dumpPathBuilderResult((PKIXCertPathBuilderResult) builderResult);
         }

         return true;

      } catch (CertPathBuilderException e) {
         _log.info("Failed to find trusted path to signing certificate <"
            + target.getSubjectX500Principal().getName() + ">", e);
         return false;

      } catch (GeneralSecurityException e) {
         String message = "Couldnt't create standard security object. "
            + "Possibly non-compliant underlying Java implementation.";
         _log.error(message, e);
         throw new KeySelectorException(message, e);
      }
   }

   /**
    * Helper: create and return a {@link KeySelectorResult} instance which
    * always returns the provided Key.
    */
   private KeySelectorResult fixedKeyResult(final Key key) {

      return new KeySelectorResult() {
         @Override
         public Key getKey() {
            return key;
         }
      };
   }

   /**
    * Helper: Log (at debug level) a path created by a PKIX CertPathBuilder
    * starting from the path's target certificate and ending at the root
    * certificate.
    */
   private void dumpPathBuilderResult(PKIXCertPathBuilderResult builderResult) {
      StringBuilder message = new StringBuilder("Trusted path found: ");

      for (Certificate cert : builderResult.getCertPath().getCertificates()) {
         message
            .append('<')
            .append(
               ((X509Certificate) cert).getSubjectX500Principal().getName())
            .append("> -> ");
      }

      message
         .append('<')
         .append(
            builderResult.getTrustAnchor().getTrustedCert()
               .getSubjectX500Principal().getName()).append('>');

      _log.debug(message.toString());
   }

   /**
    * Helper: invoked by the constructor the check that the supplied arguments
    * are all non-{@code null}.
    */
   private static void checkCtorArgsNotNull(X509Certificate[] certs) {
      if (certs == null || certs.length == 0) {
         throw new IllegalArgumentException(
            "Expected one or more trusted certificates, but got null");
      }

      for (X509Certificate cert : certs) {
         if (cert == null) {
            throw new IllegalArgumentException(
               "Expected certificate, but got null");
         }
      }
   }
}
