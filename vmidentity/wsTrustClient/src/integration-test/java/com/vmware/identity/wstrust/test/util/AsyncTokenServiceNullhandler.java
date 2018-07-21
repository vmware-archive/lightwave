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

import java.security.cert.Certificate;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

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

public class AsyncTokenServiceNullhandler implements ITokenService {

	private SecurityTokenService _stsService = null;
	private static final Logger _log = LoggerFactory
			.getLogger(AsyncTokenServiceNullhandler.class);

	public AsyncTokenServiceNullhandler(SecurityTokenService stsService) {
		_stsService = stsService;
	}

	@Override
	public SamlToken acquireToken(String subject, String password,
			TokenSpec tokenSpec) throws SsoException, InterruptedException,
			ExecutionException {
		AsyncResultValidator<SamlToken> callback = new AsyncResultValidator<SamlToken>(
				false);
		Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
                                        new UsernamePasswordCredential(subject, password),
                                        tokenSpec,
                                        callback
                                    );
		_log.debug("Retrieved future<samltoken> object - acquireTokenAsync with null handler");
		SamlToken token = tokenFuture.get();
		_log.debug("Retrieved token successfully");
		callback.verifyCorrectExecution();
		return token;
	}

	@Override
	public SamlToken acquireTokenByGSS(TokenSpec tokenSpec,
			NegotiationHandler handler) throws SsoException,
			InterruptedException, ExecutionException {
		AsyncResultValidator<SamlToken> callback = new AsyncResultValidator<SamlToken>(
				false);
		Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
				new GSSCredential(handler), tokenSpec, callback);
		_log.debug("Retrieved future<samltoken> object - acquireTokenByGSSAsync with null handler");
		SamlToken token = tokenFuture.get();
		_log.debug("Retrieved token successfully");
		callback.verifyCorrectExecution();
		return token;
	}

	@Override
	public SamlToken acquireTokenByCertificate(Certificate certificate, TokenSpec tokenSpec)
			throws SsoException, InterruptedException, ExecutionException {
		AsyncResultValidator<SamlToken> callback = new AsyncResultValidator<SamlToken>(
				false);
		Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
                                        new CertificateCredential(certificate),
                                        tokenSpec,
                                        callback
                                    );
		_log.debug("Retrieved future<samltoken> object - acquireTokenByCertificateAsync with null handler");
		SamlToken token = tokenFuture.get();
		_log.debug("Retrieved token successfully");
		callback.verifyCorrectExecution();
		return token;
	}

	@Override
	public boolean validateToken(SamlToken token) throws SsoException,
			InterruptedException, ExecutionException {

		AsyncResultValidator<Boolean> callback = new AsyncResultValidator<Boolean>(
				false);
		Future<Boolean> tokenFuture = _stsService.validateTokenAsync(token,
				callback);
		_log.debug("Retrieved future<boolean> object - validateToken with null handler");
		boolean valid = tokenFuture.get().booleanValue();
		_log.debug("Validated token - Verifying correct callback execution");
		callback.verifyCorrectExecution();
		return valid;
	}

	@Override
	public SamlToken renewToken(SamlToken token, long extendTime)
			throws SsoException, InterruptedException, ExecutionException {
		AsyncResultValidator<SamlToken> callback = new AsyncResultValidator<SamlToken>(
				false);
		Future<SamlToken> tokenFuture = _stsService.renewTokenAsync(token,
				extendTime, callback);
		_log.debug("Retrieved future<samltoken> object - renewTokenAsync with null handler");
		SamlToken tokenRenew = tokenFuture.get();
		_log.debug("Retrieved renewed token successfully - verify EmptyHandler callback");
		callback.verifyCorrectExecution();
		return tokenRenew;
	}

	@Override
	public SamlToken acquireTokenByToken(SamlToken token, TokenSpec tokenSpec)
			throws SsoException, InterruptedException, ExecutionException {
		AsyncResultValidator<SamlToken> callback = new AsyncResultValidator<SamlToken>(
				false);
		Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
                                        new TokenCredential(token),
                                        tokenSpec,
                                        callback
                                    );
		_log.debug("Retrieved future<samltoken> object - acquireTokenByTokenAsync with null handler");
		SamlToken tokenRenew = tokenFuture.get();
		_log.debug("Retrieved token by token successfully - verify EmptyHandler callback");
		callback.verifyCorrectExecution();
		return tokenRenew;
	}
}
