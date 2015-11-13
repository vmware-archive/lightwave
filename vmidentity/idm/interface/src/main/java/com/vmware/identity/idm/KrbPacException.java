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
 * This exception is thrown when a principalId(person, group) specified in
 * a find operation doesn't exists in the identity store.
 */
public class KrbPacException extends IDMException {

    /**
	 *
	 */
	private static final long serialVersionUID = 8894161037539899217L;

	public KrbPacException(String message) {
        super(message);
        // TODO Auto-generated constructor stub
    }

    public KrbPacException(Throwable ex) {
        super(ex);
        // TODO Auto-generated constructor stub
    }

    public KrbPacException(String message, Throwable ex) {
        super(message, ex);
    }
}
