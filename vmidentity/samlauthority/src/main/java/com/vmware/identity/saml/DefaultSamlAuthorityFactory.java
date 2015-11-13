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

import org.apache.commons.lang.Validate;
import org.opensaml.Configuration;
import org.opensaml.DefaultBootstrap;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.schema.XSString;
import org.opensaml.xml.schema.impl.XSStringMarshaller;
import org.opensaml.xml.schema.impl.XSStringUnmarshaller;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.config.SystemConfigurationException;
import com.vmware.identity.saml.ext.RSAAdvice;
import com.vmware.identity.saml.ext.RenewRestrictionType;
import com.vmware.identity.saml.ext.impl.RSAAdviceBuilder;
import com.vmware.identity.saml.ext.impl.RSAAdviceMarshaller;
import com.vmware.identity.saml.ext.impl.RSAAdviceUnmarshaller;
import com.vmware.identity.saml.ext.impl.RenewRestrictionTypeBuilder;
import com.vmware.identity.saml.ext.impl.RenewRestrictionTypeMarshaller;
import com.vmware.identity.saml.ext.impl.RenewRestrictionTypeUnmarshaller;
import com.vmware.identity.saml.ext.impl.XSNonTrimmingStringBuilder;
import com.vmware.identity.saml.impl.AuthnOnlyTokenValidator;
import com.vmware.identity.saml.impl.TokenAuthorityImpl;
import com.vmware.identity.saml.impl.TokenValidatorImpl;

/**
 * Creator/initializer of authority for issuing valid SAML tokens.
 */
public final class DefaultSamlAuthorityFactory implements SamlAuthorityFactory {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(DefaultSamlAuthorityFactory.class);

   private static final String CANNOT_INITIALIZE_OPENSAML_LIB = "Cannot initialize opensaml lib.";
   private final SignatureAlgorithm defaultSignatureAlgorithm;
   private final PrincipalAttributesExtractorFactory principalAttributesExtractorFactory;
   private final ConfigExtractorFactory configExtractorFactory;

   static
   {
      try {
         DefaultBootstrap.bootstrap();
         org.opensaml.xml.Configuration.registerObjectProvider(XSString.TYPE_NAME,
            new XSNonTrimmingStringBuilder(),
            new XSStringMarshaller(),
            new XSStringUnmarshaller()
         );
         Configuration.registerObjectProvider(RenewRestrictionType.TYPE_NAME,
            new RenewRestrictionTypeBuilder(),
            new RenewRestrictionTypeMarshaller(),
            new RenewRestrictionTypeUnmarshaller());
         Configuration.registerObjectProvider(RSAAdvice.TYPE_NAME,
            new RSAAdviceBuilder(), new RSAAdviceMarshaller(),
            new RSAAdviceUnmarshaller());
      } catch (ConfigurationException e) {
         log.error(CANNOT_INITIALIZE_OPENSAML_LIB, e);
         throw new IllegalStateException(CANNOT_INITIALIZE_OPENSAML_LIB, e);
      }
   }

   public DefaultSamlAuthorityFactory(
      SignatureAlgorithm defaultSignatureAlgorithm,
      PrincipalAttributesExtractorFactory principalAttributesExtractorFactory,
      ConfigExtractorFactory configExtractorFactory) {
      Validate.notNull(defaultSignatureAlgorithm);
      Validate.notNull(principalAttributesExtractorFactory);
      Validate.notNull(configExtractorFactory);

      this.defaultSignatureAlgorithm = defaultSignatureAlgorithm;
      this.configExtractorFactory = configExtractorFactory;
      this.principalAttributesExtractorFactory = principalAttributesExtractorFactory;
   }

   @Override
   public TokenAuthority createTokenAuthority(String tenantName)
      throws NoSuchIdPException, SystemException {

      Validate.notEmpty(tenantName);
      return createTokenAuthority(tenantName, getConfigExtractor(tenantName));
   }

   @Override
   public TokenValidator createTokenValidator(String tenantName)
      throws NoSuchIdPException, SystemException {

      Validate.notEmpty(tenantName);
      return createTokenValidator(tenantName, getConfigExtractor(tenantName));
   }

   @Override
   public TokenValidator createAuthnOnlyTokenValidator(String tenantName)
      throws NoSuchIdPException, SystemException {
      Validate.notEmpty(tenantName);
      return createAuthnOnlyTokenValidator(tenantName, getConfigExtractor(tenantName));
   }

   @Override
   public TokenServices createTokenServices(String tenantName)
      throws NoSuchIdPException, SystemException {

      final ConfigExtractor configExtractor = getConfigExtractor(tenantName);
      return new TokenServices(createTokenAuthority(tenantName, configExtractor),
         createTokenValidator(tenantName, configExtractor), createAuthnOnlyTokenValidator(tenantName, configExtractor));
   }

   private TokenAuthority createTokenAuthority(String tenantName, ConfigExtractor configExtractor) {

      TokenAuthority tokenAuthorityImpl =
          new TokenAuthorityImpl(
              this.defaultSignatureAlgorithm,
              principalAttributesExtractorFactory.getPrincipalAttributesExtractor(tenantName),
              configExtractor
          );

      Validate.notNull(tokenAuthorityImpl);
      log.debug("Created token authority.");

      return tokenAuthorityImpl;
   }

   private TokenValidator createTokenValidator(String tenantName, ConfigExtractor configExtractor) {

      PrincipalAttributesExtractor attributesExtractor = principalAttributesExtractorFactory
              .getPrincipalAttributesExtractor(tenantName);

      return new TokenValidatorImpl(
          new AuthnOnlyTokenValidator(
                  configExtractor,
                  attributesExtractor),
          attributesExtractor
      );
   }

   private TokenValidator createAuthnOnlyTokenValidator(String tenantName, ConfigExtractor configExtractor) {

      return new AuthnOnlyTokenValidator(configExtractor,
         principalAttributesExtractorFactory
            .getPrincipalAttributesExtractor(tenantName));
   }

   private ConfigExtractor getConfigExtractor(String tenantName) throws NoSuchIdPException,
      SystemException {
      assert tenantName != null;

      try {
         return configExtractorFactory.getConfigExtractor(tenantName);
      } catch (SystemConfigurationException e) {
         throw new SystemException(
            "Saml configuration cannot be extracted or it is missing.", e);
      }
   }
}
