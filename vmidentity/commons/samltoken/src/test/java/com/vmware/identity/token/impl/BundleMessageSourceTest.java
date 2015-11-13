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
package com.vmware.identity.token.impl;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import java.util.Collection;
import java.util.Locale;

import org.junit.Test;

import com.vmware.vim.sso.client.BundleMessageSource;
import com.vmware.vim.sso.client.BundleMessageSource.Key;
import com.vmware.vim.sso.client.exception.InvalidTimingException;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.exception.MalformedTokenException;

public class BundleMessageSourceTest {

   @Test
   public void testAllStandardMessages() {
      Locale missing = Locale.CANADA;
      // We use the armenian locale for partial l10n test, so we are
      // sure that such translation doesn't exist on the main classpath
      Locale partial = new Locale("am");
      Collection<Locale> locales = Arrays.asList(Locale.getDefault(), missing,
         partial);
      for (Locale locale : locales) {
         BundleMessageSource bundle = new BundleMessageSource(locale);
         for (Key messageKey : Key.values()) {
            assertNotNull(bundle.get(messageKey));
         }
      }
   }

   @Test
   public void testCreateMessageForException() {
      InvalidTokenException exception = new InvalidTimingException("",
         Key.INVALID_SAML_TOKEN, null, 111);
      BundleMessageSource messageSource = new BundleMessageSource(
         Locale.getDefault());
      String msg = messageSource.createMessage(exception);
      assertTrue(msg.contains("111"));
   }

   @Test
   public void testCreateMessageWithLocalizableCause() {
      InvalidTokenException exception = new MalformedTokenException("",
         Key.INVALID_SAML_TOKEN, null, Key.MALFORMED_SAML_TOKEN);
      BundleMessageSource messageSource = new BundleMessageSource(
         Locale.getDefault());
      String msg = messageSource.createMessage(exception);
      assertTrue(msg.contains(messageSource.get(Key.MALFORMED_SAML_TOKEN)));
   }

   @Test
   public void testDifferentMessagesForDifferentLocales() {
      BundleMessageSource defMessages = new BundleMessageSource(
         Locale.getDefault());
      BundleMessageSource otherLangMessages = new BundleMessageSource(
         new Locale("am"));
      assertTrue(!defMessages.get(Key.MALFORMED_SAML_TOKEN).equals(
         otherLangMessages.get(Key.MALFORMED_SAML_TOKEN)));
   }

}
