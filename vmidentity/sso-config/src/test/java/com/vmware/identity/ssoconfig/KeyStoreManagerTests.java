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
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.util.Collection;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class KeyStoreManagerTests {
    private static Properties _props;
    private static String[] _certFileNames;
    private static String _storeFileName;
    private static String _storePassword;
    private static String _storeType;
    private static String _nonExistingStoreFileName = "store_noexisting.jks";

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
        _certFileNames = new String[] { _props.getProperty("rootca1.filename"),
                _props.getProperty("rootca2.filename") };
        _storeFileName = _props.getProperty("key-store.file");
        _storePassword = _props.getProperty("key-store.password");
        _storeType = _props.getProperty("key-store.type");
        try {
            Files.delete(FileSystems.getDefault().getPath(
                    _nonExistingStoreFileName));
        } catch (NoSuchFileException e) {
        }
    }

    @AfterClass
    public static void tearDown() throws Exception {
        try {
            Files.delete(FileSystems.getDefault().getPath(
                    _nonExistingStoreFileName));
        } catch (NoSuchFileException e) {
        }
    }

    @Test
    public void KeyStoreOperationTests() throws Exception {
        KeyStoreManager keyStoreManagerWrite = new KeyStoreManager(
                _storeFileName, _storePassword, _storeType);
        keyStoreManagerWrite.removeCertByAlias(_certFileNames);
        keyStoreManagerWrite.saveToXmlFile();

        KeyStoreManager keyStoreManagerRead = new KeyStoreManager(
                _storeFileName, _storePassword, _storeType);
        Collection<String> certsInStore = keyStoreManagerRead.listCerts();
        for (String s : _certFileNames) {
            Assert.assertFalse(certsInStore.contains(s));
        }

        keyStoreManagerWrite = new KeyStoreManager(_storeFileName,
                _storePassword, _storeType);
        keyStoreManagerWrite.importCerts(_certFileNames, null);
        keyStoreManagerWrite.saveToXmlFile();

        keyStoreManagerRead = new KeyStoreManager(_storeFileName,
                _storePassword, _storeType);
        certsInStore = keyStoreManagerRead.listCerts();
        for (String s : _certFileNames) {
            Assert.assertTrue(certsInStore.contains(s));
        }

        keyStoreManagerWrite = new KeyStoreManager(_storeFileName,
                _storePassword, _storeType);
        keyStoreManagerWrite.removeCertByAlias(_certFileNames);
        keyStoreManagerWrite.saveToXmlFile();

        keyStoreManagerRead = new KeyStoreManager(_storeFileName,
                _storePassword, _storeType);
        certsInStore = keyStoreManagerRead.listCerts();
        for (String s : _certFileNames) {
            Assert.assertFalse(certsInStore.contains(s));
        }
    }

    @Test
    public void KeyStoreInitTests() throws Exception {
        KeyStoreManager keyStoreManagerWrite = new KeyStoreManager(
                _nonExistingStoreFileName, _storePassword, _storeType);
        keyStoreManagerWrite.importCerts(_certFileNames, null);
        keyStoreManagerWrite.saveToXmlFile();

        KeyStoreManager keyStoreManagerRead = new KeyStoreManager(
                _nonExistingStoreFileName, _storePassword, _storeType);
        Collection<String> certsInStore = keyStoreManagerRead.listCerts();
        for (String s : _certFileNames) {
            Assert.assertTrue(certsInStore.contains(s));
        }
    }

    public void KeyStoreDuplicateCertsTests() throws Exception {
        KeyStoreManager keyStoreManagerWrite = new KeyStoreManager(
                _nonExistingStoreFileName, _storePassword, _storeType);
        keyStoreManagerWrite.importCerts(_certFileNames, null);
        keyStoreManagerWrite.importCerts(_certFileNames, "dup");
        keyStoreManagerWrite.saveToXmlFile();

        KeyStoreManager keyStoreManagerRead = new KeyStoreManager(
                _nonExistingStoreFileName, _storePassword, _storeType);
        Collection<String> certsInStore = keyStoreManagerRead.listCerts();
        Assert.assertEquals(2, certsInStore.size());
    }
}