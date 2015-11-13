/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.Map;
import java.util.Properties;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLHandshakeException;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.xml.namespace.QName;
import javax.xml.soap.SOAPMessage;
import javax.xml.ws.BindingProvider;
import javax.xml.ws.Dispatch;
import javax.xml.ws.Service;
import javax.xml.ws.WebServiceException;
import javax.xml.ws.soap.SOAPBinding;
import javax.xml.ws.soap.SOAPFaultException;
import javax.xml.ws.spi.Provider;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.CertificateValidationException;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.SSLTrustedManagerConfig;
import com.vmware.identity.wstrust.client.ServerCommunicationException;

/**
 * SOAP over HTTP/HTTPS implementation
 *
 * <p>
 * Thread-safe. Messages of thrown exceptions are not localized.
 */
class SoapBindingImpl implements SoapBinding {

    private static final String WS_TRUST_NAMESPACE_WSDL = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/wsdl";
    private static final String STS_SERVICE = "STSService";
    private static final String STS_SERVICE_PORT = "STSService_Port";
    private static final String WS_OVER_SSL_RESOURCE = "com/vmware/identity/wstrust/client/impl/ssl/ws_over_ssl.properties";
    private static final QName serviceName = new QName(WS_TRUST_NAMESPACE_WSDL, STS_SERVICE);
    private static final QName portName = new QName(WS_TRUST_NAMESPACE_WSDL, STS_SERVICE_PORT);
    private static final Logger log = LoggerFactory.getLogger(SoapBindingImpl.class);

    private static final String SSL_SOCKET_FACTORY_KEY = getSslSocketFactoryProperty();

    private final SSLTrustedManagerConfig sslConfig;

    /**
     * @param connConfig
     *            used to configure the connection with STS web service; Cannot
     *            be <code>null</code>
     */
    public SoapBindingImpl(SSLTrustedManagerConfig sslConfig) {
        this.sslConfig = sslConfig;
    }

    @Override
    public SoapMessage sendMessage(SoapMessage message, URL serviceLocationURL) throws ServerCommunicationException,
            ParserException, SoapFaultException, CertificateValidationException {

        Dispatch<SOAPMessage> dispatch = createDispatch(message.getSoapAction(), serviceLocationURL);

        if (log.isDebugEnabled()) {
            log.debug(String.format("Sending SOAP request to the STS server: %s", serviceLocationURL));
        }

        SOAPMessage response = sendMessage(dispatch, message.getMessage(), serviceLocationURL);

        if (log.isDebugEnabled()) {
            log.debug(String.format("Received SOAP response from the STS server %s", serviceLocationURL));
        }

        return new SoapMessage(response, message.getSoapAction());
    }

    private Dispatch<SOAPMessage> createDispatch(String soapAction, URL serviceLocation)
            throws ServerCommunicationException {

        Service service = Service.create(serviceName);

        service.addPort(portName, SOAPBinding.SOAP11HTTP_BINDING, serviceLocation.toString());

        Dispatch<SOAPMessage> dispatch = null;

        try {
            dispatch = service.createDispatch(portName, SOAPMessage.class, Service.Mode.MESSAGE);

        } catch (WebServiceException e) {
            String errMsg = "Error communicating to the remote server " + serviceLocation;
            log.error(errMsg, e);
            throw new ServerCommunicationException(errMsg, e);
        }

        setSoapAction(dispatch, soapAction);
        establishSslTrust(dispatch, new TrustManager[] { new StsSslTrustManager(sslConfig) });

        return dispatch;
    }

    private void setSoapAction(Dispatch<SOAPMessage> dispatch, String soapAction) {
        Map<String, Object> requestContext = dispatch.getRequestContext();
        requestContext.put(BindingProvider.SOAPACTION_USE_PROPERTY, Boolean.TRUE);
        requestContext.put(BindingProvider.SOAPACTION_URI_PROPERTY, soapAction);
    }

    private SOAPMessage sendMessage(Dispatch<SOAPMessage> dispatch, SOAPMessage request, URL serviceLocationURL)
            throws SoapFaultException, ServerCommunicationException, CertificateValidationException {

        try {
            return dispatch.invoke(request);
        } catch (SOAPFaultException e) {
            log.error("SOAP fault", e);
            throw new SoapFaultException("", new SoapFault(e.getFault()));
        } catch (WebServiceException e) {
            if (e.getCause() instanceof SSLHandshakeException) {
                SSLHandshakeException ex = (SSLHandshakeException) e.getCause();
                if (ex.getCause() instanceof UntrustedSslCertificateException) {
                    logAndThrow((UntrustedSslCertificateException) ex.getCause());
                }
            }

            String errMsg = "Error communicating to the remote server " + serviceLocationURL;
            log.error(errMsg, e);
            throw new ServerCommunicationException(errMsg, e.getCause());
        }
    }

    private void establishSslTrust(BindingProvider bindingProvider, TrustManager[] sslTrust) {

        assert sslTrust.length != 0;

        SSLSocketFactory socketFactory = null;

        try {

            SSLContext sslCtx = SSLContext.getInstance("SSL");
            sslCtx.init(null, sslTrust, null);

            socketFactory = sslCtx.getSocketFactory();

        } catch (Exception e) {
            String errMsg = "Cannot create default SSL socket factory";
            log.error(errMsg, e);
            throw new IllegalStateException(errMsg, e);
        }

        Map<String, Object> requestContext = bindingProvider.getRequestContext();

        requestContext.put(SSL_SOCKET_FACTORY_KEY, socketFactory);
    }

    private static String getSslSocketFactoryProperty() {

        Logger log = LoggerFactory.getLogger(SoapBindingImpl.class);
        String wsProviderClass = Provider.provider().getClass().getName();

        try (InputStream is = SoapBindingImpl.class.getClassLoader().getResourceAsStream(WS_OVER_SSL_RESOURCE)) {

            Properties resource = new Properties();
            resource.load(is);

            String socketFactoryProperty = resource.getProperty(wsProviderClass);

            if (socketFactoryProperty == null) {
                log.warn(String.format("SSO client cannot connect to STS over SSL "
                        + "because WS provider '%s' is NOT configured.", wsProviderClass));

            } else if (log.isDebugEnabled()) {
                log.debug(String.format("WS provider '%s' configured with SSL." + " Socket factory property is '%s'",
                        wsProviderClass, socketFactoryProperty));
            }

            return socketFactoryProperty;

        } catch (IOException e) {
            String message = "Cannot load WS over SSL resource file";
            log.error(message, e);
            throw new IllegalStateException(message, e);
        }
    }

    private void logAndThrow(UntrustedSslCertificateException e) throws CertificateValidationException {

        log.error(e.getMessage(), e);

        throw new CertificateValidationException(e.getMessage(), e.getServerCertificateChain(),
                e.getServerCertificateThumbprint());
    }
}
