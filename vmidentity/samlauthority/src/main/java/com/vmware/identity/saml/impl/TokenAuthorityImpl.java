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

import java.security.InvalidAlgorithmParameterException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import javax.xml.crypto.MarshalException;
import javax.xml.crypto.dsig.CanonicalizationMethod;
import javax.xml.crypto.dsig.Reference;
import javax.xml.crypto.dsig.SignatureMethod;
import javax.xml.crypto.dsig.SignedInfo;
import javax.xml.crypto.dsig.Transform;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.dom.DOMSignContext;
import javax.xml.crypto.dsig.keyinfo.KeyInfo;
import javax.xml.crypto.dsig.keyinfo.KeyInfoFactory;
import javax.xml.crypto.dsig.keyinfo.X509Data;
import javax.xml.crypto.dsig.spec.C14NMethodParameterSpec;
import javax.xml.crypto.dsig.spec.ExcC14NParameterSpec;
import javax.xml.crypto.dsig.spec.TransformParameterSpec;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.saml2.core.Assertion;
import org.opensaml.saml2.core.Attribute;
import org.opensaml.saml2.core.AttributeStatement;
import org.opensaml.saml2.core.AttributeValue;
import org.opensaml.saml2.core.Audience;
import org.opensaml.saml2.core.AudienceRestriction;
import org.opensaml.saml2.core.AuthnContext;
import org.opensaml.saml2.core.AuthnContextClassRef;
import org.opensaml.saml2.core.AuthnStatement;
import org.opensaml.saml2.core.Conditions;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.KeyInfoConfirmationDataType;
import org.opensaml.saml2.core.NameID;
import org.opensaml.saml2.core.NameIDType;
import org.opensaml.saml2.core.ProxyRestriction;
import org.opensaml.saml2.core.Subject;
import org.opensaml.saml2.core.SubjectConfirmation;
import org.opensaml.saml2.core.SubjectConfirmationData;
import org.opensaml.saml2.core.impl.AdviceBuilder;
import org.opensaml.saml2.core.impl.AssertionBuilder;
import org.opensaml.saml2.core.impl.AssertionMarshaller;
import org.opensaml.saml2.core.impl.AttributeStatementBuilder;
import org.opensaml.saml2.core.impl.AudienceBuilder;
import org.opensaml.saml2.core.impl.AudienceRestrictionBuilder;
import org.opensaml.saml2.core.impl.AuthnContextBuilder;
import org.opensaml.saml2.core.impl.AuthnContextClassRefBuilder;
import org.opensaml.saml2.core.impl.AuthnStatementBuilder;
import org.opensaml.saml2.core.impl.ConditionsBuilder;
import org.opensaml.saml2.core.impl.IssuerBuilder;
import org.opensaml.saml2.core.impl.KeyInfoConfirmationDataTypeBuilder;
import org.opensaml.saml2.core.impl.NameIDBuilder;
import org.opensaml.saml2.core.impl.ProxyRestrictionBuilder;
import org.opensaml.saml2.core.impl.SubjectBuilder;
import org.opensaml.saml2.core.impl.SubjectConfirmationBuilder;
import org.opensaml.saml2.core.impl.SubjectConfirmationDataBuilder;
import org.opensaml.samlext.saml2delrestrict.Delegate;
import org.opensaml.samlext.saml2delrestrict.DelegationRestrictionType;
import org.opensaml.samlext.saml2delrestrict.impl.DelegateBuilder;
import org.opensaml.samlext.saml2delrestrict.impl.DelegationRestrictionTypeBuilder;
import org.opensaml.xml.Configuration;
import org.opensaml.xml.Namespace;
import org.opensaml.xml.XMLObjectBuilder;
import org.opensaml.xml.XMLObjectBuilderFactory;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.schema.XSString;
import org.opensaml.xml.signature.SignatureConstants;
import org.opensaml.xml.signature.impl.KeyInfoBuilder;
import org.opensaml.xml.signature.impl.X509CertificateBuilder;
import org.opensaml.xml.signature.impl.X509DataBuilder;
import org.opensaml.xml.util.Base64;
import org.opensaml.xml.util.XMLConstants;
import org.opensaml.xml.util.XMLHelper;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.PrincipalAttribute;
import com.vmware.identity.saml.PrincipalAttributeDefinition;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.SamlToken;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.saml.ext.RSAAdvice;
import com.vmware.identity.saml.ext.RenewRestrictionType;
import com.vmware.identity.saml.ext.impl.RSAAdviceBuilder;
import com.vmware.identity.saml.ext.impl.RenewRestrictionTypeBuilder;
import com.vmware.identity.util.PerfConstants;
import com.vmware.identity.util.TimePeriod;

/**
 * Issuer authority of valid SAML tokens.
 */
public final class TokenAuthorityImpl implements TokenAuthority {

   private static final String HTTP_SCHEMAS_XMLSOAP_ORG_CLAIMS_UPN = "http://schemas.xmlsoap.org/claims/UPN";

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(TokenAuthorityImpl.class);

   private final static IDiagnosticsLogger perfLog = DiagnosticsLoggerFactory
      .getLogger(PerfConstants.PERF_LOGGER.getClass());

