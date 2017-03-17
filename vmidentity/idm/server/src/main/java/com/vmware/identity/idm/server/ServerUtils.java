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

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/4/12
 * Time: 4:45 PM
 * To change this template use File | Settings | File Templates.
 */
import java.io.ByteArrayInputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.EncodedKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.SystemUtils;
import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDMReferralException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.ISystemDomainIdentityProvider;
import com.vmware.identity.interop.ldap.DirectoryStoreProtocol;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapConnectionExWithGetConnectionString;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.ISslX509VerificationCallback;
import com.vmware.identity.interop.ldap.LdapBindMethod;
import com.vmware.identity.interop.ldap.LdapConnectionFactory;
import com.vmware.identity.interop.ldap.LdapConnectionFactoryEx;
import com.vmware.identity.interop.ldap.LdapConstants;
import com.vmware.identity.interop.ldap.LdapOption;
import com.vmware.identity.interop.ldap.LdapSSLProtocols;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapSetting;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.ReferralLdapException;
import com.vmware.identity.performanceSupport.PerfBucketKey;
import com.vmware.identity.performanceSupport.PerfDataSinkFactory;
import com.vmware.identity.performanceSupport.PerfMeasurementPoint;

public class ServerUtils
{
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(ServerUtils.class);
    private static final long DEFAULT_LDAP_NETWORK_TIMEOUT = 30;

    // we support either 'user@domain', 'domain\\user', or default domain formats
    final static char UPN_SEPARATOR = '@';
    final static char NETBIOS_SEPARATOR = '\\';
    final static char[] VALID_ACCOUNT_NAME_SEPARATORS = { UPN_SEPARATOR, NETBIOS_SEPARATOR };

    public static IDMException getRemoteException(Exception ex)
    {
        assert( (ex != null) );

        IDMException idmEx = null;

        logger.error(String.format("Exception '%s'", ex.toString()),
                     ex);

        if (ex instanceof LoginException)
        {
           idmEx = new IDMLoginException(ex.getMessage());
        }
        else if (ex instanceof ReferralLdapException)
        {
           return new IDMReferralException(ex.getMessage());
        }
        else if (ex instanceof InvalidPrincipalException)
        {
           return new InvalidPrincipalException(ex.getMessage(), ((InvalidPrincipalException) ex).getPrincipal());
        }
        else if (ex.getClass().equals(IDMLoginException.class))
        {
           return new IDMLoginException(ex.getMessage(), ((IDMLoginException) ex).getUri());
        }
        else if (ex instanceof IllegalArgumentException)
        {
            idmEx = new InvalidArgumentException(ex.getMessage());
        }
        else if ( ex instanceof IDMException )
        {
            idmEx = (IDMException)ex;

            // we do not expose the chain for any other exception
            // for IDM exception we should not do this also -
            // for one we don't want to expose it, for another chain could contain
            // exception types that RMI will not be able to create on the client
            // but if the chain is not present -> no need to do anything.
            if( idmEx.getCause() != null )
            {
                Class<? extends IDMException> theClass = idmEx.getClass();

                try
                {
                    idmEx = theClass.getConstructor(new Class[] {String.class}).newInstance( new Object[] { idmEx.getMessage() } );
                }
                catch (Exception e)
                {
                    // this should never fail, but to satisfy java checked exception error, we will fall back to
                    // plain old IDM exception
                    logger.error(String.format("Exception '%s'", ex.toString()), ex);
                    idmEx = new IDMException( ex.getMessage() );
                }
            }
        }
        else
        {
           logger.error("Caught an unexpected exception", ex);
           idmEx = new IDMException(ex.getMessage());
        }

        return idmEx;
    }

    public static boolean isNullOrEmpty(String str)
    {
        return (str == null) || (str.isEmpty());
    }

    public static boolean isEquals(String src, String dst)
    {
        return (src == dst) ||
               (src == null && dst == null) ||
               (src != null && src.equalsIgnoreCase(dst));
    }

    public static String getAttributeKeyValueAsString( String keyAttribute, String valueAttribute )
    {
        ValidateUtil.validateNotEmpty( keyAttribute, "keyAttribute" );
        ValidateUtil.validateNotEmpty( valueAttribute, "valueAttribute" );

        return String.format( "%s:%s", keyAttribute, valueAttribute );
    }

