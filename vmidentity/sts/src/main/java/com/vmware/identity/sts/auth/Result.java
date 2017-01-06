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
package com.vmware.identity.sts.auth;

import java.util.Date;

import com.vmware.identity.idm.PrincipalId;

/**
 * Insert your comment for Result here
 */
public final class Result {

   static public enum AuthnMethod {
      PASSWORD, DIG_SIG, KERBEROS, NTLM, ASSERTION, EXTERNAL_ASSERTION, SMARTCARD, TIMESYNCTOKEN
   }

   private final PrincipalId principalId;
   private final Date authInstant;
   private final AuthnMethod authnMethod;
   private final byte[] serverLeg;
   private final String sessionID;

   /**
    * Creates an incomplete authentication result
    *
    * @param serverLeg
    *           not empty server generated token ('challenge') that should be
    *           transported to client's side, required
    */
   public Result(byte[] serverLeg) {
      assert serverLeg != null && serverLeg.length > 0;

      this.principalId = null;
      this.authInstant = null;
      this.authnMethod = null;
      this.serverLeg = serverLeg;
      this.sessionID = null;
   }

   /**
    * Creates a complete authentication result
    *
    * Note, that there might be a final server leg. It is, however,
    * intentionally NOT transported back to the initiator ('client'). This is as
    * specified in SSO1 documents.
    *
    * @param principalId
    * @param authnInstant
    * @param authnMethod
    */
   public Result(PrincipalId principalId, Date authnInstant,
      AuthnMethod authnMethod) {
	   this(principalId, authnInstant, authnMethod, null);
   }
   /**
    * Creates a complete authentication result
    *
    * Note, that there might be a final server leg. It is, however,
    * intentionally NOT transported back to the initiator ('client'). This is as
    * specified in SSO1 documents.
    *
    * @param principalId
    * @param authnInstant
    * @param authnMethod
    * @param serverLeg
    *           not empty server generated token ('challenge') that should be
    *           transported to client's side, required
    */
   public Result(PrincipalId principalId, Date authnInstant,
      AuthnMethod authnMethod, byte[] serverLeg) {
      assert principalId != null;
      assert authnInstant != null;
      assert authnMethod != null;

      this.principalId = principalId;
      this.authInstant = authnInstant;
      this.authnMethod = authnMethod;
      this.serverLeg = serverLeg;
      this.sessionID = null;
   }

   public Result(String sessionID, Date authnInstant) {
      assert sessionID != null;
      assert authnInstant != null;

      this.principalId = null;
      this.authInstant = authnInstant;
      this.authnMethod = AuthnMethod.TIMESYNCTOKEN;
      this.serverLeg = null;
      this.sessionID = sessionID;
   }

   public boolean completed() {

      final boolean done = getPrincipalId() != null;
      assert !done || (getAuthnInstant() != null && getAuthnMethod() != null);
      return done;
   }

   /**
    * @return the principalId; not null if completed().
    */
   public PrincipalId getPrincipalId() {
      return principalId;
   }

   /**
    * @return the authnInstant; not null if completed().
    */
   public Date getAuthnInstant() {
      return authInstant;
   }

   /**
    * @return the authnMethod; not null if completed().
    */
   public AuthnMethod getAuthnMethod() {
      return authnMethod;
   }

   /**
    * @return the serverLeg; not null if !completed().
    */
   public byte[] getServerLeg() {
      return serverLeg;
   }

   public String getSessionID() {
      return sessionID;
   }
}
