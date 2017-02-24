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
import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

import java.nio.charset.Charset;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.SecureRandom;
import java.util.Arrays;

public class CryptoAESE {
    private static String ISO_8859_1 = "ISO-8859-1";

    private SecretKeySpec key;
    private Cipher cipher;
    private Cipher decipher;
    private Charset charset;

    static public String randomKey() {
        SecureRandom random = new SecureRandom();
        StringBuilder sb = new StringBuilder();

        for(int i = 0; i < 16; i++) {
            sb.append((char)(random.nextInt(96) + 32));
        }

        return sb.toString();
    }

    /**
     * Create a new Crypto primitive using the ISO-8859-1 character set
     * @param secret secret key (must be 16 bytes)
     * @throws Exception
     */
    public CryptoAESE(String secret) throws Exception {
        this(secret, Charset.forName(ISO_8859_1));
    }

    /**
     * Create a new crypto primitive using a supplied character set
     *
     * @param secret secret key (must be at least 16 bytes in the given character set)
     * @param charset
     * @throws NoSuchAlgorithmException if a CipherSpi implementation for
     *  the specified algorithm is not available from the specified provider.
     * @throws NoSuchProviderException if the specified provider is not
     *  registered in the security provider list.
     * @throws NoSuchPaddingException if the padding scheme is not available.
     * @throws InvalidKeyException if the given key is inappropriate for
     *  initializing this cipher, or requires algorithm parameters that
     *  cannot be determined from the given key, or if the given key has a
     *  keysize that exceeds the maximum allowable keysize (as determined
     *  from the configured jurisdiction policy files).
     */
    public CryptoAESE(String secret, Charset charset)
            throws NoSuchAlgorithmException, NoSuchProviderException,
            NoSuchPaddingException, InvalidKeyException
    {
        this.charset = charset;

        // Slice off 16 bytes for AES
        byte[] secretBytes = Arrays.copyOfRange(secret.getBytes(), 0, 16);
        this.key = new SecretKeySpec(secretBytes, "AES");

        this.cipher = Cipher.getInstance("AES/ECB/NoPadding", "SunJCE");
        this.cipher.init(Cipher.ENCRYPT_MODE, key);

        this.decipher = Cipher.getInstance("AES/ECB/NoPadding", "SunJCE");
        this.decipher.init(Cipher.DECRYPT_MODE, key);
    }

    /**
     * Encrypt a plain text string
     *
     * @param plaintext
     * @return a buffer containing the cipher text
     * @throws IllegalBlockSizeException if this cipher is a block cipher,
     *  no padding has been requested (only in encryption mode),
     *  and the total input length of the data processed by this cipher is
     *  not a multiple of block size; or if this encryption algorithm is
     *  unable to process the input data provided.
     * @throws BadPaddingException  if this cipher is in decryption mode, and
     *  (un)padding has been requested, but the decrypted data is not bounded
     *  by the appropriate padding bytes
     */
    public byte[] encrypt(String plaintext) throws IllegalBlockSizeException, BadPaddingException {
        StringBuilder sb = new StringBuilder(plaintext);

        // plaintext must be multiple of 16 bytes in whatever charset we use
        while(sb.toString().getBytes(charset).length % 16 != 0) {
            sb.append('\0');
        }

        return cipher.doFinal(sb.toString().getBytes(charset));
    }

    /**
     * Decrypt a cipher text into a String
     *
     * @param ciphertext
     * @return The string decrypted from the cipher text
     * @throws IllegalBlockSizeException if this cipher is a block cipher,
     *  no padding has been requested (only in encryption mode),
     *  and the total input length of the data processed by this cipher is
     *  not a multiple of block size; or if this encryption algorithm is
     *  unable to process the input data provided.
     * @throws BadPaddingException  if this cipher is in decryption mode, and
     *  (un)padding has been requested, but the decrypted data is not bounded
     *  by the appropriate padding bytes
     */
    public String decrypt(byte[] ciphertext) throws IllegalBlockSizeException, BadPaddingException {
        String str = new String(decipher.doFinal(ciphertext), charset);

        if(str.indexOf('\0') >= 0) {
            return str.substring(0, str.indexOf('\0'));
        } else {
            return str;
        }
    }
}