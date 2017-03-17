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

import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AttributeConfig;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.InvalidPrincipalException;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.NameId;
import com.vmware.identity.saml.ServerValidatableSamlToken.SamlTokenDelegate;
import com.vmware.identity.saml.ServerValidatableSamlToken.Subject;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidator;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidatorExtractor;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.token.impl.ValidateUtil;

/**
 * This class validates that already issued SAML token is valid as of now.
 * (assertion statements: only the subject of the assertion is validated)
 */
public class AuthnOnlyTokenValidator implements TokenValidator {

   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(AuthnOnlyTokenValidator.class);

   private final ConfigExtractor configExtractor;
   private final PrincipalAttributesExtractor principalAttributesExtractor;

   /**
    * @param configExtractor
    *           extractor of configuration data for issuing SAML tokens
    *           Must not be {@code null}.
    * @param principalAttributesExtractor
    *           required
    */
   public AuthnOnlyTokenValidator(ConfigExtractor configExtractor,
      PrincipalAttributesExtractor principalAttributesExtractor) {

      Validate.notNull(configExtractor);
      Validate.notNull(principalAttributesExtractor);
      this.configExtractor = configExtractor;
      this.principalAttributesExtractor = principalAttributesExtractor;
   }

   @Override
   public ServerValidatableSamlToken validate(ServerValidatableSamlToken token)
      throws InvalidSignatureException, InvalidTokenException, SystemException {

      final Config config = this.configExtractor.getConfig();
      final X509Certificate[] trustedRootCertificates = getTrustedSigningCerts(config);
      final long clockTolerance = config.getClockTolerance();
      final SubjectValidatorExtractor subjectValidatorExtractor = new SubjectValidatorExtractorImpl(config, this.principalAttributesExtractor);

      logger.debug("Validating token.");
      try {

         // if this token is issued by STS, this means that token validity is
         // smaller than signing certificate validity, which is smaller than
         // trusted anchor validity(because of chains). This implies that there
         // is a need to make a certificate chain integrity check only for
         // tokens not issued by STS, for those issued by STS successful
         // signature validation is enough for this integrity.

         // TODO see if samlToken.jar makes certificate chain integrity check
         token.validate(trustedRootCertificates,
            TimeUnit.MILLISECONDS.toSeconds(clockTolerance), subjectValidatorExtractor);
      } catch (InvalidTokenException e) {
          logger.debug("Token validation failed with InvalidTokenException", e);
         throw e;
      }
      logger.debug("Token validated");

      logger.debug( String.format("Token is from external idp: [%s]", (token.isExternal())) );

      validateSubject( token.getSubject(), token.isExternal());
      validateDelegationChain( token );

      logger.info("Token {} for principal {} successfully validated.",
              token.getId(), ((token.getSubject().subjectUpn() != null) ? token.getSubject().subjectUpn() : token.getSubject().subjectNameId()));
      logger.info("Token {} validated with SubjectValidation {}.",
              token.getId(), token.getSubject().subjectValidation());

      return token;
   }

   private X509Certificate[] getTrustedSigningCerts(Config config){
       Set<X509Certificate> trustedSigningCertificates = new HashSet<X509Certificate>();
       for (List<Certificate> currentChain : config.getValidCerts()) {
          // bugzilla#1057643
          Certificate signingCert = currentChain.get(0);
          trustedSigningCertificates.add((X509Certificate) signingCert);
       }

       // all anchors can be used, no matter if some of them are not valid
       // certificates at the point of a token validation. This is because token
       // signed with a certificate from chain which is no longer valid will not
       // be valid itself.
       return trustedSigningCertificates.toArray(new X509Certificate[trustedSigningCertificates.size()]);
   }

   private void validateDelegationChain( ServerValidatableSamlToken token )
   throws InvalidTokenException
   {
       for( SamlTokenDelegate del : token.getDelegationChain() )
       {
           validateSubject( del.subject(), token.isExternal() );
       }
   }

   private void validateSubject(Subject subject, Boolean isExternal)
   {
       if ( ( isExternal == false ) && ( subject.subjectValidation() != SubjectValidation.Regular ) )
       {
           throw new InvalidTokenException("Token subject is invalid.");
       }
   }

   private static class SubjectValidatorExtractorImpl implements SubjectValidatorExtractor
   {
       private final Config config;
       private final PrincipalAttributesExtractor principalAttributesExtractor;

