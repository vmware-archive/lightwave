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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.StringReader;
import java.math.BigInteger;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Unmarshaller;
import javax.xml.crypto.KeySelector;
import javax.xml.crypto.MarshalException;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.dom.DOMValidateContext;
import javax.xml.datatype.XMLGregorianCalendar;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Result;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.validation.Schema;

import oasis.names.tc.saml._2_0.assertion.AdviceType;
import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.AttributeStatementType;
import oasis.names.tc.saml._2_0.assertion.AttributeType;
import oasis.names.tc.saml._2_0.assertion.AudienceRestrictionType;
import oasis.names.tc.saml._2_0.assertion.ConditionAbstractType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;
import oasis.names.tc.saml._2_0.assertion.KeyInfoConfirmationDataType;
import oasis.names.tc.saml._2_0.assertion.NameIDType;
import oasis.names.tc.saml._2_0.assertion.ProxyRestrictionType;
import oasis.names.tc.saml._2_0.assertion.StatementAbstractType;
import oasis.names.tc.saml._2_0.assertion.SubjectConfirmationDataType;
import oasis.names.tc.saml._2_0.assertion.SubjectConfirmationType;
import oasis.names.tc.saml._2_0.assertion.SubjectType;
import oasis.names.tc.saml._2_0.conditions.delegation.DelegateType;
import oasis.names.tc.saml._2_0.conditions.delegation.DelegationRestrictionType;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3._2000._09.xmldsig_.KeyInfoType;
import org.w3._2000._09.xmldsig_.X509DataType;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import com.rsa.names._2009._12.std_ext.saml2.RSAAdviceType;
import com.rsa.names._2009._12.std_ext.saml2.RenewRestrictionType;
import com.rsa.names._2010._04.std_prof.saml2.AttributeNames;
import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.client.Advice;
import com.vmware.vim.sso.client.Advice.AdviceAttribute;
import com.vmware.vim.sso.client.ConfirmationType;
import com.vmware.vim.sso.client.IssuerNameId;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.SubjectNameId;
import com.vmware.vim.sso.client.ValidatableSamlTokenEx;
import com.vmware.vim.sso.client.XmlParserFactory;
import com.vmware.vim.sso.client.exception.InvalidSignatureException;
import com.vmware.vim.sso.client.exception.InvalidTimingException;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.exception.MalformedTokenException;

/**
 * Class representing a SAML token
 */
public class SamlTokenImpl implements ValidatableSamlTokenEx {

   /**
    * The time when the validity of the token starts.
    */
   private long _startTime;

   /**
    * The time when the token expires.
    */
   private long _expirationTime;

   /**
    * True if the token is renewable.
    */
   private boolean _isRenewable;

   /**
    * True if the token is delegable.
    */
   private boolean _isDelegable;

   /**
    * The confirmation type used in token.
    */
   private ConfirmationType _confirmationType;

   /**
    * A subject of a token is the principal to which the token is issued if the
    * format is UPN. If the format is different it will be null and the value of
    * the token subject will be in _subjectId.
    */
   private PrincipalId _subjectUPN;

   /**
    * A subject of a token is the principal to which the token is issued. Here
    * both the value and its format are contained.
    */
   private SubjectNameId _subjectId;

   /**
    * issuer of the token.
    */
   private IssuerNameId _issuerId;

   /**
    * The DOM tree representation of the token
    */
   private final Document _parsedToken;

   /**
    * The time when the token issuance occurred
    */
   private long _issueInstant;

   /**
    * The token ID
    */
   private String _id;

   /**
    * The delegation chain for this token
    */
   private List<TokenDelegate> _delegationChain = Collections.emptyList();
   private List<TokenDelegateEx> _delegationChainEx = Collections.emptyList();

   /**
    * The audience restriction list
    */
   private Set<String> _audienceRestrictionList = Collections.emptySet();

   /**
    * If HoK token this field will contain the user's certificate
    */
   private X509Certificate _confirmationCertificate;

   private List<Advice> _advice = Collections.emptyList();

   /**
    * The groups the subject belongs to
    */
   private List<PrincipalId> _groups = Collections.emptyList();
   /**
    * @see SamlToken#isSolution()
    */
   private boolean _isSolution;

   private final AtomicBoolean _tokenValidated = new AtomicBoolean(false);

   private static final Schema SAML_SCHEMA = loadSamlSchema();

   private XMLGregorianCalendar _subjConfExp;

   private static final String SAML_SCHEMA_FILENAME = "profiled-saml-schema-assertion-2.0.xsd";
   private static final String DEFAULT_TIME_ZONE = "GMT";
   private static final String BEARER_CONFIRMATION = "urn:oasis:names:tc:SAML:2.0:cm:bearer";
   private static final String HOLDER_OF_KEY_CONFIRMATION = "urn:oasis:names:tc:SAML:2.0:cm:holder-of-key";
   private static final String XMLNS_NS_URI = "http://www.w3.org/2000/xmlns/";
   private static final String SIGNATURE_ELEMENT_NAME = "Signature";
   private static final String ASSERTION_ID_ATTR_NAME = "ID";
   private static final String SIGNATURE_VALIDATION_ERROR_MSG = "Signature validation failed";
   private static final String PARSING_TOKEN_ERROR_MSG = "Error parsing SAML token.";
   private static final String PARSE_DELEGATION_ERR_MSG = "Cannot parse delegation restrictions.";
   private static final String X509_CERT_FACTORY_TYPE = "X.509";
   private static final String CERTIFICATE_PARSE_ERR_MSG = "Cannot parse user's confirmation certificate";
   private static final String SUBJ_CONF_DATA_NOT_FOUNT_MSG = "Cannot find subject confirmation data";
   private static final String SUBJ_CONF_DATA_WRONG_TYPE_MSG = "SubjectConfirmationData is not instance"
      + " of KeyInfoConfirmationData type which is necessary for this kind of tokens (HOK).";
   private static final String ERR_LOADNIG_SAML_SCHEMA = "An error occured while loading SAML schema.";
   private static final String PARSE_GROUPS_ERR_MSG = "Cannot parse group information";
   private static final String PARSE_ISSOLUTION_ERR_MSG = "Value for attribute isSolution is not valid.";

