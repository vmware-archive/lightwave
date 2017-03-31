/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.vecs;

import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.CRLException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.Enumeration;
import java.util.List;

import javax.crypto.spec.SecretKeySpec;

/**
 * This class provides means to work with a particular VECS store.
 * To start working with the store, one needs to first open the store (use "openStore" method)
 * and then after the work is done, the store has to be closed (user "closeStore" method).
 * The VMwareEndpointCertificateStore are created by VecsStoreFactory. There are 2 types of VMwareEndpointCertificateStore
 * one can create. IPC type (created by calling VecsStoreFactory.getVecsStoreFactoryViaIPC()) and
 * Remoting type (created by calling VecsStoreFactory.getVecsStoreFactoryViaDomainAuth()).
 * The IPC type allows you to only communicate with the local VECS, where the authentication happens by validating the peer process credentials.
 * The Remoting type allows you to talk to remote VECS instance as well your local one,
 * however the authentication mechanism done by validating Domain user credentials in Directory Service.
 * NOTE: This class is *not* Thread-safe.
 */
public class VMwareEndpointCertificateStore implements AutoCloseable {
   private final String _storeName;
   private final ServerHandle _serverHandle;
   private PointerRef _storeHandle;
   private final String _userName;
   private final String _serverName;

   /**
    * Creates an instance which communicates with a specified store of a specified server handle.
    * To start working with the VECS store resource, one needs to first open the store (use "openStore" method)
    * and then after the work is done, the store has to be closed (user "closeStore" method).
    *
    * @param serverHandle
    *           Server handle. The server handle should be created using static function "openServer".
    *
    * @param serverName Server name for auditing purposes.
    *
    * @param userName UserName for auditing purposes.
    *
    * @param storeName
    *           Name of a store.
    */
   VMwareEndpointCertificateStore(ServerHandle serverHandle, String serverName, String userName, String storeName) {
      if (storeName == null || storeName.isEmpty()) {
         throw new IllegalArgumentException(
               "storeName cannot be null or empty.");
      }

      if (serverHandle == null) {
         throw new IllegalArgumentException(
               "serverHandle cannot be null or empty.");
      }

      _serverHandle = serverHandle;
      _storeHandle = null;
      _storeName = storeName;
      _serverName = serverName;
      _userName = userName;
   }

   /**
    * Opens the store.
    */
   public void openStore() {
      PointerRef pStore = new PointerRef();
      int error = VecsAdapter.VecsOpenCertStoreHW(_serverHandle.getHandle(), _storeName, null,
            pStore);
      BAIL_ON_ERROR(error, "Opening store '%s' failed. [Server: %s, User: %s]",
            _storeName, _serverName, _userName);

      _storeHandle = pStore;
   }

   /**
    * Closes the store.
    */
   public void closeStore() {
      if (_storeHandle !=null && !PointerRef.isNull(_storeHandle))
      {
          int error = VecsAdapter.VecsCloseCertStore(_storeHandle);;
          BAIL_ON_ERROR(error, "Closing store '%s' failed. [Server: %s, User: %s]",
                _storeName, _serverName, _userName);
          _storeHandle = null;
      }
   }

