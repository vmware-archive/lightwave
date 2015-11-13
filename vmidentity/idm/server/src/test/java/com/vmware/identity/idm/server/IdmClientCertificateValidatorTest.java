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
package com.vmware.identity.idm.server;

import static org.junit.Assert.*;

import java.security.KeyStoreException;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.GregorianCalendar;

import org.junit.Test;

import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IdmClientCertificateParsingException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.server.clientcert.IdmClientCertificateValidator;

public class IdmClientCertificateValidatorTest {

    static ClientCertTestUtils testUtil = new ClientCertTestUtils();

    @Test
    public void testUPNExtraction() throws KeyStoreException, IdmClientCertificateParsingException,
                    InvalidPrincipalException, IDMException {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        X509Certificate[] certs = testUtil.getDodValidCert1();
        ClientCertPolicy certPolicy = new ClientCertPolicy();
        certPolicy.setTrustedCAs(certs);
        IdmClientCertificateValidator validator = new IdmClientCertificateValidator(certPolicy);

        String upn = validator.extractUPN(certs[0]);
        assertTrue(upn.equals(testUtil.dodValidCert1UPN));
    }

}
