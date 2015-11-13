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
package com.vmware.vim.sso.client.tools;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.security.InvalidAlgorithmParameterException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.GregorianCalendar;
import java.util.List;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.crypto.MarshalException;
import javax.xml.crypto.dsig.CanonicalizationMethod;
import javax.xml.crypto.dsig.DigestMethod;
import javax.xml.crypto.dsig.Reference;
import javax.xml.crypto.dsig.SignedInfo;
import javax.xml.crypto.dsig.Transform;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.dom.DOMSignContext;
import javax.xml.crypto.dsig.keyinfo.KeyInfoFactory;
import javax.xml.crypto.dsig.spec.C14NMethodParameterSpec;
import javax.xml.crypto.dsig.spec.ExcC14NParameterSpec;
import javax.xml.datatype.DatatypeConfigurationException;
import javax.xml.datatype.DatatypeFactory;
import javax.xml.datatype.XMLGregorianCalendar;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.AttributeStatementType;
import oasis.names.tc.saml._2_0.assertion.AttributeType;
import oasis.names.tc.saml._2_0.assertion.AuthnContextType;
import oasis.names.tc.saml._2_0.assertion.AuthnStatementType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;
import oasis.names.tc.saml._2_0.assertion.NameIDType;
import oasis.names.tc.saml._2_0.assertion.ObjectFactory;
import oasis.names.tc.saml._2_0.assertion.SubjectConfirmationDataType;
import oasis.names.tc.saml._2_0.assertion.SubjectConfirmationType;
import oasis.names.tc.saml._2_0.assertion.SubjectType;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.rsa.names._2010._04.std_prof.saml2.AttributeNames;
import com.vmware.identity.token.impl.Util;
import com.vmware.identity.token.impl.ValidateUtil;
import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.client.ConfirmationType;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.TestTokenUtil;
import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;

/**
 * This class is used to create/modify SAML tokens
 */
public class SamlTokenCreator {

   private static final String ID_ATTR = "ID";
   private static final String ASSERTION_JAXB_PACKAGE = "oasis.names.tc.saml._2_0.assertion";
   private static final String ASSERTION_ID = "_b07b804c-7c29-ea16-7300-4f3d6f7928ac";
   private static final String SAML_VERSION = "2.0";
   private static final String SUBJECT_NAME_ID_FORMAT = "http://schemas.xmlsoap.org/claims/UPN";
   private static final String BEARER_CONFIRMATION = "urn:oasis:names:tc:SAML:2.0:cm:bearer";
   private static final String ATTRIBUTE_FORMAT_URI = "urn:oasis:names:tc:SAML:2.0:attrname-format:uri";
   private static final String SAML_TOKEN_DIR = "/saml_token/";
   private static final String ISSUER_FORMAT = "urn:oasis:names:tc:SAML:2.0:nameid-format:entity";
   private static final String ISSUER = "http://sts.example.com/service";
   private static final String AUTH_CONTEXT_PASS = "urn:oasis:names:tc:SAML:2.0:ac:classes:PasswordProtectedTransport";
   private static final char[] DEF_KEYSTORE_PRIV_KEY_PASS = "vmware"
      .toCharArray();

   private final ObjectFactory _assertionFactory = new ObjectFactory();
   private final SamlToken _spec;
   private final String samlToken;

   /**
    * Usage example
    *
    * @param args
    * @throws IOException
    * @throws ParserConfigurationException
    * @throws SAXException
    */
   public static void main(String args[]) throws Exception {
      // String samlXml = TestUtil.loadFileContent(SamlTokenCreator.class
      // .getClass().getResource(SAML_TOKEN_DIR + "saml_token_valid.xml")
      // .getFile());
      //
      // KeyStoreData keystore = TestUtil.loadDefaultKeystore();
      // SamlToken token = DefaultTokenFactory.createToken(samlXml, keystore
      // .getCertificate());
      //
      // System.out.println(new SamlTokenCreator(token,
      // keystore.getCertificate(),
      // keystore.getPrivateKey(DEF_KEYSTORE_PRIV_KEY_PASS)).getSamlToken());
      resignToken(SamlTokenCreator.class.getClass()
         .getResource(SAML_TOKEN_DIR + "saml_token_valid.xml").getFile());
      resignToken(SamlTokenCreator.class.getClass()
         .getResource(SAML_TOKEN_DIR + "saml_token_valid_groups.xml").getFile());
      resignToken(SamlTokenCreator.class.getClass()
         .getResource(SAML_TOKEN_DIR + "saml_token_valid_whitespace.xml")
         .getFile());
   }

