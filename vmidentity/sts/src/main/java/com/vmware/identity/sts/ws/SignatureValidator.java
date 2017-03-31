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

package com.vmware.identity.sts.ws;

import java.io.ByteArrayInputStream;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.List;

import javax.xml.bind.JAXBElement;
import javax.xml.crypto.MarshalException;
import javax.xml.crypto.dsig.CanonicalizationMethod;
import javax.xml.crypto.dsig.Reference;
import javax.xml.crypto.dsig.Transform;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.dom.DOMValidateContext;

import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.KeyInfoConfirmationDataType;
import oasis.names.tc.saml._2_0.assertion.SubjectConfirmationDataType;
import oasis.names.tc.saml._2_0.assertion.SubjectType;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.Validate;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.BinarySecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.KeyIdentifierType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ReferenceType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityTokenReferenceType;
import org.w3._2000._09.xmldsig_.KeyInfoType;
import org.w3._2000._09.xmldsig_.SignatureType;
import org.w3._2000._09.xmldsig_.X509DataType;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.Request.CertificateLocation;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.util.JAXBExtractor;
import com.vmware.identity.sts.ws.SOAPFaultHandler.FaultKey;

/**
 * This class is responsible for validating the WS-Trust request signatures.
 * They're used in case of HoK token request and for solution user
 * authentication. This class is thread safe.
 */
public final class SignatureValidator {

   private static final String B64_ENCODING_TYPE = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary";
   private static final String SIGNATURE_ELEMENT_NAME = "Signature";
   private static final String SAML_KEY_ID_TYPE = "http://docs.oasis-open.org/wss/oasis-wss-saml-token-profile-1.1#SAMLID";
   private static final String HOLDER_OF_KEY_CONFIRMATION = "urn:oasis:names:tc:SAML:2.0:cm:holder-of-key";
   private static final String BODY_ELEMENT_NAME = "Body";
   private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
      .getLogger(SignatureValidator.class);

   /**
    * Validates the signature contained in the SOAPHeader. Parameters
    * securityHeader and wsseSecurityElement should be different representation
    * of the same element (wsse:Security)
    *
    * @param securityHeader
    *           not null
    * @param wsseSecurityElement
    *           not null
    * @return {@link Signature} if it is valid
    * @throws WSFaultException
    *            if something goes wrong with the signature validation of
    *            certificate extraction
    */
   public Signature validate(Node securityHeader,
      SecurityHeaderType wsseSecurityElement) throws WSFaultException {
      Validate.notNull(securityHeader);
      Validate.notNull(wsseSecurityElement);

      Signature signature = extractSignature(wsseSecurityElement);
      logger.info("Got signing certificate");
      validateSignature(getSignatureNode(securityHeader), signature, getTimestampNode(securityHeader));
      return signature;
   }

   /**
    * Validates the request signature. If the signature is not valid the
    * relevant {@link WSFaultException} is thrown
    *
    * @param signatureNode
    *           not null
    * @param signature
    *           not null
    */
   private void validateSignature(Node signatureNode, Signature signature, Node timestampNode ) {
      assert signatureNode != null;
      assert signature != null;
      assert timestampNode != null;

      XMLSignatureFactory fac = XMLSignatureFactory.getInstance();
      DOMValidateContext valContext = new DOMValidateContext(signature
         .getCertificate().getPublicKey(), signatureNode);
      try {
         XMLSignature xmlSignature = fac.unmarshalXMLSignature(valContext);
         if( !xmlSignature.validate(valContext)){
            throw new WSFaultException(FaultKey.WSSE_FAILED_CHECK,
                      "Signature is invalid.");
         }

         validateCanonicalizationMethod( xmlSignature );

         validateSignatureReferences( xmlSignature, valContext, signatureNode.getOwnerDocument(), timestampNode );

      } catch (MarshalException e) {
         throw new WSFaultException(FaultKey.WSSE_FAILED_CHECK, e);
      } catch (XMLSignatureException e) {
         throw new WSFaultException(FaultKey.WSSE_FAILED_CHECK, e);
      }
   }

