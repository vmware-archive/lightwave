/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.vmware.identity.wstrust.test.util;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.KeyStore.PrivateKeyEntry;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.text.DateFormat;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Helper class: wraps a key store and provides access to the entries via
 * symbolic (i.e. enumerated constants) names (vs. KeyStore's native interface,
 * which provides access via textual names).
 * <p>
 * This helper also:
 * <ul>
 * <li>Expects all PrivateKey entries NOT to be password protected.</li>
 * <li>Converts all exceptions to {@link KeyStoreException}.</li>
 * </ul>
 */
public class KeyStoreHelper {

  protected static final Logger _log = LoggerFactory
      .getLogger(KeyStoreHelper.class);

  public static class KeyStoreException extends RuntimeException {
    private static final long serialVersionUID = 1112375504000994903L;

    KeyStoreException(Throwable cause) {
      super(cause);
    }
  }

  private final KeyStore _keyStore;

  /**
   * Create a key store helper object. This is constructor does the same as
   * invoking KeyStoreHelper(entryNamesClass, storeName, "JKS").
   */
  public KeyStoreHelper(String storeName) {
    this(storeName, "JKS");
  }

  /**
   * Create a key store helper object.
   *
   * @param storeName The name of the key store resource.
   * @param storeType The type of the key store.
   * @throws IllegalArgumentException if the key store with the given name does not exist or cannot
   *                                  be loaded.
   */
  public KeyStoreHelper(String storeName, String storeType) {

    InputStream storeStream = getClass().getResourceAsStream(storeName);

    // KeyStore.load() won't complain, so check the resource exists manually
    if (storeStream == null) {
      throw new IllegalArgumentException(String.format(
          "Store '%s' not found", storeName));
    }

    try {
      _keyStore = KeyStore.getInstance(storeType);
      _keyStore.load(storeStream, null);

    } catch (IOException e) {
      throw new IllegalArgumentException("Failed to load keystore "
          + storeName, e);

    } catch (GeneralSecurityException e) {
      throw new IllegalArgumentException("Failed to load keystore "
          + storeName, e);
    }
  }

  public KeyStoreHelper(String storeName,
                        InputStream storeStream,
                        String storeType) {


    // KeyStore.load() won't complain, so check the resource exists manually
    if (storeStream == null) {
      throw new IllegalArgumentException(String.format(
          "Store '%s' not found", storeName));
    }

    try {
      _keyStore = KeyStore.getInstance(storeType);
      _keyStore.load(storeStream, null);

    } catch (IOException e) {
      throw new IllegalArgumentException("Failed to load keystore "
          + storeName, e);

    } catch (GeneralSecurityException e) {
      throw new IllegalArgumentException("Failed to load keystore "
          + storeName, e);
    }
  }

  public static KeyStoreHelper createFromFile(String storeFileName)
      throws FileNotFoundException {

    KeyStoreHelper ksh = new KeyStoreHelper(storeFileName,
        new FileInputStream(storeFileName),
        "JKS");
    return ksh;
  }



  /**
   * Returns a X.509 certificate entry. If the entry doesn't exist, an
   * IllegalArgumentException is thrown.
   */
  public X509Certificate getCertificate(String certAlias) {
    X509Certificate cert;
    try {
      cert = (X509Certificate) _keyStore.getCertificate(certAlias);

      _log.info("Certificate is valid until- " + DateFormat.getInstance().format(cert.getNotAfter()));
    } catch (ClassCastException e) {
      throw new IllegalArgumentException("Entry " + certAlias
          + " does not correspond to an X.509 certificate", e);

    } catch (GeneralSecurityException e) {
      throw new KeyStoreException(e);
    }

    checkEntryAvailable(certAlias, cert);

    return cert;
  }

  /**
   * Returns a PrivateKey entry. If the entry doesn't exist, an
   * IllegalArgumentException is thrown.
   */
  public PrivateKey getPrivateKey(String certAlias, char[] passwd) {
    KeyStore.PrivateKeyEntry pk = null;
    try {
      KeyStore.ProtectionParameter pass = new KeyStore.PasswordProtection(passwd);

      /**
       * Helper: check that the object returned for the given entry is a real
       * thing (because the KeyStore won't do it for us).
       */

      pk = (PrivateKeyEntry) _keyStore.getEntry(certAlias, pass);

    } catch (ClassCastException e) {
      throw new IllegalArgumentException("Entry " + certAlias
          + " does not correspond to a private key", e);

    } catch (GeneralSecurityException e) {
      throw new KeyStoreException(e);
    } catch (Exception e) {
      e.printStackTrace();
    }

    checkEntryAvailable(certAlias, pk);

    return pk.getPrivateKey();
  }

  /**
   * Helper: check that the object returned for the given entry is a real thing
   * (because the KeyStore won't do it for us).
   */
  private void checkEntryAvailable(String certAlias, Object ksObj) {
    if (ksObj == null) {
      throw new IllegalArgumentException("Entry " + certAlias
          + " not found in store");
    }
  }
}
