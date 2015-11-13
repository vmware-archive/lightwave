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
 * Insert your comment for GSSResult here
 */
public final class GSSResult implements Serializable {

   private static final long serialVersionUID = -8263509542129261698L;

   private final String contextId;
   private final PrincipalId principalId;
   private final byte[] serverLeg;

   /**
    * Creates a result that indicates for not yet established GSS security
    * context. Typically, such result is returned when gss_accept_sec_context()
    * returns GSS_S_CONTINUE_NEEDED.
    *
    * @param serverLeg
    *           raw content of the server leg, required and not empty
    */
   public GSSResult(String contextId, byte[] serverLeg) {
      assert serverLeg != null && serverLeg.length > 0;

      this.principalId = null;
      this.contextId = contextId;
      this.serverLeg = serverLeg;
   }

   /**
    * Creates a result that indicates for already established GSS security
    * context and hence, for authenticated principal. Although there might be a
    * final server leg that has to be transmitted back to the initiator, we will
    * not support it.
    *
    * @param principalId
    *           authenticated principal, required
    */
   public GSSResult(String contextId, PrincipalId principalId) {
      assert principalId != null;

      this.principalId = principalId;
      this.contextId = contextId;
      this.serverLeg = null;
   }

   /**
    * Returns whether the security context establishment is completed.
    *
    * @return whether the security context establishment is completed
    */
   public boolean complete() {
      return getPrincipalId() != null;
   }

   /**
    * @return the principalId; not null if complete()
    */
   public PrincipalId getPrincipalId() {
      return principalId;
   }

   /**
    * @return the serverLeg; not null if !complete()
    */
   public byte[] getServerLeg() {
      return serverLeg;
   }

   /**
    * @return the context id associated with this session on the server
    * @return
    */
   public String getContextId() {
       return contextId;
   }
}
