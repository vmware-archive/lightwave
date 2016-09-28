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

package com.vmware.identity.ssoconfig;

import java.io.IOException;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class TCServerManagerTests {
    private static Properties _props;
    private static String _tcServerConfigFile;

    static Properties getProps() throws IOException {
        if (_props == null) {
            _props = new Properties();
            _props.load(KeyStoreManagerTests.class
                    .getResourceAsStream("/config.properties"));
        }
        return _props;
    };

    @BeforeClass
    public static void init() throws Exception {
        _props = getProps();
        _tcServerConfigFile = _props.getProperty("tc-server-config.fileName");
    }

    @AfterClass
    public static void tearDown() throws Exception {
    }

    @Test
    public void setAttrTest() throws Exception {
        TCServerManager managerWrite = new TCServerManager(_tcServerConfigFile);
        managerWrite.setAttrValue(TCServerManager.clientAuthAttrName,
                TCServerManager.clientAuthAttributeCertFalse);
        managerWrite.saveToXmlFile();
        TCServerManager managerRead = new TCServerManager(_tcServerConfigFile);
        String value = managerRead
                .getAttrValue(TCServerManager.clientAuthAttrName);
        Assert.assertEquals(TCServerManager.clientAuthAttributeCertFalse,
                value);
    }

    @Test
    public void setAttrToNullTest() throws Exception {
        TCServerManager managerWrite = new TCServerManager(_tcServerConfigFile);

        managerWrite.setAttrValue(TCServerManager.clientAuthAttrName, null);
        managerWrite.saveToXmlFile();
        TCServerManager managerRead = new TCServerManager(_tcServerConfigFile);
        String value = managerRead
                .getAttrValue(TCServerManager.clientAuthAttrName);
        Assert.assertNull(value);

        managerWrite = new TCServerManager(_tcServerConfigFile);
        managerWrite.setAttrValue(TCServerManager.clientAuthAttrName,
                TCServerManager.clientAuthAttributeCertFalse);
        managerWrite.saveToXmlFile();
        managerRead = new TCServerManager(_tcServerConfigFile);
        value = managerRead.getAttrValue(TCServerManager.clientAuthAttrName);
        Assert.assertEquals(TCServerManager.clientAuthAttributeCertFalse,
                value);

        managerWrite = new TCServerManager(_tcServerConfigFile);
        managerWrite.setAttrValue(TCServerManager.clientAuthAttrName,
                TCServerManager.clientAuthAttributeCertFalse);
        managerWrite.saveToXmlFile();
        managerRead = new TCServerManager(_tcServerConfigFile);
        value = managerRead.getAttrValue(TCServerManager.clientAuthAttrName);
        Assert.assertEquals(TCServerManager.clientAuthAttributeCertFalse,
                value);
    }
}