/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.samlservice;

import java.io.IOException;

import org.w3c.dom.Document;

/**
 * @author schai
 *
 */
public interface SAMLResponseSender {
    /**
     * Send SAML response to relying party.
     * @param spEntId   could be null
     * @param tokenDoc  could be null
     * @throws IOException
     */
    public void sendResponseToRP( String spEntId, Document tokenDoc) throws IOException;

    /**
     *
     * @param rpID  Relying party entity ID.  Must not be null
     * @return
     */
    public Document generateTokenForResponse(String rpID);

}
