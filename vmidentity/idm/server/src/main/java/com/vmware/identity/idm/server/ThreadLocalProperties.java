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

import java.util.Properties;

/**
 * extend properties object with TheadLocal ports
 * @author schai
 *
 */
public class ThreadLocalProperties extends Properties {
    /**
     *
     */
    private static final long serialVersionUID = 3161086466787003613L;
    private final ThreadLocal<Properties> localProperties = new ThreadLocal<Properties>() {
        @Override
        protected Properties initialValue() {
            return new Properties();
        }
    };

    public ThreadLocalProperties(Properties properties) {
        super(properties);
    }

    /**
     * return property from ThreadLocal if the property exist there. Otherwise get from non threadlocal copy.
     * @see java.util.Properties#getProperty(java.lang.String)
     */
    @Override
    public String getProperty(String key) {
        String localValue = localProperties.get().getProperty(key);
        return localValue == null ? super.getProperty(key) : localValue;
    }

    /**
     * set thread-sensitive property.
     * @param key
     * @param value
     * @return
     */
    public Object setThreadLocalProperty(String key, String value) {
        return localProperties.get().setProperty(key, value);
    }
}