    public static Map.Entry<String, String> getAttributeKeyValueFromString( String attibuteKeyValueString )
    {
        ValidateUtil.validateNotEmpty( attibuteKeyValueString, "attibuteKeyValueString" );

        int delimiterIndex = attibuteKeyValueString.lastIndexOf(":");
        if (delimiterIndex == -1)
        {
            throw new IllegalStateException(
                String.format( "Attribute mapping '%s' is of unexpected format.", attibuteKeyValueString)
            );
        }

        return new HashMap.SimpleImmutableEntry<String, String>(
                attibuteKeyValueString.substring(0, delimiterIndex),
                attibuteKeyValueString.substring(delimiterIndex + 1)
        );
    }

    /**
     * Checks the connectivity to an identity provider
     *
     * @param providerUri Location of identity provider. non-null non-empty, required
     * @param userName    Login identifier. non-null, required
     * @param pwd         Password    non-null non-empty, required
     * @return connection:  non-null
     * @throws IDMLoginException. If one or more of the input argument is illegal.
     *                                 Or URI syntax is incorrect.
     * @throws Exception     if no connection
     * @throws IllegalArgumentException  one or more input are empty
     */
    public static ILdapConnectionEx getLdapConnectionByURIs(
            Collection<URI> uris, String userName, String password,
            AuthenticationType authType, boolean useGcPort) throws Exception

    {
       return getLdapConnectionByURIs(uris, userName, password, authType, useGcPort, null);
    }

    /**
     * Checks the connectivity to an identity provider
     *
     * @param providerUri Location of identity provider. non-null non-empty, required
     * @param userName    Login identifier. non-null, required
     * @param pwd         Password    non-null non-empty, required
     * @param certVerifierCallback Callback called to validate SSL certificates
     * @return connection:  non-null
     * @throws IDMLoginException. If one or more of the input argument is illegal.
     *                                 Or URI syntax is incorrect.
     * @throws Exception     if no connection
     * @throws IllegalArgumentException  one or more input are empty
     */
    public static ILdapConnectionEx getLdapConnectionByURIs(
          Collection<URI> uris, String userName, String password, AuthenticationType authType, boolean useGcPort, LdapCertificateValidationSettings certValidationsettings)
                throws Exception
    {
        ValidateUtil.validateNotEmpty( uris, "uris" );

        // NB: not checking whether the port is Kerberos port 88.

        ILdapConnectionEx result = null;
        Exception latestEx = null;
        for (URI uri : uris)
        {
           if (!DirectoryStoreProtocol.isProtocolSupported(uri.getScheme()))
           {
              logger.warn(
                    String.format(
                       "protocol scheme for the specified URI is not supported: [%s]",
                       uri.toString()));
              continue;   //skip unsupported protocol
           }

           logger.trace("start creating connection {}", uri);
           try {
              result = getLdapConnection(uri, userName, password, authType, useGcPort, certValidationsettings);
              if (null != result)
              {
                 logger.trace("done creating connection");

                 return result;   //done
              }
           }
           catch (Exception e)
           {  //log an error, pin down the latest and continue
              latestEx = e;
              logger.error("cannot establish connection with uri: {}", uri);
           }
        }

        assert(result == null);
        if (latestEx != null)
        {
           throw latestEx;   // could not get connection from any of them
        }
        return null;
    }

