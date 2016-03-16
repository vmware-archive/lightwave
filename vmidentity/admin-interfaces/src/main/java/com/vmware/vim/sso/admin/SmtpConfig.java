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
package com.vmware.vim.sso.admin;

import java.util.Arrays;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * SMTP settings are used to configure the SSO server for sending e-mails. This
 * class is immutable.
 *
 * @deprecated Since SSO 2.0 this class has been deprecated.
 */
@Deprecated
public final class SmtpConfig {

   private final String _host;

   private final int _port;

   private final boolean _isAuthenticate;

   private final String _user;

   private final char[] _password;

   /**
    * Create an object representing SMTP configuration.
    *
    * @param host
    *           Host of the SMTP server. <code>null</code> values are not
    *           acceptable
    * @param port
    *           Port of the SMTP server
    * @param isAuthenticate
    *           Whether authentication will be used
    * @param user
    *           User for authentication. <code>null</code> if no authentication
    *           is required
    * @param password
    *           Password for authentication. <code>null</code> if no
    *           authentication is required
    *
    * @deprecated Since SSO 2.0 {@link SmtpConfig} class has been deprecated.
    */
   @Deprecated
   public SmtpConfig(String host, int port, boolean isAuthenticate,
      String user, char[] password) {

      ValidateUtil.validateNotNull(host, "SMTP server hostname");
      ValidateUtil.validatePositiveNumber(port, "SMTP port");

      _host = host;
      _port = port;
      _isAuthenticate = isAuthenticate;
      _user = user;
      _password = (password != null) ? Arrays.copyOf(password, password.length)
         : null;
   }

   /**
    * @return The host of the SMTP server or <code>null</code> value
    *
    * @deprecated Since SSO 2.0 {@link SmtpConfig} class has been deprecated.
    */
   @Deprecated
   public String getHost() {
      return _host;
   }

   /**
    * @return The port of the SMTP server
    *
    * @deprecated Since SSO 2.0 {@link SmtpConfig} class has been deprecated.
    */
   @Deprecated
   public int getPort() {
      return _port;
   }

   /**
    * @return Whether authentication will be used
    *
    * @deprecated Since SSO 2.0 {@link SmtpConfig} class has been deprecated.
    */
   @Deprecated
   public boolean isAuthenticate() {
      return _isAuthenticate;
   }

   /**
    * @return The user used for authentication or <code>null</code> value
    *
    * @deprecated Since SSO 2.0 {@link SmtpConfig} class has been deprecated.
    */
   @Deprecated
   public String getUser() {
      return _user;
   }

   /**
    * @return The password used for authentication or <code>null</code> value
    *
    * @deprecated Since SSO 2.0 {@link SmtpConfig} class has been deprecated.
    */
   @Deprecated
   public char[] getPassword() {
      return _password;
   }
}