   private final SignatureAlgorithm defaultSignatureAlgorithm;
   private final PrincipalAttributesExtractor principalAttributesExtractor;
   private final ConfigExtractor configExtractor;
   private final Collection<PrincipalAttributeDefinition> supportedAttributes;

   /**
    * @param defaultSignatureAlgorithm
    *           the default algorithm which will be used for signing the token
    *           if there is no such specified in either the {@link SamlTokenSpec}, or {@link SamlAuthorityConfiguration}.
    *           Cannot be null.
    * @param principalAttributesExtractor
    *           to be used for extracting attributes which should be included in
    *           the issued tokens per each principal. Cannot be null.
    * @param configExtractor
    *           extractor of configuration data for issuing SAML tokens
    *           Must not be {@code null}.
    */
   public TokenAuthorityImpl(
      SignatureAlgorithm defaultSignatureAlgorithm,
      PrincipalAttributesExtractor principalAttributesExtractor,
      ConfigExtractor configExtractor) {
      Validate.notNull(defaultSignatureAlgorithm);
      Validate.notNull(principalAttributesExtractor);
      Validate.notNull(configExtractor);

      this.defaultSignatureAlgorithm = defaultSignatureAlgorithm;
      this.principalAttributesExtractor = principalAttributesExtractor;
      this.configExtractor = configExtractor;

      this.supportedAttributes = principalAttributesExtractor
         .getAllAttributeDefinitions();
   }

   @Override
   public SamlToken issueToken(SamlTokenSpec spec) {
      log.debug("Started issuing token for spec:{}", spec);
      Validate.notNull(spec);

      final Config config = this.configExtractor.getConfig();
      final SamlAuthorityConfiguration samlAuthorityConfig = config
              .getSamlAuthorityConfig();
      final TokenRestrictions tokenRestrictions = config.getTokenRestrictions();
      final DelegationHandler delegationHandler =
          new DelegationHandler(
               tokenRestrictions.getDelegationCount(), principalAttributesExtractor);
      final SignInfo signInfo = getSignInfo(samlAuthorityConfig);
      final TokenLifetimeRemediator lifetimeRemediator =
          new TokenLifetimeRemediator(signInfo, tokenRestrictions, config.getClockTolerance());

      final boolean requesterIsTokenOwner = spec.requesterIsTokenOwner();
      final DelegationInfo delegationInfo = delegationHandler
         .getDelegationInfo(requesterIsTokenOwner, spec.getDelegationSpec());
      final int renewCount = new RenewRestrictor(requesterIsTokenOwner,
          tokenRestrictions.getRenewCount()).processRequest(spec.getRenewSpec());
      final SignatureAlgorithm signatureAlgorithm = getSignatureAlgorithm(spec
         .getSignatureAlgorithm(), samlAuthorityConfig);
      final List<Advice> advice = AdviceFilter.processRequest(
         requesterIsTokenOwner, spec.getRequestedAdvice(),
         spec.getPresentAdvice());

      // move token issue time right before create assertion, PR 1550654.
      final Date issueInstantTime = new Date();
      log.debug("Issue instant time: {}", issueInstantTime);

      final TimePeriod validity = lifetimeRemediator.remediateTokenValidity(
              spec, issueInstantTime);

      final Assertion assertion = createAssertion(spec, issueInstantTime,
         validity, delegationInfo, renewCount, advice, samlAuthorityConfig);

      final Element assertionElement = createSignatureAndSignAssertion(
         assertion, signatureAlgorithm, signInfo);
      final Document token = assertionElement.getOwnerDocument();

      log.debug("Created token(issuance time: {}):\n{}", issueInstantTime,
         XMLHelper.nodeToString(token.getDocumentElement()));

      // we can use confirmation type from the spec because it is directly used
      // for the issued token without remediation.
      final ConfirmationType confirmationType = spec.getConfirmationData()
         .getType();

      return new SamlToken(token, signatureAlgorithm, validity,
         confirmationType, delegationInfo.isDelegableToken(), renewCount > 0);
   }

