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
package com.vmware.identity.saml.ext;

import javax.xml.namespace.QName;

import org.opensaml.saml2.core.Condition;

/**
 * Renewable condition
 *
 */
public interface RenewableType extends Condition {
    public static final String TYPE_LOCAL_NAME = "RenewableType";
    public static final QName TYPE_NAME = new QName(
       "http://vmware.com/schemas/attr-names/2012/04/Extensions", TYPE_LOCAL_NAME,
       "vmes");
}