   /**
    * Adds a VecsEntryType.CERT_ENTRY_TYPE_TRUSTED_CERT entry to the store.
    *
    * @param alias
    *           Alias of an entry. It can be null or empty in which case it is
    *           going to be generated.
    * @param cert
    *           x509 certificate
    * @throws CertificateEncodingException
    *            If string encoding of the certificate failed.
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
   public void addTrustedCertEntry(String alias, X509Certificate cert)
         throws CertificateEncodingException {
      if (cert == null) {
         throw new IllegalArgumentException("'cert' cannot be null");
      }

      String pemCert = VecsUtils
            .encodeX509CertificatesToString(new X509Certificate[] { cert });

      int error = VecsAdapter.VecsAddEntryW(_storeHandle,
               VecsEntryType.CERT_ENTRY_TYPE_TRUSTED_CERT.getValue(), alias,
               pemCert, null, null, false);

      BAIL_ON_ERROR(error,
               "Adding VecsEntryType.CERT_ENTRY_TYPE_TRUSTED_CERT entry "
                     + "into store '%s' failed. [Server: %s, User: %s]",
                     _storeName, _serverName, _userName);
   }

   /**
    * Adds a VecsEntryType.CERT_ENTRY_TYPE_CRL entry to the store.
    *
    * @param alias
    *           Alias of an entry. It can be null or empty in which case it is
    *           going to be generated.
    * @param crl
    *           x509CRL crl
    * @throws CRLException
    *           if an encoding error occurs.
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
   public void addCrlEntry(String alias, X509CRL crl)
         throws CertificateEncodingException, CRLException {
      if (crl == null) {
         throw new IllegalArgumentException("'crl' cannot be null");
      }

      String pemCrl = VecsUtils.encodeX509CRLToString(crl);

      int error = VecsAdapter.VecsAddEntryW(_storeHandle,
               VecsEntryType.CERT_ENTRY_TYPE_CRL.getValue(), alias,
               pemCrl, null, null, false);

      BAIL_ON_ERROR(error,
               "Adding VecsEntryType.CERT_ENTRY_TYPE_CRL entry "
                     + "into store '%s' failed. [Server: %s, User: %s]",
                     _storeName, _serverName, _userName);
   }

   /**
    * Adds a VecsEntryType.CERT_ENTRY_TYPE_SECRET_KEY entry to the store.
    *
    * @param alias
    *           Alias of an entry. It can be null or empty in which case it is
    *           going to be generated.
    * @param key
    *           Secret key object.
    * @param keyPassword
    *           Password for private key protection. (currently ignored, thus
    *           you can supply 'null')
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
   public void addSecretKeyEntry(String alias,
         SecretKeySpec key, char[] keyPassword) {

      if (key == null) {
         throw new IllegalArgumentException(String.format("'key' cannot be null."
               + "[Store: %s, Server: %s, User: %s]",
               _storeName, _serverName, _userName));
      }

      String pemKey = VecsUtils.encodeSecretKeyToBase64String(key);
      String passwordString = VecsUtils.encodeCharArrToString(keyPassword);

      int error = VecsAdapter.VecsAddEntryW(_storeHandle,
               VecsEntryType.CERT_ENTRY_TYPE_SECRET_KEY.getValue(), alias,
               null, pemKey, passwordString, false);

      BAIL_ON_ERROR(error,
               "Adding VecsEntryType.CERT_ENTRY_TYPE_SECRET_KEY entry "
                     + "into store '%s' failed. [Server: %s, User: %s]",
                     _storeName, _serverName, _userName);
   }


   /**
    * Adds a VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY entry to the store.
    *
    * @param alias
    *           Alias of an entry. It can be null or empty in which case it is
    *           going to be generated.
    * @param certChain
    *           X509 certificate chain.
    * @param key
    *           Private key object.
    * @param keyPassword
    *           Password for private key protection. (currently ignored, thus
    *           you can supply 'null')
    * @param autoRefresh
    *           This flag is in use and is ignored.
    * @throws CertificateEncodingException
    *            If string encoding of the certificate failed.
    * @throws UnsupportedEncodingException
    *            If string encoding of the key failed.
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
   public void addPrivateKeyEntry(String alias, X509Certificate[] certChain,
         PrivateKey key, char[] keyPassword, boolean autoRefresh)
         throws CertificateEncodingException, UnsupportedEncodingException {
      if (certChain == null || certChain.length < 1) {
         throw new IllegalArgumentException(
               String.format("'certChain' cannot be null or be empty. "
                     + "[Store: %s, Server: %s, User: %s]",
                     _storeName, _serverName, _userName));
      }

      if (key == null) {
         throw new IllegalArgumentException(String.format("'key' cannot be null. "
               + "[Store: %s, Server: %s, User: %s]",
               _storeName, _serverName, _userName));
      }

      String pemCertChain = VecsUtils.encodeX509CertificatesToString(certChain);
      String pemKey = VecsUtils.encodePrivateKeyToString(key);
      String passwordString = VecsUtils.encodeCharArrToString(keyPassword);

      int error = VecsAdapter.VecsAddEntryW(_storeHandle,
               VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY.getValue(), alias,
               pemCertChain, pemKey, passwordString, false);

      BAIL_ON_ERROR(error,
               "Adding VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY entry "
                     + "into store '%s' failed. [Server: %s, User: %s]", _storeName, _serverName, _userName);

   }

   /**
    * Gets an entry by alias from the store based on the info level (see
    * VecsEntryInfoLevel).
    *
    * @param alias
    *           Alias of an entry. It cannot be null or empty.
    * @param infoLevel
    * @return
    * @throws CertificateException
    *            if certificate generation from Vecs entry failed.
 * @throws CRLException
    */
   public VecsEntry getEntryByAlias(String alias, VecsEntryInfoLevel infoLevel)
         throws CertificateException, CRLException {
      if (alias == null || alias.isEmpty()) {
         throw new IllegalArgumentException(String.format("'alias' cannot be null or empty. "
               + "[Store: %s, Server: %s, User: %s]",
               _storeName, _serverName, _userName));
      }

      if (infoLevel == null) {
         throw new IllegalArgumentException(String.format("'infoLevel' cannot be null. "
               + "[Store: %s, Server: %s, User: %s]",
               _storeName, _serverName, _userName));
      }

      VecsEntry resultEntry = null;
      VecsEntryNative pEntry = new VecsEntryNative();
      int error = VecsAdapter.VecsGetEntryByAliasW(_storeHandle, alias,
            infoLevel.getValue(), pEntry);
      if (error == VecsAdapter.ERROR_OBJECT_NOT_FOUND) {
         resultEntry = null;
      } else {
         BAIL_ON_ERROR(error,
               "Getting entry by alias '%s' from store '%s' on server failed. [Server: %s, User: %s]",
               alias, _storeName, _serverName, _userName);
         resultEntry = convertVecsEntryNativeToVecsEntry(pEntry);
      }
      return resultEntry;
   }