   /**
    * Validate the canonicalization method of the signature.
    * @param xmlSignature the XMLSignature to validate the canonicalization method of.
    * @throws XMLSignatureException when validation fails.
    */
   private void validateCanonicalizationMethod(XMLSignature xmlSignature) throws XMLSignatureException {
      assert xmlSignature!= null;

      // Exclusive canonicalization without comments (xml-exc-c14n) must be used prior to signature generation.
      if (!CanonicalizationMethod.EXCLUSIVE.equals(
          xmlSignature.getSignedInfo().getCanonicalizationMethod().getAlgorithm() ) ){
          throw new XMLSignatureException(
              String.format(
                 "Canonicalization algorithm '%s' is not supported.",
                 xmlSignature.getSignedInfo().getCanonicalizationMethod().getAlgorithm() )
          );
      }
   }

   /**
    * Validate references present in the XmlSignature.
    * @param xmlSignature the xml signature whose references are to be validated. not null.
    * @param valContext validation context used to validate the signature itself. not null.
    * @param document document the signature belongs to. not null.
    * @param timestampNode the timestamp node of the soap security header within the document.
    * @throws XMLSignatureException when the validation fails.
    */
   private void validateSignatureReferences(
      XMLSignature xmlSignature, DOMValidateContext valContext,
      Document document, Node timestampNode ) throws XMLSignatureException {

      assert xmlSignature!= null;
      assert valContext!= null;
      assert document!= null;
      assert timestampNode!= null;

      //    If a signature is applied to a request then it must include:
      //    Either the <S11:Body>, or the WS-Trust element as a direct child of the <S11:Body>
      //    The <wsu:Timestamp>, if present in the <S11:Header>. 
      //        (in fact this must be present as per same spec, and SOAPHeaderExtractor validates it)

      Node soapBody = getSoapBody(document);
      Node wsTrustNode = getWsTrustNode( soapBody );
      boolean foundTimestampElement = false;
      boolean foundBodyOrWSTrustElement = false;

      List<Reference> references = xmlSignature.getSignedInfo().getReferences();
      if ( ( references == null ) || (references.size() == 0) ) {
          throw new XMLSignatureException(
                    "Signature's SignInfo does not contain any references."
          );
      }

      for(Reference reference : references) {

          if(reference != null){
              validateReferenceTransforms( reference );
              validateReferenceUri(reference);
              // note: order is important, we should not try to validate digests
              // before we checked expected transforms, and uri etc.
              if (!reference.validate(valContext)){
                  throw new XMLSignatureException(
                      String.format("Signature reference '%s' is invalid.", reference.getURI())
                  );
              }

              if( !foundTimestampElement || !foundBodyOrWSTrustElement ){
                  String id = org.jcp.xml.dsig.internal.dom.Utils.parseIdFromSameDocumentURI(reference.getURI());
                  Node referencedNode = document.getElementById(id);
                  foundTimestampElement = (foundTimestampElement) || ( timestampNode.isSameNode(referencedNode) );
                  foundBodyOrWSTrustElement = ( foundBodyOrWSTrustElement ) ||
                                              ( soapBody.isSameNode(referencedNode) ) ||
                                              ( wsTrustNode.isSameNode(referencedNode) );
              }
          }
      } // for each reference

      if( !foundTimestampElement || !foundBodyOrWSTrustElement ){
          throw new XMLSignatureException(
              "Signature must include <wsu:Timestamp> and either SoapBody, or the WSTrust element within it."
          );
      }
   }

   /**
    * Validate the signature reference transforms are as expected.
    * (Only the exclusive canonicalization transform is supported).
    *
    * @param reference signature reference to validate the transforms of.
    * @throws XMLSignatureException when validation fails.
    */
   private void validateReferenceTransforms(Reference reference) throws XMLSignatureException{
      assert reference != null;

      List<Transform> transforms = reference.getTransforms();
      if ( (transforms != null) && (transforms.size() > 1) ) {
          throw new XMLSignatureException(
                    "Unexpected number of transforms. Only an exclusive canonicalization is supported."
          );
       } else if ((transforms != null) &&
                  (transforms.size() > 0) &&
                  ( !CanonicalizationMethod.EXCLUSIVE.equals(transforms.get(0).getAlgorithm()) ) ) {
           throw new XMLSignatureException(
               String.format("Unexpected Transform '%s'. Only an exclusive canonicalization is supported.", transforms.get(0).getAlgorithm() )
           );
       }
   }

