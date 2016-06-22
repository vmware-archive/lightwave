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
package com.vmware.identity.rest.core.util;

/**
 * An utility class to convert domain name to AD's standard distinguished name and vice-versa.
 */
public class DomainUtil {

    /**
     * This function will follow AD's standard way to translate from domain <-> DN
     * (i.e. each component of child1.ssolabs.com will match to dc=child1,dc=ssolabs,dc=com)
     */
    public static String getDomainDN(String domain)
    {
        StringBuilder sb = new StringBuilder();
        if( isNullOrEmpty(domain) == false )
        {
            String[] domainParts = domain.split("\\.");

            int iPart = 0;

            for (String part : domainParts)
            {
                if (iPart > 0)
                {
                    sb.append(",");
                }

                sb.append(String.format("DC=%s", part));

                iPart++;
            }
        }
        return sb.toString().toLowerCase();
    }

    /**
     * This function will follow AD's standard way to translate from DN <-> domain
     * (i.e. each component of dc=child1,dc=ssolabs,dc=com will match as child1.ssolabs.com)
     */
    public static String getDomainFromDN(String dn)
    {
        final String dcPrefix = "DC=";
        StringBuilder sb = new StringBuilder();

        if( isNullOrEmpty(dn) == false )
        {
            String[] parts = dn.toUpperCase().split(",");

            int iPart = 0;

            for (String part : parts)
            {
                if (part.startsWith(dcPrefix))
                {
                    if (iPart++ > 0)
                    {
                        sb.append(".");
                    }

                    sb.append(part.substring(dcPrefix.length()));
                }
            }
        }
        return sb.toString().toLowerCase();
    }

    public static boolean isNullOrEmpty(String str)
    {
        return (str == null) || (str.isEmpty());
    }

}