   /**
    * Gets an entry by alias from the store based on the info level (see
    * VecsEntryInfoLevel).
    *
    * @param alias
    *           Alias of an entry. It cannot be null or empty.
    * @param infoLevel
    * @return Key object
    * @throws NoSuchAlgorithmException
    *            if the key cannot be recovered (e.g., the given password is
    *            wrong).
    * @throws UnrecoverableKeyException
    *            if the algorithm for recovering the key cannot be found
    */
   public Key getKeyByAlias(String alias, char[] password)
         throws UnrecoverableKeyException, NoSuchAlgorithmException {
      if (alias == null || alias.isEmpty()) {
         throw new IllegalArgumentException(String.format("'alias' cannot be null or empty. "
               + "[Store: %s, Server: %s, User: %s]",
               _storeName, _serverName, _userName));
      }

      String passwordStr = VecsUtils.encodeCharArrToString(password);
      StringRef pKeyStr = new StringRef();

      int error = VecsAdapter.VecsGetKeyByAliasW(_storeHandle, alias, passwordStr,
            pKeyStr);
      if (error == VecsAdapter.ERROR_OBJECT_NOT_FOUND) {
         pKeyStr.str = null;
      } else {
         BAIL_ON_ERROR(error,
               "Getting key by alias '%s' from store '%s' failed. [Server: %s, User: %s]", alias, _storeName, _serverName, _userName);
      }

      Key resultKey = null;
      if (pKeyStr.str != null && pKeyStr.str.indexOf(VecsUtils.PEM_PRIVATEKEY_PREFIX) < 0) {
         resultKey = VecsUtils.getSecretKeyFromString(pKeyStr.str);
      } else {
         resultKey = VecsUtils.getPrivateKeyFromString(pKeyStr.str);
      }

      return resultKey;
   }

   /**
    * Gets the number of entries in the store.
    *
    * @return Number of entries
    */
   public int getEntryCount() {
      IntRef pCount = new IntRef();
      int error = VecsAdapter.VecsGetEntryCount(_storeHandle, pCount);

      BAIL_ON_ERROR(error, "Getting entry count of store '%s' failed. [Server: %s, User: %s]",
               _storeName, _serverName, _userName);
      return pCount.number;
   }

   /**
    * Gets Enumeration object for the store entries.
    * Note: This Enumeration instance is *not* thread-safe. Also, make sure to keep the store open when enumerating entries.
    * @return Number of entries
    */
   public VecsEntryEnumeration enumerateEntries(VecsEntryInfoLevel infoLevel) {
      return new VecsEntryEnumeration(this, _storeHandle, _serverName, _userName, _storeName, infoLevel);
   }