    private static ILdapConnectionEx getLdapConnection(
         URI uri, String userName, String password, AuthenticationType authType, boolean useGcPort, LdapCertificateValidationSettings certValidationsettings) throws Exception
    {
        ValidateUtil.validateNotNull( uri, "uri" );

        boolean isLdaps = uri.getScheme()
                .compareToIgnoreCase(DirectoryStoreProtocol.LDAPS.getName()) == 0;

        List<LdapSetting> connOptions = null;
        List<LdapSetting> settings = new ArrayList<LdapSetting>();
        settings.add(new LdapSetting(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                LdapConstants.LDAP_VERSION3));
        settings.add(new LdapSetting(LdapOption.LDAP_OPT_REFERRALS,
                Boolean.FALSE));
        settings.add(new LdapSetting(LdapOption.LDAP_OPT_NETWORK_TIMEOUT,
        DEFAULT_LDAP_NETWORK_TIMEOUT));

    if (isLdaps) {
       //if is ldaps connection and certificate validation is enabled set the options for validation
       boolean isLdapsCertValidationEnabled = certValidationsettings != null &&
                   (certValidationsettings.isForceValidation() || IdmServerConfig.getInstance().isLdapsCertValidationEnabled() || !certValidationsettings.isLegacy());
       if (isLdapsCertValidationEnabled)
       {
           ISslX509VerificationCallback certVerifierCallback = certValidationsettings.getCertVerificationCallback(uri);
           settings.add(new LdapSetting(
                   LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT,
                   LdapConstants.LDAP_OPT_X_TLS_DEMAND));
           settings.add(new LdapSetting(
                   LdapOption.LDAP_OPT_X_CLIENT_TRUSTED_FP_CALLBACK,
                   certVerifierCallback));

           int sslMinProtocol = certValidationsettings.isLegacy() ? LdapSSLProtocols.getDefaultLegacyMinProtocol().getCode() : LdapSSLProtocols.getDefaultMinProtocol().getCode();
           settings.add(new LdapSetting(
                   LdapOption.LDAP_OPT_X_TLS_PROTOCOL,
                   sslMinProtocol));
        }
        else
        {
            settings.add(new LdapSetting(LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT, LdapConstants.LDAP_OPT_X_TLS_NEVER));
        }
    }
        // When doing GSSAPI authentication, LDAP SASL binding by default does reverse DNS lookup to validate the
        // target name, this causes authentication failures because Most DNS servers in AD do not have PTR records
        // registered for all DCs, any of which could be the binding target.
        if (!SystemUtils.IS_OS_WINDOWS && authType == AuthenticationType.USE_KERBEROS || authType == AuthenticationType.SRP) {
           settings.add(new LdapSetting(LdapOption.LDAP_OPT_X_SASL_NOCANON, LdapConstants.LDAP_OPT_ON));
        }

        connOptions = Collections.unmodifiableList(settings);

        ILdapConnectionEx connection = null;

        // if No port# or the default port of 389 (ldap) or 636 (ldaps) is specified then useGcport takes effect;
        // otherwise, go with the explicit specified port#
        if (authType == AuthenticationType.SRP)
        {
            connection = (ILdapConnectionEx) LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true);
        }
        else if ((uri.getPort() == -1 || uri.getPort() == LdapConstants.LDAP_PORT || uri.getPort() == LdapConstants.LDAP_SSL_PORT)
            && useGcPort)
        {
            connection =
                    LdapConnectionFactoryEx.getInstance().getLdapConnection(uri.getHost(),
                                                                          isLdaps ? LdapConstants.LDAP_SSL_GC_PORT : LdapConstants.LDAP_GC_PORT,
                                                                          connOptions);
        }
        else
        {
            connection =
                    LdapConnectionFactoryEx.getInstance().getLdapConnection(uri, connOptions);
        }

