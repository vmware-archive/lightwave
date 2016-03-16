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

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * This class represents the information needed for sending e-mails.
 *
 * @deprecated Since SSO 2.0 this class has been deprecated.
 */
@Deprecated
public final class MailContent {

   private final String _from;

   private final String _to;

   private final String _subject;

   private final String _content;

   /**
    * Creates an object representing mail content
    *
    * @param from
    *           E-mail address of the sender. <code>null</code> values are not
    *           acceptable
    * @param to
    *           E-mail address of the recipient. <code>null</code> values are
    *           not acceptable
    * @param subject
    *           Subject of the e-mail message. <code>null</code> values are not
    *           acceptable
    * @param content
    *           Content of the e-mail message. <code>null</code> values are not
    *           acceptable
    *
    * @deprecated Since SSO 2.0 {@link MailContent} class has been deprecated.
    */
   @Deprecated
   public MailContent(String from, String to, String subject, String content) {

      ValidateUtil.validateNotEmpty(from, "From field");
      ValidateUtil.validateNotEmpty(to, "To field");
      ValidateUtil.validateNotEmpty(subject, "Mail subject");
      ValidateUtil.validateNotNull(content, "Mail content");
      this._from = from;
      this._to = to;
      this._subject = subject;
      this._content = content;
   }

   /**
    * @return The e-mail address of the sender. Not <code>null</code>
    *
    * @deprecated Since SSO 2.0 {@link MailContent} class has been deprecated.
    */
   @Deprecated
   public String getFrom() {
      return _from;
   }

   /**
    * @return The e-mail address of the recipient. Not <code>null</code>
    *
    * @deprecated Since SSO 2.0 {@link MailContent} class has been deprecated.
    */
   @Deprecated
   public String getTo() {
      return _to;
   }

   /**
    * @return The subject of the e-mail message. Not <code>null</code>
    *
    * @deprecated Since SSO 2.0 {@link MailContent} class has been deprecated.
    */
   @Deprecated
   public String getSubject() {
      return _subject;
   }

   /**
    * @return The content of the e-mail message. Not <code>null</code>
    *
    * @deprecated Since SSO 2.0 {@link MailContent} class has been deprecated.
    */
   @Deprecated
   public String getContent() {
      return _content;
   }
}
