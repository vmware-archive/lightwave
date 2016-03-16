/* **********************************************************************
 * Copyright 2015 VMware, Inc.
 * ********************************************************************
 */
package com.vmware.identity.cdc;


/**
 * This class provides means to manipulate VECS stores's creation, deletion, enumeration, and creation of VMwareEndpointCertificateStore.
 */
public class CdcFactory {

   /**
    * Instantiates a CdcSession for a specified domain user for a specified server.
    * @param serverName
    *              Server Name (ip address or host name)
    * @param userName
    *              Domain user name, which is a user's upn (e.g. user@domain)
    * @param userPassword
    *              User's password
    */
   public static CdcSession createCdcSessionViaDomainAuth(String serverName, String userName, String userPassword) {
      if (serverName == null || serverName.length() == 0) {
         throw new IllegalArgumentException("Server name is not specified");
      }

      if (userName == null || userName.length() == 0) {
          throw new IllegalArgumentException("User name is not specified");
      }
      return new CdcSession(serverName, userName, userPassword);
   }

   /**
    * Instantiates a CdcSession to work with VmAfd via IPC (Inter-Process Communication) mechanism.
    * This means that the VmAfd is going to authenticate user based on the peer process credentials.
    * @return new CdcSession instance via IPC mechanism.
    */
   public static CdcSession createCdcSessionViaIPC() {
      return new CdcSession(null, null, null);
   }

}