   private static final String UPN_FORMAT_URI = "http://schemas.xmlsoap.org/claims/UPN";

   private static final long MILLISECONDS_PER_SECOND = 1000;
   private static final XmlParserFactory xmlParserFactory = XmlParserFactory.Factory
      .createSecureXmlParserFactory();

   private final Logger _log = LoggerFactory.getLogger(SamlTokenImpl.class);
   private final JAXBContext _jaxbContext;

   /**
    * Internal constructor: parse and validate token Document and populate the
    * extracted properties.
    */
   private SamlTokenImpl(String sourceType, Document tokenDoc,
      JAXBContext jaxbContext, Boolean allowNonUpnFormat) throws InvalidTokenException {

      ValidateUtil.validateNotNull(tokenDoc, "token " + sourceType);
      ValidateUtil.validateNotNull(jaxbContext, "JAXBContext");

      _jaxbContext = jaxbContext;
      _parsedToken = tokenDoc;
      validateAndPopulate(allowNonUpnFormat);
      // After the xml is validated against the schema, we can safely assume
      // that the root element is saml2:Assertion.
      markAssertionIdAttribute(_parsedToken.getDocumentElement());

      _log.info("SAML token for " + _subjectId + " successfully parsed from "
         + sourceType);
   }

   /**
    * Parses and validates the provided SAML xml string. If an object is created
    * it is guaranteed that the input SAML xml string's schema format semantics
    * are correct.
    *
    * @param xml
    *           The string containing a SAML token. Cannot be {@code null}.
    * @param jaxbContext
    *           the JAXBContext that will be used for unmarshalling the
    *           assertion
    * @throws InvalidTokenException
    *            Thrown if there is xml parsing or semantical error
    */
   public SamlTokenImpl(String xml, JAXBContext jaxbContext)
      throws InvalidTokenException {

      this("XML", parseTokenXmlToDom(xml), jaxbContext, false);
   }

   /**
    * Parse and validate the DOM subtree as SAML. Successful creation indicates
    * the tree contained valid token.
    *
    * @param tokenRoot
    *           The root of the DOM tree containing a SAML token. Cannot be
    *           {@code null}. The implementation will retain a copy of the
    *           sub-tree and the original element will not be modified.
    * @param jaxbContext
    *           the JAXBContext that will be used for unmarshalling the
    *           assertion
    * @throws InvalidTokenException
    *            Thrown if there is syntactical or semantical error
    */
   public SamlTokenImpl(Element tokenRoot, JAXBContext jaxbContext)
      throws InvalidTokenException {

      this("Element", createStandaloneCopy(tokenRoot), jaxbContext, false);
   }

   /**
    * Parses and validates the provided SAML xml string. If an object is created
    * it is guaranteed that the input SAML xml string's schema format semantics
    * are correct.
    *
    * @param xml
    *           The string containing a SAML token. Cannot be {@code null}.
    * @param jaxbContext
    *           the JAXBContext that will be used for unmarshalling the
    *           assertion
    * @param allowDelegateInNonUpnFormat Whether to restrict the delegation spec
    *           to subjects in Upn format.
    * @throws InvalidTokenException
    *            Thrown if there is xml parsing or semantical error
    */
   public SamlTokenImpl(String xml, JAXBContext jaxbContext, Boolean allowDelegateInNonUpnFormat)
      throws InvalidTokenException {

      this("XML", parseTokenXmlToDom(xml), jaxbContext, allowDelegateInNonUpnFormat);
   }

   /**
    * Parse and validate the DOM subtree as SAML. Successful creation indicates
    * the tree contained valid token.
    *
    * @param tokenRoot
    *           The root of the DOM tree containing a SAML token. Cannot be
    *           {@code null}. The implementation will retain a copy of the
    *           sub-tree and the original element will not be modified.
    * @param jaxbContext
    *           the JAXBContext that will be used for unmarshalling the
    *           assertion
    * @param allowDelegateInNonUpnFormat Whether to restrict the delegation spec
    *           to subjects in Upn format.
    * @throws InvalidTokenException
    *            Thrown if there is syntactical or semantical error
    */
   public SamlTokenImpl(Element tokenRoot, JAXBContext jaxbContext, Boolean allowDelegateInNonUpnFormat)
      throws InvalidTokenException {

      this("Element", createStandaloneCopy(tokenRoot), jaxbContext, allowDelegateInNonUpnFormat);
   }

   /**
    * @return the startTime
    */
   @Override
   public Date getStartTime() {
      checkIsSignatureValidated();
      return new Date(_startTime);
   }

   /**
    * @return the expiration time
    */
   @Override
   public Date getExpirationTime() {
      checkIsSignatureValidated();
      return new Date(_expirationTime);
   }

   /**
    * @return the isRenewable
    */
   @Override
   public boolean isRenewable() {
      checkIsSignatureValidated();
      return _isRenewable;
   }

