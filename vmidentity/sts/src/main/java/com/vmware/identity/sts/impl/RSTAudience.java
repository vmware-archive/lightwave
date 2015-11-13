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
package com.vmware.identity.sts.impl;

import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;

import org.oasis_open.docs.ws_sx.ws_trust._200512.ParticipantType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.w3._2005._08.addressing.EndpointReferenceType;

/**
 * Abstraction that provides RST:Participants as Iterator for the sake of
 * building token specification with no intermediate and extra copy into a set.
 */
final class RSTAudience implements Iterable<String> {

    // WS-TRUST-1.4 spec:
    //<wst:Participants>
    //    <wst:Primary>...</wst:Primary>
    //    <wst:Participant>...</wst:Participant>
    //</wst:Participants>
    //
    ///wst:RequestSecurityToken/wst:Participants/
    //    This OPTIONAL element specifies the participants sharing the security token.
    //    Arbitrary types MAY be used to specify participants, but a typical case is a
    //    security token or an endpoint reference (see [WS-Addressing]).
    //
    ///wst:RequestSecurityToken/wst:Participants/wst:Primary
    //    This OPTIONAL element specifies the primary user of the token (if one exists).
    //
    ///wst:RequestSecurityToken/wst:Participants/wst:Participant
    //    This OPTIONAL element specifies participant (or multiple participants by repeating the element)
    //    that play a (profile-dependent) role in the use of the token or who are allowed to use the token.

   private final List<ParticipantType> participants;
   private final ParticipantType primary;

   RSTAudience(RequestSecurityTokenType rst) {
      assert rst != null;

      if(rst.getParticipants() != null){
          this.participants = rst.getParticipants().getParticipant();
          this.primary = rst.getParticipants().getPrimary();
      } else {
          this.participants = null;
          this.primary = null;
      }
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public Iterator<String> iterator() {
      return new Iterator<String>() {
         private int index = ((primary != null) ? -1 : 0);
         private final int numberOfParticipants =
             ((participants != null) ? participants.size() : 0 );

         @Override
         public void remove() {
            throw new UnsupportedOperationException();

         }

         @Override
         public String next() {
            if (!hasNext()) {
               throw new NoSuchElementException();
            }

            EndpointReferenceType endpointReference = null;
            assert index < numberOfParticipants;
            if ( index == -1 ){
                endpointReference = primary.getEndpointReference();
            } else {
                endpointReference = participants.get( index ).getEndpointReference();
            }
            index++;
            assert endpointReference != null
                    && endpointReference.getAddress() != null : "WST schema has been changed or validation has been skipped!";

            return endpointReference.getAddress().getValue();
         }

         @Override
         public boolean hasNext() {
            return index < numberOfParticipants;
         }
      };
   }
}