   /**
    * @param spec
    *           object containing SAML artifacts used during token creation.
    *           Cannot be null.
    * @param issueInstantDateTime
    *           the time at which the assertion is issued. Cannot be null.
    * @param tokenValidity
    *           validated and remediated token validity. Cannot be null,
    * @param delegationInfo
    *           all the delegation information needed to be included into this
    *           assertion. Cannot be null.
    * @param renewCount
    *           number of renew count which will be included into this
    *           assertion. Non-negative.
    * @param advice
    *           advice to put in the token. Cannot be null.
    * @return an assertion object representing the token
    */
   private Assertion createAssertion(SamlTokenSpec spec,
      Date issueInstantDateTime, TimePeriod tokenValidity,
      DelegationInfo delegationInfo, int renewCount, List<Advice> advice,
      SamlAuthorityConfiguration samlAuthorityConfig) {
      assert spec != null;
      assert issueInstantDateTime != null;
      assert tokenValidity != null;
      assert tokenValidity.getStartTime() != null;
      assert tokenValidity.getEndTime() != null;
      assert delegationInfo != null;
      assert renewCount >= 0;
      assert advice != null;

      AuthenticationData authnData = spec.getAuthenticationData();
      Set<PrincipalAttribute> principalAttributes = getTokenAttributesAndIdentityAttribute(spec);
      PrincipalAttribute identityAttribute = extractIdentityAttribute(
         authnData.getIdentityAttrName(), principalAttributes);
      Set<PrincipalAttribute> tokenAttributes = extractTokenAttributes(
         identityAttribute, principalAttributes, spec);

      Assertion assertion = new AssertionBuilder().buildObject();
      assertion.setID(generateAssertionId());
      assertion.setVersion(SAMLVersion.VERSION_20);
      assertion.setIssueInstant(new DateTime(issueInstantDateTime));
      assertion.setIssuer(createIssuer(samlAuthorityConfig));
      assertion.setConditions(createConditions(tokenValidity, delegationInfo,
         renewCount, spec.getAudience()));
      if (!advice.isEmpty()) {
         assertion.setAdvice(createAdvice(advice));
      }
      assertion.getAuthnStatements().add(createAuthnStatement(authnData));
      assertion.getAttributeStatements().add(
         createAttributeStatement(tokenAttributes));
      assertion.setSubject(createSubject(identityAttribute,
         tokenValidity.getEndTime(), spec.getConfirmationData(),
         delegationInfo.lastDelegate()));

      registerNS(assertion, XMLConstants.XSD_NS, XMLConstants.XSD_PREFIX);
      registerNS(assertion, XMLConstants.XSI_NS, XMLConstants.XSI_PREFIX);

      return assertion;
   }

   private Set<PrincipalAttribute> getTokenAttributesAndIdentityAttribute(
      SamlTokenSpec spec) {
      assert spec != null;
      final PrincipalId tokenSubject = spec.requesterIsTokenOwner() ? spec
         .getAuthenticationData().getPrincipalId() : spec.getDelegationSpec()
         .getDelegationHistory().getTokenSubject();
      assert tokenSubject != null;

      Collection<PrincipalAttributeDefinition> attributeDefinitions = new HashSet<PrincipalAttributeDefinition>();

      PrincipalAttributeDefinition resolvedIdentityAttribute = resolveToAttributeDefinitions(spec
         .getAuthenticationData().getIdentityAttrName());
      if (resolvedIdentityAttribute == null) {
         throw new IllegalStateException("Identity attribute is not supported");
      }
      attributeDefinitions.add(resolvedIdentityAttribute);

      attributeDefinitions.addAll(resolveToAttributeDefinitions(spec
         .getAttributeNames()));

      return principalAttributesExtractor.getAttributes(tokenSubject,
         attributeDefinitions);
   }

   private PrincipalAttributeDefinition resolveToAttributeDefinitions(
      String identityAttributeName) {
      assert identityAttributeName != null;

      PrincipalAttributeDefinition result = null;
      for (PrincipalAttributeDefinition attrDef : supportedAttributes) {
         if (identityAttributeName.equals(attrDef.getName())) {
            result = attrDef;
            break;
         }
      }

      return result;
   }

   private Collection<PrincipalAttributeDefinition> resolveToAttributeDefinitions(
      Collection<String> attributeNames) {
      assert attributeNames != null;

      Set<PrincipalAttributeDefinition> result = new HashSet<PrincipalAttributeDefinition>();
      for (String attrName : attributeNames) {
         PrincipalAttributeDefinition resolvedAttr = resolveToAttributeDefinitions(attrName);
         if (resolvedAttr != null) {
            result.add(resolvedAttr);
         }
      }

      return result;
   }

   private PrincipalAttribute extractIdentityAttribute(
      String identityAttributeName, Set<PrincipalAttribute> principalAttributes) {
      assert identityAttributeName != null;
      assert principalAttributes != null;

      log.debug("Identity attribute name for token subject: {}", identityAttributeName);

      PrincipalAttribute identityAttribute = null;
      for (PrincipalAttribute principalAttribute : principalAttributes) {
         if (principalAttribute.getName().equals(identityAttributeName)) {
            identityAttribute = principalAttribute;
            break;
         }
      }
      if (identityAttribute == null || identityAttribute.getValues() == null
         || identityAttribute.getValues().length != 1) {
         // this should not be illegal state, but request failed exception
         throw new IllegalStateException(
            "The desired identity attribute "
               + identityAttributeName
               + " is not supported(not found in the schema or found but does not have value or have multiple values).");
      }

      return identityAttribute;
   }

   private Set<PrincipalAttribute> extractTokenAttributes(
      PrincipalAttribute identityAttribute,
      Set<PrincipalAttribute> principalAttributes, SamlTokenSpec spec) {

      assert principalAttributes != null;
      assert identityAttribute != null;
      assert spec != null;

      Set<PrincipalAttribute> tokenAttributes = principalAttributes;

      if (!spec.getAttributeNames().contains(
         spec.getAuthenticationData().getIdentityAttrName())) {
         tokenAttributes.remove(identityAttribute);
      }

      return tokenAttributes;
   }

