/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import org.w3c.dom.Element;

import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.identity.wstrust.client.impl.ParserException;

/**
 * Utility class for containing common test helper methods
 */
public class TestTokenUtil {

    public static final String TEST_KEYSTORE_PRIV_KEY_PASSWORD = com.vmware.vim.sso.client.TestTokenUtil.TEST_KEYSTORE_PRIV_KEY_PASSWORD;

    /**
     * Loads a file into string
     *
     * @param fileName
     * @return String content of the file
     * @throws IOException
     */
    public static String loadStreamContent(InputStream stream) throws IOException {
        StringBuilder content = new StringBuilder();

        BufferedReader reader = new BufferedReader(new InputStreamReader(stream, "UTF-8"));
        try {
            char[] buff = new char[1024];
            int i = 0;
            while ((i = reader.read(buff)) != -1) {
                content.append(buff, 0, i);
            }
        } finally {
            reader.close();
        }

        return content.toString();
    }

    /**
     * Loads the default keystore for the test cases
     *
     * @return KeyStoreData
     * @throws com.vmware.identity.token.util.SsoKeyStoreOperationException
     * @throws SsoKeyStoreOperationException
     */
    public static KeyStoreData loadDefaultKeystore()
            throws com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException {
        return com.vmware.vim.sso.client.TestTokenUtil.loadDefaultKeystore();
    }

    /**
     * Loads a valid token string.
     *
     * @return
     * @throws ParserException
     */
    public static String getValidSamlTokenString() throws ParserException {
        try {
            return com.vmware.vim.sso.client.TestTokenUtil.getValidSamlTokenString();
        } catch (com.vmware.identity.token.impl.exception.ParserException e) {
            throw new ParserException(null, e);
        }
    }

    /**
     * Load a valid token DOM element.
     *
     * @return
     * @throws ParserException
     */
    public static Element getValidSamlTokenElement() throws ParserException {
        try {
            return com.vmware.vim.sso.client.TestTokenUtil.getValidSamlTokenElement();
        } catch (com.vmware.identity.token.impl.exception.ParserException e) {
            throw new ParserException(null, e);
        }
    }

    /**
     * Load another valid token DOM element.
     *
     * @return
     * @throws ParserException
     */
    public static Element getAnotherValidSamlTokenElement() throws ParserException {
        try {
            return com.vmware.vim.sso.client.TestTokenUtil.getAnotherValidSamlTokenElement();
        } catch (com.vmware.identity.token.impl.exception.ParserException e) {
            throw new ParserException(null, e);
        }
    }
}
