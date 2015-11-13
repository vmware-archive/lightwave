/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.io.UnsupportedEncodingException;

import com.sun.jna.Platform;

public class LdapFilterString
{
    static final String ASTERISK_TRANSLATION = "\\2a";
    static final String LPAREN_TRANSLATION = "\\28";
    static final String RPAREN_TRANSLATION = "\\29";
    static final String COMMA_TRANSLATION = "\\2c";
    static final String FWDSLASH_TRANSLATION = "\\5c";
    static final String NULL_TRANSLATION = "\\00";

    //
    // @see RFC 4515
    // @see RFC 3269, en.wikipedia.org/wiki/UTF-8
    //
    public static String encode(String filter)
    {
        return encode(filter, true);
    }

    public static String encodeMatchingRuleInChainDnFilter(String dn)
    {
        // win ldap client wants to see commas intact for this filter.
        return encode(dn, (Platform.isWindows() == false));
    }

    private static String encode(String filter, boolean encodeComma)
    {
        String encodedString = null;
        if ( filter != null )
        {
            StringBuilder sb = new StringBuilder();

            int length = filter.length();

            for (int i = 0; i < length; i++)
            {
                char ch = filter.charAt(i);

                switch (ch) {
                case '*':

                    sb.append(ASTERISK_TRANSLATION);

                    break;

                case '(':

                    sb.append(LPAREN_TRANSLATION);

                    break;

                case ')':

                    sb.append(RPAREN_TRANSLATION);

                    break;

                case '\\':

                    sb.append(FWDSLASH_TRANSLATION);

                    break;

                case '\u0000':

                    sb.append(NULL_TRANSLATION);

                    break;
                case ',':
                    if ( encodeComma == true )
                    {
                        sb.append(COMMA_TRANSLATION);
                    }
                    else
                    {
                        sb.append(ch);
                    }
                    break;
                default:

                    if (ch <= 0x7f)
                    {
                        sb.append(ch); // 1 byte UTF-8 char
                    } else if (ch >= 0x080)
                    {
                       try
                       {
                          String s;
                          if (Character.isHighSurrogate(ch))
                          {
                             assert(i < (length-1));
                             s = filter.substring(i, i+2);
                             ++i;
                          }
                          else
                          {
                             s = String.valueOf(ch);
                          }
                          for (byte b : s.getBytes("UTF8"))
                          {
                             int c = b & 0x00ff;//convert byte to int w/o sign preservation
                             sb.append('\\');
                             if (c < 0x10)
                             {
                                sb.append(0); //padding
                             }
                             sb.append(Integer.toHexString(c));
                          }
                       } catch (UnsupportedEncodingException e)
                       {
                       }
                    }
                }
            }

            encodedString = sb.toString();
        }

        return encodedString;
    }
}
