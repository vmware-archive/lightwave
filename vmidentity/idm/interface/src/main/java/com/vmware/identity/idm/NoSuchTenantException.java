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
 * This exception is thrown when a tenant with the given name does not exist. */
public class NoSuchTenantException extends IDMException {

	/**
     * Serial version id
     */

	private static final long serialVersionUID = -3956038771966875281L;


    public NoSuchTenantException(String message) {
        super(message);
    }

    public NoSuchTenantException(Throwable ex) {
        super(ex);
    }

    public NoSuchTenantException(String message, Throwable ex) {
        super(message, ex);
    }
}
