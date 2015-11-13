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
 * This exception is thrown when extraneous certificates are presented in the
 * certificate chain during ExternalIDP configuration setup.
 *
 */
public class ExternalIDPExtraneousCertsInCertChainException extends
        IDMException
{

    private static final long serialVersionUID = -2568744582589078475L;

    List<X509Certificate> chain;

    public ExternalIDPExtraneousCertsInCertChainException(
            List<X509Certificate> aChain)
    {
        super(ExternalIDPExtraneousCertsInCertChainException.class.getName());
        this.chain = aChain;
    }

    public ExternalIDPExtraneousCertsInCertChainException(Throwable t,
            List<X509Certificate> aChain)
    {
        super(t);
        this.chain = aChain;
    }
}