   /**
    * This method loads a file containing SAML token, parses the token and
    * applies a new signature over the token. This is needed when something in
    * the token needs to be changed (this will break the signature).
    *
    * @param fileName
    *           It is expected to be in the default directory where SAML token
    *           files for tests are kept.
    * @throws IOException
    * @throws ParserConfigurationException
    * @throws SAXException
    * @throws SsoKeyStoreOperationException
    * @throws ParserException
    */
   public static void resignToken(String fileName) throws IOException,
      SAXException, ParserConfigurationException,
      SsoKeyStoreOperationException, ParserException {
      FileInputStream fileStream = new FileInputStream(fileName);
      String samlXml;
      try {
         samlXml = TestTokenUtil.loadStreamContent(fileStream);
      } finally {
         fileStream.close();
      }

      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      dbf.setNamespaceAware(true);
      ByteArrayInputStream tokenStream = new ByteArrayInputStream(
         samlXml.getBytes());
      Document parsedToken = dbf.newDocumentBuilder().parse(tokenStream);

      NodeList signatureList = parsedToken.getElementsByTagNameNS(
         "http://www.w3.org/2000/09/xmldsig#", "Signature");
      if (signatureList == null || signatureList.getLength() != 1) {
         // nothing to sign
         return;
      }
      parsedToken.getDocumentElement().removeChild(signatureList.item(0));

      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();

      String assertionId = parsedToken.getDocumentElement().getAttribute(
         ID_ATTR);
      signMessage(parsedToken.getDocumentElement(), "#" + assertionId,
         keystore.getPrivateKey(DEF_KEYSTORE_PRIV_KEY_PASS),
         keystore.getCertificate());

      PrintWriter pw = new PrintWriter(new File(fileName));
      try {
         pw.print(Util.serializeToString(parsedToken));
         pw.flush();
      } finally {
         pw.close();
      }
   }

   /**
    * Create new SAML token. Needed data: token specification, certificate and
    * private key to sign the new token.
    *
    * @param spec
    * @param stsCert
    *           the STS certificate used to sign the token
    * @throws ParserException
    */
   public SamlTokenCreator(SamlToken spec, Certificate stsCert, Key key)
      throws ParserException {
      _spec = spec;

      Document assertion = createAssertion();
      signMessage(assertion.getDocumentElement(), "#" + ASSERTION_ID, key,
         stsCert);
      samlToken = Util.serializeToString(assertion);
   }

   /**
    * Get the SAML token created with the input data
    *
    * @return
    */
   public String getSamlToken() {
      return samlToken;
   }

   /**
    * Create assertion according to the spec
    *
    * @return
    * @throws ParserException
    */
   private Document createAssertion() throws ParserException {
      AssertionType assertion = _assertionFactory.createAssertionType();
      assertion.setID(ASSERTION_ID);
      assertion.setVersion(SAML_VERSION);
      addAttributeStatements(assertion);
      assertion.setIssuer(createIssuer());
      try {
         addAuthStatement(assertion);
         assertion.setConditions(createConditions());
         assertion.setIssueInstant(getIssueInstant());
         assertion.setSubject(createSubject());
      } catch (DatatypeConfigurationException e) {
         throw new RuntimeException("Error creating assertion", e);
      }

      return marshallAssertion(assertion);
   }

   /**
    * Create token issuer
    *
    * @return
    */
   private NameIDType createIssuer() {
      NameIDType issuer = _assertionFactory.createNameIDType();
      issuer.setFormat(ISSUER_FORMAT);
      issuer.setValue(ISSUER);

      return issuer;
   }

