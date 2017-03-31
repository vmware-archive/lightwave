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
package com.vmware.identity.sts.impl;

import java.security.cert.X509Certificate;

import org.apache.commons.lang.Validate;
import org.oasis_open.docs.ws_sx.ws_trust._200512.UseKeyType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.w3._2000._09.xmldsig_.SignatureType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.sts.ContradictoryHoKConditionsException;
import com.vmware.identity.sts.InvalidSecurityHeaderException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.util.JAXBExtractor;

/**
 * This class has the purpose to provider functionality that is capable of
 * analyzing HoK conditions and decide what the signing certificate should be
 */
public final class HoKConditionsAnalyzer {

   public static final String BEARER_KEY_TYPE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Bearer";
   public static final String HOK_KEY_TYPE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/PublicKey";
   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(HoKConditionsAnalyzer.class);

   /**
    * Evaluates the different conditions for token confirmation type and outputs
    * the certificate that should be embedded into the SAML token subject
    * confirmation data.
    *
    * @param req
    *           not null
    * @param delegateCertificate
    *           the certificate of the subject extracted from wst:delegateTo
    *           field. can be null
    * @param authSamlToken
    *           SamlToken by which the request was authenticated. can be null.
    * @return the certificate that should be embedded into the SAML token as
    *         subject confirmation data or null if the token should be bearer
    */
   public X509Certificate getSigningCertificate(Request req,
      X509Certificate delegateCertificate, ServerValidatableSamlToken authSamlToken) {
      Validate.notNull(req);
      String keyType = req.getRst().getKeyType();
      String signatureId = extractSignatureId(req.getRst().getUseKey(),
         req.getHeader());
      X509Certificate reqSigningCertificate = (req.getSignature() == null) ? null
         : req.getSignature().getCertificate();

      return analyze(keyType, signatureId, delegateCertificate,
         reqSigningCertificate, actAsReq(req), authSamlToken);
   }

   /**
    * Extracts the signature id from the usekey element and verify that the
    * signature id is valid
    *
    * @param useKey
    *           can be null
    * @param header
    *           cannot be null
    * @return
    */
   private String extractSignatureId(UseKeyType useKey,
      SecurityHeaderType header) {
      assert header != null;
      String signatureId = null;
      if (useKey != null) {
         // If UseKey is not null then it should have not null signature
         // (enforced by schema)
         signatureId = useKey.getSig();
         SignatureType signature = JAXBExtractor.extractFromSecurityHeader(header, SignatureType.class);
         if (signature == null
            || !signatureId.equalsIgnoreCase(signature.getId())) {
            throw new InvalidSecurityHeaderException(
               "SignatureId from UseKey doesn't match signature from the SecurityHeader");
         }
      }
      return signatureId;
   }

   /**
    * Evaluates the different conditions for token confirmation type and outputs
    * the certificate that should be embedded into the SAML token subject
    * confirmation data.
    *
    * @param keyType
    *           the requested KeyType. Can be null.
    * @param signatureId
    *           the value of the UseKey element. Can be null.
    * @param delegateCert
    *           the certificate of the principal which the token should be
    *           delegated to. Can be null.
    * @param requestSigningCertificate
    *           the certificate used to sign the request. Can be null only if
    *           the signatureId is null.
    * @param actAsReq
    *           whether the request is for 'actAs' delegation
    * @param SamlToken
    *           SamlToken by which the request was authenticated. can be null.
    * @return the subject confirmation certificate or null if the token should
    *         be bearer.
    * @throws ContradictoryHoKConditionsException
    */
   private X509Certificate analyze(String keyType, String signatureId,
      X509Certificate delegateCert, X509Certificate requestSigningCertificate,
      boolean actAsReq, ServerValidatableSamlToken samlToken) throws ContradictoryHoKConditionsException {

      if (signatureId != null && requestSigningCertificate == null) {
         throw new InvalidSecurityHeaderException(
            "SignatureId is not null but the signing certificate is");
      }

      validateNotContradicting(signatureId, delegateCert);

      X509Certificate hokCertificate = null;
      if (keyType == null) {
         hokCertificate = getHoKCertificate(requestSigningCertificate,
            signatureId, delegateCert, samlToken);
      } else if (keyType.equalsIgnoreCase(BEARER_KEY_TYPE)) {
         // UseKey value doesn't matter in this case because of the requested
         // KeyType
         if (delegateToReq(delegateCert) || actAsReq) {
            throw new ContradictoryHoKConditionsException(
               "Cannot issue delegated bearer token");
         }
      } else if (keyType.equalsIgnoreCase(HOK_KEY_TYPE)) {
          if ( ( samlToken != null ) && (samlToken.getConfirmationType() == ConfirmationType.BEARER) ) {
              throw new ContradictoryHoKConditionsException(
                      "Cannot issue Hok token authenticating by bearer token.");
          }
         hokCertificate = getHoKCertificate(requestSigningCertificate,
            signatureId, delegateCert, samlToken);
         if (hokCertificate == null) {
            throw new ContradictoryHoKConditionsException(
               "Cannot find certificate to use.");
         }
      } else {
         throw new ContradictoryHoKConditionsException("Unknown KeyType");
      }
      logger.debug("Found HoK certificate {}", hokCertificate);

      return hokCertificate;
   }

   /**
    * @param signingCertificate
    *           the certificate used to sign the request. Can be null only if
    *           the signatureId is null.
    * @param signatureId
    *           the value of the UseKey element. Can be null.
    * @param delegateCert
    *           the certificate of the principal which the token should be
    *           delegated to. Can be null.
    * @param samlToken
    *           SamlToken by which the request was authenticated. can be null.
    * @return
    */
   private X509Certificate getHoKCertificate(
      X509Certificate signingCertificate, String signatureId,
      X509Certificate delegateCert, ServerValidatableSamlToken samlToken) {

      X509Certificate hokCertificate = null;
      if (signatureId != null) {
          if ( ( samlToken == null ) || (samlToken.getConfirmationType() == ConfirmationType.HOLDER_OF_KEY ) ) {
              hokCertificate = signingCertificate;
          }
      } else {
          if ( delegateToReq(delegateCert) ){
              if ( ( samlToken == null ) || (samlToken.getConfirmationType() == ConfirmationType.HOLDER_OF_KEY) ) {
                  hokCertificate = delegateCert;
              } else {
                  throw new ContradictoryHoKConditionsException(
                          "Cannot issue delegated token authenticating by bearer token.");
              }
          } else {
              if ( ( samlToken == null ) || (samlToken.getConfirmationType() == ConfirmationType.HOLDER_OF_KEY) ) {
                  hokCertificate = signingCertificate;
              }
          }
      }
      return hokCertificate;
   }

   /**
    * Validates that UseKey and DelegateTo are not present at the same time
    * because in this case the request cannot be satisfied.
    *
    * @param signatureId
    * @param delegateCert
    * @throws ContradictoryHoKConditionsException
    */
   private void validateNotContradicting(String signatureId,
      X509Certificate delegateCert) throws ContradictoryHoKConditionsException {
      if (signatureId != null && delegateToReq(delegateCert)) {
         throw new ContradictoryHoKConditionsException(
            "Request cannot be satisfied - both UseKey and DelegateTo are present");
      }
   }

   private boolean actAsReq(Request req) {
      return req.getActAsToken() != null;
   }

   private boolean delegateToReq(X509Certificate delegateCert) {
      return delegateCert != null;
   }

}