       public SubjectValidatorExtractorImpl(Config inConfig, PrincipalAttributesExtractor inPrincipalAttributesExtractor)
       {
           ValidateUtil.validateNotNull(inConfig, "inConfig");
           ValidateUtil.validateNotNull(inPrincipalAttributesExtractor, "inPrincipalAttributesExtractor");
           this.config = inConfig;
           this.principalAttributesExtractor = inPrincipalAttributesExtractor;
       }

        @Override
        public SubjectValidator getSubjectValidator(NameId issuer)
        {
            IDPConfig externalIdp = getExternalIdpConfig(this.config, issuer);
            return new SubjectValidatorImpl(this.principalAttributesExtractor, externalIdp);
        }

        /**
         * Checks if a token issuer is external idp. And if so returns the correspondign IDPConfig;
         * @param token
         * @return if the token issued by an external idp, returns corresponding IDPConfig. null otherwise.
         */
        private static IDPConfig getExternalIdpConfig(Config config, NameId issuer)
        {
            IDPConfig externalIdp = null;
            final SamlAuthorityConfiguration samlAuthorityConfig = config.getSamlAuthorityConfig();

            logger.debug(
                String.format(
                    "SamlAuthority issuer=[%s]; Issuer={val=[%s], format=[%s]}",
                    samlAuthorityConfig.getIssuer(),
                    ( (issuer != null) ? issuer.getName() : "(null)" ),
                    ( (issuer != null) ? issuer.getNameFormat() : "(null)" )
                )
            );

            // this could be external idp if
            // issuer does not match ourself, and issuer is specified
            if ( ( issuer != null )
                 &&
                 (issuer.getName() != null)
                 &&
                 (issuer.getName().isEmpty() == false)
                 &&
                 ( samlAuthorityConfig.getIssuer().equals(issuer.getName()) == false )
                 &&
                 ( org.opensaml.saml2.core.Issuer.ENTITY.equals(issuer.getNameFormat()) )
               )
            {
                externalIdp = config.getExternalIdps().get(issuer.getName());
            }

            return externalIdp;
        }
   }

   private static class SubjectValidatorImpl implements SubjectValidator
   {
        private final PrincipalAttributesExtractor principalAttributesExtractor;
        private final IDPConfig externalIdp;
        private final static String JIT_USER_SEARCH_ATTR = "vmwSTSExternalIdpUserId";
        private static final Set<String> blacklistedDomainsForExtEdp;
        private static final String idmHost = "localhost";

        static
        {
            CasIdmClient client = new CasIdmClient(idmHost);
            blacklistedDomainsForExtEdp = new HashSet<>();

            try {
                EnumSet<DomainType> systemDomains = EnumSet.of(DomainType.SYSTEM_DOMAIN);
                Iterator<IIdentityStoreData> iter = client
                        .getProviders(client.getSystemTenant(), systemDomains).iterator();
                // only one system domain
                blacklistedDomainsForExtEdp.add(iter.next().getName());

                EnumSet<DomainType> localDomains = EnumSet.of(DomainType.LOCAL_OS_DOMAIN);
                iter = client
                        .getProviders(client.getSystemTenant(), localDomains).iterator();
                // only one local domain
                blacklistedDomainsForExtEdp.add(iter.next().getName());
            } catch (Exception e) {
                logger.warn("Failded to add blacklisted domains for external idp.", e);
            }
        }

        public SubjectValidatorImpl( PrincipalAttributesExtractor inPrincipalAttributesExtractor, IDPConfig inExternalIdp)
        {
            ValidateUtil.validateNotNull(inPrincipalAttributesExtractor, "inPrincipalAttributesExtractor");
            this.principalAttributesExtractor = inPrincipalAttributesExtractor;
            this.externalIdp = inExternalIdp;
        }

        @Override
        public Boolean IsIssuerExternal()
        {
            return (this.externalIdp != null);
        }