        try
        {
            // All the client options are set, bind now

            long startTime = System.nanoTime();
            if(AuthenticationType.SRP == authType){
                ValidateUtil.validateNotEmpty( userName, "userName" );
                ValidateUtil.validateNotEmpty( password, "password" );
                ((ILdapConnectionExWithGetConnectionString)connection).bindSaslSrpConnection(
                           userName,
                           password);
            }
            else if (AuthenticationType.USE_KERBEROS == authType)
            {
                String userUPN = null;
                int idxSep = 0;
                if (!ServerUtils.isNullOrEmpty(userName))
                {
                    userUPN = ValidateUtil.normalizeIdsKrbUserName(userName);
                    idxSep = userUPN.indexOf(ValidateUtil.UPN_SEPARATOR);
                }

                connection.bindSaslConnection(ServerUtils.isNullOrEmpty(userUPN) ? null : userUPN.substring(0,idxSep),
                                              ServerUtils.isNullOrEmpty(userUPN) ? null : userUPN.substring(idxSep+1),
                                              password);
            }
            else if (AuthenticationType.PASSWORD == authType)
            {
                ValidateUtil.validateNotEmpty( userName, "userName" );
                ValidateUtil.validateNotEmpty( password, "password" );
                connection.bindConnection(
                           userName,
                           password,
                           LdapBindMethod.LDAP_BIND_SIMPLE);
            }
            else
            {
                String errMsg = String.format("Unsupported authenticationType to bind connection: [%s, %s]", uri, userName);
                logger.warn(errMsg);
                throw new IllegalStateException(errMsg);
            }

            long delta = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime);
            if (logger.isTraceEnabled())
            {
               logger.trace(String.format("\tbinding connection took [%d] ms", delta));
            }
            if (PerfDataSinkFactory.getPerfDataSinkInstance() != null)
            {
                PerfDataSinkFactory.getPerfDataSinkInstance().addMeasurement(
                        new PerfBucketKey(
                                PerfMeasurementPoint.LdapBindConnection,
                                uri.toString()),
                        delta);
            }
        }
        catch (Exception ex)
        {
            logger.warn(String.format("cannot bind connection: [%s, %s]", uri, userName));
            if (connection != null)
            {
                connection.close();
            }
            throw ex;
        }

        return connection;
    }

    /**
     * This function will follow AD's standard way to translate from domain <-> DN
     * (i.e. each component of child1.ssolabs.com will match to dc=child1,dc=ssolabs,dc=com)
     */
    public static String getDomainDN(String domain)
    {
        StringBuilder sb = new StringBuilder();
        if( ServerUtils.isNullOrEmpty(domain) == false )
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
        return sb.toString();
    }

    /**
     * This function will follow AD's standard way to translate from domain <-> DN
     * (i.e. each component of child1.ssolabs.com will match to dc=child1,dc=ssolabs,dc=com)
     */
    public static String getDomainFromDN(String dn)
    {
        final String dcPrefix = "DC=";
        StringBuilder sb = new StringBuilder();

        if( ServerUtils.isNullOrEmpty(dn) == false )
        {
            String[] parts = dn.split(",");

            int iPart = 0;

            for (String part : parts)
            {
                if (part.toUpperCase().startsWith(dcPrefix))
                {
                    if (iPart++ > 0)
                    {
                        sb.append(".");
                    }

                    sb.append(part.substring(dcPrefix.length()));
                }
            }
        }

        return sb.toString();
    }

    public static String getStringValue( LdapValue[] value )
    {
        String strValue = null;
        if ( ( value != null ) && (value.length == 1) )
        {
            strValue = value[0].getString();
        }
        return strValue;
    }

    public static boolean getBooleanValue( LdapValue[] value )
    {
        boolean bVal = false;

        if ( ( value != null ) && (value.length == 1) )
        {
            bVal = Boolean.parseBoolean(value[0].getString());
        }

        return bVal;
    }

    public static String[] getMultiStringValue( LdapValue[] value )
    {
        String[] strValues = null;
        if ( ( value != null ) && (value.length > 0) )
        {
            strValues = new String[value.length];
            for(int i = 0; i < value.length; i++)
            {
                strValues[i] = value[i].getString();
            }
        }
        return strValues;
    }

    public static Collection<String> getMultiStringValueAsCollection( LdapValue[] value )
    {
        String[] strings = ServerUtils.getMultiStringValue(value);
        return (strings == null) ? null : Arrays.asList(ServerUtils.getMultiStringValue(value));
    }

    public static byte[] getBinaryValue( LdapValue[] value )
    {
        byte[] binaryValue = null;
        if ( ( value != null ) && (value.length == 1) )
        {
            binaryValue = value[0].getValue();
        }
        return binaryValue;
    }

    public static LdapValue[] getLdapValue(Collection<String> multiString)
    {
        LdapValue[] value = null;
        if( (multiString != null) && (multiString.isEmpty() == false) )
        {
            value = new LdapValue[multiString.size()];
            int i = 0;
            for( String provider : multiString)
            {
                value[i] = new LdapValue( provider );
                i++;
            }
        }

        return value;
    }

    public static LdapValue[] getLdapValue(String str)
    {
        LdapValue[] value = null;
        if( ServerUtils.isNullOrEmpty( str ) == false )
        {
           value = new LdapValue[] { new LdapValue( str ) };
        }

        return value;
    }

    public static LdapValue[] getLdapValue(int val)
    {
        return new LdapValue[] { new LdapValue( val ) };
    }

    public static LdapValue[] getLdapValue(boolean val)
    {
        return val ? getLdapValue("TRUE") : getLdapValue("FALSE");
    }

    public static LdapValue[] getLdapValue(long val)
    {
        return new LdapValue[] { new LdapValue( val ) };
    }

    public static LdapValue[] getLdapValue(byte[] bytes)
    {
        LdapValue[] value = null;
        if ((bytes != null) && (bytes.length > 0) )
        {
            value = new LdapValue[] {
                new LdapValue( bytes )
            };
        }
        return value;
    }

    public static LdapValue[] getLdapValue(int[] integers)
    {
        LdapValue[] value = null;
        if( (integers != null) && (integers.length > 0) )
        {
            value = new LdapValue[integers.length];
            int i = 0;
            for( int integerVal : integers)
            {
                value[i] = new LdapValue( integerVal );
                i++;
            }
        }

        return value;
    }

    private static byte[] getCertBytes(Certificate cert)
    {
       byte[] bytes = null;

        try {
                bytes = cert.getEncoded();
        }
        catch (CertificateEncodingException ex)
        {
            throw new RuntimeException(
                    "Failed to get certificate encoded bytes");
        }

        return bytes;
    }

    public static LdapValue[] getLdapValue(Certificate cert)
    {
        LdapValue[] value = null;
        if( cert != null)
        {
            byte[] bytes = getCertBytes(cert);

            if ((bytes != null) && (bytes.length > 0) )
            {
                value = new LdapValue[] {
                    new LdapValue( bytes )
                };
            }
        }

        return value;
    }

    public static LdapValue[] getLdapValue(ArrayList<? extends Certificate> certs)
    {
        LdapValue[] value = null;
        if( (certs != null) && (certs.isEmpty() == false) )
        {
            value = new LdapValue[certs.size()];
            int i = 0;
            for( Certificate cert : certs)
            {
                byte[] bytes = getCertBytes(cert);

                if ((bytes != null) && (bytes.length > 0) )
                {
                    value[i] = new LdapValue( bytes );
                }
                i++;
            }
        }

        return value;
    }

    public static LdapValue[] getLdapValue(PrivateKey key)
    {
        LdapValue[] value = null;
        if( key != null)
        {
            byte[] bytes = key.getEncoded();

            if ((bytes != null) && (bytes.length > 0) )
            {
                value = new LdapValue[] {
                        new LdapValue( bytes )
                };
            }
        }

        return value;
    }

    public static Integer getIntegerValue( LdapValue[] value )
    {
        Integer intValue = null;
        if ( ( value != null ) && (value.length == 1) )
        {
            intValue = value[0].getInteger();
        }
        return intValue;
    }

    public static int getIntValue( LdapValue[] value )
    {
        if ( ( value == null ) || (value.length != 1) )
        {
            throw new IllegalArgumentException("Int value cannot be null.");
        }

        return value[0].getInt();
    }

    public static Long getLongValue( LdapValue[] value )
    {
        Long longValue = null;
        if ( ( value != null ) && (value.length == 1) )
        {
            longValue = value[0].getLong();
        }
        return longValue;
    }

    public static long getNativeLongValue(LdapValue[] value)
    {
        if ( ( value == null ) || (value.length != 1) )
        {
            throw new IllegalArgumentException("long value cannot be null.");
        }

        return value[0].getNativeLong();
    }

    public static int[] getMultiIntValue( LdapValue[] value )
    {
        int[] intValues = null;
        if ( ( value != null ) && (value.length > 0) )
        {
            intValues = new int[value.length];
            for(int i = 0; i < value.length; i++)
            {
                intValues[i] = value[i].getInt();
            }
        }
        return intValues;
    }

    public static PrivateKey getPrivateKeyValue(LdapValue[] value)
    {
        PrivateKey privateKey = null;

        if( ( value != null ) && (value.length == 1) )
        {
            byte[] privateKeyBytes = value[0].getValue();
            if (privateKeyBytes != null)
            {
                try
                {
                    KeyFactory keyFactory = KeyFactory.getInstance("RSA");

                    EncodedKeySpec privateKeySpec = new PKCS8EncodedKeySpec(
                            privateKeyBytes);

                    privateKey = keyFactory.generatePrivate(privateKeySpec);
                }
                catch (NoSuchAlgorithmException ex1)
                {
                    throw new RuntimeException("No such algorithm");
                }
                catch (InvalidKeySpecException ex2)
                {
                    throw new RuntimeException("Invalid key spec");
                }
            }
        }
        return privateKey;
    }

    private static X509Certificate getCert(LdapValue value)
    {
        X509Certificate cert = null;

        if (value != null)
        {
            byte[] certBytes = value.getValue();

            if (certBytes != null)
            {
                try
                {
                    ByteArrayInputStream inpstream = new ByteArrayInputStream(certBytes);
                    CertificateFactory cf = CertificateFactory.getInstance("X.509");
                    cert = (X509Certificate)cf.generateCertificate(inpstream);
                }
                catch (CertificateException e)
                {
                    throw new RuntimeException(
                                "Failed to generate certificate");
                }
            }
        }

        return cert;
    }

    public static X509Certificate getCertificateValue(LdapValue[] value)
    {
        X509Certificate cert = null;

        if ( value != null  && value.length == 1 && value[0] != null)
        {
            cert = getCert(value[0]);
        }
        return cert;
    }

    public static ArrayList<Certificate> getCertificateValues(LdapValue[] value)
    {
        ArrayList<Certificate> certs = new ArrayList<Certificate>();

        if ( value != null  && value.length >= 1)
        {
            for (int i = 0; i < value.length; i++)
            {
                X509Certificate cert = getCert(value[i]);

                if (cert!=null)
                {
                    certs.add(cert);
                }
            }
        }
        return certs;
    }

    public static Collection<X509Certificate> getX509CertificateValues(LdapValue[] value)
    {
        Collection<X509Certificate> certs = new ArrayList<X509Certificate>();

        if ( value != null  && value.length >= 1)
        {
            for (int i = 0; i < value.length; i++)
            {
                X509Certificate cert = getCert(value[i]);

                if (cert!=null)
                {
                    certs.add(cert);
                }
            }
        }
        return certs;
    }

    public static Collection<String> getConnectionStringFromUris(Collection<URI> uris)
    {
       List<String> connectionStrs = new ArrayList<String>();
       for (URI uri : uris)
       {
          connectionStrs.add(uri.toString());
       }
       return Collections.unmodifiableCollection(connectionStrs);
    }

    public static Collection<URI> toURIObjects(Collection<String> uriStrings)
    {
       List<URI> result = new ArrayList<URI>();
       for (String uriStr : uriStrings)
       {
          try {
             result.add(new URI(uriStr));
          }catch (URISyntaxException e)
          {
             logger.warn(String.format("bad uri: [%s]", uriStr));
          }
       }
       return Collections.unmodifiableCollection(result);
    }

    public static void validateNotEmptyUsername(
        String username) throws InvalidPrincipalException
    {
        if (ValidateUtil.isEmpty(username)) {
            throw new InvalidPrincipalException("Empty principal name is not allowed", username);
        }
    }

    public static void validateNotNullTenant(
        TenantInformation tenantInfo,
        String tenantName) throws NoSuchTenantException
    {
        if (tenantInfo == null)
        {
            throw new NoSuchTenantException(
                        String.format("No such tenant [%s]", tenantName));
        }

        return;
    }

    public static void validateNotNullIdp(
        IIdentityProvider provider,
        String tenantName,
        String domainName) throws NoSuchIdpException
    {
        if (provider == null)
        {
            throw new NoSuchIdpException(
                        String.format(
                                "No %s provider in tenant [%s] for domain [%s]",
                                provider instanceof ISystemDomainIdentityProvider ? "system" : "such",
                                tenantName,
                                domainName));
        }

        return;
    }

    public static void validateNotNullSystemIdp(
        ISystemDomainIdentityProvider provider,
        String tenantName) throws NoSuchIdpException
    {
        if (provider == null)
        {
            throw new NoSuchIdpException(
                            String.format(
                                    "No system provider in tenant [%s]",
                                    tenantName));
        }

        return;
    }

    /**
     * Extract the domain name from the specified FQDN {@code hostName}
     * @param hostName cannot be null or empty
     * @return
     */
    public static String extractDomainName(String hostName)
    {
        Validate.notEmpty(hostName);
        return hostName.substring(hostName.indexOf('.')+1);
    }

    /**
     * Construct a principalId with accountName and domainName
     * @param accountName cannot be null
     * @param domainAlias can be null
     * @return a constructed principalId or null (if domainName is null)
     */
    public static PrincipalId getPrincipalAliasId(String principalName, String domainAlias)
    {
        return !ServerUtils.isNullOrEmpty(domainAlias) ? new PrincipalId(principalName,
                domainAlias): null;
    }

    /**
     * Given an optional upn, an account name and domain name construct PrincipalId.
     * @param upn optional upn name. Id specified, must be in Upn format.
     * @param accountName account name of the principal. required.
     * @param domainName domain name of the domain the principal is in. required.
     * @return PrincipalId representing the user.
     */
    public static PrincipalId getPrincipalId( String upn, String accountName, String domainName )
    {
        ValidateUtil.validateNotNull(accountName, "accountName");
        ValidateUtil.validateNotNull(domainName, "domainName");

        // we are unifying on iUpn for now.
        // keeping upn parameter in case we switch to eUpn,
        // or decide to include eUpn into the PrincipalId.
        PrincipalId id = new PrincipalId( accountName, domainName );

        return id;
    }

    /**
     * Convert a given {@code PrinciplaId} object to UPN string.
     * @param id cannot be null
     * @return string ID of the object in UPN format.
     */
    public static String getUpn(PrincipalId id)
    {
       ValidateUtil.validateNotNull(id, "id");
       return String.format("%s@%s", id.getName(), id.getDomain());
    }

    /**
     * Convert a given pair of {@code accountName} and {@code domainName} to UPN string.
     * @param accountName cannot be null or empty
     * @param domainName  cannot be null or empty
     * @return string ID of the object in UPN format.
     */
    public static String getUpn(String accountName, String domainName)
    {
       ValidateUtil.validateNotNull(accountName, "accountName");
       ValidateUtil.validateNotNull(domainName, "domainName");
       ValidateUtil.validateNotEmpty(accountName, "accountName");
       ValidateUtil.validateNotEmpty(domainName, "domainName");
       return String.format("%s@%s", accountName, domainName);
    }

    /**
     * @param A list of strings
     * @param String to lookup
     * @param whether the lookup is case sensitive or not
     * @return true or false (if there is a match in the desired case sensitive mode)
     */
    public static boolean contains(List<String> list, String str, boolean bcaseSensitive)
    {
        if (list == null || list.isEmpty() || isNullOrEmpty(str))
            return false;

        if (bcaseSensitive)
        {
            return list.contains(str);
        }

        for (String s : list)
        {
            if (s.equalsIgnoreCase(str)) return true;
        }
        return false;
    }

    public static PrincipalId normalizeAliasInPrincipal( PrincipalId principal, String domainName, String aliasName )
    {
        ValidateUtil.validateNotNull(principal, "principal");
        ValidateUtil.validateNotEmpty(domainName, "domainName");

        PrincipalId normalizedPrincipal = null;
        // make the user principal to always contain domain name, not alias
        if( principal.getDomain().equalsIgnoreCase( aliasName ) )
        {
            normalizedPrincipal = new PrincipalId( principal.getName(), domainName );
        }
        else
        {
            normalizedPrincipal = principal;
        }

        return normalizedPrincipal;
    }

    public static String getGroupAliasNetbios(Group g)
    {
        String aliasNetbios = null;
        if ( ( g != null ) && (g.getAlias() != null) )
        {
            aliasNetbios = String.format(
                "%s%c%s",
                g.getAlias().getDomain(),
                ValidateUtil.NETBIOS_SEPARATOR,
                g.getAlias().getName()
            );
        }

        return aliasNetbios;
    }

    public static boolean isValidDN(ILdapConnectionEx connection, String dn)
    {
        String[] attrsList = {};

        ILdapMessage message = null;
        try
        {
           message = connection.search(dn, LdapScope.SCOPE_BASE, "objectclass=*", attrsList, false);
            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length != 1)
            {
                return false;
            }
        } catch (Exception e) {
            logger.info(String.format("DN: [%s] does not exist", dn));
            return false;
        }
        finally
        {
           if (message!= null) {
            message.close();
           }
        }
        return true;
    }

    public static void disposeLdapMessages(Collection<ILdapMessage> messages)
    {
        if ( ( messages != null ) && (messages.size() > 0) )
        {
            for(ILdapMessage message : messages)
            {
                if( message != null )
                {
                    message.close();
                }
            }
        }
    }

    public static void check_directory_service() throws Exception
    {
        IdmServerConfig config = IdmServerConfig.getInstance();
        // Gives total 2 minutes wait time for vmware directory to come up
        int retryCount = 1;
        boolean keepTrying = false;

        ILdapConnectionEx connection = null;
        try
        {
            do
            {
                keepTrying = false;
                try
                {
                    connection = ServerUtils.getLdapConnectionByURIs(config.getSystemDomainConnectionInfo(),
                                                         config.getSystemDomainUserName(),
                                                         config.getSystemDomainPassword(),
                                                         config.getSystemDomainAuthenticationType(),
                                                         false);
                }
                catch(Exception ex)
                {
                    logger.info(String.format("Waiting for vmware directory to come up - Retry %d", retryCount));
                    keepTrying = true;
                    if (retryCount == 12)
                    {
                        logger.error("Failed to connect to vmware directory.");
                        throw ex;
                    }
                    else Thread.sleep(1000*10); // retry every 10 seconds
                    retryCount++;
                }
            }
            while(keepTrying);
        }
        finally
        {
            if (connection != null)
            {
                connection.close();
            }
        }
    }

    public static PrincipalId getPrincipalId(String upn){
        ValidateUtil.validateNotEmpty(upn, "upn");
        String[] parts = upn.split(String.valueOf(ValidateUtil.UPN_SEPARATOR));
        if(parts.length == 2)
            return new PrincipalId(parts[0], parts[1]);
        else
            throw new IllegalArgumentException("the upn format is invalid");
    }

    /**
     * Split name and domain at '@'. "name" is user identity attribute defined
     * in RSA AM or user hint attribute designated in case of PIV.
     * 
     * @param userName
     *            in the form of name@domain or domain\name format.
     * @return an array of two strings: [name,domain]. null if there is no
     *         domain part
     * @throws IDMLoginException
     */
    public static String[] separateUserIDAndDomain(String userName) throws IDMLoginException {
        Validate.notEmpty(userName, "userName");

        String name = null;
        String domainName;

        int i = userName.lastIndexOf(ValidateUtil.UPN_SEPARATOR);

        if (i == -1) {
            int j = userName.lastIndexOf(ValidateUtil.NETBIOS_SEPARATOR);

            if (j == -1) {
                throw new IDMLoginException(String.format(
                        "User name %s does not contain domain - expected in format of name@domain or domain\\username", userName));
            }
            domainName = userName.substring(0, j);
            Validate.notEmpty(domainName, "expect domain name after the last \'@\' in user name.");

            name = userName.substring(j + 1);
            Validate.notEmpty(name, "Empty name string before the last \'@\' in user name string.");
        } else {
            name = userName.substring(0, i);
            Validate.notEmpty(name, "Empty name string before the last \'@\' in user name string.");

            domainName = userName.substring(i + 1);
            Validate.notEmpty(domainName, "expect domain name after the last \'@\' in user name.");
        }
        String[] userInfo = { name, domainName };

        return userInfo;
    }

    public static PrincipalId getUserPrincipal(TenantInformation tenantInfo, String userPrincipal) throws Exception {
        PrincipalId principal = null;

        String userName = userPrincipal;
        String domain = null;

        // if multiple '@','\' or leading '@','\' exception is thrown
        int idxSep = -1;
        try {
            idxSep = ValidateUtil.getValidIdxAccountNameSeparator(userPrincipal, VALID_ACCOUNT_NAME_SEPARATORS);
        } catch (Exception e) {
            throw new InvalidPrincipalException(String.format(
                    "Invalid user name format [%s]: multiple/leading UPN or NetBIOS separators are not allowed.", userName), userPrincipal);
        }

        // Found separator
        if (idxSep != -1) {
            if (userPrincipal.charAt(idxSep) == ServerUtils.UPN_SEPARATOR) {
                userName = userPrincipal.substring(0, idxSep);
                domain = userPrincipal.substring(idxSep + 1);
            } else if (userPrincipal.charAt(idxSep) == ServerUtils.NETBIOS_SEPARATOR) {
                domain = userPrincipal.substring(0, idxSep);
                userName = userPrincipal.substring(idxSep + 1);
            }
        }

        if ((ServerUtils.isNullOrEmpty(userName) == false) && (ServerUtils.isNullOrEmpty(domain) == true)) {
            Collection<String> defaultProviders = tenantInfo.getDefaultProviders();
            if ((defaultProviders != null) && (defaultProviders.size() > 0)) {
                domain = defaultProviders.iterator().next();
            }
        }

        if ((ServerUtils.isNullOrEmpty(userName) == false) && (ServerUtils.isNullOrEmpty(domain) == false)) {
            principal = new PrincipalId(userName, domain);
        }

        return principal;
    }
}
