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
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 07/02/12
 * Time: 11:59 am
 */
public enum HashingAlgorithmType {

    // can enumerate more hashing algorithm when it is supported in vmware-directory
    SHA512   ("vmdird-digest"), // vmware-directory default password schema
    SHA256_5 ("SSO-v1-1"), // One iteration SHA256 hash with 5 bytes pre-salt.
    SHA256_0 ("SSO-v1-2"); // One iteration SHA256 hash without pre-salt.

    private String _hashAlgorithm;

    private HashingAlgorithmType(String hashingAlgorithm)
    {
        ValidateUtil.validateNotEmpty(hashingAlgorithm, "Algorithm name");
        _hashAlgorithm = hashingAlgorithm;
    }

    public String getHashAlgorithm()
    {
        return _hashAlgorithm;
    }

    private static final HashingAlgorithmType[] copyOfValues = values();

    public static boolean isValidHashingAlgorithm(String algorithmName) {
        for (HashingAlgorithmType value : copyOfValues) {
            if (value._hashAlgorithm.equals(algorithmName)) {
                return true;
            }
        }
        return false;
    }
}
