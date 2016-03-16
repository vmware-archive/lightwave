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

package com.vmware.identity.idm.server.clientcert;

import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.security.cert.CRLException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509CRL;


import java.util.Date;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

import com.vmware.identity.idm.CrlDownloadException;

import sun.security.action.GetIntegerAction;
import sun.security.x509.X509CRLImpl;

public class IdmCrlCache extends ConcurrentHashMap<String, X509CRL> {

    /**
     *
     */
    private static final long serialVersionUID = 5260192068779024807L;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmCrlCache.class);

    private static CertificateFactory _certFactory;

    private static final int CRL_CONNECTION_TIMEOUT_DEFAULT = 15;  //in seconds

    private static final int CRL_CONNECTION_TIMEOUT = initializeCRLConnectionTimeOut();
    static {
        try {
            _certFactory = CertificateFactory.getInstance("X.509");
        } catch (Exception e) {
            logger.error("Fails in getting X509CertificateFacotry!",e);
        }
    }

    /**
     * @return CRL connection time out in milliseconds.
     */
    private static int initializeCRLConnectionTimeOut() {

        Integer retVal = null;

        /*  this call throws from unit tests. for now use default.*/
        retVal = java.security.AccessController.doPrivileged(
              new GetIntegerAction("com.sun.security.crl.timeout", CRL_CONNECTION_TIMEOUT_DEFAULT)) *1000;

        if (retVal == null || retVal < 0) {
            retVal = CRL_CONNECTION_TIMEOUT_DEFAULT*1000;
        }
        logger.info(String.format("CRL download connection time out initialized in %s milliseconds ",retVal));
        return retVal;
    }
    /**
     * The periodical CRL updating. Any CRL not able to update will get next try during next fresh cycle.
     *  This allow update later when the resource is available or
     * the communication line is up.
     *
     * In checking for update, crl will be downloaded only if there is newer update
     * @throws CrlDownloadException  if at least one of the URI is not reachable or fails to refresh.
     */
    public void refresh() throws CrlDownloadException{

        String error = null;  //accumulative error string to be thrown.

        for (Map.Entry<String, X509CRL> entry : this.entrySet()) {
            Date update = entry.getValue().getNextUpdate();

            //download only if there is a new copy.
            if (update.before(new Date())) {
                String uriName = entry.getKey();

                X509CRL newCrl;
                try {
                    newCrl = downloadCrl(uriName);
                } catch (CrlDownloadException e) {
                    //continue download remaining crl in case of error
                    if (error == null) {
                        error = String.format("Unable to update CRL from %s during periodical refresh", uriName);
                    } else {
                        error += String.format(error+", %s", uriName);
                    }
                    logger.error(String.format("Unable to update CRL from %s during periodical refresh", uriName));
                    continue;
                }

                if (newCrl != null) {
                    logger.info(
                            "Refreshed CRL from URI "+ uriName);

                    this.put(uriName, newCrl);
                }
            }
        }

        if (error != null) {
            throw new CrlDownloadException(error);
        }
    }

    /**
     * Download and return crl. Connection time out 15 seconds. LDAP is not supported and will be ignored
     * @param uriName
     * @return CRL or null if fails
     * @throws CrlDownloadException if not successful for any reason.
     */
    public static X509CRL downloadCrl(String uriName) throws CrlDownloadException {

        X509CRL crl = null;


        if (uriName.startsWith("ldap://") || uriName.startsWith("ldaps://")) {
            logger.warn(
                    "LDAP CRL stores is not supported. Ignore this URI: "+uriName);
        } else {
            URLConnection conn;

            try {
                conn = new URL(uriName).openConnection();
                conn.setConnectTimeout(CRL_CONNECTION_TIMEOUT);
            } catch (MalformedURLException e) {
                logger.error(String.format("MalformedURLException for CRL URI %s", uriName));
                throw new CrlDownloadException("Error reading CRL: "
                        + uriName, e);

            } catch (IOException e) {
                logger.error(String.format("IOException in connecting to %s", uriName));
                throw new CrlDownloadException("Error reading CRL: "
                        + uriName , e);
            }
            InputStream crlInputStream;
            try {
                crlInputStream = conn.getInputStream();
            } catch (IOException e) {
                logger.error(String.format("IOException from URLConnection.getInputStream() URI: %s", uriName));
                throw new CrlDownloadException("Error reading CRL: "
                        + uriName, e);
            }

            try {
                try {
                    crl = (X509CRLImpl) _certFactory
                            .generateCRL(crlInputStream);

                    if (logger.isDebugEnabled()) {
                        logger.debug("Successful downloaded CRL from "+uriName);
                    }
                } catch (CRLException e) {
                    logger.error(String.format("CRLException from CertificateFactory.generateCRL() URI: %s", uriName));
                    throw new CrlDownloadException("Error reading CRL: "
                            + uriName , e);
                }
            } finally {
                try {
                    crlInputStream.close();
                } catch (IOException e) {
                    logger.error(String.format("IOException from crlInputStream.close() URI: %s", uriName));
                    throw new CrlDownloadException("Error reading CRL: "
                            + uriName , e);
                }
            }

        }
        return crl;
    }
}