   private SignatureAlgorithm getSignatureAlgorithm(
      SignatureAlgorithm desiredSignatureAlgorithmInRequest,
      SamlAuthorityConfiguration samlAuthorityConfig) {

       SignatureAlgorithm signatureAlgorithm = desiredSignatureAlgorithmInRequest;
       if ( signatureAlgorithm == null ){
           final String signatureAlgorithmInConfig = samlAuthorityConfig.getSignatureAlgorithm();

           if (signatureAlgorithmInConfig != null) {
              SignatureAlgorithm signatureAlgorithmFromConfig = SignatureAlgorithm
                 .getSignatureAlgorithmForURI(signatureAlgorithmInConfig);
              if (signatureAlgorithmFromConfig != null) {
                 signatureAlgorithm = signatureAlgorithmFromConfig;
              } else {
                 log.warn("{} algorithm is not supported.",
                    signatureAlgorithmInConfig);
              }
           }

           if (signatureAlgorithm == null){
               signatureAlgorithm = this.defaultSignatureAlgorithm;
           }
       }

       log.debug("{} signature algorithm will be used for signing the token.",
               signatureAlgorithm);

       return signatureAlgorithm;
   }

   /**
    * Registers NS in assertion.
    */
   private void registerNS(Assertion assertion, String uri, String prefix) {
      assert assertion != null;
      assert uri != null;
      assert prefix != null;

      assertion.getNamespaceManager().registerNamespace(
         new Namespace(uri, prefix));
   }

   /**
    * @return generated assertion Id
    */
   private String generateAssertionId() {
      String assertionId = "_" + UUID.randomUUID().toString();
      log.debug("Generated assertion id: {}", assertionId);
      return assertionId;
   }

   /**
    * Creates conditions section of the token. Inserts in it only NotBefore and
    * NotOnOrAfter attributes.
    *
    * @param tokenValidity
    *           time period of the token validity. Cannot be null.
    * @param delegationInfo
    *           Cannot be null.
    * @param renewSpec
    *           renew specification. Cannot be null.
    * @param audience
    *           audience. Cannot be null.
    * @return initialized Conditions object representing subject part of the
    *         token.
    */
   private Conditions createConditions(TimePeriod tokenValidity,
      DelegationInfo delegationInfo, int renewCount, Set<String> audience) {
      assert tokenValidity != null;
      assert tokenValidity.getStartTime() != null;
      assert tokenValidity.getEndTime() != null;
      assert delegationInfo != null;
      assert renewCount >= 0;
      assert audience != null;

      Conditions conditions = new ConditionsBuilder().buildObject();
      conditions.setNotBefore(new DateTime(tokenValidity.getStartTime()));
      conditions.setNotOnOrAfter(new DateTime(tokenValidity.getEndTime()));

      log.debug("Created conditions - notBefore: {}, notOnOrAfter: {}",
         tokenValidity.getStartTime(), tokenValidity.getEndTime());

      conditions.getConditions().add(
         createProxyRestriction(delegationInfo.getDelegationCount()));

      List<TokenDelegate> delegationChain = delegationInfo.getDelegationChain();
      if (delegationChain != null && delegationChain.size() > 0) {
         conditions.getConditions().add(createDelegationChain(delegationChain));
      }

      addRenewRestriction(conditions, renewCount);

      final AudienceRestriction audienceCondition = createAudienceRestriction(audience);
      if (audienceCondition != null) {
         conditions.getAudienceRestrictions().add(audienceCondition);
      }
      return conditions;
   }

   /**
    * @param proxyCount
    * @return
    */
   private ProxyRestriction createProxyRestriction(int proxyCount) {

      ProxyRestriction proxy = new ProxyRestrictionBuilder().buildObject();
      proxy.setProxyCount(proxyCount);

      log.debug("Token delegation count is {}", proxyCount);

      return proxy;
   }

   /**
    * @param delegationChain
    *           required
    * @return
    */
   private DelegationRestrictionType createDelegationChain(
      List<TokenDelegate> delegationChain) {
      assert delegationChain != null;

      DelegationRestrictionType delegationRestriction = new DelegationRestrictionTypeBuilder()
         .buildObject();
      List<Delegate> delegateList = delegationRestriction.getDelegates();
      for (TokenDelegate tokenDelegate : delegationChain) {
         Delegate delegate = new DelegateBuilder().buildObject();
         delegate.setDelegationInstant(new DateTime(tokenDelegate
            .getDelegationDate()));
         NameID delegateName = createUPNNameId(tokenDelegate.getSubject());
         delegate.setNameID(delegateName);
         delegateList.add(delegate);
      }

      log.debug("Created delegation chain");

      return delegationRestriction;
   }

   private void addRenewRestriction(Conditions conditions, int renewCount) {
      assert conditions != null;
      assert renewCount >= 0;

      if (renewCount > 0) {
         conditions.getConditions().add(createRenewRestriction(renewCount));
      } else {
         log.debug("Token renew count is 0 hence no renew restriction is added.");
      }
   }

   private RenewRestrictionType createRenewRestriction(int count) {

      RenewRestrictionType proxy = new RenewRestrictionTypeBuilder()
         .buildObject();
      proxy.setCount(count);

      log.debug("Token renew count is {}", count);
      return proxy;
   }

   private AudienceRestriction createAudienceRestriction(Set<String> audience) {
      assert audience != null;

      final AudienceRestriction result = audience.isEmpty() ? null
         : new AudienceRestrictionBuilder().buildObject();
      for (String audienceParty : audience) {
         Audience samlAudience = new AudienceBuilder().buildObject();
         samlAudience.setAudienceURI(audienceParty);
         result.getAudiences().add(samlAudience);
         log.trace("Audience restriction added for  {}", audienceParty);
      }

      return result;
   }