   /**
    * @return the isDelegable
    */
   @Override
   public boolean isDelegable() {
      checkIsSignatureValidated();
      return _isDelegable;
   }

   /**
    * @return the confirmationType
    */
   @Override
   public ConfirmationType getConfirmationType() {
      checkIsSignatureValidated();
      return _confirmationType;
   }

   /**
    * @return the xmlToken as string without XML declaration
    */
   @Override
   public String toXml() {
      checkIsSignatureValidated();
      try {
         return Util.serializeToString(_parsedToken.getDocumentElement());

      } catch (ParserException e) {
         throw new IllegalStateException(e);
      }
   }

   /**
    * Imports a copy of the XML representation of the token into the
    * given document as a first child of the provided node
    *
    * <p>
    * The copied token signature will remain valid if the original one was.
    *
    * @param parentNode required
    */
   @Override
   public Node importTo(Document hostDocument) {
      ValidateUtil.validateNotNull(hostDocument, "Host document");

      Element clonedTokenElement = (Element) hostDocument.importNode(
         _parsedToken.getDocumentElement(), true /* deep */);
      markAssertionIdAttribute(clonedTokenElement);
      return clonedTokenElement;
   }

   /**
    * Export a copy of the token's XML structure to the supplied destination.
    * <p>
    * Usage: for exporting a serialized (String) representation, supply
    * {@link javax.xml.transform.stream.StreamResult}.
    *
    * <pre>
    * {@code
    *    SamlToken token = ...
    *    StringWriter writer = new StringWrtier();
    *    token.export(new StreamResult(writer));
    *    ...
    *    writer.toString(); // will return the token as string
    * }
    * </pre>
    *
    * For exporting the DOM representation, supply
    * {@link javax.xml.transform.dom.DOMResult}:
    *
    * <pre>
    * {@code
    *    SamlToken token = ...
    *    DOMResult dom = new DOMResult();
    *    token.export(dom);
    *    ...
    *    dom.getNode(); // will return Document instance containing the token
    * }
    * </pre>
    *
    * @param destination
    *           The destination for the export operation.
    */
   public void export(Result destination) {
      checkIsSignatureValidated();
      TransformerFactory xfrmFactory = TransformerFactory.newInstance();
      Transformer xfrm;
      try {
         xfrm = xfrmFactory.newTransformer();

      } catch (TransformerConfigurationException e) {
         throw new IllegalStateException(
            "Failed to create XML identity transformer "
               + "(incompliant Java implementation?)", e);
      }

      try {
         xfrm.transform(new DOMSource(_parsedToken), destination);

      } catch (TransformerException e) {
         throw new IllegalArgumentException(
            "Exporting SAML XML failed with the supplied destination", e);
      }
   }

   /**
    * @return the subject of the token
    */
   @Override
   public PrincipalId getSubject() {
      checkIsSignatureValidated();
      return _subjectUPN;
   }

   /**
    * @return the subject of the token together with its format
    */
   @Override
   public SubjectNameId getSubjectNameId() {
      checkIsSignatureValidated();
      return _subjectId;
   }

   @Override
   public IssuerNameId getIssuerNameId()
   {
       checkIsSignatureValidated();
       return _issuerId;
   }

   @Override
   public String getId() {
      checkIsSignatureValidated();
      return _id;
   }

   @Override
   public List<TokenDelegate> getDelegationChain() {
      checkIsSignatureValidated();
      return _delegationChain;
   }

   @Override
   public List<TokenDelegateEx> getDelegationChainEx()
   {
       checkIsSignatureValidated();
       return _delegationChainEx;
   }

   @Override
   public Set<String> getAudience() {
      checkIsSignatureValidated();
      return _audienceRestrictionList;
   }

   @Override
   public X509Certificate getConfirmationCertificate() {
      checkIsSignatureValidated();
      return _confirmationCertificate;
   }

   @Override
   public List<Advice> getAdvice() {
      checkIsSignatureValidated();
      return _advice;
   }

   @Override
   public List<PrincipalId> getGroupList() {
      checkIsSignatureValidated();
      return _groups;
   }

   @Override
   public boolean isSolution() {
      checkIsSignatureValidated();
      return _isSolution;
   }

   @Override
   public void validate(X509Certificate[] trustedRootCertificates,
      long clockToleranceSec) throws InvalidTokenException {
      ValidateUtil.validateNotEmpty(trustedRootCertificates,
         "Trusted root certificates");
      KeySelector signKeySelector = new X509TrustChainKeySelector(
         trustedRootCertificates);
     if (validateSignature(signKeySelector)) {
         validateWithinTokenLifePeriod(clockToleranceSec);
         validateSubjectConfirmationExpDate();
         _tokenValidated.set(true);
      } else {
         _log.info("SAML token cannot be constructed: "
            + SIGNATURE_VALIDATION_ERROR_MSG);
         throw new InvalidSignatureException(SIGNATURE_VALIDATION_ERROR_MSG);
      }
      _log.debug("Token is successfully validated");
   }

