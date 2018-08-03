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
package com.vmware.vim.sso.client;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.xml.XMLConstants;
import javax.xml.transform.TransformerFactory;

/**
 * TransformerFactory based off @{javax.xml.transform.TransformerFactory}
 * that can create secure Transformer objects for untrusted XML input
 */
public class SecureTransformerFactory {
    private static final Logger log = LoggerFactory.getLogger(SecureTransformerFactory.class);

    /**
     * Creates a transformer factory for user input that is protected against DTD
     * @return TransformerFactory object
     */
    public static TransformerFactory newTransformerFactory() {
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        trySetAttribute(transformerFactory, XMLConstants.ACCESS_EXTERNAL_DTD, "");
        trySetAttribute(transformerFactory, XMLConstants.ACCESS_EXTERNAL_STYLESHEET, "");
        return transformerFactory;
    }

    private static void trySetAttribute(TransformerFactory transformerFactory, String key, String value) {
        try {
            transformerFactory.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
            transformerFactory.setAttribute(XMLConstants.ACCESS_EXTERNAL_STYLESHEET, "");
        } catch (IllegalArgumentException e) {
            log.warn("The current implementation of transformerFactory does not support {}", value);
        }
    }
}
