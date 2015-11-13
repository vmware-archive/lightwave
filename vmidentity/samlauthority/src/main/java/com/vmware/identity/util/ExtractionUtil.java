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

package com.vmware.identity.util;

import org.apache.commons.lang.Validate;

import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.SamlTokenSpec;

public final class ExtractionUtil
{
    public static final String extractDomainNameFromTokenSpec(SamlTokenSpec spec) {
        Validate.notNull(spec);

        return spec.getAuthenticationData().getPrincipalId().getDomain();
    }

    public static final String extractDomainNameFromSamlToken(ServerValidatableSamlToken token) {
        Validate.notNull(token);

        return ((token.getSubject().subjectUpn() != null) ? token.getSubject().subjectUpn().getDomain() : "");
    }
}
