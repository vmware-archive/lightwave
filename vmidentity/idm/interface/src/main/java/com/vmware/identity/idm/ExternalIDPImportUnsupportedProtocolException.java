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

/**
 * This exception is throws when the protocolSupportEnumeration node of the
 * configuration document does not contain the required
 * urn:oasis:names:tc:SAML:2.0:protocol
 *
 */
public class ExternalIDPImportUnsupportedProtocolException extends IDMException
{

    private static final long serialVersionUID = 6645504261086373549L;

    String invalidProtocolName;

    public ExternalIDPImportUnsupportedProtocolException(String message,
            String protocolName)
    {
        super(ExternalIDPImportUnsupportedProtocolException.class.getName());
        invalidProtocolName = protocolName;
    }

    public ExternalIDPImportUnsupportedProtocolException(Throwable t,
            String protocolName)
    {
        super(t);
        invalidProtocolName = protocolName;
    }
}
