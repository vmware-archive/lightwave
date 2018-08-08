/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.test.util.common;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collections;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.*;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import com.vmware.vim.sso.client.SamlToken;

import javax.xml.parsers.*;

import junit.framework.Assert;

public class SamlTokenValidator {
  private static final Logger log = LoggerFactory.getLogger(SamlTokenValidator.class);
  private Document doc = null;
  private boolean isValidSamlToken = false;
  private String samlData;

  private void init(String samlData) {
    this.samlData = samlData; isValidSamlToken = processSamlData();
  }

  public SamlTokenValidator(String samlData) {
    init(samlData);
  }


  public SamlTokenValidator(SamlToken samlToken) {
    init(samlToken.toXml());

  }

  public boolean isValidSamlToken() {
    return isValidSamlToken;
  }

  public Element getTokenXmlElement() {
    processSamlData();
    NodeList elementsWithTag = doc.getElementsByTagName("saml2:Assertion");
    Assert.assertFalse(
      elementsWithTag == null || elementsWithTag.getLength() <= 0 ||
        elementsWithTag.item(0).getNodeType() != Node.ELEMENT_NODE);

    return (Element) elementsWithTag.item(0);

  }

  public Document getTokenDOM() {
    processSamlData(); return doc;
  }

  public String getNameID() {
    return getValueAstring("saml2:NameID", null, null);
  }

  public boolean validateNameID(String expectedName) {
    String tagName = "saml2:NameID";

    return validateField(tagName, null, null, expectedName);
  }

  public boolean validateToken() {
    return false;
  }

  public boolean validateGroupMembership(ArrayList<String> groups) {
    String tagName = "saml2:Attribute"; String attrName = "FriendlyName"; String attrVal = "Groups";

    return validateMultipleFields(tagName, attrName, attrVal, groups);
  }

  public String getAudience() {
    return getValueAstring("saml2:Audience", null, null);
  }

  public boolean validateAudience(String expectedValue) {
    String tagName = "saml2:Audience"; return validateField(tagName, null, null, expectedValue);
  }

  // Validates the field against the expected value.
  boolean validateField(
    String elementName, String attrName, String attrValue, String expectedValue
  ) {
    String samlField = getValueAstring(elementName, attrName, attrValue);
    if (samlField == null || samlField.length() <= 0) {
      return false;
    }

    // Check for the mere presence of the element
    if (expectedValue == null) {
      return (samlField != null && samlField.length() > 0);
    } return samlField.equalsIgnoreCase(expectedValue);

  }

  boolean validateMultipleFields(
    String parentElementName, String attrName, String attrVal, ArrayList<String> expectedValues
  ) {
    ArrayList<String> valuesFromSaml = getValuesAsArray(parentElementName, attrName, attrVal);

    if (valuesFromSaml == null || valuesFromSaml.size() <= 0) {
      return false;
    }

    // Check for the mere presence of the elements
    if (expectedValues == null) {
      return (valuesFromSaml != null && valuesFromSaml.size() > 0);
    } if (expectedValues.size() != valuesFromSaml.size()) {
      log.error(String.format("Expected %d groups token has %d groups", expectedValues.size(), valuesFromSaml.size()));
      return false;
    } Collections.sort(valuesFromSaml); Collections.sort(expectedValues); boolean matched = true;
    for (int i = 0; i < valuesFromSaml.size(); ++i) {
      if (!valuesFromSaml.get(i).equalsIgnoreCase(expectedValues.get(i))) {
        log.error(String.format("Expected %s found %s", expectedValues.get(i), valuesFromSaml.get(i))); return false;
      }
    }

    return matched;
  }


  /**
   * getValueAstring: extract the text value of a node
   * matched by the following params.
   *
   * @param elementName: the element name in the XML
   * @param attrName:    the attribute name whose value to be checked.
   *                     Pass null to check just by the elementName
   * @param attrVal:     the value of the attribute to be checked.
   *                     Pass null to check just for the presence of attribute.
   *                     <p>
   *                     This should be called when the element itself has the value of interest.
   */
  String getValueAstring(String elementName, String attrName, String attrVal) {
    String attributeVal = null; processSamlData();

    NodeList elementsWithTag = doc.getElementsByTagName(elementName); if (elementsWithTag.getLength() <= 0) {
      log.error("No elements with name - " + elementName); return attributeVal;
    }

    // If matches multiple. Always get the first one.
    attributeVal = elementsWithTag.item(0).getTextContent();

    if (attributeVal == null || attributeVal.length() <= 0) {
      log.error("Cannot find the element '" + elementName + "' in SAMLToken");
    }

    return attributeVal;
  }


