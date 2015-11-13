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
import java.security.cert.X509Certificate;

import org.apache.commons.lang.Validate;

/**
 * Insert your comment for SolutionUser here
 */

public final class SolutionUser extends Principal implements Serializable
{
   private static final long serialVersionUID = -313384458621280990L;

   private final SolutionDetail detail;
   private final boolean disabled;
   private final boolean external;

   /**
    * Constructs solution user type of principal.
    *
    * @param id
    *           user id; {@code not-null} value is required
    * @param details
    *           user details; {@code not-null} value is required
    * @param disabled
    *           whether the user is disabled
    * Retrieves the common name of the solution user
    *
    * @return Common Name of the service principal/solution user
    */

   public SolutionUser(PrincipalId id, SolutionDetail detail, boolean disabled)
   {
	   // No alias
	   this(id, null /*no alias*/,null /*no objectSid*/, detail, disabled);
  }

   @Deprecated
   public
   SolutionUser(
        PrincipalId    id,
        PrincipalId    alias,
        String         objectSid,
        SolutionDetail detail,
        boolean        disabled
        )
   {
      this(id, alias, objectSid, detail, disabled, false /* internal user */);
   }

   /**
    * Constructs solution user type of principal.
    *
    * @param id
    *           principal id; {@code not-null} value is required
    * @param alias
    *           principal alias; {@code null} value when alias is not known or
    *           the corresponding domain has no alias specified; note that the
    *           alias should not be equal to the ID
    * @param objectSid
    *           principal objectSid; {@code null} value when objectSid is an existing
    *           attribute in the identity provider
    * @param details
    *           solution user detail; {@code not-null} value is required
    * @param disabled
    *           whether the user is disabled
    * @param external
    *           whether the user is external
    * Sets the common name of the solution user
    *
    * @param name Common Name of service principal/solution user
    */
   public
   SolutionUser(
        PrincipalId    id,
        PrincipalId    alias,
        String         objectSid,
        SolutionDetail detail,
        boolean        disabled,
        boolean        external
        )
   {
      super(id, alias, objectSid);

      Validate.notNull(detail, "Null SolutionDetail");
      Validate.notNull(id, "Null PrincipalId");
      this.detail = detail;
      this.disabled = disabled;
      this.external = external;
   }

   public SolutionDetail getDetail()
   {
      return this.detail;
   }

   /**
    * Specified whether the user is disabled.
    * <p>
    * Disabled users cannot acquire tokens from STS and their existing tokens
    * cannot be validated.
    *
    * @return whether the user is disabled
    */
   public boolean isDisabled()
   {
      return this.disabled;
   }

   /**
    * Specifies whether the user is external.
    * @return whether the user is external
    */
   public boolean isExternal() {
      return this.external;
   }

   /**
    * @param id
    *           the id to set
    */
   public void setId(PrincipalId id)
   {
      Validate.notNull(id, "Null PrincipalId");
      this.id = id;
   }

   /**
    * @return the certificate
    */
   public X509Certificate getCert()
   {
       return this.detail.getCertificate();
    }

   /**
    * @param cert
    *           the cert to set
    */
   public void setCert(X509Certificate cert)
   {
      if (this.detail != null)
      {
          this.detail.setCertificate(cert);
      }
   }

   /**
    * @param alias
    *        service principal's alias. For instance, VMWARE\joe
    */
    public void setAlias(PrincipalId alias)
    {
        this.alias = alias;
    }
}