   private org.opensaml.saml2.core.Advice createAdvice(List<Advice> advice) {
      assert advice != null && !advice.isEmpty();
      final org.opensaml.saml2.core.Advice result = new AdviceBuilder()
         .buildObject();
      for (Advice adviceMember : advice) {
         final RSAAdvice rsaAdvice = toOpenSamlAdvice(adviceMember);
         result.getChildren().add(rsaAdvice);
      }
      return result;
   }

   private RSAAdvice toOpenSamlAdvice(Advice advice) {
      final RSAAdvice openSamlAdvice = new RSAAdviceBuilder().buildObject();
      openSamlAdvice.setSource(advice.sourceURI());
      for (Advice.Attribute adviceAttr : advice.attributes()) {
         // TODO unify PrincipalAttribute and Advice.Attribute
         final PrincipalAttribute principalAttribute = new PrincipalAttribute(
            adviceAttr.nameURI(), NameIDType.UNSPECIFIED,
            adviceAttr.friendlyName(), adviceAttr.values().toArray(
               new String[0]));
         final Attribute openSamlAttribute = convertToOpenSamlAttribute(principalAttribute);
         openSamlAdvice.getAttributes().add(openSamlAttribute);
      }
      return openSamlAdvice;
   }

   /**
    * Create subject section of the token
    *
    * @return initialized Subject object representing Subject part of the token.
    */
   private Subject createSubject(PrincipalAttribute identityAttribute,
      Date tokenEndTime, Confirmation confirmation, TokenDelegate lastDelegate) {
      assert identityAttribute != null;
      assert tokenEndTime != null;
      assert confirmation != null;

      String[] tokenIdentityValue = identityAttribute.getValues();
      assert tokenIdentityValue.length == 1;

      String tokenSubject = tokenIdentityValue[0];
      String tokenSubjectFormat = identityAttribute.getName();

      Subject subject = new SubjectBuilder().buildObject();
      subject.setNameID(createNameId(tokenSubject, tokenSubjectFormat));
      subject.getSubjectConfirmations().add(
         createSubjectConfirmation(tokenEndTime, confirmation, lastDelegate));

      log.debug("Subject created : {}.", subject);
      return subject;
   }

   /**
    * Creates confirmation part of the token subject section.
    *
    * @param endTime
    * @param confirmation
    * @param lastDelegate
    * @return initialized subject confirmation object. representing
    *         SubjectConfirmation in a Subject part of the token
    */
   private SubjectConfirmation createSubjectConfirmation(Date endTime,
      Confirmation confirmation, TokenDelegate lastDelegate) {
      assert endTime != null;
      assert confirmation != null;

      SubjectConfirmation conf = new SubjectConfirmationBuilder().buildObject();
      SubjectConfirmationData confData;
      if (confirmation.getType().equals(ConfirmationType.BEARER)) {
         conf.setMethod(SubjectConfirmation.METHOD_BEARER);
         confData = createSubjectConfirmationData(confirmation, endTime);
         log.debug("Created subject confirmation - method: BEARER");
      } else {
         conf.setMethod(SubjectConfirmation.METHOD_HOLDER_OF_KEY);
         confData = createHoKSubjectConfData(confirmation);
         log.debug("Created subject confirmation - method: HoK");
      }

      conf.setSubjectConfirmationData(confData);
      if (lastDelegate != null) {
         conf.setNameID(createUPNNameId(lastDelegate.getSubject()));
      }

      return conf;
   }

   private KeyInfoConfirmationDataType createHoKSubjectConfData(
      Confirmation confirmation) {
      assert confirmation != null;
      assert confirmation.getCertificate() != null;

      KeyInfoConfirmationDataType data = new KeyInfoConfirmationDataTypeBuilder()
         .buildObject();

      org.opensaml.xml.signature.X509Data x509Data = new X509DataBuilder()
         .buildObject();
      org.opensaml.xml.signature.KeyInfo ki = new KeyInfoBuilder()
         .buildObject();
      org.opensaml.xml.signature.X509Certificate cert = new X509CertificateBuilder()
         .buildObject();
      try {
         cert.setValue(Base64.encodeBytes(confirmation.getCertificate()
            .getEncoded()));
      } catch (CertificateEncodingException e) {
         // TODO check [848537] for details
         throw new IllegalStateException("Cannot encode X509Certificate", e);
      }
      x509Data.getX509Certificates().add(cert);
      ki.getX509Datas().add(x509Data);
      data.getKeyInfos().add(ki);

      log.debug(
         "Created SubjectConfirmation data, Certificate with Subject: {}",
         confirmation.getCertificate().getSubjectX500Principal().getName());
      return data;
   }

