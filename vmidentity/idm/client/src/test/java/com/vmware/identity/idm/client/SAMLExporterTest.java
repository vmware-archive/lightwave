/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.vmware.identity.idm.client;

import java.io.IOException;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import junit.framework.Assert;

import org.junit.BeforeClass;
import org.junit.Test;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class SAMLExporterTest {

   private static Properties props = null;
   private static CasIdmClient client = null;
   private static SAMLExporter exporter = null;

   private static final String TENANT_NAME = "vsphere.local";

   @BeforeClass
   public static void init() throws IOException
   {
      props = IdmClientTestUtil.getProps();
      String hostname = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_HOSTNAME);
      Assert.assertNotNull(hostname);
      client = new CasIdmClient(hostname);
      exporter = new SAMLExporter(client);
   }

   @Test
   public void convertToIPV6ShortFormTest() throws MalformedURLException
   {
       // verify the compression output basing on RFC 5952 standard

       //keep hostname format
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://sc-rdops-vm08-dhcp-250-178.eng.vmware.com:7444/websso/SAML2/Metadata/vsphere.local"), "https://sc-rdops-vm08-dhcp-250-178.eng.vmware.com:7444/websso/SAML2/Metadata/vsphere.local");
       //IPV4 address
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://123.0.0.0:7444/websso"), "https://123.0.0.0:7444/websso");
       //normal compression case
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[2001:db8:0:0:0:0:2:1]/websso"), "https://[2001:db8::2:1]/websso");
       //only compress sequence with more than one '0'
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[2001:db8:0:1:1:1:1:1]"), "https://[2001:db8:0:1:1:1:1:1]");
       //pick longest sequence
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[2001:0:0:1:0:0:0:1]/websso"), "https://[2001:0:0:1::1]/websso");
       //pick the first if multiple sequence with same length
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[2001:db8:0:0:1:0:0:1]/websso"), "https://[2001:db8::1:0:0:1]/websso");
       //trailing sequence handling
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[2001:db8:0:0:1:0:0:0]/websso"), "https://[2001:db8:0:0:1::]/websso");
       //leading zero's
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[2001:0db8:0:0000:00:000:0002:0000]/websso"), "https://[2001:db8::2:0]/websso");
       //leading sequence
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[0:0:0:0:0:0:0:1]/websso"), "https://[::1]/websso");
       //pick the longest in case of there is leading zeros
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[0:0:1:0:0:0:0:1]/websso"), "https://[0:0:1::1]/websso");
       //pick the longest in case of both leading and trailing zeros
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[0:0:1:0:0:0:0:0]/websso"), "https://[0:0:1::]/websso");
       Assert.assertEquals(SAMLExporter.convertToIPV6ShortForm("https://[fc00:10:140:38:0:0:0:41]/websso"), "https://[fc00:10:140:38::41]/websso");

   }
   @Test
   public void testEntitiesDescriptorAttributes()
   {
      Document doc = getDocument(TENANT_NAME, false);
      NodeList nodes = doc.getElementsByTagName(SAMLNames.ENTITIESDESCRIPTOR);

      for (int i = 0; i < nodes.getLength(); i++) {
         Node node = nodes.item(i);
         Assert.assertNotNull(SAMLNames.ENTITIESDESCRIPTOR + " is null", node);
         Assert.assertEquals("Namespaces do not match!",
               SAMLNames.NS_NAME_SAML_METADATA, node.getNamespaceURI());

         NamedNodeMap attributes = node.getAttributes();
         Assert.assertNotNull("Attributes are null", attributes);

         Map<String, String> expectedAttributes = getEntitiesAttributes(TENANT_NAME);

         for (String attrName : expectedAttributes.keySet()) {
            String expectedValue = expectedAttributes.get(attrName);

            Node attribute = attributes.getNamedItem(attrName);

            if (expectedValue == null) {
               Assert.assertNotNull(
                     String.format("Attribute '%s' does not exist for this node!", attrName),
                     attribute);
            } else {
               Assert.assertEquals(
                     String.format("Attribute '%s' does not match!", attrName),
                     expectedValue, attribute.getNodeValue());
            }

         }
      }
   }

   /**
    * Create a map of attributes for the {@code EntitiesDescriptor}
    * tag and their expected values.
    *
    * @param name
    *          name of the entity (usually tenant name).
    * @return map of expected attributes for {@code EntitiesDescriptor}
    *         and their expected values.
    */
   private static Map<String, String> getEntitiesAttributes(String name)
   {
      Map<String, String> map = new HashMap<String, String>();
      map.put(SAMLNames.NS_NAME_SAML_SAML,      SAMLNames.NS_VAL_SAML_SAML);
      map.put(SAMLNames.NS_NAME_SAML_VMWARE_ES, SAMLNames.NS_VAL_SAML_VMWARE_ES);
      map.put(SAMLNames.NAME,                   name);
      map.put(SAMLNames.VALIDUNTIL,             null); // Ensure existence, but don't match
      return map;
   }

   /**
    * Fetch the configuration for a given tenant.
    *
    * @param exportPrivateData
    *          whether to export the complete configuration or not.
    * @return Document describing the configuration for {@code tenantName}.
    */
   private static Document getDocument(String tenantName, boolean exportPrivateData)
   {
      Document doc = null;
      try {
         doc = exporter.exportConfiguration(tenantName, exportPrivateData);
      } catch (Exception e) {
         Assert.fail(e.getMessage());
      }

      if (doc == null) {
         Assert.fail("Doc for '" + tenantName + "' is null");
      }

      return doc;
   }

   /**
    * Converts a node to a raw XML string.
    *
    * @param node
    *          node to convert.
    * @return String representing the XML of {@code node}.
    */
   @SuppressWarnings("unused")
   private static String convertNodeToXML(Node node)
   {
      StringWriter writer = new StringWriter();
      Transformer transformer;
      try {
         transformer = TransformerFactory.newInstance().newTransformer();
         transformer.transform(new DOMSource(node), new StreamResult(writer));
      } catch (TransformerException | TransformerFactoryConfigurationError e) {
         Assert.fail(e.getMessage());
      }

      return writer.toString();
   }

}
