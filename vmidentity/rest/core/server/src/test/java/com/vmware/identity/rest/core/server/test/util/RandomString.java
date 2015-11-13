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

package com.vmware.identity.rest.core.server.test.util;

import java.util.Random;

public class RandomString {

    private static final char[] symbols;

    static {
        StringBuilder sb = new StringBuilder();
        for (char ch = '0'; ch <= '9'; ++ch) {
            sb.append(ch);
        }
        for (char ch = 'a'; ch <= 'z'; ++ch) {
            sb.append(ch);
        }

        for (char ch ='A'; ch <= 'Z'; ++ch) {
            sb.append(ch);
        }

        symbols = sb.toString().toCharArray();
    }

    private final Random random;

    private final char[] str;

    public RandomString(int length) {
        random = new Random();
        str = new char[length];
    }

    public RandomString(int length, long seed) {
        random = new Random(seed);
        str = new char[length];
    }

    public String nextString() {
        for (int i = 0; i < str.length; i++) {
            str[i] = symbols[random.nextInt(symbols.length)];
        }
        return new String(str);
    }

}
