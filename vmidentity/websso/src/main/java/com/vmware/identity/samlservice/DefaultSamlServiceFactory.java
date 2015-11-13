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
package com.vmware.identity.samlservice;


import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.cert.CertPath;

import org.apache.commons.lang.Validate;
import org.opensaml.xml.ConfigurationException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.samlservice.impl.SamlServiceImpl;

public class DefaultSamlServiceFactory implements SamlServiceFactory {
	private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(DefaultSamlServiceFactory.class);
	private static final String CANNOT_INITIALIZE_OPENSAML_LIB = "Cannot initialize opensaml lib.";

   static {
       try {
          Shared.bootstrap();
       } catch (ConfigurationException e) {
          log.error(CANNOT_INITIALIZE_OPENSAML_LIB, e);
          throw new IllegalStateException(CANNOT_INITIALIZE_OPENSAML_LIB, e);
       }
   }

   public DefaultSamlServiceFactory() {
   }

   @Override
public SamlService createSamlService(
           PrivateKey privateKey,
           SignatureAlgorithm signAlgorithm,
		   SignatureAlgorithm checkAlgorithm,
		   String issuer,
		   CertPath certPath) {

	   SamlServiceImpl samlServiceImpl = null;
		try {
			samlServiceImpl = new SamlServiceImpl();
            samlServiceImpl.setPrivateKey(privateKey);
            samlServiceImpl.setSignAlgorithm(signAlgorithm);
			samlServiceImpl.setCheckAlgorithm(checkAlgorithm);
			samlServiceImpl.setIssuer(issuer);
			samlServiceImpl.setCertPath(certPath);
		} catch (NoSuchAlgorithmException e) {
			log.debug("Caught exception " + e.toString());
		}
      Validate.notNull(samlServiceImpl, "samlServiceImpl");

      return samlServiceImpl;
   }
}