   /**
    * Create conditions according to the spec
    *
    * @return
    * @throws DatatypeConfigurationException
    */
   private ConditionsType createConditions()
      throws DatatypeConfigurationException {
      ConditionsType conditions = _assertionFactory.createConditionsType();

      GregorianCalendar gc = new GregorianCalendar();
      gc.setTime(_spec.getStartTime());
      XMLGregorianCalendar xmlDate = DatatypeFactory.newInstance()
         .newXMLGregorianCalendar(gc);
      conditions.setNotBefore(xmlDate.normalize());

      gc.setTime(_spec.getExpirationTime());
      xmlDate = DatatypeFactory.newInstance().newXMLGregorianCalendar(gc);
      conditions.setNotOnOrAfter(xmlDate.normalize());

      return conditions;
   }

   /**
    * Create subject according to the spec
    *
    * @return
    * @throws DatatypeConfigurationException
    */
   private SubjectType createSubject() throws DatatypeConfigurationException {
      SubjectType subject = _assertionFactory.createSubjectType();

      NameIDType nameId = _assertionFactory.createNameIDType();
      nameId.setFormat(SUBJECT_NAME_ID_FORMAT);
      nameId.setValue(_spec.getSubject().toString()); // TODO use formatter
      subject.setNameID(nameId);

      SubjectConfirmationType sc = _assertionFactory
         .createSubjectConfirmationType();
      SubjectConfirmationDataType scd = _assertionFactory
         .createSubjectConfirmationDataType();
      if (_spec.getConfirmationType() == ConfirmationType.BEARER) {
         sc.setMethod(BEARER_CONFIRMATION);

         GregorianCalendar gc = new GregorianCalendar();
         gc.setTime(_spec.getExpirationTime());
         gc.roll(Calendar.MINUTE, false);
         XMLGregorianCalendar scDate = DatatypeFactory.newInstance()
            .newXMLGregorianCalendar(gc);

         scd.setNotOnOrAfter(scDate.normalize());
         sc.setSubjectConfirmationData(scd);
      }
      subject.setSubjectConfirmation(sc);

      return subject;
   }

   /**
    * Create issue instant time by subtracting time from startTime
    *
    * @return
    * @throws DatatypeConfigurationException
    */
   private XMLGregorianCalendar getIssueInstant()
      throws DatatypeConfigurationException {
      GregorianCalendar gc = new GregorianCalendar();
      gc.setTime(_spec.getStartTime());
      gc.roll(Calendar.MINUTE, false);
      XMLGregorianCalendar xmlDate = DatatypeFactory.newInstance()
         .newXMLGregorianCalendar(gc);

      return xmlDate.normalize();
   }

   /**
    * Add AuthStatement
    *
    * @param assertion
    * @throws DatatypeConfigurationException
    */
   private void addAuthStatement(AssertionType assertion)
      throws DatatypeConfigurationException {
      AuthnStatementType authStmt = _assertionFactory
         .createAuthnStatementType();
      AuthnContextType authnContextType = _assertionFactory
         .createAuthnContextType();
      authnContextType.setAuthnContextClassRef(AUTH_CONTEXT_PASS);
      authStmt.setAuthnContext(authnContextType);
      authStmt.setAuthnInstant(getIssueInstant());
      assertion.getAuthnStatementOrAttributeStatement().add(authStmt);
   }

   /**
    * Add attribute statements
    *
    * @param assertion
    */
   private void addAttributeStatements(AssertionType assertion) {

      // Currently only groups are added
      AttributeStatementType attrStmt = _assertionFactory
         .createAttributeStatementType();
      AttributeType attr = _assertionFactory.createAttributeType();
      attr
         .setName(AttributeNames.HTTP_RSA_COM_SCHEMAS_ATTR_NAMES_2009_01_GROUP_IDENTITY
            .toString());
      attr.getAttributeValue().addAll(serializeGroups(_spec.getGroupList()));
      attr.setFriendlyName("Group");
      attr.setNameFormat(ATTRIBUTE_FORMAT_URI);
      attrStmt.getAttribute().add(attr);
      assertion.getAuthnStatementOrAttributeStatement().add(attrStmt);
   }

