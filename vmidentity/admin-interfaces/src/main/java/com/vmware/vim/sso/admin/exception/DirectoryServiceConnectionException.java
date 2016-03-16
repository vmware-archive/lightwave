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