   private SubjectConfirmationData createSubjectConfirmationData(
      Confirmation confirmation, Date endTime) {
      assert confirmation != null;
      assert endTime != null;

      SubjectConfirmationData data = new SubjectConfirmationDataBuilder()
         .buildObject();
      data.setNotOnOrAfter(new DateTime(endTime));
      assert confirmation != null;
      assert data != null;

      if (confirmation.getInResponseTo() != null) {
         data.setInResponseTo(confirmation.getInResponseTo());
      }
      if (confirmation.getRecipient() != null) {
         data.setRecipient(confirmation.getRecipient());
      }

      log.debug(
         "Created SubjectConfirmation data, NotOnOrAfter: {}, InResponseTo: {}, Recipient: {}",
         new Object[] { endTime, confirmation.getInResponseTo(),
            confirmation.getRecipient() });
      return data;
   }

   private NameID createUPNNameId(PrincipalId principalId) {
      return createNameId(toUPN(principalId),
         HTTP_SCHEMAS_XMLSOAP_ORG_CLAIMS_UPN);
   }

   /**
    * Creates name id for the sake of token subject, delegates, subject
    * confirmation, etc.
    *
    * @param subject
    *           Cannot be null.
    * @param format
    *           Cannot be null.
    * @return initialized NameId object representing NameId in a Subject part of
    *         the token.
    */
   private NameID createNameId(String subject, String format) {
      assert subject != null;
      assert format != null;

      NameID nameId = new NameIDBuilder().buildObject();
      nameId.setValue(subject);
      nameId.setFormat(format);

      log.debug("Created nameId: {}", subject);
      return nameId;
   }

   /**
    * Creates issuer part of the token.
    *
    * @return initialized issuer object. representing SubjectConfirmation in a
    *         Subject part of the token.
    */
   private Issuer createIssuer(SamlAuthorityConfiguration samlAuthorityConfig) {
      IssuerBuilder b = new IssuerBuilder();
      Issuer issuerSamlObj = b.buildObject();
      String issuer = samlAuthorityConfig.getIssuer();

      issuerSamlObj.setFormat(NameIDType.ENTITY);
      issuerSamlObj.setValue(issuer);

      log.debug("Created Issuer: {}", issuer);
      return issuerSamlObj;
   }

   /**
    * Creates AuthenticationStatement section of the token.
    *
    * @param authnData
    *           the entire authentication data for current request, containing
    *           how authentication is done (by pass, kerberos, etc.), time of
    *           authentication, authenticated principal, etc. Cannot be null.
    * @return initialized AuthnStatement part of the token.
    */
   private AuthnStatement createAuthnStatement(AuthenticationData authnData) {
      assert authnData != null;

      DateTime authnInstantTime = new DateTime(authnData.getAuthnTime());
      assert authnInstantTime != null;

      AuthnStatement authnStatement = new AuthnStatementBuilder().buildObject();
      authnStatement.setAuthnInstant(authnInstantTime);
      if (authnData.getSessionIndex() != null) {
         authnStatement.setSessionIndex(authnData.getSessionIndex());
      }

      AuthnContext authnContext = new AuthnContextBuilder().buildObject();
      AuthnContextClassRef authnContextClassRef = new AuthnContextClassRefBuilder()
         .buildObject();
      authnContextClassRef.setAuthnContextClassRef(getAuthnMethod(authnData));

      authnContext.setAuthnContextClassRef(authnContextClassRef);
      authnStatement.setAuthnContext(authnContext);

      Date sessionExpiredate = authnData.getSessionExpireDate();
      if(sessionExpiredate != null){
          DateTime expireDateTime = new DateTime(sessionExpiredate);
          authnStatement.setSessionNotOnOrAfter(expireDateTime);
      }

      return authnStatement;
   }

   /**
    * @param authnData
    *           not null
    * @return the authentication method that should be inserted into the authn
    *         statement of the SAML token
    */
   private String getAuthnMethod(AuthenticationData authnData) {
      assert authnData != null;
      AuthnMethod authnMethod = authnData.getAuthnMethod();
      assert authnMethod != null;

      String authnMethodValue;
      switch (authnMethod) {
      case PASSWORD:
         authnMethodValue = AuthnContext.PPT_AUTHN_CTX;
         break;
      case KERBEROS:
         authnMethodValue = AuthnContext.KERBEROS_AUTHN_CTX;
         break;
      case XMLDSIG:
         authnMethodValue = AuthnContext.XML_DSIG_AUTHN_CTX;
         break;
      case ASSERTION:
         authnMethodValue = AuthnContext.PREVIOUS_SESSION_AUTHN_CTX;
         break;
      case NTLM:
         authnMethodValue = AuthnContext.KERBEROS_AUTHN_CTX;
         break;
      case TLSCLIENT:
          authnMethodValue = AuthnContext.TLS_CLIENT_AUTHN_CTX;
          break;
      case TIMESYNCTOKEN:
          authnMethodValue = AuthnContext.TIME_SYNC_TOKEN_AUTHN_CTX;
          break;
      case SMARTCARD:
          authnMethodValue = AuthnContext.SMARTCARD_PKI_AUTHN_CTX;
          break;
      default:
         throw new IllegalStateException("Unknown authentication method");
      }
      return authnMethodValue;
   }