   /**
    * Deletes entry by alias from the store.
    *
    * @param alias
    *           Alias of an entry. It cannot be null or empty.
    */
   public void deleteEntryByAlias(String alias) {
      if (alias == null || alias.isEmpty()) {
         throw new IllegalArgumentException(String.format("'alias' cannot be null or empty."
               + "[Store: %s, Server: %s, User: %s]",
               _storeName, _serverName, _userName));
      }

      int error = VecsAdapter.VecsDeleteEntryW(_storeHandle, alias);

      BAIL_ON_ERROR(error,
               "Deleting entry by alias '%s' from store '%s' failed. "
               + "[Server: %s, User: %s]", alias,
               _storeName, _serverName, _userName);
   }

   /**
    * Sets store permission READ/WRITE for a user.
    *
    * @param userName
    *           User name. It cannot be null or empty.
    * @param accessMask
    *           Access mask READ/WRITE.
    */
   public void setPermission(String userName, VecsPermission.AccessMask accessMask) {
      int error = VecsAdapter.VecsSetPermissionW(_storeHandle, userName, accessMask.getValue());

      BAIL_ON_ERROR(error, "Setting permission of store '%s' for user '%s' failed. [Server: %s, User: %s]",
               _storeName, userName, _serverName, _userName);
   }

   /**
    * Revokes store permission RED/WRITE for a user
    *
    * @param userName
    *           User name. It cannot be null or empty.
    * @param accessMask
    *           Access mask READ/WRITE.
    */
   public void revokePermission(String userName, VecsPermission.AccessMask accessMask) {
      int error = VecsAdapter.VecsRevokePermissionW(_storeHandle, userName, accessMask.getValue());

      BAIL_ON_ERROR(error, "Revoking permission of store '%s' for user '%s' failed. [Server: %s, User: %s]",
               _storeName, userName, _serverName, _userName);
   }

   /**
    * Gets a list of VECS permissions for the store.
    *
    * @return Permissions for the store.
    */
   public List<VecsPermission> getPermissions() {
      List<VecsPermissionNative> pStorePermissionsNative = new ArrayList<VecsPermissionNative>();
      StringRef pOwner = new StringRef();

      int error = VecsAdapter.VecsGetPermissionsW(_storeHandle, pOwner, pStorePermissionsNative);

      BAIL_ON_ERROR(error, "Get permissions of store '%s' failed. [Server: %s, User: %s]", _storeName, _serverName, _userName);

      List<VecsPermission> pStorePermissions = new ArrayList<VecsPermission>(pStorePermissionsNative.size());
      for (VecsPermissionNative permission : pStorePermissionsNative) {
         VecsPermission.AccessMask accessMask = (permission.accessMask == VecsPermission.AccessMask.READ.getValue())?VecsPermission.AccessMask.READ:VecsPermission.AccessMask.WRITE;
         pStorePermissions.add(new VecsPermission(permission.userName, accessMask));
      }

      return pStorePermissions;
   }

   // ////PRIVATE HELPER METHODS

   private static VecsEntry convertVecsEntryNativeToVecsEntry(
         VecsEntryNative entryNative) throws CertificateException, CRLException {
      String alias = entryNative.alias;
      VecsEntryType type = VecsEntryType.getEntryType(entryNative.entryType);

      X509CRL crl = null;
      X509Certificate[] certs = null;

      if (type == VecsEntryType.CERT_ENTRY_TYPE_CRL) {
         crl = VecsUtils.getX509CRLFromString(entryNative.certificate);
      } else {
         certs = VecsUtils
               .getX509CertificatesFromString(entryNative.certificate);
      }
      Date date = new Date(entryNative.date*1000);


      return new VecsEntry(type, date, alias, certs, crl);
   }

   /**
    *
    * @param error
    *           VECS error code.
    * @param message
    * @throws VecsGenericException
    *            generic vecs exception.
    */
   private static void BAIL_ON_ERROR(final int error, final String format,
         Object... vargs) {
      switch (error) {
      case 0:
         break;
      default:
         throw new VecsGenericException(String.format(format, vargs), error);
      }
   }

   @Override
   public void close(){
      closeStore();
   }

   protected void finalize() throws Throwable {
      try {
         closeStore();
      } finally {
         super.finalize();
      }
   }
}
