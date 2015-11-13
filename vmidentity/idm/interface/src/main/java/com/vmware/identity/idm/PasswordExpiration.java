/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.io.Serializable;

/**
 * A password expiration configuration is a set of adjustments defining the
 * server's behavior regarding passwords which are about to expire. The
 * granularity is one e-mails per day.
 */
public final class PasswordExpiration implements Serializable {

    //Password expiration notification default settings
    public static final boolean PWD_EXP_EMAIL_ENABLED = true;
    public static final String  PWD_EXP_EMAIL_FROM = "someone@example.com";
    public static final String  PWD_EXP_EMAIL_SUBJECT = "[PASSWORD] Your password is about to expire";
    public static final int[]   PWD_EXP_EMAIL_DAYS = {10, 5, 3, 2, 1};

    /**
     * Serial version id
     */
    private static final long serialVersionUID = 4584523381767840799L;

    private final boolean isEmailNotificationEnabled;

    private final String emailFrom;

    private final String emailSubject;

    private final int[] notificationDays;

    /**
     * Create a password expiration configuration.
     *
     * @param isNotificationEnabled
     *            Whether the password expiration notification option is enabled
     * @param fromAddr
     *            The email from which to sent e-mail notifications.
     *            <code>null</code> if notification is disabled
     * @param subject
     *            The email subject. <code>null</code> if notification is
     *            disabled
     * @param notificationTime
     *            The number of days before to send reminder email with password
     *            expiration information. Multiple days are accepted because
     *            repeated notifications are possible. Only positive numbers are
     *            accepted. TODO (check) Order is not important.
     *            <code>null</code> if notification is disabled
     */
    public PasswordExpiration(boolean isNotificationEnabled, String fromAddr,
            String subject, int[] notificationTime) {

        this.isEmailNotificationEnabled = isNotificationEnabled;
        this.emailFrom = fromAddr;
        this.emailSubject = subject;
        this.notificationDays = notificationTime;
    }

    /**
     * @return Whether the password expiration notification option is enabled
     */
    public boolean isEmailNotificationEnabled() {
        return isEmailNotificationEnabled;
    }

    /**
     * @return The email from which to sent e-mail notifications or
     *         <code>null</code>
     */
    public String getEmailFrom() {
        return emailFrom;
    }

    /**
     * @return The email subject or <code>null</code>
     */
    public String getEmailSubject() {
        return emailSubject;
    }

    /**
     * @return The number of days to take action for notification before
     *         password is expired. Repeated notifications are possible that's
     *         the reason for multiple days parameter. <code>null</code> if
     *         still not configured
     */
    public int[] getNotificationDays() {
        return notificationDays;
    }

    public static PasswordExpiration createDefaultSettings()
    {
      return new PasswordExpiration(PWD_EXP_EMAIL_ENABLED, PWD_EXP_EMAIL_FROM,
            PWD_EXP_EMAIL_SUBJECT, PWD_EXP_EMAIL_DAYS);
    }
}
