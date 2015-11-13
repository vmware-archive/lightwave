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
package com.vmware.vim.sso.client;

/**
 * Describes the token's confirmation type.
 */
public enum ConfirmationType {

   /**
    * Confirmation type indicating that the token does not contain
    * requester-specific confirmation information.
    */
   BEARER,

   /**
    * Confirmation type indicating that the requester's certificate is embedded
    * into the token. This allows the requester to prove its right to use the
    * token by signing a message containing the token using the private key
    * corresponding to the certificate.
    */
   HOLDER_OF_KEY
}