   /**
    * String representation of {@link PrincipalId}. Used to create SAML token
    * object from STS acquire token response.
    *
    * @param groupList
    *           groups to transform
    * @return list of transformed groups
    */
   private List<String> serializeGroups(List<PrincipalId> groupList) {

      assert groupList != null;

      List<String> groupResult = new ArrayList<String>(groupList.size());

      for (PrincipalId group : groupList) {

         ValidateUtil.validateNotNull(group, "group");

         groupResult.add(group.getName() + '/' + group.getDomain());

      }

      return groupResult;
   }

   /**
    * Marshal JAXB Assertion to XML Document
    *
    * @param assertion
    * @return
    * @throws ParserException
    */
   private Document marshallAssertion(AssertionType assertion)
      throws ParserException {

      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      dbf.setNamespaceAware(true);
      Document result = null;
      try {
         JAXBContext jaxbContext = JAXBContext
            .newInstance(ASSERTION_JAXB_PACKAGE);
         result = dbf.newDocumentBuilder().newDocument();
         jaxbContext.createMarshaller().marshal(assertion, result);
      } catch (JAXBException jaxbException) {
         throw new ParserException(
            "Unable to marshall the SAML assertion element", jaxbException);
      } catch (ParserConfigurationException pce) {
         throw new ParserException(
            "Unable to generate document container for the assertion element",
            pce);
      }

      return result;
   }

   /**
    * Sign a XML Document
    *
    * @param nodeToSign
    * @param referenceURI
    * @param privateKey
    * @param cert
    */
   private static void signMessage(Node nodeToSign, String referenceURI,
      Key privateKey, Certificate cert) {
      XMLSignatureFactory xmlsig = XMLSignatureFactory.getInstance();

      try {
         List<Transform> transformList = new ArrayList<Transform>();
         transformList.add(xmlsig.newCanonicalizationMethod(
            Transform.ENVELOPED, (C14NMethodParameterSpec) null));
         transformList.add(xmlsig.newCanonicalizationMethod(
            CanonicalizationMethod.EXCLUSIVE,
            new ExcC14NParameterSpec(Arrays.asList("xs", "xsi"))));
         Reference ref = xmlsig.newReference(referenceURI,
            xmlsig.newDigestMethod(DigestMethod.SHA256, null), transformList,
            null, null);

         SignedInfo signedInfo = xmlsig.newSignedInfo(xmlsig
            .newCanonicalizationMethod(CanonicalizationMethod.EXCLUSIVE,
               (C14NMethodParameterSpec) null), xmlsig.newSignatureMethod(
            "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256", null),
            Collections.singletonList(ref));

         Node insertBefore = nodeToSign
            .getOwnerDocument()
            .getElementsByTagNameNS("urn:oasis:names:tc:SAML:2.0:assertion",
               "Subject").item(0);

         DOMSignContext dsc = new DOMSignContext(privateKey, nodeToSign,
            insertBefore);
         dsc.putNamespacePrefix(XMLSignature.XMLNS, "ds");
         KeyInfoFactory kif = KeyInfoFactory.getInstance();
         XMLSignature signature = xmlsig.newXMLSignature(signedInfo, kif
            .newKeyInfo(Collections.singletonList(kif.newX509Data(Collections
               .singletonList(cert)))));
         signature.sign(dsc);
      } catch (NoSuchAlgorithmException e) {
         throw new RuntimeException(
            "XML digital signature configuration error", e);
      } catch (InvalidAlgorithmParameterException e) {
         throw new RuntimeException(
            "XML digital signature configuration error", e);
      } catch (MarshalException e) {
         throw new RuntimeException("Error while signing document", e);
      } catch (XMLSignatureException e) {
         throw new RuntimeException("Error while signing document", e);
      }
   }
}