   /**
    * @param principalAttributes
    *           Cannot be null.
    * @return object representation of AttributeStatement part of token.
    */
   private AttributeStatement createAttributeStatement(
      Set<PrincipalAttribute> principalAttributes) {
      assert principalAttributes != null;

      AttributeStatement attributeStatement = null;
      if (!principalAttributes.isEmpty()) {
         attributeStatement = new AttributeStatementBuilder().buildObject();
         List<Attribute> attributes = attributeStatement.getAttributes();

         for (PrincipalAttribute principalAttribute : principalAttributes) {
            Attribute currentAttribute = convertToOpenSamlAttribute(principalAttribute);
            attributes.add(currentAttribute);

            log.debug("Added Attribute to statement.");
         }
      } else {
         log.trace("No attributes found so no attribute statement is added!");
      }
      return attributeStatement;
   }

   /**
    * Creates attribute in attribute statement part of the token.
    *
    * @param principalAttribute
    *           cannot be null.
    * @return attribute object
    */
   private Attribute convertToOpenSamlAttribute(
      PrincipalAttribute principalAttribute) {
      assert principalAttribute != null;

      XMLObjectBuilderFactory builderFactory = Configuration
         .getBuilderFactory();
      @SuppressWarnings("unchecked")
      XMLObjectBuilder<Attribute> builder = builderFactory
         .getBuilder(Attribute.DEFAULT_ELEMENT_NAME);
      Attribute attribute = builder.buildObject(Attribute.DEFAULT_ELEMENT_NAME);

      attribute.setName(principalAttribute.getName());
      attribute.setNameFormat(principalAttribute.getNameFormat());

      String friendlyName = principalAttribute.getFriendlyName();
      if (friendlyName != null) {
         attribute.setFriendlyName(friendlyName);
      }

      String[] values = principalAttribute.getValues();
      if (values != null) {
         @SuppressWarnings("unchecked")
         final XMLObjectBuilder<XSString> stringBuilder = builderFactory.getBuilder(XSString.TYPE_NAME);
         for (String value : values) {
            XSString attributeVal = stringBuilder.buildObject(
               AttributeValue.DEFAULT_ELEMENT_NAME, XSString.TYPE_NAME);

            attributeVal.setValue(value);
            attribute.getAttributeValues().add(attributeVal);
         }
      }

      log.debug("Created Attribute - name: {}, nameFormat: {}",
         principalAttribute.getName(), principalAttribute.getNameFormat());
      return attribute;
   }

   /**
    * Creates signature part of assertion. Uses digest method algorithm
    * corresponding to the signature algorithm used.
    *
    * @param assertion
    * @param signatureAlgorithm
    * @return
    */
   private Element createSignatureAndSignAssertion(Assertion assertion,
      SignatureAlgorithm signatureAlgorithm, SignInfo signInfo) {
      assert assertion != null;
      assert signatureAlgorithm != null;

      XMLSignatureFactory factory = XMLSignatureFactory.getInstance();
      Element assertionElement = marshallAssertion(assertion);
      List<Transform> transforms = createTransforms();
      Reference ref = createReference(transforms,
         assertionElement.getAttribute(Assertion.ID_ATTRIB_NAME),
         // here we use the digest method which is corresponding to the
         // signature algorithm used
         signatureAlgorithm.getDigestMethod().toString());
      SignedInfo signedInfo = createSignedInfo(Collections.singletonList(ref),
         signatureAlgorithm);

      DOMSignContext signingContext = new DOMSignContext(
         signInfo.getPrivateKey(), assertionElement);
      signingContext.putNamespacePrefix(
         SignatureConstants.TRANSFORM_C14N_EXCL_OMIT_COMMENTS, "ec");
      signingContext.putNamespacePrefix(XMLSignature.XMLNS, "ds");

      // signature should be the second section in the assertion - after issuer
      // here we are sure that the structure of assertion is as follows:
      // 1) issuer 2) subject
      // we get subject node and enter signature before it and the result is:
      // 1) issuer 2) signature 3) subject
      Node subjectNode = assertionElement.getChildNodes().item(1);
      signingContext.setNextSibling(subjectNode);
      log.debug("Set SigningContext into assertion (after Issuer or as a first child in the assertion DOM).");

      final KeyInfo keyInfo = createKeyInfo(signInfo);
      XMLSignature xmlSignature = factory.newXMLSignature(signedInfo, keyInfo);

      try {
         final long start = System.nanoTime();
         xmlSignature.sign(signingContext);
         perfLog.trace("'signature.sign' took {} ms.",
            TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - start));
      } catch (MarshalException e) {
         throw new IllegalStateException(e);
      } catch (XMLSignatureException e) {
         throw new IllegalStateException(e);
      }
      log.debug("Created Signature and sign it.");