   /**
    * Validate the Signature Reference URI is the same document Uri.
    * It should not point to external resources.
    * @param reference Signature reference to validate the uri of. not null.
    * @throws XMLSignatureException when the validation fails.
    */
   private void validateReferenceUri(Reference reference) throws XMLSignatureException {
      assert reference != null;

      if (!org.jcp.xml.dsig.internal.dom.Utils.sameDocumentURI(reference.getURI()) ){
          throw new XMLSignatureException(
              String.format("Invalid reference '%s'. Only a same-document references are aupported.", reference.getURI() ) 
          );
      }
   }

   /**
    * Retrieves the SoapBody element from the document.
    * @param document not null
    * @return Node of the Soap Body element.
    * @throws XMLSignatureException when unable to locate the Soap Body element.
    */
   private Node getSoapBody( Document document ) throws XMLSignatureException {
      assert document!= null;

      NodeList nodes = null;
      Node soapBody = null;

      nodes = document.getElementsByTagNameNS(
              javax.xml.soap.SOAPConstants.URI_NS_SOAP_ENVELOPE, BODY_ELEMENT_NAME);

      if ( ( nodes == null ) || (nodes.getLength() == 0) ) {
          throw new XMLSignatureException(
                  "Unexpected soap format - unable to find soap body."
          );
      } else if (nodes.getLength() > 1 ) {
          throw new XMLSignatureException(
                  "Unexpected soap format - found more than 1 soap body elements."
          );
      } else {
          soapBody = nodes.item(0);
      }

      if(soapBody == null) {
          throw new XMLSignatureException(
                    "Unexpected soap format - unable to resolve soap body."
          );
      }

      return soapBody;
   }

   /**
    * retrieves the WSTrust element from the soap body.
    * @param soapBody not null
    * @return Node of the WSTrust element.
    * @throws XMLSignatureException when unable to locate the WSTrust element.
    */
   private Node getWsTrustNode(Node soapBody) throws XMLSignatureException {
      assert soapBody!= null;

      Node wsTrustNode = null;

      //   - All <wst:RequestSecurityToken>, <wst:RequestSecurityTokenResponse>,
      //     and <wst:RequestSecurityTokenResponseCollection> elements must be carried
      //     as the single direct child of the body of a SOAP 1.1 <S11:Envelope> element.
      wsTrustNode = soapBody.getFirstChild();
      if( wsTrustNode == null ) {
          throw new XMLSignatureException(
              "Unexpected Soap structure. Body element is empty."
          );
      } else if ( wsTrustNode.getNodeType() != Node.ELEMENT_NODE ) {
          throw new XMLSignatureException(
              String.format(
                  "Unexpected Soap structure. Body element has a child of type '%s'. Expect WSTrust element.",
                  wsTrustNode.getNodeType())
          );
      }

      return wsTrustNode;
   }

   /**
    * @param header
    *           not null
    * @return the XML signature node (ds:Signature)
    */
   private Node getSignatureNode(Node securityHeader) {
      return this.getChildNode(securityHeader, SIGNATURE_ELEMENT_NAME);
   }

   /**
    * @param parentNode
    *           not null
    * @param elementName name of the element to find
    * @return the XML node
    */
   private Node getChildNode(Node parentNode, String elementName) {
      assert parentNode != null;
      NodeList childNodes = parentNode.getChildNodes();
      for (int i = 0; i < childNodes.getLength(); i++) {
         Node nextNode = childNodes.item(i);

         final String nextNodeName = nextNode.getLocalName();
         if (nextNodeName != null
            && nextNodeName.equals(elementName)) {
            return nextNode;
         }
      }

      throw new WSFaultException(FaultKey.WSSE_INVALID_SECURITY,
         String.format("XML node '%s' cannot be found", elementName ) );
   }
   /**
    * @param signatureNode
    *           not null
    * @return the XML signature time stamp node (Timestamp)
    */
   private Node getTimestampNode(Node signatureNode) {
      return this.getChildNode(signatureNode, WsConstants.WSU_TIMESTAMP_ELEMENT_NAME);
   }

