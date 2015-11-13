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
package com.vmware.identity.sts.idm;

import com.vmware.identity.sts.NoSuchIdPException;

/**
 * This interface is responsible for extraction of the STS configuration.
 */
public interface STSConfigExtractor {

   /**
    * @return not null STS configuration
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws SystemException
    */
   STSConfiguration getConfig() throws NoSuchIdPException, SystemException;
}