        @Override
        public Subject validateSubject(
            PrincipalId upnSubject, NameId subject)
            throws InvalidTokenException
        {
            SubjectValidation subjectValidation;
            PrincipalId resolvedSubject = null;
            if ( upnSubject != null )
            {
                if ( this.IsIssuerExternal() && blacklistedDomainsForExtEdp.contains(upnSubject.getDomain()))
                {
                    logger.warn("Subject upn domain {} is blacklisted for external tokens.", upnSubject.getDomain());
                    throw new InvalidTokenException("The domain of the subject upn is not allowed.");
                }

                resolvedSubject = upnSubject;
                try
                {
                    checkPrincipal(upnSubject);
                    subjectValidation = SubjectValidation.Regular;
                }
                catch(InvalidTokenException ex)
                {
                    if ( this.IsIssuerExternal() == true )
                    {
                        subjectValidation = SubjectValidation.None;
                    }
                    else
                    {
                        throw ex;
                    }
                }
            }
            else if ( externalIdp == null ) {
                logger.warn(
                    "Unable to identify token's subect. Looks like it is not in UPN format. [{}]",
                    ( (subject != null) ? subject.toString() : "(NULL)")
                );
                throw new InvalidTokenException("Token subject must be in UPN format.");
            } else {
                resolvedSubject = checkNonUpnPrincipal(subject, externalIdp);
                if (resolvedSubject != null) {
                    subjectValidation = SubjectValidation.Regular;
                } else {
                    // set subject validation status to None in order to perform JIT provision later if it is enabled
                    subjectValidation = SubjectValidation.None;
                }
            }

            return new ServerValidatableSamlToken.SubjectImpl(resolvedSubject, subject, subjectValidation);
        }

        /**
         * Returns PrincipalId of the subject if it is found. Otherwise return null.
         *
         * @throws InvalidTokenException
         */
        private PrincipalId checkNonUpnPrincipal(NameId subjectNameId, IDPConfig externalIdp)
            throws InvalidTokenException
        {
            assert (externalIdp != null);
            PrincipalId activeSubject = null;

            if ( subjectNameId == null ) {
                logger.warn(
                    "Unable to identify token's subject. getSubjectNameId() is null."
                );
                throw new InvalidTokenException("Token subject MUST not be null.");
            }

            // if subjectFormatMapping is set, we first try to find the user based on the subjectFormatMapping
            String subjectAttributeMapping = findAttributeMapping(externalIdp, subjectNameId.getNameFormat());
            if ( ( subjectAttributeMapping != null ) && ( subjectAttributeMapping.length() > 0 ) )
            {
                logger.debug(String.format("Subject attribute mapping for format [%s] is [%s]", subjectNameId.getNameFormat(), subjectAttributeMapping));
                try
                {
                    activeSubject = this.principalAttributesExtractor.findActiveUser(subjectAttributeMapping, subjectNameId.getName());
                }
                catch(InvalidPrincipalException ex)
                {
                    logger.debug(String.format("Looking up regular user failed with [%s]", ex.getMessage()), ex);
                }
            }

            // try find the user from jit user pool
            if (activeSubject == null)
            {
                try
                {
                    activeSubject = this.principalAttributesExtractor.findActiveUser(JIT_USER_SEARCH_ATTR, subjectNameId.getName());
                }
                catch(InvalidPrincipalException ex)
                {
                    logger.debug(String.format("Looking up jit user failed with [%s]", ex.getMessage()), ex);
                }
            }

            logger.debug("Found active principal " + ((activeSubject != null) ? activeSubject.toString() : "(null).") );
            return activeSubject;
        }

        private void checkPrincipal(PrincipalId id) throws InvalidTokenException {
            assert id != null;

            final boolean active;
            try {
               active = principalAttributesExtractor.isActive(id);
            } catch (InvalidPrincipalException e) {
               throw new InvalidTokenException(String.format(
                  "Principal %s not found", id), e);
            }

            if (!active) {
               throw new InvalidTokenException(String.format(
                  "Principal %s is not active.", id));
            }
            logger.debug("Principal " + id + "is active");
         }

        private static String findAttributeMapping(IDPConfig externalIdp, String subjectNameFormat )
        {
            String subjectAttributeMapping = null;
            AttributeConfig[] configs = externalIdp.getSubjectFormatMappings();
            for(AttributeConfig c : configs)
            {
                if ( (  c != null ) && ( c.getTokenSubjectFormat().equalsIgnoreCase(subjectNameFormat) ) )
                {
                    subjectAttributeMapping = c.getStoreAttribute();
                    break;
                }
            }
            return subjectAttributeMapping;
        }
   }
}