   /**
    * Parses the KeyInfo from the XML signature and extracts the signing
    * certificate. KeyInfo should have the following format:
    *
    * <KeyInfo> <SecurityTokenReference> <Reference URI=""/> OR <KeyIdentifier
    * /> </SecurityTokenReference> </KeyInfo>
    *
    * @param header
    *           not null
    * @return the signing certificate
    */
   private Signature extractSignature(SecurityHeaderType header) {
      assert header != null;

      Signature result = null;
      KeyInfoType keyInfo = ((SignatureType) JAXBExtractor.extractFromSecurityHeader(header, SignatureType.class)).getKeyInfo();
      if (keyInfo == null) {
         throwInvalidSecurity("KeyInfo not found");
      }
      logger.debug("Found KeyInfo");
      SecurityTokenReferenceType stRef = getTheValue(keyInfo.getContent(),
         SecurityTokenReferenceType.class, FaultKey.WSSE_INVALID_SECURITY,
         "SecurityTokenReference not found");
      logger.debug("Found SecurityTokenReference");
      List<Object> stRefContent = stRef.getAny();
      if (stRefContent == null || stRefContent.size() != 1) {
         throwInvalidSecurity("SecurityTokenReference is empty");
      }
      if (!(stRefContent.get(0) instanceof JAXBElement<?>)) {
         throwInvalidSecurity("Unknown SecurityTokenReference content type");
      }
      JAXBElement<?> jaxbElement = (JAXBElement<?>) stRefContent.get(0);
      if (ReferenceType.class.equals(jaxbElement.getDeclaredType())) {
         ReferenceType reference = (ReferenceType) jaxbElement.getValue();
         result = parseReferenceType(header, reference);
      } else if (KeyIdentifierType.class.equals(jaxbElement.getDeclaredType())) {
         KeyIdentifierType keyIdentifier = (KeyIdentifierType) jaxbElement
            .getValue();
         result = parseKeyIdentifier(header, keyIdentifier);
      } else {
         throw new WSFaultException(FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN,
            "Unknown SecurityTokenReference content");
      }

      return result;
   }

   /**
    * @param header
    *           not null
    * @param keyIdentifier
    *           not null
    * @return the referenced certificate
    */
   private Signature parseKeyIdentifier(SecurityHeaderType header,
      KeyIdentifierType keyIdentifier) {
      logger.debug("Found KeyIdentifier element");
      String assertionId = keyIdentifier.getValue();
      String valueType = keyIdentifier.getValueType();
      if (assertionId == null || valueType == null
         || !valueType.equalsIgnoreCase(SAML_KEY_ID_TYPE)) {
         throw new WSFaultException(FaultKey.WSSE_INVALID_SECURITY,
            "assertionId = " + assertionId + " valueType = " + valueType);
      }
      AssertionType assertion = JAXBExtractor.extractFromSecurityHeader(header, AssertionType.class);
      if (assertion == null) {
         throwTokenUnavailable("Assertion missing");
      }
      if (!assertion.getID().equalsIgnoreCase(assertionId)) {
         throwTokenUnavailable("Assertion ID mismatch");
      }
      if (!assertion.getSubject().getSubjectConfirmation().getMethod()
         .equalsIgnoreCase(HOLDER_OF_KEY_CONFIRMATION)) {
         throwTokenUnavailable("Invalid subject confirmation method "
            + assertion.getSubject().getSubjectConfirmation().getMethod());
      }
      X509Certificate resultCert = parseHolderOfKeyConfirmation(assertion
         .getSubject());
      return new Signature(resultCert, CertificateLocation.ASSERTION);
   }

