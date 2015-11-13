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

import java.security.cert.X509Certificate;

import org.apache.commons.lang.ObjectUtils;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.saml.ServerValidatableSamlToken;

/**
 * Insert your comment for Request here
 */
public final class Request {

   private final SecurityHeaderType header;
   private final RequestSecurityTokenType rst;
   private final Signature signature;
   private final ServerValidatableSamlToken token;
   private final ServerValidatableSamlToken actAsToken;

   /**
    * Creates a request
    *
    * @param header
    *           request header, mandatory
    * @param rst
    *           request body, mandatory
    * @param signature
    *           signing certificate plus its location in the request, optional.
    *           Indicate for presence of a valid request signature calculated
    *           with the private key corresponding to the public key inside this
    *           certificate.
    * @param token
    *           represents schema validated saml token taken from the request,
    *           which is not token validated, optional.
    * @param actAsToken
    *           represents schema validated saml token taken from the
    *           RST::ActAs, which is not token validated, optional.
    */
   public Request(SecurityHeaderType header, RequestSecurityTokenType rst,
      Signature signature, ServerValidatableSamlToken token,
      ServerValidatableSamlToken actAsToken) {
      assert header != null;
      assert rst != null;
      assert (rst.getActAs() != null) == (actAsToken != null) : "Failed ActAs token precondition!";

      this.header = header;
      this.rst = rst;
      this.signature = signature;
      this.token = token;
      this.actAsToken = actAsToken;
   }

   /**
    * @return the header
    */
   public SecurityHeaderType getHeader() {
      return header;
   }

   /**
    * @return the rst
    */
   public RequestSecurityTokenType getRst() {
      return rst;
   }

   /**
    * @return the signature
    */
   public Signature getSignature() {
      return signature;
   }

   /**
    * @return SAML token
    */
   public ServerValidatableSamlToken getSamlToken() {
      return token;
   }

   /**
    * @return actAs SAML token
    */
   public ServerValidatableSamlToken getActAsToken() {
      return actAsToken;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + header.hashCode();
      result = prime * result + rst.hashCode();
      result = prime * result
         + ((signature == null) ? 0 : signature.hashCode());
      result = prime * result + ((token == null) ? 0 : token.hashCode());
      result = prime * result
         + ((actAsToken == null) ? 0 : actAsToken.hashCode());
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || getClass() != obj.getClass()) {
         return false;
      }

      Request other = (Request) obj;
      return header.equals(other.header) && rst.equals(other.rst)
         && ObjectUtils.equals(other.signature, signature)
         && ObjectUtils.equals(other.token, token)
         && ObjectUtils.equals(other.actAsToken, actAsToken);
   }

   /**
    * Instances of this class will hold signing certificate and place it is
    * located per request.
    */
   public static class Signature {

      private X509Certificate certificate;
      private CertificateLocation location;

      /**
       * Creates signature from signing certificate and location it is in the
       * request.
       *
       * @param signingCertificate
       *           mandatory
       * @param location
       *           mandatory
       */
      public Signature(X509Certificate signingCertificate,
         CertificateLocation location) {
         assert signingCertificate != null;
         assert location != null;

         this.certificate = signingCertificate;
         this.location = location;
      }

      /**
       * @return signing certificate. Cannot be null.
       */
      public X509Certificate getCertificate() {
         return certificate;
      }

      /**
       * @return location signing certificate is placed in the request. Cannot
       *         be null.
       */
      public CertificateLocation getLocation() {
         return location;
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = 1;
         result = prime * result + certificate.hashCode();
         result = prime * result + location.hashCode();
         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (this == obj) {
            return true;
         }
         if (obj == null || getClass() != obj.getClass()) {
            return false;
         }

         Signature other = (Signature) obj;
         return certificate.equals(other.certificate)
            && location != other.location;
      }

   }

   /**
    * Represents places signing certificate is located in the request.
    */
   public static enum CertificateLocation {
      /**
       * BinarySecurityToken from {@link SecurityHeaderType}
       */
      BST,
      /**
       * Assertion from {@link SecurityHeaderType}
       */
      ASSERTION
   }
}
