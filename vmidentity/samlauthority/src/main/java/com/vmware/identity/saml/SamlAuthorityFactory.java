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

/**
 * Implementation of this interface creates/initializes authority for issuing
 * valid SAML tokens per tenant.
 */
public interface SamlAuthorityFactory {

   /**
    * Creates token authority for a tenant.
    *
    * @param tenantName
    *           Cannot be null.
    * @throws NoSuchIdPException
    *            when there is no such IdP
    * @throws SystemException
    *            when tenant configuration cannot be extracted.
    * @return token authority. Cannot be null.
    */
   TokenAuthority createTokenAuthority(String tenantName)
      throws NoSuchIdPException, SystemException;

   /**
    * Creates token validator for a given tenant
    * (validates all statements within the assertion)
    * @param tenantName
    *           Cannot be null.
    * @throws NoSuchIdPException
    *            when there is no such IdP
    * @throws SystemException
    *            when tenant configuration cannot be extracted.
    * @return token validator. Cannot be null.
    */
   TokenValidator createTokenValidator(String tenantName)
      throws NoSuchIdPException, SystemException;

   /**
    * Creates Authn only token validator for a given tenant
    * (only the subject of the assertion is validated)
    *
    * @param tenantName
    *           Cannot be null.
    * @throws NoSuchIdPException
    *            when there is no such IdP
    * @throws SystemException
    *            when tenant configuration cannot be extracted.
    * @return token validator. Cannot be null.
    */
   TokenValidator createAuthnOnlyTokenValidator(String tenantName)
      throws NoSuchIdPException, SystemException;

   /**
    * Creates all token services for a given tenant
    *
    * @param tenantName
    *           Cannot be null.
    * @throws NoSuchIdPException
    *            when there is no such IdP
    * @throws SystemException
    *            when tenant configuration cannot be extracted.
    * @return token services. Cannot be null.
    */
   TokenServices createTokenServices(String tenantName)
      throws NoSuchIdPException, SystemException;

   /**
    * Container for all SAML token services
    */
   static public final class TokenServices {
      private final TokenAuthority authority;
      private final TokenValidator validator;
      private final TokenValidator authnOnlyValidator;

      public TokenServices(TokenAuthority authority, TokenValidator validator, TokenValidator authnOnlyValidator) {
         assert authority != null;
         assert validator != null;
         assert authnOnlyValidator != null;
         this.authority = authority;
         this.validator = validator;
         this.authnOnlyValidator = authnOnlyValidator;

      }

      /**
       * @return the authority, not null
       */
      public TokenAuthority getAuthority() {
         return authority;
      }

      /**
       * @return the validator, not null
       */
      public TokenValidator getValidator() {
         return validator;
      }

      /**
       * @return the authnonly validator, not null
       */
      public TokenValidator getAuthnOnlyValidator() {
         return authnOnlyValidator;
      }
   }
}
