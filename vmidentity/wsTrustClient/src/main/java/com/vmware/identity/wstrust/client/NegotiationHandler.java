/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client;

/**
 * This interface provides means to participate in the negotiation process
 * between initiator (client) and acceptor (server) in order to establish
 * security context.
 */
public interface NegotiationHandler {

    /**
     * Returns the next authentication leg, which the client passes to
     * GSSContext.initSecContext. The procedure is repeated either until the GSS
     * Context is successfully established or any exception during negotiation
     * is thrown.
     *
     * @param leg
     *            this leg is generated from the acceptor and is a result of
     *            client interaction. The first leg is <code>null</code> giving
     *            the initiator chance to start the negotiation process
     *
     * @return leg representing the initiator's iteration, or <code>null</code>
     *         indicating successfully established security context
     */
    byte[] negotiate(byte[] leg);
}