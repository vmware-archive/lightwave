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

package com.vmware.identity.sts.util;

import javax.xml.bind.JAXBElement;

import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;

public class JAXBExtractor {

    @SuppressWarnings("unchecked")
    public static <T> T extractFromSecurityHeader(SecurityHeaderType securityHeader, Class<T> c) {
        assert securityHeader != null;

        // only extract one occurrence of a specified class value.
        for (Object o : securityHeader.getAny()) {
            if (o instanceof JAXBElement<?>) {
                JAXBElement<?> jaxb = (JAXBElement<?>) o;
                if (jaxb.getDeclaredType().equals(c)) {
                    return (T) jaxb.getValue();
                }
            }
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    public static <T> T extractFromUsernameToken(UsernameTokenType usernameToken, Class<T> c) {
        assert usernameToken != null;

        // only extract one occurrence of a specified class value.
        for (Object o : usernameToken.getAny()) {
            if (o instanceof JAXBElement<?>) {
                JAXBElement<?> jaxb = (JAXBElement<?>) o;
                if (jaxb.getDeclaredType().equals(c)) {
                    return (T) jaxb.getValue();
                }
            }
        }
        return null;
    }
}