  /**
   * Given the parent node, extract the text values of the children
   *
   * @param parentElementName: the node whose children contain the data of interest
   * @param attrName:          the attribute name whose value to be checked.
   *                           Pass null to check just by the elementName
   * @param attrVal:           the value of the attribute to be checked.
   *                           Pass null to check just for the presence of attribute.
   * @return Array of the String values of the children
   * <p>
   * This should be called if interested in the text values of child nodes.
   * Call with the immediate parent.
   */
  ArrayList<String> getValuesAsArray(String parentElementName, String attrName, String attrVal) {

    ArrayList<String> fieldValues = null;

    Node parentNode = findWithNameAndAttrVal(parentElementName, attrName, attrVal); if (parentNode == null) {
      log.error("Cannot find the parent node with name - " + parentElementName); return fieldValues;
    }

    NodeList children = parentNode.getChildNodes(); if (children.getLength() <= 0) {
      log.error("Cannot find any child nodes for " + parentElementName); return fieldValues;
    }

    fieldValues = new ArrayList<String>(); for (int i = 0; i < children.getLength(); i++) {
      Node childNode = children.item(i); String value = childNode.getTextContent(); fieldValues.add(value);
    } log.info("Text values of matched node: " + fieldValues); return fieldValues;
  }


  /**
   * findWithNameAndAttrVal: Finds the element given the element name and name and value of any attribute
   *
   * @param elementName: the element name in the XML
   * @param attrName:    the attribute name whose value to be checked.
   *                     Pass null to check just by the elementName
   * @param attrVal:     the value of the attribute to be checked.
   *                     Pass null to check just for the presence of attribute.
   * @return the matched Node or null if not found.
   */
  public Node findWithNameAndAttrVal(String elementName, String attrName, String attrVal) {

    NodeList elementsWithTag = doc.getElementsByTagName(elementName); Node nodeToreturn = null;

    if (elementsWithTag.getLength() <= 0) {
      log.error("No elements with name - " + elementName); return nodeToreturn;
    }

    for (int i = 0; i < elementsWithTag.getLength(); i++) {
      Node tmp = elementsWithTag.item(i);

      // Match just by the elementName.
      if (attrName == null) {
        nodeToreturn = tmp; break;
      }

      // Has no attributes.
      if (!tmp.hasAttributes()) {
        continue;
      }

      Attr attr = (Attr) tmp.getAttributes().getNamedItem(attrName);

      // If no attrVal is specified, look for just the presence of the attribute
      if (attrVal == null || attrVal.length() <= 0 || attrVal.equals(attr.getValue())) {
        nodeToreturn = tmp; break;
      }
    } return nodeToreturn;
  }

  // Parse the saml data and extract the fields.
  public boolean processSamlData() {
    boolean isValidSamlToken = false;
    if (doc != null) {
      return isValidSamlToken;
    }

    DocumentBuilderFactory docFactory = null; DocumentBuilder docBuilder = null;

    try {
      docFactory = DocumentBuilderFactory.newInstance(); docFactory.setNamespaceAware(true);
      docBuilder = docFactory.newDocumentBuilder();
      InputSource is = new InputSource();
      is.setCharacterStream(new StringReader(samlData));
      doc = docBuilder.parse(is);
      String audience = getAudience();
      if (audience == null || audience.isEmpty()) {
        isValidSamlToken = false;
      } else {
        isValidSamlToken = true;
      }
    } catch (ParserConfigurationException | SAXException | IOException e) {
      log.error("Failed to parse the SAML data."); log.info("Exception: " + e.getMessage()); e.printStackTrace();
    }
    return isValidSamlToken;
  }
}
