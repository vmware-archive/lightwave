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
package com.vmware.identity.rest.core.server.util;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;

public class PrincipalUtil {

    /**
     * Splits an entity name of the format <tt>"[name]@[domain]"</tt>
     * or <tt>"[domain]\[name]"</tt> into its individual components.
     *
     * @param name user name to split
     * @return PrincipalId containing the component parts
     * @throws BadRequestException if the string is not in the correct format
     */
    public static PrincipalId fromName(String name) throws BadRequestException {
        // UPN format: NAME@DOMAIN
        String[] parts = name.split("@");
        if (parts.length == 2) {
            return new PrincipalId(parts[0], parts[1]);
        }

        // NETBIOS format: DOMAIN\NAME
        parts = name.split("\\\\");
        if (parts.length == 2) {
            return new PrincipalId(parts[1], parts[0]);
        }

        throw new BadRequestException("Invalid name format: '" + name + "'");
    }

    public static Collection<PrincipalId> fromNames(Collection<String> names) throws BadRequestException {
        List<PrincipalId> principals = new LinkedList<PrincipalId>();
        for (String name : names) {
            principals.add(fromName(name));
        }
        return principals;
    }

    public static String createNetBios(PrincipalId subject) {
        return subject.getDomain() + "\\" + subject.getName();
    }

    public static String createNetBios(com.vmware.vim.sso.PrincipalId subject) {
        return subject.getDomain() + "\\" + subject.getName();
    }

    public static String createUPN(PrincipalId subject) {
        return subject.getName() + "@" + subject.getDomain();
    }

    public static String createUPN(com.vmware.vim.sso.PrincipalId subject) {
        return subject.getName() + "@" + subject.getDomain();
    }

}
