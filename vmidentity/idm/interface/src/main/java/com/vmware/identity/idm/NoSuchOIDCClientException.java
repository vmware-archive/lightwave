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

public class NoSuchOIDCClientException extends IDMException {

    private static final long serialVersionUID = 7405376636121986185L;

    public NoSuchOIDCClientException(String message) {
        super(message);
    }

    public NoSuchOIDCClientException(Throwable ex) {
        super(ex);
    }

    public NoSuchOIDCClientException(String message, Throwable ex) {
        super(message, ex);
    }
}