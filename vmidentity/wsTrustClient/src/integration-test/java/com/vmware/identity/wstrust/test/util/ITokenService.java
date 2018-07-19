/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.wstrust.test.util;

/**
 * Interface to abstract the details of sync/async/async with handler api's
 * <p>
 * Implementations of this interface handle/validate the async calls and return
 * the same type as the Sync api
 */

import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.NegotiationHandler;
import com.vmware.vim.sso.client.SamlToken;

import java.security.cert.Certificate;

public interface ITokenService {

  public SamlToken acquireToken(String subject,
                                String password,
                                TokenSpec tokenSpec) throws Exception;

  public SamlToken acquireTokenByGSS(TokenSpec tokenSpec,
                                     NegotiationHandler handler) throws Exception;

  public SamlToken acquireTokenByCertificate(Certificate certificate,
                                             TokenSpec tokenSpec) throws Exception;

  public boolean validateToken(SamlToken token) throws Exception;

  public SamlToken renewToken(SamlToken token,
                              long extendTime) throws Exception;

  public SamlToken acquireTokenByToken(SamlToken token,
                                       TokenSpec tokenSpec) throws Exception;

}