   /**
    * Parses the xml string representing the SAML token.<br>
    * Validates that the schema format is correct and the token fields are
    * semantically correct.<br>
    * Populates the class members with the values provided in the token.<br>
    *
    * @param stsCertificate
    *           The certificate which is expected to be used in the token
    *           signature.
    * @throws InvalidTokenException
    *            Thrown if there is semantical error
    * @throws MalformedTokenException
    *            when token cannot be parsed
    */
   @SuppressWarnings("unchecked")
   private void validateAndPopulate(Boolean allowNonUpnFormat) throws InvalidTokenException {

      JAXBElement<AssertionType> jaxbParserResult = null;

      //
      // parse the input xml
      //
      try {
         Unmarshaller unmarshaller = _jaxbContext.createUnmarshaller();

         //
         // verify that SAML token has correct schema format
         //
         unmarshaller.setSchema(SAML_SCHEMA);

         jaxbParserResult = (JAXBElement<AssertionType>) unmarshaller
            .unmarshal(_parsedToken);
      } catch (JAXBException e) {
         _log.info(PARSING_TOKEN_ERROR_MSG, e);
         throw new MalformedTokenException(PARSING_TOKEN_ERROR_MSG, e);
      }

      AssertionType assertion = jaxbParserResult.getValue();

      parseAssertionAttributes(assertion);

      // parse conditions before the subject because _startTime is needed in
      // subject element verification
      parseConditions(assertion.getConditions(), allowNonUpnFormat);
      parseSubject(assertion.getSubject());
      parseIssuer(assertion.getIssuer());

      //
      // Parse the authentication and advice statements
      //
      parseAuthnStatement(assertion.getAuthnStatementOrAttributeStatement());
      if (assertion.getAuthnStatementOrAttributeStatement() != null) {
         parseAttributeStatement(assertion.getAuthnStatementOrAttributeStatement());
      }
      if (assertion.getAdvice() != null) {
         parseAdvice(assertion.getAdvice());
      }

      _log.debug("Token fields are successfully populated");
   }

   /**
    * Parse and validate the assertion attributes
    *
    * @param assertion
    */
   private void parseAssertionAttributes(AssertionType assertion) {

      _issueInstant = assertion
         .getIssueInstant()
         .toGregorianCalendar(TimeZone.getTimeZone(DEFAULT_TIME_ZONE), null,
            null).getTimeInMillis();

      _id = assertion.getID();
      assert _id != null : "assertion ID is required attribute";

      if (_log.isDebugEnabled()) {
         _log.debug("SAML assertion attributes successfully parsed."
            + " Got issueInstant: " + new Date(_issueInstant));
      }
   }

   /**
    * Validates the signature element of the SAML token.
    *
    * @param publicKey
    * @return true if the signature is correct.<br>
    *         false if the signature is not correct.
    * @throws MalformedTokenException
    *            when token signature cannot be parsed
    */
   private boolean validateSignature(KeySelector keySelector)
      throws MalformedTokenException {

      // Note that single instance of signature node should always be found
      // because this is enforced by the schema
      NodeList securityNodeList = _parsedToken.getElementsByTagNameNS(
         Constants.DIGITAL_SIGNATURE_NAMESPACE, SIGNATURE_ELEMENT_NAME);

      XMLSignatureFactory fac = XMLSignatureFactory.getInstance();
      DOMValidateContext valContext = new DOMValidateContext(keySelector,
         securityNodeList.item(0));

      boolean isValid = false;
      try {
         XMLSignature signature = fac.unmarshalXMLSignature(valContext);
         isValid = signature.validate(valContext);
      } catch (MarshalException e) {
         _log.error(SIGNATURE_VALIDATION_ERROR_MSG, e);
         throw new MalformedTokenException(SIGNATURE_VALIDATION_ERROR_MSG, e);
      } catch (XMLSignatureException e) {
         _log.error(SIGNATURE_VALIDATION_ERROR_MSG, e);
         throw new MalformedTokenException(SIGNATURE_VALIDATION_ERROR_MSG, e);
      }

      _log.debug("SAML token signature is valid status: " + isValid);

      return isValid;
   }

   /**
    * Parse the token's string representation to DOM Document representation.
    *
    * @return
    * @throws MalformedTokenException
    *            when token cannot be parsed
    */
   private static Document parseTokenXmlToDom(String xmlToken)
      throws MalformedTokenException {

      if (xmlToken == null) {
         return null;
      }

      Logger log = LoggerFactory.getLogger(SamlTokenImpl.class);

      final Document parsedToken;
      try {
         parsedToken = xmlParserFactory.newDocumentBuilder().parse(
            new InputSource(new StringReader(xmlToken)));

      } catch (SAXException e) {
         log.info(PARSING_TOKEN_ERROR_MSG, e);
         throw new MalformedTokenException(PARSING_TOKEN_ERROR_MSG, e);

      } catch (IOException e) {
         String message = "Error reading from in-memory stream "
            + "(heap space exhausted?)";
         log.error(message, e);
         throw new IllegalStateException(message, e);

      } catch (ParserConfigurationException e) {
         String message = "DOM Document builder is not available "
            + "(incompatible Java implementation?)";
         log.error(message, e);
         throw new IllegalStateException(message, e);
      }

      return parsedToken;
   }

