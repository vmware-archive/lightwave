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
package com.vmware.vim.sso.client.util;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStore.PrivateKeyEntry;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableEntryException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;

/**
 * KeyStoreData class represents a certificate/private key for a given alias
 * from java keystore file.
 */
public class KeyStoreData {

   private final String _keystorePath;
   private final String _certAlias;
   private final KeyStore _keyStore;
   private final String LOAD_ERROR_MSG;
   private final Logger _log = LoggerFactory.getLogger(KeyStoreData.class);

   /**
    * Creates a KeyStoreData object.
    *
    * @param keystorePath
    *           The path to the java keystore file
    * @param storePass
    *           The password for the keystore file
    * @param certAlias
    *           The alias of the certificate that should be loaded
    * @throws SsoKeyStoreOperationException
    */
   public KeyStoreData(String keystorePath, char[] storePass, String certAlias)
      throws SsoKeyStoreOperationException {

      if (_log.isDebugEnabled()) {
         _log.debug("Loading keystore: " + keystorePath);
      }

      _keystorePath = keystorePath;
      _certAlias = certAlias;

      LOAD_ERROR_MSG = "Error while trying to load certificate entry "
         + _certAlias + " from " + _keystorePath;

      FileInputStream fis = null;
      try {
         _keyStore = KeyStore.getInstance("JKS");
         fis = new FileInputStream(keystorePath);
         _keyStore.load(fis, storePass);
      } catch (KeyStoreException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } catch (NoSuchAlgorithmException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } catch (CertificateException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } catch (FileNotFoundException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } catch (IOException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } finally {
         if (fis != null) {
            try {
               fis.close();
            } catch (IOException e) {
               _log.error(LOAD_ERROR_MSG, e);
            }
         }
      }
   }

   /**
    * @return the private key for the specified certificate alias
    * @throws SsoKeyStoreOperationException
    */
   public Key getPrivateKey(char[] keyPass)
      throws SsoKeyStoreOperationException {
      PrivateKeyEntry keyEntry;
      try {
         // TODO : Check the alias exists
         keyEntry = (PrivateKeyEntry) _keyStore.getEntry(_certAlias,
            new KeyStore.PasswordProtection(keyPass));
      } catch (NoSuchAlgorithmException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } catch (UnrecoverableEntryException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      } catch (KeyStoreException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      }

      return keyEntry.getPrivateKey();
   }

   /**
    * @return the certificate for the specified certificate alias
    * @throws SsoKeyStoreOperationException
    */
   public X509Certificate getCertificate() throws SsoKeyStoreOperationException {
      try {
         // TODO: check the alias exists and that it corresponds to an X.509
         // Certificate
         return (X509Certificate) _keyStore.getCertificate(_certAlias);
      } catch (KeyStoreException e) {
         _log.error(LOAD_ERROR_MSG, e);
         throw new SsoKeyStoreOperationException(LOAD_ERROR_MSG, e);
      }
   }
}