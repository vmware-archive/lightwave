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

package com.vmware.identity.idm.server;

import java.security.AlgorithmParameters;
import java.security.SecureRandom;
import java.security.spec.KeySpec;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;

public class CryptoAES {

	private final int    SALT_LENGTH = 8; // bytes
	private final int    NUM_ITERS   = 65536;
	private final int    KEYLEN_BITS = 128;
	
	private final Cipher _cipher;
	private final Cipher _decipher;
	private final byte[] _salt;
	private final byte[] _iv;

	public CryptoAES(String password) throws Exception
	{
		if (password == null || password.length() == 0)
		{
			throw new IllegalArgumentException("An invalid secret key was specified");
		}
		
		_salt = new byte[SALT_LENGTH];
		
		SecureRandom rand = new SecureRandom();
		rand.nextBytes(_salt);
		
		SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
		
		KeySpec spec = new PBEKeySpec(password.toCharArray(), _salt, NUM_ITERS, KEYLEN_BITS);
		
		SecretKey key = factory.generateSecret(spec);
		
		SecretKey secret = new SecretKeySpec(key.getEncoded(), "AES");
        
        _cipher = Cipher.getInstance("AES/CBC/PKCS5Padding", "SunJCE");
        
        _cipher.init(Cipher.ENCRYPT_MODE, secret);
        
        AlgorithmParameters params = _cipher.getParameters();
        _iv = params.getParameterSpec(IvParameterSpec.class).getIV();

        _decipher = Cipher.getInstance("AES/CBC/PKCS5Padding", "SunJCE");
        
        _decipher.init(Cipher.DECRYPT_MODE,  secret, new IvParameterSpec(_iv));
	}
	
	public byte[] encrypt(String message) throws Exception
	{
		return (message != null) ?
				_cipher.doFinal(message.getBytes("UTF-8")) : null;
	}
	
	public String decrypt(byte[] message) throws Exception
	{
		return (message != null) ? 
				new String(_decipher.doFinal(message), "UTF-8") : null;
	}
}