   /**
    * Create a standalone copy of the provided DOM Element and place it into a
    * Document of its own. In addition the the original element's attributes,
    * the copy will also have all namespace declarations visible in the original
    * element.
    */
   private static Document createStandaloneCopy(Element element) {

      Map<String, String> visibleNamespaces = new HashMap<String, String>();
      Node walker = element.getParentNode();
      while (walker != null && walker.getNodeType() == Node.ELEMENT_NODE) {
         NamedNodeMap attrs = walker.getAttributes();
         for (int i = 0; i < attrs.getLength(); ++i) {
            Attr attr = (Attr) attrs.item(i);
            if (XMLNS_NS_URI.equals(attr.getNamespaceURI())
               && !visibleNamespaces.containsKey(attr.getName())) {

               visibleNamespaces.put(attr.getName(), attr.getValue());
            }
         }

         walker = walker.getParentNode();
      }

      Transformer tx;
      try {
         tx = TransformerFactory.newInstance().newTransformer();

      } catch (TransformerException e) {
         throw new IllegalStateException("Failed to create identity XML "
            + "transformer. Incompatible Java platform?", e);
      }

      DOMResult result = new DOMResult();
      try {
         tx.transform(new DOMSource(element), result);

      } catch (TransformerException e) {
         throw new IllegalStateException("Unexpected failure in Identity "
            + "DOM-to-DOM transformation", e);
      }
      Document standaloneDoc = (Document) result.getNode();

      Element standaloneToken = standaloneDoc.getDocumentElement();
      for (Map.Entry<String, String> nsAttr : visibleNamespaces.entrySet()) {
         standaloneToken.setAttributeNS(XMLNS_NS_URI, nsAttr.getKey(),
            nsAttr.getValue());
      }

      return standaloneDoc;
   }

   /**
    * Parse and validate the conditions element.<br>
    * This elements contains conditions which should be met for a token to be
    * considered valid<br>
    * Conditions element is a child of the assertion element
    *
    * @param conditions
    * @throws MalformedTokenException
    *            when delegation subject cannot be parsed
    */
   private void parseConditions(ConditionsType conditions, Boolean allowNonUpnFormat)
      throws MalformedTokenException {

      _startTime = conditions
         .getNotBefore()
         .toGregorianCalendar(TimeZone.getTimeZone(DEFAULT_TIME_ZONE), null,
            null).getTimeInMillis();
      _expirationTime = conditions
         .getNotOnOrAfter()
         .toGregorianCalendar(TimeZone.getTimeZone(DEFAULT_TIME_ZONE), null,
            null).getTimeInMillis();

      List<ConditionAbstractType> conditionList = conditions
    		  .getConditionOrAudienceRestrictionOrOneTimeUseOrProxyRestriction();
      for (ConditionAbstractType condition : conditionList) {
         if (condition instanceof ProxyRestrictionType) {
            BigInteger count = ((ProxyRestrictionType) condition).getCount();
            _isDelegable = (count != null && count.longValue() > 0) ? true
               : false;
         } else if (condition instanceof AudienceRestrictionType) {
            HashSet<String> audienceSet = new HashSet<String>();
            audienceSet.addAll(((AudienceRestrictionType) condition)
               .getAudience());
            _audienceRestrictionList = Collections.unmodifiableSet(audienceSet);
         } else if (condition instanceof RenewRestrictionType) {
            BigInteger count = ((RenewRestrictionType) condition).getCount();
            _isRenewable = (count != null && count.longValue() > 0) ? true
               : false;
         } else if (condition instanceof DelegationRestrictionType) {
            parseDelegationChain((DelegationRestrictionType) condition, allowNonUpnFormat);
         }
      }

      if (_log.isDebugEnabled()) {
         _log.debug("Conditions parsed successfully." + " Got startTime: "
            + new Date(_startTime) + " expirationTime: "
            + new Date(_expirationTime));
      }
   }

   /**
    * Validates that the token is within its lifetime
    *
    * @param clockToleranceSec
    * @throws InvalidTimingException
    *            when token lifetime is malformed or token is expired
    */
   private void validateWithinTokenLifePeriod(long clockToleranceSec)
      throws InvalidTimingException {
      if (_expirationTime < _startTime) {
         String message = "Start time / Expiration time not valid: "
            + "StartTime: " + new Date(_startTime) + " ExpirationTime: "
            + new Date(_expirationTime);
         _log.error(message);
         throw new InvalidTimingException(message);
      }

      long currentTime = Calendar.getInstance(
         TimeZone.getTimeZone(DEFAULT_TIME_ZONE)).getTimeInMillis();
      long effectiveExpirationTime = _expirationTime + clockToleranceSec
         * MILLISECONDS_PER_SECOND;

      if (effectiveExpirationTime < currentTime) {
         String message = "Token expiration date: " + new Date(_expirationTime)
            + " is in the past.";
         _log.info(message);
         throw new InvalidTimingException(message);
      }
   }

   /**
    * Parses delegation chain
    *
    * @param delegation
    * @throws MalformedTokenException
    *            when delegation subject cannot be parsed
    */
   private void parseDelegationChain(DelegationRestrictionType delegation, Boolean allowNonUpnFormat)
      throws MalformedTokenException {
      List<TokenDelegateEx> delegationChain = new ArrayList<TokenDelegateEx>();
      for (DelegateType delegate : delegation.getDelegate()) {

         PrincipalId subject;
         try {

            subject = parseSubject(delegate.getNameID(), allowNonUpnFormat);

         } catch (ParserException e) {
            _log.error(PARSE_DELEGATION_ERR_MSG, e);
            throw new MalformedTokenException(PARSE_DELEGATION_ERR_MSG, e);
         }

         delegationChain.add(
             new TokenDelegateExImpl(
                  new SubjectNameId(delegate.getNameID().getValue(), delegate.getNameID().getFormat()),
                  subject,
                  delegate.getDelegationInstant().toGregorianCalendar(
                      TimeZone.getTimeZone(DEFAULT_TIME_ZONE), null, null).getTimeInMillis()
             )
         );
      }

      Collections.sort(delegationChain, new Comparator<TokenDelegate>() {
         @Override
         public int compare(TokenDelegate o1, TokenDelegate o2) {
            long o1Date = o1.getDelegationDate().getTime();
            long o2Date = o2.getDelegationDate().getTime();
            return (o1Date < o2Date) ? -1 : (o1Date == o2Date) ? 0 : 1;
         }
      });

      _delegationChainEx = Collections.<TokenDelegateEx>unmodifiableList(delegationChain);
      _delegationChain = Collections.<TokenDelegate>unmodifiableList(delegationChain);
   }

