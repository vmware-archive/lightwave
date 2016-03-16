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

import javax.crypto.spec.SecretKeySpec;
import javax.crypto.Cipher;
import java.security.SecureRandom;

public class CryptoAESE {
    private SecretKeySpec key;
    private Cipher cipher;
    private Cipher decipher;
    private String cs="ISO-8859-1";

    static public String randomKey(){
        SecureRandom random = new SecureRandom();
        StringBuilder sb=new StringBuilder();
        for(int i=0;i<16;i++)
            sb.append((char)(random.nextInt(96)+32));
        return sb.toString();
    }

    //secret must be 16 bytes
    public CryptoAESE(String secret) throws Exception{
        key = new SecretKeySpec(secret.getBytes(cs), "AES");

        cipher = Cipher.getInstance("AES/ECB/NoPadding", "SunJCE");
        cipher.init(Cipher.ENCRYPT_MODE, key);

        decipher = Cipher.getInstance("AES/ECB/NoPadding", "SunJCE");
        decipher.init(Cipher.DECRYPT_MODE, key);
    }

    public byte[] encrypt(String plaintext) throws Exception{
        StringBuilder sb=new StringBuilder(plaintext);

        //plaintext must be multiple of 16 bytes
        for(int i=0; (plaintext.length()+i)%16!=0; i++) sb.append('\0');
        return cipher.doFinal(sb.toString().getBytes(cs));
    }

    public String decrypt(byte[] ciphertext) throws Exception{
        String str=new String(decipher.doFinal(ciphertext),cs);
        if(str.indexOf('\0')>=0)
            return str.substring(0,str.indexOf('\0'));
        else
            return str;
    }
}