      return assertionElement;
   }

   /**
    * Create a SignInfo instance with signCertificateChain from samlAuthorityConfig.
    * @param samlAuthorityConfig
    * @return created SignInfo.
    */
   private static SignInfo getSignInfo(SamlAuthorityConfiguration samlAuthorityConfig)
   {
        List<Certificate> signingCertificateChain = samlAuthorityConfig
               .getSigningCertificateChain();

        final CertPath certPath;
        try {
           final CertificateFactory certificateFactory = CertificateFactory
              .getInstance("X.509");
           certPath = certificateFactory
              .generateCertPath(signingCertificateChain);
        } catch (CertificateException e) {
           throw new RuntimeException(e);
        }

        SignInfo signInfo = new SignInfo(samlAuthorityConfig.getAuthorityKey(),
           certPath, null); // TODO provider

        return signInfo;
   }

   /**
    * Create KeyInfo section representation.
    *
    * @return KeyInfo
    */
    private KeyInfo createKeyInfo(SignInfo signInfo) {
      List<? extends Certificate> stsCertificates = signInfo
         .getCertificationPath().getCertificates();

      XMLSignatureFactory factory = XMLSignatureFactory.getInstance();
      KeyInfoFactory keyInfoFactory = factory.getKeyInfoFactory();
      X509Data certificatesData = keyInfoFactory.newX509Data(stsCertificates);

      log.debug("Created KeyInfo section from certificates: {}",
         stsCertificates);
      return keyInfoFactory.newKeyInfo(Collections
         .singletonList(certificatesData));
   }

   /**
    * Creates SignedInfo section part of Signature.
    *
    * @param references
    *           references to be included in SignedInfo. Cannot be null.
    * @param signatureAlgorithm
    * @return returns SignedInfo object representing SignedInfo section
    * @throws NoSuchAlgorithmException
    * @throws InvalidAlgorithmParameterException
    */
   private SignedInfo createSignedInfo(List<Reference> references,
      SignatureAlgorithm signatureAlgorithm) {
      assert references != null;
      assert signatureAlgorithm != null;

      XMLSignatureFactory factory = XMLSignatureFactory.getInstance();

      CanonicalizationMethod canonicalizationMethod;
      try {
         canonicalizationMethod = factory.newCanonicalizationMethod(
            SignatureConstants.TRANSFORM_C14N_EXCL_OMIT_COMMENTS,
            (C14NMethodParameterSpec) null);
      } catch (Exception e) {
         throw new IllegalStateException(
            "Cannot create canonicalization object.", e);
      }

      SignatureMethod signatureMethod;
      try {
         signatureMethod = factory.newSignatureMethod(
            signatureAlgorithm.toString(), null);
      } catch (Exception e) {
         throw new IllegalStateException(
            "Cannot create signature algorithm object.", e);
      }

      SignedInfo signedInfo = factory.newSignedInfo(canonicalizationMethod,
         signatureMethod, references);

      log.debug("Created SignedInfo section with signatureAlgorithm: {}",
         signatureAlgorithm);
      return signedInfo;
   }

   /**
    * Creates a Reference part of Signature section
    *
    * @param transforms
    * @param id
    * @param digestMethod
    * @return
    * @throws NoSuchAlgorithmException
    * @throws InvalidAlgorithmParameterException
    */
   private Reference createReference(List<Transform> transforms, String id,
      String digestMethod) {
      assert transforms != null;
      assert id != null;
      assert digestMethod != null;

      XMLSignatureFactory factory = XMLSignatureFactory.getInstance();

      javax.xml.crypto.dsig.DigestMethod digestAlgorithm;
      try {
         digestAlgorithm = factory.newDigestMethod(digestMethod, null);
      } catch (Exception e) {
         throw new IllegalStateException("Cannot create digest method object.",
            e);
      }

      log.debug("Created reference with id: {} and digestMethod: {}", id,
         digestMethod);
      return factory.newReference("#" + id, digestAlgorithm, transforms, null,
         null);
   }

   /**
    * Creates a list of transform part of Reference section in Signature
    *
    * @return
    * @throws NoSuchAlgorithmException
    * @throws InvalidAlgorithmParameterException
    */
   private List<Transform> createTransforms() {
      XMLSignatureFactory factory = XMLSignatureFactory.getInstance();

      List<Transform> transforms = new ArrayList<Transform>(2);

      List<String> prefixList = new ArrayList<String>(2);
      prefixList.add(XMLConstants.XSD_PREFIX);
      prefixList.add(XMLConstants.XSI_PREFIX);

      try {
         transforms.add(factory.newTransform(CanonicalizationMethod.ENVELOPED,
            (TransformParameterSpec) null));
         transforms.add(factory.newTransform(CanonicalizationMethod.EXCLUSIVE,
            new ExcC14NParameterSpec(prefixList)));
      } catch (Exception e) {
         throw new IllegalStateException(
            "Cannot create enveloped or exclusive transform objects.", e);
      }

      log.debug("Created transforms: {} and {}",
         CanonicalizationMethod.ENVELOPED, CanonicalizationMethod.EXCLUSIVE);
      return transforms;
   }

   /**
    * Marshalls created assertion object.
    *
    * @param assertion
    *           object
    * @return
    */
   private Element marshallAssertion(Assertion assertion) {
      assert assertion != null;

      AssertionMarshaller marshaller = new AssertionMarshaller();
      Element assertionElement;
      try {
         assertionElement = marshaller.marshall(assertion);
      } catch (MarshallingException e) {
         throw new IllegalStateException(e);
      }

      log.debug("Successfully marshalled assertion: {}", assertionElement);
      return assertionElement;
   }

   /**
    * @param principalId
    *           principal id. Cannot be null.
    * @return Universal Principal Name(UPN) of the principal id.
    */
   private String toUPN(PrincipalId principalId) {
      assert principalId != null;
      return principalId.getName() + "@" + principalId.getDomain();
   }

}