   /**
    * Parse and validate the subject element
    *
    * @param subject
    * @throws MalformedTokenException
    *            when subject cannot be parsed
    */
   private void parseSubject(SubjectType subject)
      throws MalformedTokenException {
      NameIDType subjectNameID = subject.getNameID();
      _subjectId = getSubjectId(subjectNameID);
      try {
         if (_subjectId.getFormat().equalsIgnoreCase(UPN_FORMAT_URI)) {
            this._subjectUPN = PrincipalIdParser.parseUpn(subjectNameID.getValue());
         }

      } catch (ParserException e) {
         String upnParsingErrMsg = "Cannot parse subject because its value is not in UPN format";
         _log.debug(upnParsingErrMsg, e);
         throw new MalformedTokenException(upnParsingErrMsg, e);
      }

      //
      // Parse the subject confirmation data
      //
      SubjectConfirmationType subConf = subject.getSubjectConfirmation();
      if (subConf.getMethod().equalsIgnoreCase(BEARER_CONFIRMATION)) {
         SubjectConfirmationDataType subConfData = subConf
            .getSubjectConfirmationData();
         _subjConfExp = subConfData.getNotOnOrAfter();

         _confirmationType = ConfirmationType.BEARER;
      } else if (subConf.getMethod().equalsIgnoreCase(
         HOLDER_OF_KEY_CONFIRMATION)) {
         parseHolderOfKeyConfirmation(subject);
         _confirmationType = ConfirmationType.HOLDER_OF_KEY;
      }

      if (_log.isDebugEnabled()) {
         _log.debug(_subjectId + " successfully extracted from the token");
         _log.debug("Got confirmation type: " + _confirmationType);
      }
   }

   /**
    * Parse the issuer element
    *
    * @param issuer
    * @throws MalformedTokenException
    *            when issuer cannot be parsed
    */
   private void parseIssuer(NameIDType issuer)
      throws MalformedTokenException {

       this._issuerId = null;
       if ( issuer != null )
       {
           try
           {
               this._issuerId = new IssuerNameId(issuer.getValue(), issuer.getFormat());
               if (_log.isDebugEnabled()) {
                   _log.debug(_issuerId + " successfully extracted from the token");
                }
           }
           catch(Exception ex)
           {
               _log.debug("Cannot parse issuer.", ex);
               throw new MalformedTokenException("Invalid issuer.", ex);
           }
       }
   }

   private void validateSubjectConfirmationExpDate()
      throws InvalidTimingException {
      if (_subjConfExp != null) {
         long subjConfExp = _subjConfExp.toGregorianCalendar(
            TimeZone.getTimeZone(DEFAULT_TIME_ZONE), null, null)
            .getTimeInMillis();
         if (subjConfExp > _expirationTime) {
            String message = "Subject confirmation expiration time is not valid: "
               + "Subject time: "
               + new Date(subjConfExp)
               + " Token ExpirationTime: " + new Date(_expirationTime);
            _log.error(message);
            throw new InvalidTimingException(message);
         }
      }
   }

   private static PrincipalId parseSubject(NameIDType subject, Boolean allowNonUpnFormat)
      throws ParserException {

      String subjectFormat = subject.getFormat();
      String nameQualifier = subject.getNameQualifier();

      if ( (allowNonUpnFormat == false) && ((!subjectFormat.equalsIgnoreCase(UPN_FORMAT_URI)) || (nameQualifier != null))) {

         throw new ParserException(String.format(
            "Failed to parse subject: format = '%s', name qualifier = '%s'",
            subjectFormat, nameQualifier));
      }
      PrincipalId principal = null;
      if ( subjectFormat.equalsIgnoreCase(UPN_FORMAT_URI) ) {
          principal = PrincipalIdParser.parseUpn(subject.getValue());
      }

      return principal;
   }

   private static SubjectNameId getSubjectId(NameIDType subject) {
      assert subject != null;

      return new SubjectNameId(subject.getValue(), subject.getFormat());
   }

   /**
    * Parses holder of key confirmation data
    *
    * @param subject
    * @throws MalformedTokenException
    *            when HoK cert cannot be parsed or subject confirmation element
    *            is malformed
    */
   private void parseHolderOfKeyConfirmation(SubjectType subject)
      throws MalformedTokenException {

      SubjectConfirmationDataType subjectConfirmationData = subject
         .getSubjectConfirmation().getSubjectConfirmationData();

      if (!(subjectConfirmationData instanceof KeyInfoConfirmationDataType)) {
         _log.error(SUBJ_CONF_DATA_WRONG_TYPE_MSG);
         throw new MalformedTokenException(SUBJ_CONF_DATA_WRONG_TYPE_MSG);
      }

      // Start searching for
      // saml:SubjectConfirmationData/ds:KeyInfo/ds:X509Data/ds:X509Certificate
      // all casts are needed because the schema cannot be modified to
      // reflect the restrictions in a "JAXB friendly" way
      KeyInfoType keyInfo = getTheValue(subjectConfirmationData.getContent(),
         KeyInfoType.class);

      X509DataType x509Data = keyInfo != null ? getTheValue(
         keyInfo.getContent(), X509DataType.class) : null;

      byte[] cert = x509Data != null ? getTheValue(
         x509Data.getX509IssuerSerialOrX509SKIOrX509SubjectName(), byte[].class)
         : null;

      if (cert != null) {
         try {
            CertificateFactory cf = CertificateFactory
               .getInstance(X509_CERT_FACTORY_TYPE);

            _confirmationCertificate = (X509Certificate) cf
               .generateCertificate(new ByteArrayInputStream(cert));

         } catch (CertificateException e) {
            _log.error(CERTIFICATE_PARSE_ERR_MSG, e);
            throw new MalformedTokenException(CERTIFICATE_PARSE_ERR_MSG, e);
         }
      }

      if (_confirmationCertificate == null) {
         _log.error(SUBJ_CONF_DATA_NOT_FOUNT_MSG);
         throw new MalformedTokenException(SUBJ_CONF_DATA_NOT_FOUNT_MSG);
      }
   }

