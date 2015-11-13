/**
 *
 * Copyright 2012 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.websso.client;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXParseException;

public class SamlParserErrorHandler implements ErrorHandler {
    private static Logger logger = LoggerFactory.getLogger(SamlParserErrorHandler.class);

    @Override
    public void warning(SAXParseException e) throws SAXParseException {
        logger.warn(e.getMessage());
    }

    @Override
    public void error(SAXParseException e) throws SAXParseException {
        logger.error(e.getMessage());
    }

    @Override
    public void fatalError(SAXParseException e) throws SAXParseException {
        logger.error(e.getMessage());
    }
}
