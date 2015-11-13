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
package com.vmware.identity.rest.core.client;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.http.client.utils.URIBuilder;

import com.vmware.identity.rest.core.client.exceptions.ClientException;

public class URIFactory {

    public static URI buildURI(HostRetriever host, String path) throws ClientException {
        return buildURI(host, path, (Object[]) null);
    }

    @SuppressWarnings("unchecked")
    public static URI buildURI(HostRetriever host, String path, Object... args) throws ClientException {
        Map<String, Object> parameters = null;

        if (args != null) {
            if (args[args.length - 1] instanceof Map) {
                parameters = (Map<String, Object>) args[args.length - 1];
                args = Arrays.copyOf(args, args.length - 1);
            }
        }

        URIBuilder builder = host.getURIBuilder()
                .setPath(String.format(path, args));

        if (parameters != null && !parameters.isEmpty()) {
            for (Entry<String, Object> param : parameters.entrySet()) {
                if (param.getValue() instanceof Collection) {
                    Collection<Object> list = (Collection<Object>) param.getValue();
                    Iterator<Object> iter = list.iterator();

                    while (iter.hasNext()) {
                        builder.setParameter(param.getKey(), iter.next().toString());
                    }

                } else {
                    builder.setParameter(param.getKey(), param.getValue().toString());
                }
            }
        }

        try {
            return builder.build();
        } catch (URISyntaxException e) {
            throw new ClientException("An error occurred while building the URI", e);
        }
    }

    public static Map<String, Object> buildParameters(Object... params) throws ClientException {
        Map<String, Object> map= new HashMap<String, Object>();

        if (params != null) {
            if ((params.length % 2) != 0) {
                throw new ClientException("Uneven number of parameters: " + params.length);
            }

            String key = null;
            for (int i = 0; i < params.length; i++) {
                if (i % 2 == 0) {
                    key = params[i].toString();
                } else {
                    if (params[i] != null) {
                        map.put(key, params[i]);
                    }
                }
            }
        }

        return map;
    }

}