   /**
    * Evaluates if the provided list contains a single instance of a JAXBElement
    * which is of the given type
    *
    * @param list
    * @param valueType
    * @return
    */
   @SuppressWarnings("unchecked")
   private static <T> T getTheValue(List<?> list, Class<T> valueType) {

      JAXBElement<?> theOnlyElement = getSingleJaxbElement(list);
      return valueType.isInstance(theOnlyElement.getValue()) ? (T) theOnlyElement
         .getValue() : null;
   }

   private static JAXBElement<?> getSingleJaxbElement(List<?> list) {
      JAXBElement<?> theOnlyElement = null;
      for (Object obj : list) {
         if (obj instanceof JAXBElement<?>) {
            if (theOnlyElement != null) {
               return null;
            }
            theOnlyElement = (JAXBElement<?>) obj;
         }
      }
      return theOnlyElement;
   }

   /**
    * Parse the authentication statement element
    *
    * @param authnStatement
    */
   private void parseAuthnStatement(List<StatementAbstractType> authnStatement) {
      // TODO the SamlToken interface does not expose methods for accessing
      // authentication statements. They are needed for auditing purposes.
      //authnStatement.getAuthnInstant();
      //authnStatement.getAuthnContext().getAuthnContextClassRef();
   }

   /**
    * Parses the advice list
    *
    * @param advice
    */
   private void parseAdvice(AdviceType advice) {
      List<Advice> adviceList = new ArrayList<Advice>();
      for (RSAAdviceType rsaAdvice : advice.getRSAAdvice()) {
         String source = rsaAdvice.getAdviceSource();
         List<AdviceAttribute> adviceAttributes = new ArrayList<AdviceAttribute>();

         if (rsaAdvice.getAttribute() != null) {
            for (AttributeType attribute : rsaAdvice.getAttribute()) {
               List<String> attrValues = attribute.getAttributeValue();
               String attrName = attribute.getName();
               String friendlyName = attribute.getFriendlyName();

               if ( attrValues == null ) {
                   attrValues = new ArrayList<String>();
               }

               adviceAttributes.add(
                   new AdviceAttribute( attrName, friendlyName, attrValues )
               );
            }
         }

         adviceList.add(new Advice(source, adviceAttributes));
      }

      _advice = Collections.unmodifiableList(adviceList);
   }

   /**
    * Parse the attribute statements.
    *
    * @param attrStatement
    * @throws MalformedTokenException
    *            when attributes cannot be parsed or they are malformed
    */
   private void parseAttributeStatement(List<StatementAbstractType> statementList)
      throws MalformedTokenException {

       if ( statementList != null )
       {
          for( StatementAbstractType stmt : statementList )
          {
              if ( stmt instanceof AttributeStatementType )
              {
                  AttributeStatementType attrStatement = (AttributeStatementType)stmt;
                  List<AttributeType> attributeList = attrStatement.getAttribute();
                  List<PrincipalId> groupList = new ArrayList<PrincipalId>();

                  for (AttributeType attribute : attributeList)
                  {
                     String attributeName = attribute.getName();
                     // group attribute
                     if (attributeName
                        .equals(AttributeNames.HTTP_RSA_COM_SCHEMAS_ATTR_NAMES_2009_01_GROUP_IDENTITY
                           .value())) {
                        try {
                           groupList.addAll(parseGroup(attribute.getAttributeValue()));
                        } catch (ParserException e) {
                           _log.debug(PARSE_GROUPS_ERR_MSG, e);
                           throw new MalformedTokenException(PARSE_GROUPS_ERR_MSG, e);
                        }
                        _log.debug("Groups successfully extracted from token");
                     } else if (attributeName
                        .equals(AttributeNames.HTTP_VMWARE_COM_SCHEMAS_ATTR_NAMES_2011_07_IS_SOLUTION
                           .value())) {
                        List<String> attributeValue = attribute.getAttributeValue();
                        // We do not support default value for this attribute.
                        // If present, exactly one value should be set for it
                        if (null != attributeValue && 1 == attributeValue.size()) {
                           _isSolution = Boolean.parseBoolean(attributeValue.get(0));
                           _log.debug("isSolution attribute parsed successfully from "
                              + attributeValue + " to: " + _isSolution);
                        } else {
                           throw new MalformedTokenException(PARSE_ISSOLUTION_ERR_MSG);
                        }
                     }
                  }
                  _groups = Collections.unmodifiableList(groupList);
                  _log.debug("Attribute statements successfully parsed");
              }
          }
      }
   }

