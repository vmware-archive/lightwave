/**
 *
 * Copyright 2014 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.ssoconfig;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;

/**
 * Simple wrapper on java keystore to import and list certs in it.
 * @author qiangw
 *
 */
public class KeyStoreManager {
    private String _keyStoreFilePath;
    private String _keyStorePwd;
    private String _keyStoreType;
    private KeyStore _keyStore;

    /**
     * Ctor using keystore path, password and type. Create a empty keystore if the provided path doesn't exist.
     * @param path
     * @param password
     * @param type
     */
    public KeyStoreManager(String path, String password, String type) {
        this._keyStoreFilePath = path;
        this._keyStorePwd = password;
        this._keyStoreType = type;
        this._keyStore = loadKeyStore(this._keyStoreFilePath,
                this._keyStorePwd.toCharArray());
    }

    /**
     * Import a list of certs to keystore.
     * @param certFileNames
     * @param aliasPrefix
     * @throws CertificateException
     * @throws IOException
     * @throws KeyStoreException
     * @throws NoSuchAlgorithmException
     */
    public void importCerts(String[] certFileNames, String aliasPrefix)
            throws CertificateException, IOException, KeyStoreException,
            NoSuchAlgorithmException {
        if (certFileNames == null) {
            return;
        }
        CertificateFactory certFactory = CertificateFactory
                .getInstance("X.509");
        for (String fn : certFileNames) {
            InputStream inStream = new FileInputStream(fn);
            X509Certificate cert = (X509Certificate) certFactory
                    .generateCertificate(inStream);
            inStream.close();
            String alias = aliasPrefix == null ? fn : aliasPrefix + fn;
            _keyStore.setCertificateEntry(alias, cert);
        }
    }

    /**
     * List certs inside the keystore.
     * @return
     * @throws KeyStoreException
     */
    public Collection<String> listCerts() throws KeyStoreException {
        Collection<String> certAlias = new ArrayList<String>();
        for (Enumeration<String> e = _keyStore.aliases(); e.hasMoreElements();) {
            String s = e.nextElement();
            certAlias.add(s);
        }
        return certAlias;
    }

    /**
     * Remove certs referenced by aliases.
     * @param certAliases
     * @throws KeyStoreException
     */
    public void removeCertByAlias(String[] certAliases) throws KeyStoreException {
        if (certAliases == null) {
            return;
        }
        for (String s : certAliases) {
            _keyStore.deleteEntry(s);
        }
        return;
    }

    /**
     * Save the changed keystore to its keystore file.
     * @throws KeyStoreException
     * @throws NoSuchAlgorithmException
     * @throws CertificateException
     * @throws IOException
     */
    public void saveToXmlFile() throws KeyStoreException,
            NoSuchAlgorithmException, CertificateException, IOException {
        FileOutputStream writeStream = new FileOutputStream(
                this._keyStoreFilePath);
        try {
            _keyStore.store(writeStream, this._keyStorePwd.toCharArray());
        } finally {
            writeStream.close();
        }
    }

    private KeyStore loadKeyStore(String fileName, char[] pass) {
        try {
            KeyStore keyStore = KeyStore.getInstance(_keyStoreType);
            if (Files.exists(FileSystems.getDefault().getPath(fileName))) {
                FileInputStream fis = new FileInputStream(fileName);
                if (fis != null) {
                    try {
                        keyStore.load(fis, pass);
                    } finally {
                        fis.close();
                    }
                }
            } else {
                keyStore.load(null, pass);
            }
            return keyStore;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
