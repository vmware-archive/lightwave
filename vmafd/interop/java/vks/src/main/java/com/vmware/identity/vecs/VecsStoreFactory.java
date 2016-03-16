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

import java.util.ArrayList;
import java.util.List;

/**
 * This class provides means to manipulate VECS stores's creation, deletion, enumeration, and creation of VMwareEndpointCertificateStore.
 */
public class VecsStoreFactory {

   private final String _userPassword;
   private final String _serverName;
   private final String _userName;
   private final boolean _isIPC;

   private static final String LOCALHOST = "__localhost__";
   private static final String LOCALUSER = "__localuser__";

   /**
    * Instantiates a VecsStoreFactory for a specified domain user for a specified server.
    * @param serverName
    *              Server Name (ip address or host name)
    * @param userName
    *              Domain user name, which is a user's upn (e.g. user@domain)
    * @param userPassword
    *              User's password
    */
   private VecsStoreFactory(String serverName, String userName, String userPassword) {
      _isIPC = (serverName == null);
      _serverName = (serverName == null) ? LOCALHOST : serverName;
      _userName = (userName == null) ? LOCALUSER : userName;
      _userPassword = userPassword;
   }

   /**
    * Instantiates a VecsStoreFactory for a specified domain user for a specified server.
    * @param serverName
    *              Server Name (ip address or host name)
    * @param userName
    *              Domain user name, which is a user's upn (e.g. user@domain)
    * @param userPassword
    *              User's password
    */
   public static VecsStoreFactory getVecsStoreFactoryViaDomainAuth(String serverName, String userName, String userPassword) {
      if (serverName == null || serverName.length() == 0) {
         throw new IllegalArgumentException("Server name is not specified");
      }

      if (userName == null || userName.length() == 0) {
          throw new IllegalArgumentException("User name is not specified");
      }
      return new VecsStoreFactory(serverName, userName, userPassword);
   }

   /**
    * Instantiates a VecsStoreFactory to work with VECS via IPC (Inter-Process Communication) mechanism.
    * This means that the VECS is going to authenticate user based on the peer process credentials.
    * @return new VMwareEndpointCertificateStore instance via IPC mechanism.
    */
   public static VecsStoreFactory getVecsStoreFactoryViaIPC() {
      return new VecsStoreFactory(null, null, null);
   }

   /**
    * Opens a server for a specified user.
    * Usually, this constructor would be used when communicating with a remote server.
    *
    * @param serverName
    *           Server name
    *
    * @param userName
    *           Domain user name, which is a user's upn (e.g. user@domain)
    *
    * @param userPassword
    *           User's password
    *
    * @return Server Handle
    */
   private ServerHandle openServer() {
      PointerRef pServer = new PointerRef();
      int error = 0;
      if (!_isIPC) {
         error = VecsAdapter.VmAfdOpenServerW(_serverName, _userName, _userPassword, pServer);
      } else {
         error = VecsAdapter.VmAfdOpenServerW(null, null, null, pServer);
      }
      if (error != 0) {
         throw new VecsGenericException(
              String.format("Error opening server '%s' for user '%s'",
                 _serverName, _userName), error);
      }
      return new ServerHandle(pServer);
   }

   /**
    * Opens a server for a specified user.
    * Usually, this constructor would be used when communicating with a remote server.
    *
    * @param serverName
    *           Server name
    *
    * @param userName
    *           Domain user name, which is a user's upn (e.g. user@domain)
    *
    * @param userPassword
    *           User's password
    *
    * @return Server Handle
    */
   static void closeServer(PointerRef serverHandle) {
      if (serverHandle == null) {
         throw new IllegalArgumentException("Invalid server handle provided");
      }
      PointerRef pServer = (PointerRef) serverHandle;
      if (!PointerRef.isNull(pServer)) {
         VecsAdapter.VmAfdCloseServer(pServer);
      }
   }

   /**
    * Returns a new VMwareEndpointCertificateStore instance for a specified store.
    * @param storeName Store name
    * @return new VMwareEndpointCertificateStore instance
    */
   public VMwareEndpointCertificateStore getVecsStore(String storeName) {
      ServerHandle serverHandle = openServer();
      VMwareEndpointCertificateStore store = new VMwareEndpointCertificateStore(serverHandle, _serverName, _userName, storeName);
      return store;
   }

   /**
    * Creates a store with the provided name.
    *
    * @param storeName
    *           Name of a store.
    * @throws AlreadyExistsException
    *            if the store with the provided name already exists
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
   public VMwareEndpointCertificateStore createCertStore(String storeName)
         throws AlreadyExistsException {
      if (storeName == null || storeName.isEmpty()) {
         throw new IllegalArgumentException(
               String.format("storeName cannot be null or empty. "
                  + "[Server: %s, User: %s]", _serverName, _userName));
      }

      ServerHandle serverHandle = openServer();
      try {
         int error = VecsAdapter.VecsCreateCertStoreHW(serverHandle.getHandle(), storeName,
               null, null);

         if (error == VecsAdapter.ERROR_ALREADY_EXISTS) {
            throw new AlreadyExistsException(String.format("%s store already exists. "
                  + "[Server: %s, User: %s]", storeName, _serverName, _userName));
         }
         if (error != 0) {
             throw new VecsGenericException(
                  String.format("Creating store '%s' failed. [Server: %s, User: %s]",
                     storeName, _serverName, _userName), error);
         }
      } catch (Exception e) {
         closeServer(serverHandle.getHandle());
         throw e;
      }

      VMwareEndpointCertificateStore store = new VMwareEndpointCertificateStore(serverHandle, _serverName, _userName, storeName);
      return store;
   }

   /**
    * Enumerates stores.
    * @return List of store names.
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
    public List<String> enumCertStores() {
      List<String> storeNameList = new ArrayList<String>();
      try (ServerHandle serverHandle = openServer()) {
         int error = VecsAdapter.VecsEnumCertStoreHW(serverHandle.getHandle(), storeNameList);

         if (error != 0) {
             throw new VecsGenericException(
                  String.format("Enumerating stores failed. [Server: %s, User: %s]",
                     _serverName, _userName), error);
         }
      }
      return storeNameList;
   }

   /**
    * Deletes a store with the provided name.
    * @param storeName
    *           Name of a store.
    * @throws VecsGenericException
    *            if anything else happened during runtime.
    */
   public void deleteCertStore(String storeName) {
      try (ServerHandle serverHandle = openServer()) {
         int error = VecsAdapter.VecsDeleteCertStoreHW(serverHandle.getHandle(), storeName);

         if (error != VecsAdapter.ERROR_OBJECT_NOT_FOUND) {
            if (error != 0) {
               throw new VecsGenericException(
                  String.format("Deleting store '%s' failed. [Server: %s, User: %s]",
                      storeName,_serverName, _userName), error);
            }
         }
      }
   }
}
