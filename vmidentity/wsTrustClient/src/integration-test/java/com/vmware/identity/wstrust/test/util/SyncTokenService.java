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

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.CertificateCredential;
import com.vmware.identity.wstrust.client.GSSCredential;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.UsernamePasswordCredential;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.NegotiationHandler;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.vim.sso.client.exception.SsoException;

import java.security.cert.Certificate;

/**
 * Sync implementation of TokenServiceMethod
 */
public class SyncTokenService implements ITokenService {

	/*
	 * Security Token Service instance
	 */
	private SecurityTokenService _stsService = null;

	private static final Logger _log = LoggerFactory.getLogger(TokenServiceFactory.class);

	SyncTokenService(SecurityTokenService stsService) {
		_stsService = stsService;
	}

	/**
	 * Retrieves the SAMLToken using the sync method
	 */
	@Override
	public SamlToken acquireToken(String subject, String password,
			TokenSpec tokenSpec) throws SsoException {
		return _stsService.acquireToken(
		          new UsernamePasswordCredential(subject, password),
              tokenSpec
            );
	}

	/**
	 * Retrieves the SAMLToken using the sync GSS method	 *
	 */
	@Override
	public SamlToken acquireTokenByGSS(TokenSpec tokenSpec,
			NegotiationHandler handler) throws SsoException {

		return _stsService.acquireToken(new GSSCredential(handler), tokenSpec);
	}

	/**
	 * Retrieves the SAMLToken using the sync Token by Certificate api
	 */
	@Override
	public SamlToken acquireTokenByCertificate(Certificate certificate, TokenSpec tokenSpec)
	                 throws SsoException {
		return _stsService.acquireToken(new CertificateCredential(certificate), tokenSpec);
	}

	/**
	 * Validates the SAMLToken using the sync validateToken api
	 */
	@Override
	public boolean validateToken(SamlToken token) throws SsoException {

		return _stsService.validateToken(token);
	}

	/**
	 * Renew a SAML token using sync renewToken api
	 */
	@Override
	public SamlToken renewToken(SamlToken token,
			long extendTime) throws SsoException {
		return _stsService.renewToken(token,extendTime);
	}

	@Override
	public SamlToken acquireTokenByToken(SamlToken token, TokenSpec tokenSpec)
			throws Exception {
		return _stsService.acquireToken(new TokenCredential(token),tokenSpec);
	}

}
