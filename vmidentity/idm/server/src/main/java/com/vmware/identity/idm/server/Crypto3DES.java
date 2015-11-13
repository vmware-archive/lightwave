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

import java.security.MessageDigest;
import java.util.Arrays;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class Crypto3DES {
	
	private final Cipher _cipher;
	private final Cipher _decipher;

	public Crypto3DES(String secret) throws Exception
	{
		if (secret == null)
		{
			throw new IllegalArgumentException("An invalid secret key was specified");
		}
		
		MessageDigest md = MessageDigest.getInstance("md5");
		
        byte[] digestOfPassword = md.digest(secret.getBytes("UTF-8"));
        
        byte[] keyBytes = Arrays.copyOf(digestOfPassword, 24);
        
        for (int j = 0, k = 16; j < 8;) {
                keyBytes[k++] = keyBytes[j++];
        }
        
        SecretKey key = new SecretKeySpec(keyBytes, "DESede");
        
        IvParameterSpec iv = new IvParameterSpec(new byte[8]);
        
        _cipher = Cipher.getInstance("DESede/CBC/PKCS5Padding");
        
        _cipher.init(Cipher.ENCRYPT_MODE, key, iv);
        
        _decipher = Cipher.getInstance("DESede/CBC/PKCS5Padding");
        
        _decipher.init(Cipher.DECRYPT_MODE, key, iv);
	}
	
	public byte[] encrypt(String message) throws Exception
	{
		return (message != null) ?
				_cipher.doFinal( message.getBytes("UTF-8")) : null;
	}
	
	public String decrypt(byte[] message) throws Exception
	{
		return (message != null) ? 
					new String(_decipher.doFinal(message), "UTF-8") : null;
	}
}
