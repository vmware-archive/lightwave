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
package com.vmware.identity.saml;

import org.w3c.dom.Document;

import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.util.TimePeriod;

/**
 * This class will be used for the result of an issue token operation. It will
 * contain the issued token, signature algorithm used for signing, the time
 * interval in which the token will be valid and the type of issued token -
 * BEARER, HoK.
 */
public final class SamlToken {

   private final Document token;
   private final SignatureAlgorithm signatureAlgorithm;
   private final TimePeriod validity;
   private final ConfirmationType confirmationType;
   private final boolean delegable;
   private final boolean renewable;

   public SamlToken(Document token, SignatureAlgorithm signatureAlgorithm,
      TimePeriod validity, ConfirmationType confirmationType, boolean delegable, boolean renewable) {
      assert token != null;
      assert signatureAlgorithm != null;
      assert validity != null;
      assert confirmationType != null;

      this.token = token;
      this.signatureAlgorithm = signatureAlgorithm;
      this.validity = validity;
      this.confirmationType = confirmationType;
      this.delegable = delegable;
      this.renewable = renewable;
   }

   public Document getDocument() {
      return token;
   }

   public SignatureAlgorithm getSignatureAlgorithm() {
      return signatureAlgorithm;
   }

   public TimePeriod getValidity() {
      return validity;
   }

   public ConfirmationType getConfirmationType() {
      return confirmationType;
   }

   public boolean isDelegable() {
      return delegable;
   }

   public boolean isRenewable() {
      return renewable;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + signatureAlgorithm.hashCode();
      result = prime * result + validity.hashCode();
      result = prime * result + token.hashCode();
      result = prime * result + confirmationType.hashCode();
      result = prime * result + ((delegable) ? 1 : 0);
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

      SamlToken other = (SamlToken) obj;
      return signatureAlgorithm == other.signatureAlgorithm
         && validity.equals(other.validity) && token.equals(other.token)
         && confirmationType.equals(other.confirmationType)
         && delegable == other.delegable;
   }

   @Override
   public String toString() {
      return "IssuedToken [token=" + token + ", algorithmUsed="
         + signatureAlgorithm + ", validity=" + validity
         + ", confirmationType=" + confirmationType.toString() + ", delegable="
         + delegable + "]";
   }

}
