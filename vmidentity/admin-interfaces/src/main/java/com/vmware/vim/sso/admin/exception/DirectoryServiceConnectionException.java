/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

import com.vmware.vim.sso.admin.DomainManagement;

/**
 * Thrown by {@link DomainManagement#probeConnectivity probeConnectivity} to indicate the SSO
 * Server failed to connect or authenticate to the service at the specified URI.
 */
public class DirectoryServiceConnectionException extends ServiceException {

   private static final long serialVersionUID = -370719619302641493L;
   private final java.net.URI uri;

   public DirectoryServiceConnectionException(String message, java.net.URI uri) {
      super(message);
      assert (null != uri);
      this.uri = uri;
   }

   public DirectoryServiceConnectionException(String message, java.net.URI uri,
      Throwable cause) {
      super(message, cause);
      assert (null != uri);
      this.uri = uri;
   }

   public java.net.URI getUri() {
      return uri;
   }
}
