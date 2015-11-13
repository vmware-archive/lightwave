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

import java.security.cert.X509Certificate;
import java.util.List;

/**
 * This exception will be thrown when there is no trusted path found in the
 * certificate chain during the external IDP configuration setup. The
 * chain for the external IDP configuration should have the certificates in
 * correct order where user's signing certificate first and root CA certificate
 * last, with trusted path anchored at the root CA certificate.
 *
 */
public class ExternalIDPCertChainInvalidTrustedPathException extends
        IDMException
{

    private static final long serialVersionUID = 331821025761652914L;

    List<X509Certificate> chain;

    public ExternalIDPCertChainInvalidTrustedPathException(String message,
            List<X509Certificate> chain)
    {
        super(message);
        this.chain = chain;
    }

    public ExternalIDPCertChainInvalidTrustedPathException(Throwable t,
            List<X509Certificate> chain)
    {
        super(t);
        this.chain = chain;
    }

    public ExternalIDPCertChainInvalidTrustedPathException(String message,
            Throwable t, List<X509Certificate> chain)
    {
        super(message, t);
        this.chain = chain;
    }
}