   /**
    * Parses holder of key confirmation data
    *
    * @param subject
    */
   private X509Certificate parseHolderOfKeyConfirmation(SubjectType subject) {

      SubjectConfirmationDataType subjectConfirmationData = subject
         .getSubjectConfirmation().getSubjectConfirmationData();

      if (!(subjectConfirmationData instanceof KeyInfoConfirmationDataType)) {
         throw new WSFaultException(FaultKey.WSSE_UNSUPPORTED_SECURITY_TOKEN,
            "Unknow SubjectConfirmation type");
      }

      // Start searching for
      // saml:SubjectConfirmationData/ds:KeyInfo/ds:X509Data/ds:X509Certificate
      // all casts are needed because the schema cannot be modified to
      // reflect the restrictions in a "JAXB friendly" way
      KeyInfoType keyInfo = getTheValue(subjectConfirmationData.getContent(),
         KeyInfoType.class, FaultKey.WSSE_INVALID_SECURITY_TOKEN,
         "Assetion KeyInfo not found/valid");

      X509DataType x509Data = getTheValue(keyInfo.getContent(),
         X509DataType.class, FaultKey.WSSE_INVALID_SECURITY_TOKEN,
         "X509 data not found");

      byte[] cert = getTheValue(
         x509Data.getX509IssuerSerialOrX509SKIOrX509SubjectName(),
         byte[].class, FaultKey.WSSE_INVALID_SECURITY_TOKEN,
         "Confirmation certificate not found");

      return decodeCertificate(cert);
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
   private <T> T getTheValue(List<?> list, Class<T> valueType, FaultKey key,
      String cause) {

      if (list == null) {
         throw new WSFaultException(key, cause);
      }

      JAXBElement<?> theOnlyElement = getSingleJaxbElement(list);
      if (theOnlyElement == null
         || !valueType.isInstance(theOnlyElement.getValue())) {
         throw new WSFaultException(key, cause);
      }
      return (T) theOnlyElement.getValue();
   }

   private JAXBElement<?> getSingleJaxbElement(List<?> list) {
      assert list != null;
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
    * @param header
    *           not null
    * @param reference
    *           not null
    * @return the referenced certificate
    */
   private Signature parseReferenceType(SecurityHeaderType header,
      ReferenceType reference) {
      assert header != null;
      assert reference != null;

      logger.debug("Found reference with URI: {}", reference.getURI());

      // Reference element should point to BST
      BinarySecurityTokenType bst = JAXBExtractor.extractFromSecurityHeader(header, BinarySecurityTokenType.class);
      validateBSTAttributes(bst, reference);
      X509Certificate resultCert = decodeCertificate(Base64.decodeBase64(bst
         .getValue()));
      return new Signature(resultCert, CertificateLocation.BST);
   }

   /**
    * Decode a byte array to an Certificate object.
    */
   private X509Certificate decodeCertificate(byte[] certificateDer) {
      assert certificateDer != null;

      ByteArrayInputStream stream = new ByteArrayInputStream(certificateDer);
      CertificateFactory certFactory;
      try {
         certFactory = CertificateFactory.getInstance("X.509");
      } catch (CertificateException e) {
         throw new IllegalStateException("Internal error: X.509 Certificate "
            + "Factory is not available (uncompliant JRE?)", e);
      }

      X509Certificate certificate = null;
      try {
         certificate = (X509Certificate) certFactory
            .generateCertificate(stream);
      } catch (CertificateException e) {
         throw new WSFaultException(FaultKey.WSSE_INVALID_SECURITY_TOKEN, e);
      }

      return certificate;
   }

   /**
    * Validates the BST attributes and throws a SOAP fault if something is wrong
    *
    * @param bst
    *           can be null
    * @param reference
    *           cannot be null
    */
   private void validateBSTAttributes(BinarySecurityTokenType bst,
      ReferenceType reference) {
      assert reference != null;
      if (bst == null) {
         throwTokenUnavailable("BST missing");
      }
      // Reference should use internal direct reference (#elementId)
      if (!bst.getId().equalsIgnoreCase(reference.getURI().substring(1))) {
         throwTokenUnavailable("BST ID mismatch");
      }
      if (!reference.getValueType().equalsIgnoreCase(bst.getValueType())) {
         throwTokenUnavailable("BST value type mismatch");
      }
      if (!bst.getEncodingType().equalsIgnoreCase(B64_ENCODING_TYPE)) {
         throwTokenUnavailable("Unknown BST encoding");
      }
   }

   /**
    * Helper - throws an {@link WSFaultException} with IvalidSecurity fault with
    * specified cause message
    *
    * @param cause
    *           cannot be null
    */
   private void throwInvalidSecurity(String cause) {
      assert cause != null;
      throw new WSFaultException(
         SOAPFaultHandler.FaultKey.WSSE_INVALID_SECURITY, cause);
   }

   /**
    * Helper - throws an {@link WSFaultException} with SecurityTokenUnavailable
    * fault with specified cause message
    *
    * @param cause
    *           cannot be null
    */
   private void throwTokenUnavailable(String cause) {
      assert cause != null;
      throw new WSFaultException(FaultKey.WSSE_SECURITY_TOKEN_UNAVAILABLE,
         cause);
   }
}

