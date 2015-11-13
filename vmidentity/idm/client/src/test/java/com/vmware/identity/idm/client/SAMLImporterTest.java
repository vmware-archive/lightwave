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

import static org.junit.Assert.fail;

import java.text.ParseException;

import javax.xml.datatype.DatatypeConfigurationException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.junit.Test;
import junit.framework.Assert;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
public class SAMLImporterTest {

    @Test
    public void isExpiredTest() throws ParseException, ParserConfigurationException, DatatypeConfigurationException
    {
        /**
         * Test time string according to SAML core 1.3.
         * From: UTC
         * Zone: no tome zone
         * formats: with or without millisecond
         *
         */
        final String dateStrings[] = {"2045-06-29T19:19:51.000Z",
                                    "2045-06-29T19:19:51.10",
                                    "2045-06-29T19:19:51Z"};

        DocumentBuilder docBuilder = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder();
        Document doc = docBuilder.newDocument();
        SAMLImporter importer = new SAMLImporter(new CasIdmClient(null));
        Element entEle = doc.createElementNS(null, SAMLNames.ENTDESCRIPTOR);

        for (String dateString: dateStrings) {
            entEle.setAttribute(SAMLNames.VALIDUNTIL, dateString);
            Assert.assertFalse("unexpected 'validUtil' date validation result using "+dateString, importer.isExpired(entEle));
        }

        final String dateStringsExpired[] = {"2015-06-29T19:19:51.000Z",
                "2015-05-29T19:19:51.10Z",
                "2015-06-29T19:19:51Z"};

        for (String dateString: dateStringsExpired) {
            entEle.setAttribute(SAMLNames.VALIDUNTIL, dateString);
            Assert.assertTrue( "unexpected 'validUtil' date validation result using "+dateString,importer.isExpired(entEle));
        }

        //verify the method does not support non UTC form date
        try {
            entEle.setAttribute(SAMLNames.VALIDUNTIL, "Jun 20, 4:00PM EDT");
            importer.isExpired(entEle);
            fail();
        } catch (IllegalArgumentException e) {
            //pass
        } catch (Exception x) {
            fail("unexprect exception");
        }

        //verify the method does not support non UTC form date
        try {
            entEle.setAttribute(SAMLNames.VALIDUNTIL, "2045-06-29T19:19:51.10+05:00");
            importer.isExpired(entEle);
            fail();
        } catch (IllegalArgumentException e) {
            //pass
        } catch (Exception x) {
            fail("unexprect exception");
        }

    }

}
