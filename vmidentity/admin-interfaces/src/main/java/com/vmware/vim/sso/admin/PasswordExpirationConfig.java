/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.util.Arrays;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * A password expiration configuration is a set of adjustments defining the
 * server's behavior regarding passwords which are about to expire. The
 * granularity is one e-mails per day.
 */
public final class PasswordExpirationConfig {

   private final boolean _isEmailNotificationEnabled;

   private final String _emailFrom;

   private final String _emailSubject;

   private final int[] _notificationDays;

   /**
    * Create a password expiration configuration.
    *
    * @param isEmailNotificationEnabled
    *           Whether the password expiration notification option is enabled
    * @param emailFrom
    *           The email from which to sent e-mail notifications.
    *           <code>null</code> if notification is disabled
    * @param emailSubject
    *           The email subject. <code>null</code> if notification is disabled
    * @param notificationDays
    *           The number of days before to send reminder email with password
    *           expiration information. Multiple days are accepted because
    *           repeated notifications are possible. Only positive numbers are
    *           accepted. TODO (check) Order is not important. <code>null</code>
    *           if notification is disabled
    */
   private PasswordExpirationConfig(boolean isEmailNotificationEnabled,
      String emailFrom, String emailSubject, int[] notificationDays) {

      _isEmailNotificationEnabled = isEmailNotificationEnabled;
      _emailFrom = emailFrom;
      _emailSubject = emailSubject;
      _notificationDays = notificationDays;
   }

   /**
    * @return Whether the password expiration notification option is enabled
    */
   public boolean isEmailNotificationEnabled() {
      return _isEmailNotificationEnabled;
   }

   /**
    * @return The email from which to sent e-mail notifications or
    *         <code>null</code>
    */
   public String getEmailFrom() {
      return _emailFrom;
   }

   /**
    * @return The email subject or <code>null</code>
    */
   public String getEmailSubject() {
      return _emailSubject;
   }

   /**
    * @return The number of days to take action for notification before password
    *         is expired. Repeated notifications are possible that's the reason
    *         for multiple days parameter. <code>null</code> if still not
    *         configured
    */
   public int[] getNotificationDays() {
      return _notificationDays;
   }

   /**
    * @return {@link PasswordExpirationConfig} with disabled password expiration
    *         notification notification
    */
   public static PasswordExpirationConfig createNotificationDisabledConfig() {
      return new PasswordExpirationConfig(false, null, null, null);
   }

   /**
    * Creates a {@link PasswordExpirationConfig} with enabled password
    * expiration notification.
    *
    * @param emailFrom
    *           The email from which to sent e-mail notifications. Cannot be
    *           <code>null</code> or empty.
    * @param emailSubject
    *           The email subject. Cannot be <code>null</code> or empty.
    * @param notificationDays
    *           The number of days before password expiration to send reminder
    *           email. Multiple days are accepted because repeated notifications
    *           are possible. Only positive numbers are accepted. Cannot be
    *           <code>null</code> or empty.
    * @return the config object created
    */
   public static PasswordExpirationConfig createNotificationEnabledConfig(
      String emailFrom, String emailSubject, int[] notificationDays) {
      ValidateUtil.validateNotEmpty(emailFrom, "Email address");
      ValidateUtil.validateNotEmpty(emailSubject, "Email subject");
      ValidateUtil.validateNotNull(notificationDays, "Notification days");
      ValidateUtil.validatePositiveNumber(notificationDays.length,
         "Notification days count");
      for (int day : notificationDays) {
         ValidateUtil.validatePositiveNumber(day, "Notification day");
      }

      return new PasswordExpirationConfig(true, emailFrom, emailSubject, Arrays
         .copyOf(notificationDays, notificationDays.length));
   }
}
