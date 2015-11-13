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

import org.apache.commons.lang.ObjectUtils;

public class PersonDetail extends PrincipalDetail implements Serializable{

    private static final long serialVersionUID = -8624396765137448824L;
    public static final long UNSPECIFIED_TS_VALUE = 0;
    public static final long UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE = 0;
    public static final long UNSPECIFIED_LOCKOUT_TIME_VALUE = 0;
    /**
     * Email address; optional field
     */
    private final String emailAddress;

    private final String userPrincipalName;

    /**
     * User first name; optional field
     */
    private final String firstName;

    /**
     * User last name; optional field
     */
    private final String lastName;

    /**
     * Timestamp for user password last reset value is in second since unix epoch
     * time 00:00:00 UTC on 1 January 1970 (or 1970-01-01T00:00:00Z ISO 8601)
     * <p>
     * Optional value with default to be {@code UNSPECIFIED_TS_VALUE}
     */
    private final long pwdLastSet;

    /**
     * value of password life time in seconds as determined from password policy
     * <p>
     * Optional value with default to be {@code UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE}
     */
    private final long pwdLifeTime;

    public static class Builder {
        private String description;
        private String emailAddress;
        private String userPrincipalName;
        private String firstName;
        private String lastName;
        private long   pwdLastSet = UNSPECIFIED_TS_VALUE;
        private long   pwdLifeTime  = UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;

        public Builder description(String aDescription)
        {
            description = aDescription;
            return this;
        }

        public Builder emailAddress(String aEmailAddress)
        {
            emailAddress = aEmailAddress;
            return this;
        }

        public Builder userPrincipalName(String aPrincipalName)
        {
            userPrincipalName = aPrincipalName;
            return this;
        }

        public Builder firstName(String aFirstName)
        {
            firstName = aFirstName;
            return this;
        }

        public Builder lastName(String aLastName)
        {
            lastName = aLastName;
            return this;
        }

        public Builder pwdLastSet(long aPwdLastSet, IdentityStoreType storeType)
        {
            switch (storeType)
            {
            case IDENTITY_STORE_TYPE_VMWARE_DIRECTORY:
            case IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY:
            case IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING:
                pwdLastSet = aPwdLastSet;
                break;
            default:
                throw new UnsupportedOperationException(
                        String.format("pwdLastSet not supported for %s", storeType));
            }
            return this;
        }

        /**
         *
         * @param aPwdLifeTime  password life time in provider-specific format
         * @param storeType
         * @return
         */
        public Builder pwdLifeTime(long aPwdLifeTime, IdentityStoreType storeType)
        {
            switch (storeType)
            {
            case IDENTITY_STORE_TYPE_VMWARE_DIRECTORY:
            case IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY:
            case IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING:
                pwdLifeTime = aPwdLifeTime;
                break;
            default:
                throw new UnsupportedOperationException(
                        String.format("maxPwdAge not supported for %s", storeType));
            }
            return this;
        }

        public PersonDetail build()
        {
            return new PersonDetail(this);
        }
    }

    private PersonDetail(Builder builder)
    {
        super(builder.description);
        emailAddress = builder.emailAddress;
        userPrincipalName=builder.userPrincipalName;
        firstName    = builder.firstName;
        lastName     = builder.lastName;
        pwdLastSet   = builder.pwdLastSet;
        pwdLifeTime    = builder.pwdLifeTime;
    }

    /**
     * Retrieve user's email address
     *
     * @return the email address or {@code null} if not set
     */
    public String getEmailAddress() {
        return this.emailAddress;
    }

    public String getUserPrincipalName() {
        return this.userPrincipalName;
    }

    /**
     * Retrieve user's first name
     *
     * @return first name or {@code null} if not specified
     */
    public String getFirstName() {
        return this.firstName;
    }

    /**
     * Retrieve user's last name
     *
     * @return last name or {@code null} if not specified
     */
    public String getLastName() {
        return this.lastName;
    }

    /**
     * Retrive user's pwd last timestamp
     * The timestamp is always in seconds from Unix epoch time 01.01.1970UTC
     * @return value in seconds or {@code UNKNOWN_TS_VALUE}
     */
    public long getPwdLastSet()
    {
        return this.pwdLastSet;
    }

    /**
     * Retrive user's pwd life time
     * The timestamp is always in seconds
     * @return value in seconds or {@code UNKNOWN_PASSWORD_LIFE_TIME_VALUE}
     */
    public long getPwdLifeTime()
    {
        return this.pwdLifeTime;
    }

    @Override
    public int hashCode()
    {
       final int prime = 31;
       int result = super.hashCode();
       result = prime * result
          + ((emailAddress == null) ? 0 : emailAddress.hashCode());
       result = prime * result
          + ((firstName == null) ? 0 : firstName.hashCode());
       result = prime * result
               + ((userPrincipalName == null) ? 0 : userPrincipalName.hashCode());
       result = prime * result + ((lastName == null) ? 0 : lastName.hashCode());
       result = prime * result + (int)(pwdLastSet^(pwdLastSet>>>32));
       result = prime * result + (int)(pwdLifeTime^(pwdLifeTime>>>32));
       return result;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (this == obj)
        {
           return true;
        }
        if (obj == null || this.getClass() != obj.getClass())
        {
           return false;
        }

        PersonDetail other = (PersonDetail) obj;
        return ObjectUtils.equals(emailAddress, other.emailAddress)
            && ObjectUtils.equals(firstName, other.firstName)
            && ObjectUtils.equals(lastName, other.lastName)
            && ObjectUtils.equals(userPrincipalName, other.userPrincipalName)
            && pwdLastSet == other.pwdLastSet
            && pwdLifeTime == other.pwdLifeTime;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Object[] getDetailFields() {
        return new Object[] { getDescription(), getEmailAddress(),
                getFirstName(), getLastName(), getPwdLastSet(), getPwdLifeTime()};
    }
}