   /**
    * {@link PrincipalId} representation of groups as provided in SAML token.
    *
    * @param groupList
    *           groups to transform
    * @return list of transformed groups
    *
    * @throws ParserException
    */
   private static List<PrincipalId> parseGroup(List<String> groupList)
      throws ParserException {

      assert groupList != null;

      List<PrincipalId> groupResult = new ArrayList<PrincipalId>(
         groupList.size());

      for (String group : groupList) {
         groupResult.add(PrincipalIdParser.parseGroupId(group));
      }

      return groupResult;
   }

   /**
    * Verifies that the element format URI matches the expected value
    *
    * @param gotFormat
    * @param expectedFormat
    * @throws MalformedTokenException
    *            The URI does not match the expected value
    */
/*   private void verifyElementFormatURI(String gotFormat, String expectedFormat)
      throws MalformedTokenException {
      // check the format has the correct type
      if (!gotFormat.equalsIgnoreCase(expectedFormat)) {
         String message = "Element format does not match the expected URI format. Got: "
            + gotFormat + " Expected: " + expectedFormat;
         _log.error(message);
         throw new MalformedTokenException(message);
      }
   }
*/
   private static Schema loadSamlSchema()
   {
       try
       {
           Schema samlSchema = Util.loadXmlSchemaFromResource(SamlTokenImpl.class, SAML_SCHEMA_FILENAME);
           return samlSchema;
       }
       catch(IllegalArgumentException e)
       {
           LoggerFactory.getLogger(SamlTokenImpl.class).error(
                   String.format("Schema resource `%s' is missing.", SAML_SCHEMA_FILENAME), e);
           throw new DeploymentError(String.format(
                   "Schema resource `%s' is missing.", SAML_SCHEMA_FILENAME));
       }
       catch(SAXException e)
       {
           LoggerFactory.getLogger(SamlTokenImpl.class).error(
                   ERR_LOADNIG_SAML_SCHEMA, e);
           throw new DeploymentError(ERR_LOADNIG_SAML_SCHEMA, e);
       }
   }

   /**
    * Thrown to indicate incompatible Java platform or missing required
    * resources.
    */
   static class DeploymentError extends Error {
      private static final long serialVersionUID = -6610749680263268064L;

      public DeploymentError(String message, Throwable cause) {
         super(message, cause);
      }

      public DeploymentError(String message) {
         super(message);
      }
   }

   @Override
   public boolean equals(Object other) {
      return other instanceof SamlToken
         && getId().equals(((SamlToken) other).getId());
   }

   @Override
   public int hashCode() {
      return getId().hashCode();
   }

   /**
    * Implementation of {@link TokenDelegate}
    */
   public static class TokenDelegateImpl implements TokenDelegate {

      private final PrincipalId _subject;
      private final long _delegationDate;

      /**
       * @param subject
       *           cannot be null.
       * @param delegationDate
       */
      public TokenDelegateImpl(PrincipalId subject, long delegationDate) {
         assert subject != null;

         _subject = subject;
         _delegationDate = delegationDate;
      }

      @Override
      public PrincipalId getSubject() {
         return _subject;
      }

      @Override
      public Date getDelegationDate() {
         return new Date(_delegationDate);
      }

      // Generated by Eclipse. Doesn't contain private data.
      @Override
      public String toString() {
         return String.format(
            "TokenDelegateImpl [subject=%s, delegationDate=%s]", _subject,
            _delegationDate);
      }

   }

   /**
    * Implementation of {@link TokenDelegateEx}
    */
   public static class TokenDelegateExImpl implements TokenDelegateEx {

      protected SubjectNameId _subjectNameId;
      protected PrincipalId _subject;
      protected long _delegationDate;

      /**
       * @param subjectNameId
       *           cannot be null.
       * @param subject
       *           subject in UpnFormat
       * @param delegationDate
       */
      public TokenDelegateExImpl(SubjectNameId subjectNameId, PrincipalId subject, long delegationDate) {
         assert subjectNameId != null;

         _subjectNameId = subjectNameId;
         _subject = subject;
         _delegationDate = delegationDate;
      }

      @Override
      public PrincipalId getSubject() {
         return _subject;
      }

      @Override
      public Date getDelegationDate() {
         return new Date(_delegationDate);
      }

      @Override
      public SubjectNameId getSubjectNameId()
      {
          return _subjectNameId;
      }

      @Override
      public String toString() {
         return String.format(
            "TokenDelegateImpl [subject=%s, delegationDate=%s]", _subjectNameId,
            _delegationDate);
      }
   }

   /**
    * Throws {@link IllegalStateException} if signature is not validated. Only
    * validate should allowed to be invoked in such state.
    */
   private void checkIsSignatureValidated() {
      if (!_tokenValidated.get()) {
         throw new IllegalStateException(
            "Until token signature is validated accessors cannot be used.");
      }
   }

   /**
    * Marks the ID attribute of given element as xml:id, required
    * for signature validation. During validation, the signed element is
    * retrieved using getElementById.
    *
    * @param assertionElement the saml2:Assertion element, required
    */
   private static void markAssertionIdAttribute(Element assertionElement) {
      assert assertionElement != null;

      // based on the schema
      assert assertionElement.hasAttribute(ASSERTION_ID_ATTR_NAME);

      /* Normally, ID attributes are marked during parsing when schema is
       * used. The schema or DTD define which attributes are IDs.
       * However, token string-to-DOM parsing doesn't use a schema
       * due to reasons explained in SecureXmlParserFactory.
       * Additionally, IDness is lost when importing nodes into other
       * documents, serializing DOM to text and similar operations.
       */
      assertionElement.setIdAttribute(ASSERTION_ID_ATTR_NAME, true /* isId */);
   }

}
