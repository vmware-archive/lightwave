/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.net.URI;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Arrays;

import org.apache.commons.lang.Validate;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.TypeMapper;
import com.sun.jna.platform.win32.WinBase.FILETIME;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.win32.W32APIOptions;
import com.sun.jna.win32.W32APITypeMapper;
import com.vmware.identity.interop.PlatformUtils;
import com.vmware.identity.interop.ldap.certContext.CertContextNative;
import com.vmware.identity.interop.ldap.certContext.CertInfoNative;

/**
 * ILdapClientLibrary implementation for windows.
 */
class WinLdapClientLibrary implements ILdapClientLibrary
{
   private static final Log logger = LogFactory.getLog(WinLdapClientLibrary.class);

    // see windows Ldap docs at : http://msdn.microsoft.com/en-us/library/windows/desktop/aa366961(v=vs.85).aspx
    // see Wldap32.dll headers (WinLdap.h, Winber.h) at:
    //     - perforce-toolchain.eng.vmware.com:1666
    //     - //toolchain/main/win32/winsdk-7.0.7600/Include/

    private static final WinLdapClientLibrary _instance = new WinLdapClientLibrary();
    private LdapSetting networkTimeoutSetting = null;
    private final long LDAP_NO_TIMEOUT_LIMIT = 0;

    public static WinLdapClientLibrary getInstance() { return _instance; }

    private static final int LDAP_OPT_SSL = 0x0a;
    private static final int LDAP_OPT_SERVER_CERTIFICATE = 0x81;
    private static final int LDAP_OPT_SSL_INFO = 0x93;
    private static final int SP_PROT_SSL3_SERVER = 0x10;
    private static final int SP_PROT_TLS1_0_SERVER = 0x40;
    private static final int SP_PROT_TLS1_1_SERVER = 0x100;
    private static final int SP_PROT_TLS1_2_SERVER = 0x400;

    @Override
    public LdapConnectionCtx ldap_initializeWithUri(URI uri, List<LdapSetting> connOptions)
    {
       PlatformUtils.validateNotNull(uri.getHost(), "host");

       logger.debug("start initializing client: " + uri);

       String hostNamePort = (uri.getPort() != -1) ?
               String.format("%s:%d", uri.getHost(), uri.getPort()) :
               uri.getHost();

       int defaultPortNum = LdapConstants.LDAP_SSL_PORT;
       int useSSL = 1;
       Pointer p = null;
       if (uri.getScheme().toLowerCase().equals("ldap"))
       {
          defaultPortNum = LdapConstants.LDAP_PORT;
          useSSL = 0;
       }
       if (logger.isDebugEnabled())
       {
           logger.debug(String.format("ssl_init(%s, %d, %d)", hostNamePort, defaultPortNum, useSSL ) );
       }
       p = LdapClientLibrary.INSTANCE.ldap_sslinit(
             hostNamePort,
             defaultPortNum,
             useSSL);

       if( p == null || p == Pointer.NULL )
       {
           WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.LdapGetLastError());
       }
       logger.debug("ssl_init() succeeded. Setting ldap_options.");

       LdapConnectionCtx connectionCtx = ldapSetOptions(p, connOptions);

       logger.debug("successfully set ldap_options.");

       if ( useSSL == 1 )
       {
           // MSDN: sample of opening an SSL connection
           //    http://msdn.microsoft.com/en-us/library/windows/desktop/aa366105(v=vs.85).aspx
           IntByReference op = new IntByReference(0);
           // ensure SSL_OPT on
           WinLdapClientLibrary.CheckError(
               LdapClientLibrary.INSTANCE.ldap_get_option( p, LDAP_OPT_SSL, op.getPointer() )
           );

           //  If SSL is not enabled, enable it.
           if (op.getValue() != LdapConstants.LDAP_OPT_ON)
           {
               logger.debug("SSL is not enabled. Enabling...");
               Pointer pVal = new IntByReference(LdapConstants.LDAP_OPT_ON).getPointer();
               op.setValue(LdapConstants.LDAP_OPT_ON);
               WinLdapClientLibrary.CheckError(
                   LdapClientLibrary.INSTANCE.ldap_set_option( p, LDAP_OPT_SSL, pVal )
               );
               logger.debug("Enabled SSL.");
           }
           else
           {
               logger.debug("SSL is enabled.");
           }
       }

       logger.debug("client initialized");
       return connectionCtx;
    }

    private LdapConnectionCtx ldapSetOptions(Pointer ld, List<LdapSetting> connOptions)
    {
       // Callback object (from platform) to client layer, where it is set
       ISslX509VerificationCallback callback = null;
       // This is the Windows Crypt32 callback during the process of SSL handshake.
       IServerCertVerify sslCertVerifyObj = null;
       boolean tlsDemand = false;
       int sslMinimumProtocol = 0;

       for (LdapSetting setting : connOptions)
       {
          LdapOption option = setting.getLdapOption();
          Object     val    = setting.getValue();
          if (logger.isTraceEnabled())
          {
             logger.trace(String.format("setting option: [%s, %s]",
                     option, val.toString()));
          }

          switch (option)
          {
          case LDAP_OPT_PROTOCOL_VERSION:
             Validate.notNull(val);
             IntByReference rIntVal = new IntByReference(((Integer)val).intValue());
             logger.debug(String.format("Setting LDAP_OPT_PROTOCOL_VERSION=%d", rIntVal.getValue()));
             WinLdapClientLibrary.CheckError(
                 LdapClientLibrary.INSTANCE.ldap_set_option(
                     ld,
                     option.getCode(),
                     rIntVal.getPointer())
             );
             break;

          case LDAP_OPT_X_CLIENT_TRUSTED_FP_CALLBACK:
              Validate.notNull(val);
              Validate.isTrue(val instanceof ISslX509VerificationCallback);

              callback = (ISslX509VerificationCallback)val;
              sslCertVerifyObj = new ServerCertVerifyTLSDemand(callback);
              break;

          case LDAP_OPT_X_TLS_REQUIRE_CERT:
             Validate.notNull(val);
             // windows does not have LDAP_OPT_X_TLS_REQUIRE_CERT
             // we will emulate this by setting the server cert checker to our function
             // which will just ok the cert
             if ( (((Integer)val).intValue()) == LdapConstants.LDAP_OPT_X_TLS_NEVER )
             {
                 logger.debug("Found LDAP_OPT_X_TLS_REQUIRE_CERT = LdapConstants.LDAP_OPT_X_TLS_NEVER. Setting server cert verify callback.");

                 WinLdapClientLibrary.CheckError(
                     LdapClientLibrary.INSTANCE.ldap_set_option( ld, LDAP_OPT_SERVER_CERTIFICATE, _serverCertVerifyTLSNever )
                 );
             }
             else if( ((Integer)val).intValue() == LdapConstants.LDAP_OPT_X_TLS_DEMAND)
             {
                tlsDemand = true;
             }
             break;
          case LDAP_OPT_REFERRALS:
             Validate.notNull(val);
             Pointer pVal = Pointer.NULL;
             if (((Boolean)val).equals(Boolean.TRUE))
             {
                pVal = new IntByReference(1).getPointer();
             }
             logger.debug(String.format("Setting LDAP_OPT_REFERRALS=%s", ((pVal==Pointer.NULL) ? "(NULL)" : "1")));
             WinLdapClientLibrary.CheckError(
                 LdapClientLibrary.INSTANCE.ldap_set_option(
                    ld,
                    option.getCode(),
                    pVal)
             );
             break;
          case LDAP_OPT_NETWORK_TIMEOUT:
              logger.debug("Processing LDAP_OPT_NETWORK_TIMEOUT.");
              // record this setting to 'networkTimeoutSetting' and set option during bind for windows
              // so that we can unset it after the bind happens on windows
              this.networkTimeoutSetting = setting;
              break;
          case LDAP_OPT_X_TLS_PROTOCOL:
              Validate.notNull(val);
              if (val instanceof Integer)
              {
                 sslMinimumProtocol = (Integer)val;
              }
              break;
          default:
             Validate.notNull(val);
             String msg = String.format("unsupport options: [%s, %d]", option, val);
             logger.warn(msg);
             throw new IllegalArgumentException(msg);
          }
       }

       if (tlsDemand)
       {
          logger.debug("Found LDAP_OPT_X_TLS_REQUIRE_CERT = LdapConstants.LDAP_OPT_X_TLS_DEMAND. Setting server cert verify callback.");
          Validate.notNull(callback);
          Validate.notNull(sslCertVerifyObj);
          WinLdapClientLibrary.CheckError(
              LdapClientLibrary.INSTANCE.ldap_set_option(ld, LDAP_OPT_SERVER_CERTIFICATE, sslCertVerifyObj));
       }
       //return connection context object to the caller.
       return new LdapConnectionCtx(ld, sslCertVerifyObj, sslMinimumProtocol);
    }

    @Override
    public Pointer ldap_init(String hostName, int portNumber)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_init( hostName, portNumber );
        if( p == null || p == Pointer.NULL )
        {
            /*
            MSDN:
            If the function fails, it returns NULL. Use LdapGetLastError to retrieve the error code.
            */
            WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.LdapGetLastError());
        }
        return p;
    }

    @Override
    public void ldap_set_option(Pointer ld, int option, Pointer value)
    {
        // we may want to translate 'LDAP_OPT_NETWORK_TIMEOUT' here to be consistent with ldap_bind
        WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_set_option(ld, option, value));
    }

    private void ldap_set_timeoutlimit(Pointer ld, LdapSetting timeoutLimitSetting)
    {
        Object val = timeoutLimitSetting.getValue();
        Validate.notNull(val);
        TimevalNative rTimeoutVal = new
                TimevalNative(((Long)val).longValue(), 0);
        WinLdapClientLibrary.CheckError(
            LdapClientLibrary.INSTANCE.ldap_set_option(
                ld,
                LdapOption.LDAP_OPT_TIMELIMIT.getCode(),
                rTimeoutVal.getPointer()));    }

    @Override
    public void ldap_bind_s(LdapConnectionCtx ctx, String dn, String cred, int method)
    {
        Pointer ld = ctx.getConnection();
        boolean bNeedResetTimeout = false;
        TimevalNative rTimeoutVal = new TimevalNative(0,0);

        // set time out for bind
        if (this.networkTimeoutSetting != null &&
            this.networkTimeoutSetting.getLdapOption().getCode() == LdapOption.LDAP_OPT_NETWORK_TIMEOUT.getCode())
        {
            // Get the current LDAP_OPT_TIMELIMT value if it is set so that we can preserve it after a successful bind
            try
            {
                WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_get_option(ld,
                        LdapOption.LDAP_OPT_TIMELIMIT.getCode(),
                        rTimeoutVal.getPointer()));
            }
            catch(LdapException e)
            {
                // ignore not found error
            }

            try
            {
                ldap_set_timeoutlimit(ld, this.networkTimeoutSetting);
            }
            catch(LdapException e)
            {
                // ignore not found error
            }
            bNeedResetTimeout = true;
        }

        try
        {
            WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_connect(ld, rTimeoutVal.tv_sec > 0 ? rTimeoutVal.getPointer() : null));

            checkSSLProtocolVersion(ld, ctx.getSSLMinimumProtocol());

            logger.debug(String.format("ldap_bind_s(ld, %s, cred, %d).", dn, method));
            WinLdapClientLibrary.CheckError( LdapClientLibrary.INSTANCE.ldap_bind_s( ld, dn, cred, method ) );
        }
        catch(Exception ex)
        {
            logger.error("Failed ldap_bind_s().", ex);

            if(ex instanceof LdapException)
            {
                throw ((LdapException)ex);
            }
            else
            {
                throw new LdapException( LdapErrors.LDAP_OTHER.getCode(), ex.getMessage() );
            }
        }
        finally
        {
            // We want to set 'LDAP_OPT_TIMELIMIT' to 0 (default timeout value) or the original timeoutval
            // can just use 'rTimeoutVal.GetTimeSec()' but state this way to be more obvious what the code achieves
            if (bNeedResetTimeout)
            {
                ldap_set_timeoutlimit(ld,
                      new LdapSetting(LdapOption.LDAP_OPT_TIMELIMIT,
                                      rTimeoutVal.tv_sec > 0 ? rTimeoutVal.tv_sec : LDAP_NO_TIMEOUT_LIMIT ));
            }
        }
    }

    @Override
    public void ldap_sasl_bind_s(Pointer ld, String userName, String domainName, String password)
    {
        boolean bNeedResetTimeout = false;
        TimevalNative rTimeoutVal = new TimevalNative(0,0);
        SecWinntAuthId secCreds = null;
        if (userName != null && userName.length() != 0
            && domainName != null && domainName.length() != 0)
        {
            secCreds = new SecWinntAuthId(userName, domainName, password);
        }

        // set time out for bind
        if (this.networkTimeoutSetting != null &&
            this.networkTimeoutSetting.getLdapOption().getCode() == LdapOption.LDAP_OPT_NETWORK_TIMEOUT.getCode())
        {
            // Get the current LDAP_OPT_TIMELIMT value if it is set so that we can preserve it after a successful bind
            try
            {
                WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_get_option(ld,
                        LdapOption.LDAP_OPT_TIMELIMIT.getCode(),
                        rTimeoutVal.getPointer()));
            }
            catch(LdapException e)
            {
                // ignore not found error
            }

            try
            {
                ldap_set_timeoutlimit(ld, this.networkTimeoutSetting);
            }
            catch(LdapException e)
            {
                // ignore not found error
            }
            bNeedResetTimeout = true;
        }

        try
        {
            WinLdapClientLibrary.CheckError( LdapClientLibrary.INSTANCE.ldap_bind_s( ld,
                    null,
                    secCreds == null ? Pointer.NULL : secCreds.getPointer(),
                    LdapBindMethod.LDAP_AUTH_NEGOTIATE.getCode()
                   ));
        }
        catch (LdapException e)
        {
            if (e.getErrorCode() == LdapErrors.LDAP_WinLdap_LOCAL_ERROR.getCode())
            {
                throw new SaslBindFailLdapException(e.getErrorCode(), "Ldap_sasl_bind failed due to local errors, for instance, kerberos-related failures");
            }

            throw new SaslBindFailLdapException(e.getErrorCode(), "Ldap_sasl_bind failed"+e.getMessage());
        }
        finally
        {
            // We want to set 'LDAP_OPT_TIMELIMIT' to 0 (default timeout value) or the original timeoutval
            // can just use 'rTimeoutVal.GetTimeSec()' but state this way to be more obvious what the code achieves
            if (bNeedResetTimeout)
            {
                ldap_set_timeoutlimit(ld,
                      new LdapSetting(LdapOption.LDAP_OPT_TIMELIMIT,
                                      rTimeoutVal.tv_sec > 0 ? rTimeoutVal.tv_sec : LDAP_NO_TIMEOUT_LIMIT ));
            }
        }
    }

    @Override
    public void ldap_add_s(Pointer ld, String dn, Pointer[] attributes)
    {
//comment out tracing, uncommented when needed.
//        if (logger.isTraceEnabled())
//        {
//            logger.trace("entering ldap_add_s");
//            for (Pointer attr : attributes)
//            {
//                if (attr == Pointer.NULL)
//                    break;   //done
//
//                int offset = 0;
//                int modOp = attr.getInt(offset); /*LdapModOperation.ADD | LDAP_MOD_BVALUES*/
//                assert(modOp == (LdapModOperation.ADD.getCode() | LdapModNative.LDAP_MOD_BVALUES));
//                offset+=8;  // int size = 8
//
//                Pointer ptrModType = attr.getPointer(offset);
//                String modType = ptrModType.getString(0, true);
//                offset+=8;
//
//                Pointer ptrValues = attr.getPointer(offset);
//                Pointer[] values = ptrValues.getPointerArray(0);
//                logger.trace(String.format("---modOp: [%d], modType: [%s], counts: [%d] ", modOp, modType, values.length));
//                for (Pointer ldapValues : values)
//                {
//                    //HEADER
//                    int ldapValuesOffset = 0;
//                    int hdrBufLen = 16;  /*byte[0~7]: contentLen; byte[8~0xf]: ptr to contentBuffer*/
//                    byte[] hdrBuf = new byte[hdrBufLen];
//                    //raw binary format
//                    ldapValues.read(0, hdrBuf, 0, hdrBufLen);
//                    StringBuilder hdrData = new StringBuilder();
//                    for (byte b : hdrBuf)
//                    {
//                        hdrData.append(String.format(" %02x", b));
//                    }
//                    logger.trace(String.format("  HEADER binary: %s", hdrData));
//                    //decoded format
//                    int hdrLen = ldapValues.getInt(ldapValuesOffset);
//                    ldapValuesOffset+= 8;
//                    Pointer hdrContentPtr = ldapValues.getPointer(ldapValuesOffset);
//                    boolean isStringType = !modType.equalsIgnoreCase("userCertificate");
//                    if (!isStringType)
//                    {
//                        logger.trace("    skip dumping ascii content of non-string typpe");
//                    }else
//                    {
//                        logger.trace(String.format("    HEADER: Len [%d] bytes, contentPtr: [%s]",
//                                    hdrLen, hdrContentPtr.toString()));
//                    }
//
//                    //PAY_LOAD
//                    //raw binary format
//                    byte[] buf = new byte[hdrLen];
//                    hdrContentPtr.read(0, buf, 0, hdrLen);
//                    String payLoad = null;
//                    if (isStringType)
//                    {
//                        try {
//                            payLoad = new String(buf, "UTF-8");
//                        } catch (Exception e) {
//                            logger.error("failed to decode bytes");
//                        }
//                    }else
//                    {
//                        payLoad = "--SKIPPED--";
//                    }
//
//                    StringBuilder sb = new StringBuilder();
//                    for (byte b: buf)
//                    {
//                        sb.append(String.format(" %02x", b));
//                    }
//                    logger.trace(String.format("  PAY_LOAD: string [%s] binary [%s]", payLoad, sb));
//                }
//            }
//        }
        WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_add_s(ld, dn, attributes));
    }

    @Override
    public void ldap_modify_s(Pointer ld, String dn, Pointer[] attributes)
    {
        WinLdapClientLibrary.CheckError( LdapClientLibrary.INSTANCE.ldap_modify_s( ld, dn, attributes ) );
    }

    @Override
    public Pointer ldap_search_s(
            Pointer ld, String base, int scope, String filter,
            String[] attrs, int attrsonly
    )
    {
        PointerByReference values = new PointerByReference();
        values.setValue(Pointer.NULL);

        try
        {
            WinLdapClientLibrary.CheckError(
                    LdapClientLibrary.INSTANCE.ldap_search_s(
                            ld, base, scope, filter, attrs, attrsonly, values
                    )
            );

            return values.getValue();
        }
        catch(Exception ex)
        {
            if (values.getValue() != Pointer.NULL)
            {
                LdapClientLibrary.INSTANCE.ldap_msgfree(values.getValue());
            }

            if(ex instanceof LdapException)
            {
                throw ((LdapException)ex);
            }
            else
            {
                throw new LdapException( LdapErrors.LDAP_OTHER.getCode(), ex.getMessage() );
            }
        }
    }

    @Override
    public Pointer ldap_search_ext_s(
        Pointer ld, String base, int scope, String filter,
        String[] attrs, int attrsonly, Pointer[] sctrls, Pointer[] cctrls, Pointer timeout, int sizelimit
    )
    {
        PointerByReference values = new PointerByReference();
        values.setValue(Pointer.NULL);

        try
        {
            int retVal =
                LdapClientLibrary.INSTANCE.ldap_search_ext_s(
                        ld, base, scope, filter, attrs, attrsonly, sctrls, cctrls, timeout, sizelimit, values
                );

            // size limit exception can result together with valid results when results are partial
            // i.e. such as when we specify a limit and it is exceeded on server and only the limited
            // results comes back
            // allow for such case propagating the results and not exception.
            if ( ( retVal != LdapErrors.LDAP_SIZELIMIT_EXCEEDED.getCode() ) ||
                 (sizelimit <= 0 ) ||
                 (values.getValue() == null) ||
                 (values.getValue() == Pointer.NULL)
               )
            {
                WinLdapClientLibrary.CheckError(retVal);
            }
            else
            {
                // if the number of returned entries does not match our limit
                // propagate error
                int entries = LdapClientLibrary.INSTANCE.ldap_count_entries(ld, values.getValue());
                if ( ( entries == -1 ) || ( entries != sizelimit ) )
                {
                    WinLdapClientLibrary.CheckError(retVal);
                }
            }

            return values.getValue();
        }
        catch(Exception ex)
        {
            if (values.getValue() != Pointer.NULL)
            {
                LdapClientLibrary.INSTANCE.ldap_msgfree(values.getValue());
            }

            if(ex instanceof LdapException)
            {
                throw ((LdapException)ex);
            }
            else
            {
                throw new LdapException( LdapErrors.LDAP_OTHER.getCode(), ex.getMessage() );
            }
        }
    }

    @Override
    public LdapPagedSearchResultPtr ldap_one_paged_search(
           Pointer ld,
           String base,
           int scope,
           String filter,
           String[] attrs,
           int pageSize,
           Pointer pBerCookie
           )
    {
        char pagingCriticality = 'T';
        PointerByReference pPageControl = new PointerByReference();
        pPageControl.setValue(Pointer.NULL);

        Pointer[] ppInputControls = { Pointer.NULL, Pointer.NULL };

        PointerByReference ppReturnedControls = new PointerByReference(Pointer.NULL);

        PointerByReference errorcodep = new PointerByReference(Pointer.NULL);

        PointerByReference pageCountp = new PointerByReference(Pointer.NULL);

        boolean bSearchFinished = false;

        PointerByReference pMessage = new PointerByReference(Pointer.NULL);

        PointerByReference pNewBerCookie = new PointerByReference(Pointer.NULL);

        try
        {
            WinLdapClientLibrary.CheckError(
                    LdapClientLibrary.INSTANCE.ldap_create_page_control(ld,
                               pageSize,
                               pBerCookie,
                               pagingCriticality,
                               pPageControl)
                    );

            ppInputControls[0] = pPageControl.getValue();

            WinLdapClientLibrary.CheckError(
                    LdapClientLibrary.INSTANCE.ldap_search_ext_s(
                            ld, base, scope, filter, attrs, 0, ppInputControls, null, null, 0, pMessage)
                    );

            WinLdapClientLibrary.CheckError(
                    LdapClientLibrary.INSTANCE.ldap_parse_result(ld, pMessage.getValue(), errorcodep, null, null, null, ppReturnedControls, 0)
                    );

            WinLdapClientLibrary.CheckError(
                    LdapClientLibrary.INSTANCE.ldap_parse_page_control(ld, ppReturnedControls.getValue(), pageCountp, pNewBerCookie)
                    );

            Pointer pNewBerCookieVal = pNewBerCookie.getValue();
            if (pNewBerCookieVal == Pointer.NULL)
            {
                bSearchFinished = true;
            }
            else
            {
                BerValNative berCookie = new BerValNative(pNewBerCookieVal);
                if (berCookie.length < 1)
                {
                    bSearchFinished = true;
                }
            }

            return new LdapPagedSearchResultPtr(pMessage.getValue(), bSearchFinished, pNewBerCookie.getValue());
        }
        catch(Exception ex)
        {
            if (pMessage.getValue() != Pointer.NULL)
            {
                LdapClientLibrary.INSTANCE.ldap_msgfree(pMessage.getValue());
            }

            if (pNewBerCookie.getValue() != Pointer.NULL)
            {
                LdapClientLibrary.INSTANCE.ber_bvfree(pNewBerCookie.getValue());
                pNewBerCookie = new PointerByReference();
                pNewBerCookie.setValue(Pointer.NULL);
            }

            String ctx =
                   String.format("base=%s, scope=%d, filter=%s, attrs=%s, attrsonly=0, sizelimit=0",
                         base==null?"null":base,
                         scope,
                         filter==null?"null":filter,
                         attrs==null?"null":attrs.toString());
            logger.error("Exception when calling ldap_one_paged_search: " + ctx, ex);

            if(ex instanceof LdapException)
            {
                throw ((LdapException)ex);
            }
            else
            {
                throw new LdapException( LdapErrors.LDAP_OTHER.getCode(), ex.getMessage() );
            }
        }
        finally
        {
            // clean up
            if (ppReturnedControls.getValue() != Pointer.NULL)
            {
                LdapClientLibrary.INSTANCE.ldap_controls_free(ppReturnedControls.getValue());
                ppReturnedControls = new PointerByReference();
                ppReturnedControls.setValue(Pointer.NULL);
            }

            ppInputControls[0] = Pointer.NULL;
            if (pPageControl.getValue() != Pointer.NULL)
            {
                LdapClientLibrary.INSTANCE.ldap_control_free(pPageControl.getValue());
                pPageControl = new PointerByReference();
                pPageControl.setValue(Pointer.NULL);
            }
        }
     }

    @Override
    public void ldap_delete_s(Pointer ld, String dn) {
        WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_delete_s(ld, dn));
    }

    @Override
    public Pointer ldap_get_dn(Pointer ld, Pointer msg)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_get_dn( ld, msg );
        WinLdapClientLibrary.CheckError(ld, p);
        return p;
    }

    @Override
    public Pointer ldap_first_entry(Pointer ld, Pointer pMessage)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_first_entry( ld, pMessage );
        WinLdapClientLibrary.CheckError(ld,p);
        return p;
    }

    @Override
    public Pointer ldap_next_entry(Pointer ld, Pointer pEntry)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_next_entry( ld, pEntry );
        WinLdapClientLibrary.CheckError(ld,p);
        return p;
    }

    @Override
    public int ldap_count_entries(Pointer ld, Pointer pMessage)
    {
        int res = LdapClientLibrary.INSTANCE.ldap_count_entries(ld, pMessage);
        CheckErrorOnIntResult(ld, res);
        return res;
    }

    @Override
    public Pointer ldap_first_attribute(Pointer ld, Pointer pEntry, PointerByReference ppBerElem)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_first_attribute( ld, pEntry, ppBerElem);
        WinLdapClientLibrary.CheckError(ld,p);
        return p;
    }

    @Override
    public Pointer ldap_next_attribute(Pointer ld, Pointer pEntry, Pointer pBerElem)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_next_attribute(ld, pEntry, pBerElem);
        WinLdapClientLibrary.CheckError(ld,p);
        return p;
    }

    @Override
    public Pointer ldap_get_values_len(Pointer ld, Pointer pEntry, String attrName)
    {
        Pointer p = LdapClientLibrary.INSTANCE.ldap_get_values_len(ld, pEntry, attrName);
        WinLdapClientLibrary.CheckError(ld,p);
        return p;
    }

    @Override
    public int ldap_count_values_len(Pointer ppBerValues)
    {
        return LdapClientLibrary.INSTANCE.ldap_count_values_len(ppBerValues);
    }

    @Override
    public void ldap_value_free_len(Pointer ppBerValues)
    {
        WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_value_free_len(ppBerValues));
    }

    @Override
    public void ldap_unbind(Pointer ld)
    {
        WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_unbind(ld));
    }

    @Override
    public void ldap_msgfree(Pointer msg)
    {
        /*
        MSDN: Returns LDAP_SUCCESS.
        */
        LdapClientLibrary.INSTANCE.ldap_msgfree(msg);
    }

    @Override
    public void ldap_memfree(Pointer mem)
    {
        LdapClientLibrary.INSTANCE.ldap_memfree(mem);
    }

    @Override
    public void ber_free(Pointer berElement, int freebuf)
    {
        LdapClientLibrary.INSTANCE.ber_free( berElement, freebuf );
    }

    @Override
    public void ber_bvfree(Pointer berVal)
    {
       LdapClientLibrary.INSTANCE.ber_bvfree( berVal );
    }

    @Override
    public String ldap_err2string(int errorCode)
    {
        return LdapClientLibrary.INSTANCE.ldap_err2string(errorCode);
    }

    @Override
    public String getString(Pointer ptr) {
        String str = null;
        if( ( ptr != null ) &&( ptr != Pointer.NULL ) )
        {
            // find the \0\0
            int bufLen = -1;
            for (int i = 0; i < Integer.MAX_VALUE/2; i++)
            {
                byte[] next2Bytes = ptr.getByteArray(i*2, 2);
                if (next2Bytes[0] == 0 && next2Bytes[1] == 0)
                {
                    // Reached end of string
                    bufLen = i*2;
                    break;
                }
            }

            if ( ( bufLen == -1 ) || (bufLen > Integer.MAX_VALUE) )
            {
                throw new RuntimeException("Invalid native string.");
            }
            else
            {
                if(bufLen == 0)
                {
                    str = "";
                }
                else
                {
                    byte[] chars = ptr.getByteArray( 0 , bufLen );
                    // Unicode encoding for Windows
                    str = new String(chars, PlatformUtils.getWindowsWcharCharSet());
                }
            }
        }

        return str;
    }

    @Override
    public TypeMapper getTypeMapper(){
        return W32APITypeMapper.UNICODE;
    }

    // private ctor
    private WinLdapClientLibrary() {}

    private void checkSSLProtocolVersion(Pointer ld, int sslMinProtocol) {

        //min protocol is not set when certificate validation is not enabled
        if (sslMinProtocol == 0)
            return;

        ConnectionInfoNative sslInfo = new ConnectionInfoNative();
        IntByReference op = new IntByReference(0);
        WinLdapClientLibrary
                .CheckError(LdapClientLibrary.INSTANCE.ldap_get_option(ld, LDAP_OPT_SSL, op.getPointer()));

        // If SSL is enabled check the protocol is minimum sslMinProtocol
        if (op.getValue() == LdapConstants.LDAP_OPT_ON) {

            WinLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_get_option(ld, LDAP_OPT_SSL_INFO,
                    sslInfo.getPointer()));
            sslInfo.read();

            if (sslInfo.dwProtocol < getWinLdapProtocol(sslMinProtocol))
                throw new ServerDownLdapException(LdapErrors.LDAP_WinLdap_SERVER_DOWN.getCode(),
                        String.format("Minimum protocol required is %s, but was used %s", LdapSSLProtocols.getProtocolByCode(sslMinProtocol).getName(), getLdapSSLProtocol(sslInfo.dwProtocol).getName()));
        }
    }

    private int getWinLdapProtocol(int sslMinProtocol) {
        LdapSSLProtocols ldapSSLMinProtocol = LdapSSLProtocols
                .getProtocolByCode(sslMinProtocol);
        switch (ldapSSLMinProtocol) {
        case SSLv3:
            return SP_PROT_SSL3_SERVER;
        case TLSv1_0:
            return SP_PROT_TLS1_0_SERVER;
        case TLSv1_1:
            return SP_PROT_TLS1_1_SERVER;
        case TLSv1_2:
            return SP_PROT_TLS1_2_SERVER;
        default:
            return SP_PROT_TLS1_0_SERVER;
        }
    }

    private LdapSSLProtocols getLdapSSLProtocol(int winLdapProtocol)
    {
        if (winLdapProtocol < SP_PROT_TLS1_0_SERVER)
            return LdapSSLProtocols.SSLv3;
        if (winLdapProtocol == SP_PROT_TLS1_1_SERVER)
            return LdapSSLProtocols.TLSv1_0;
        if (winLdapProtocol == SP_PROT_TLS1_2_SERVER)
            return LdapSSLProtocols.TLSv1_1;
        else
            return LdapSSLProtocols.TLSv1_0;
    }

    private static void CheckError(int errorCode)
    {
        LdapErrorChecker.CheckError( errorCode, _instance  );
    }

    private static void CheckErrorOnIntResult(Pointer ld, int res)
    {
        if (res == -1)
        {
            CheckSessionError(ld);
        }
    }

    private static void CheckError(Pointer ld, Pointer pointer)
    {
        if ((pointer == null) || (pointer == Pointer.NULL) )
        {
            CheckSessionError(ld);
        }
    }

    private static void CheckSessionError( Pointer ld )
    {
        IntByReference errorNumber = new IntByReference();
        errorNumber.setValue(LdapErrors.LDAP_SUCCESS.getCode());

        LdapClientLibrary.INSTANCE.ldap_get_option(
            ld, LdapOption.LDAP_OPT_RESULT_CODE.getCode(), errorNumber.getPointer()
        );

        WinLdapClientLibrary.CheckError( errorNumber.getValue() );
    }

    // This function is called after the secure connection has been established. The
    // certificate of the server is supplied for examination by the client. If the
    // client approves it, it returns TRUE else, it returns false and the secure
    // connection is torn down.
    //
    private interface IServerCertVerify extends Callback
    {
        //typedef BOOLEAN (_cdecl VERIFYSERVERCERT) (
        //     PLDAP Connection,
        //     PCCERT_CONTEXT* pServerCert
        //     );

       //connection will be torn down when this callback returns 0/FALSE;
        int callback( Pointer ld, PointerByReference ppServerCert );
    }

    private static final IServerCertVerify _serverCertVerifyTLSNever = new IServerCertVerify()
    {
        @Override
        public int callback(Pointer ld, PointerByReference ppServerCert)
        {
            // MSDN:
            // Even though VERIFYSERVERCERT is declared as receiving a PCCERT_CONTEXT,
            // it in fact receives a PCCERT_CONTEXT*.
            // The ppServerCert can be used to verify the certificate.
            // CertFreeCertificateContext should be called before this function returns.
            // The call to this function should be made as follows:
            // CertFreeCertificateContext(*ppServerCert);

            logger.debug("Within _serverCertVerifyTLSNever.callback.");

            if ( ( ppServerCert != null && ppServerCert.getValue() != Pointer.NULL ) )
            {
                Crypt32Library.INSTANCE.CertFreeCertificateContext(ppServerCert.getValue());
            }

            logger.debug("returning TRUE.");
            // TRUE
            return 1;
        }
    };

    private static class ServerCertVerifyTLSDemand implements IServerCertVerify
    {
       ISslX509VerificationCallback verificationCallback;

        private ServerCertVerifyTLSDemand(ISslX509VerificationCallback callback)
        {
            this.verificationCallback = callback;
        }

        /**
         * @return 1 for success, 0 for error. Returning error will cause the
         *          connection being torn down by the native code.
         *          Throwing exceptions from this callback is not desirable due
         *          to the intermediate layer of JNA and native code processing.
         */
        @Override
        public int callback(Pointer ld, PointerByReference ppServerCert)
        {
           try {
               logger.debug("In ServerCertVerifyTLSDemand callback");
               if ( ( ppServerCert != null && ppServerCert.getValue() != null && ppServerCert.getValue() != Pointer.NULL ) )
               {
                  try
                  {
                     Pointer pCertContext = ppServerCert.getValue();
                     CertContextNative certContext = new CertContextNative(pCertContext);

                     byte[] certBytes = new byte[certContext.cbCertEncoded];
                     certContext.pbCertEncoded.read(0, certBytes, 0, certContext.cbCertEncoded);
                     if (!verificationCallback.isTrustedCertificate(
                            SslUtil.decodeCertificate(certBytes))) {
                        logger.error(
                           String.format("Server SSL certificate not trusted; " +
                                         "encoding: %d, bytes: %s",
                                         certContext.dwCertEncodingType,
                                         Arrays.toString(certBytes)));
                        return 0;  //FALSE
                     }
                     CertInfoNative certInfo = new CertInfoNative(certContext.pCertInfo);

                     int version = certInfo.dwVersion; //CERT_V1==0;CERT_V3==2
                     logger.debug(String.format("cert version: [%d]", version));
                     String serialNumberStr = certInfo.getSerialNumberStr();
                     logger.debug(String.format("serial number: %s", serialNumberStr));

                     final Date now = new GregorianCalendar().getTime();
                     final Date notBefore = FILETIME.filetimeToDate(
                                               certInfo.NotBefore.dwHighDateTime,
                                               certInfo.NotBefore.dwLowDateTime);
                     if (now.before(notBefore)) {
                        logger.error(String.format("certificate is not valid until [%s]", notBefore));
                        return 0;  //FALSE
                     }

                     final Date notAfter = FILETIME.filetimeToDate(
                                               certInfo.NotAfter.dwHighDateTime,
                                               certInfo.NotAfter.dwLowDateTime);
                     if (now.after(notAfter)) {
                        logger.error(String.format("certificate expired at [%s]", notAfter));
                        return 0;  //FALSE
                     }
                  }
                  catch (Exception e) {
                      logger.error("exception during callback", e);
                      return 0;
                  }
                  finally
                  {
                     Crypt32Library.INSTANCE.CertFreeCertificateContext(ppServerCert.getValue());
                  }
               }
           } catch (Throwable t) {
              logger.error("Error during callcack: " + t.getMessage(), t);
              return 0;
           }
           return 1; //TRUE
        }
    };

    private interface Crypt32Library extends Library
    {
        Crypt32Library INSTANCE =
                (Crypt32Library) Native.loadLibrary( "crypt32.dll", Crypt32Library.class );

        int CertFreeCertificateContext( Pointer pCertContext );
/**
        BOOL WINAPI CryptHashCertificate(
                _In_     HCRYPTPROV_LEGACY hCryptProv,
                _In_     ALG_ID Algid,
                _In_     DWORD dwFlags,
                _In_     const BYTE *pbEncoded,
                _In_     DWORD cbEncoded,
                _Out_    BYTE *pbComputedHash,
                _Inout_  DWORD *pcbComputedHash
              );
*/
        int CryptHashCertificate(Pointer hCryptProv, int Algid, int dwFlags,
              Pointer pbEncoded, int cbEncoded, Pointer pbComputedHash, IntByReference pcbComputedHash);
    }

    private interface LdapClientLibrary extends Library
    {
        // all of the functions are cdelc
        // if changing options here, review LdapUtils.getString() and places where it is used
        LdapClientLibrary INSTANCE =
                (LdapClientLibrary) Native.loadLibrary(
                        "wldap32.dll",
                        LdapClientLibrary.class, W32APIOptions.UNICODE_OPTIONS);


        //LDAP* ldap_init( __in  PCHAR HostName, __in  ULONG PortNumber );
        /**
         * The ldap_init function initializes a session with an LDAP server.
         *
         * @param hostName A pointer to a null-terminated string that contains a domain name,
         *                 or a space-separated list of host names or dotted strings that represent
         *                 the IP address of hosts running an LDAP server to which to connect.
         *                 Each host name in the list can include an optional port number
         *                 which is separated from the host itself with a colon (:).
         *
         * @param portNumber Contains the TCP port number to which to connect.
         *                   Set to LDAP_PORT to obtain the default port, 389.
         *                   This parameter is ignored if a host name includes a port number.
         *
         * @return If the function succeeds, it returns a session handle, in the form of a pointer to an LDAP
         * data structure. The session handle must be freed with a call to ldap_unbind when it is no longer required.
         * If the function fails, it returns NULL. Use LdapGetLastError to retrieve the error code.
         */
        Pointer ldap_init(String hostName, int  portNumber);

        //LDAP* ldap_sslinit( __in  PCHAR HostName, __in  ULONG PortNumber, __in int secure );
        /**
         * The ldap_sslinit function initializes a Secure Sockets Layer (SSL) session with an LDAP server.
         *
         * @param hostName A pointer to a null-terminated string that contains a space-separated
         *                 list of host names or dotted strings representing the IP address of hosts
         *                 running an LDAP server to which to connect. Each host name in the list can
         *                 include an optional port number which is separated from the host itself
         *                 with a colon (:) character.
         *
         * @param portNumber Contains the TCP port number to which to connect.
         *                   This parameter is ignored if a host name includes a port number.
         *
         * @param secure  If nonzero, the function uses SSL encryption. If the value is 0, the function
         *                establishes a plain TCP connection and uses clear text (no encryption).
         * @return If the function succeeds, it returns a session handle, in the form of a pointer to an LDAP
         *         data structure. The session handle must be freed with a call to ldap_unbind when it is no
         *         longer required.
         */
        Pointer ldap_sslinit(String hostName, int  portNumber, int secure);


        // ULONG ldap_set_option( __in  LDAP *ld, __in  int option, __in  void *invalue );
        /**
         * The ldap_set_option function sets options on connection blocks.
         *
         * @param ld The session handle.
         *
         * @param option The name of the option set.
         *
         * @param value A pointer to the value that the option is to be given.
         *              The actual type of this parameter depends on the setting of the option parameter.
         *              The constants LDAP_OPT_ON and LDAP_OPT_OFF can be given for options
         *              that have on or off settings.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_set_option(Pointer ld, int option, Pointer value);
        int ldap_set_option(Pointer ld, int option, IServerCertVerify value);

        // ULONG ldap_get_option( __in LDAP *ld, __in int option, __out void *outvalue );
        /**
         * The ldap_get_option function retrieves the current values of session-wide parameters.
         *
         * @param ld The session handle.
         *
         * @param option The name of the option accessed.
         *
         * @param outValue The address of the option value.
         *                 The actual type of this parameter depends on the setting of the option parameter.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_get_option(Pointer ld, int option, Pointer outValue);

        // ULONG ldap_bind_s( __in LDAP *ld, __in PCHAR dn, __in PCHAR cred, __in ULONG method );
        /**
         * The ldap_bind_s function synchronously authenticates a client to the LDAP server.
         *
         * @param ld The session handle.
         *
         * @param dn Pointer to a null-terminated string that contains the distinguished name
         *           of the entry used to bind. This can be a DN, a UPN, a Windows NT style user name,
         *           or other name that the directory server will accept as an identifier.
         *
         * @param cred Pointer to a null-terminated string that contains the credentials
         *             with which to authenticate. Arbitrary credentials can be passed using this parameter.
         *             The format and content of the credentials depends on the setting of the method parameter.
         *
         * @param method Indicates the authentication method to use.
         *
         * @return  If the function succeeds, the return value is LDAP_SUCCESS.
         *          If the function fails, it returns an error code.
         */
        int ldap_bind_s(Pointer ld, String dn, String cred, int method);

        int ldap_bind_s(Pointer ld, String dn, Pointer cred, int method);

        /**
         * The ldap_connect function establishes a connection with the server.
         * The client is connected to the LDAP server as an anonymous user.
         *
         * @param ld
         *            The session handle.
         *
         * @param timeout
         *            Pointer to an LDAP_TIMEVAL structure that specifies the
         *            number of seconds to spend in an attempt to establish a
         *            connection before a timeout. If NULL, the function uses a
         *            default timeout value.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_connect(Pointer ld, Pointer timeout);

        // ULONG ldap_add_s( __in  LDAP *ld, __in  PCHAR dn, __in  LDAPMod *attrs[] );
        /**
         *  The ldap_add_s function initiates a synchronous add operation that adds an entry to a tree.
         *  The parent of the entry being added must already exist or the parent must be empty
         *  (equal to the root distinguished name) for an add operation to succeed.
         *
         * @param ld The session handle.
         *
         * @param dn A pointer to a null-terminated string that contains the distinguished name of the entry to add.
         *
         * @param attributes An array of pointers to LDAPMod structures. Each structure specifies a single attribute.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_add_s(Pointer ld, String dn, Pointer[] attributes);

        // ULONG ldap_modify_s( __in  LDAP *ld, __in  PCHAR dn, __in  LDAPMod *mods[] );
        /**
         * The ldap_modify_s function changes an existing entry.
         *
         * @param ld The session handle.
         *
         * @param dn A pointer to a null-terminated string that contains the name of the entry to modify.
         *
         * @param attributes A null-terminated array of modifications to make to the entry.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_modify_s(Pointer ld, String dn, Pointer[] attributes);

        // ULONG ldap_search_s( __in   LDAP *ld, __in   PCHAR base, __in   ULONG scope, __in   PCHAR filter,
        //                      __in   PCHAR attrs[], __in   ULONG attrsonly, __out  LDAPMessage **res );
        /**
         * The ldap_search_s function synchronously searches the LDAP directory
         * and returns a requested set of attributes for each matched entry.
         *
         * @param ld Session handle.
         *
         * @param base Pointer to a null-terminated string that contains the distinguished name
         *             of the entry at which to start the search.
         *
         * @param scope Specifies one of the following values to indicate the search scope.
         *
         * @param filter Pointer to a null-terminated string that specifies the search filter.
         *
         * @param attrs A null-terminated array of null-terminated strings indicating the attributes
         *              to return for each matching entry. Pass NULL to retrieve all available attributes.
         *
         * @param attrsonly Boolean value that should be zero if both attribute types and values are to be returned,
         *                  nonzero if only types are required.
         *
         * @param res Contains the results of the search upon completion of the call.
         *            Can also contain partial results or extended data when the function call fails
         *            with an error code. Free results returned with a call to ldap_msgfree when
         *            they are no longer required by the application.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails it returns an error code, however ldap_search_s can fail and
         *         can still allocate pMsg. For example, both LDAP_PARTIAL_RESULTS and LDAP_REFERRAL
         *         error code allocate pMsg.
         */
        int ldap_search_s(
                Pointer            ld,
                String             base,
                int                scope,
                String             filter,
                String[]           attrs,
                int                attrsonly,
                PointerByReference res
        );

        int ldap_search_ext_s(
                Pointer            ld,
                String             base,
                int                scope,
                String             filter,
                String[]           attrs,
                int                attrsonly,
                Pointer[]          sctrls,
                Pointer[]          cctrls,
                Pointer            timeout,
                int                sizelimit,
                PointerByReference res
        );

        //  ULONG ldap_delete_s( __in  LDAP *ld, __in  PCHAR dn );
        /**
         *  The ldap_delete_s function is a synchronous operation that removes a leaf entry from the directory tree.
         *  (Be aware that LDAP does not support deletion of entire subtrees in a single operation.)
         *
         * @param ld The session handle.
         *
         * @param dn A pointer to a null-terminated string that contains the distinguished name of the entry to delete.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_delete_s(Pointer ld, String dn);

        // PCHAR ldap_get_dn( __in  LDAP *ld, __in  LDAPMessage *entry );
        /**
         * The ldap_get_dn function retrieves the distinguished name for a given entry.
         *
         * @param ld The session handle.
         *
         * @param msg The entry whose distinguished name is to be retrieved.
         *
         * @return If the function succeeds, it returns the distinguished name as a pointer
         *         to a null-terminated character string.
         *         If the function fails, it returns NULL and sets the session error parameters
         *         in the LDAP data structure.
         *
         *         The ldap_get_dn function retrieves the distinguished name for an entry
         *         that was returned by ldap_first_entry, or ldap_next_entry.
         *         When the returned name is no longer needed, free the string by calling ldap_memfree.
         */
        Pointer ldap_get_dn(Pointer ld, Pointer msg);

        // LDAPMessage* ldap_first_entry( __in  LDAP *ld, __in  LDAPMessage *res );
        /**
         * The ldap_first_entry function returns the first entry of a message.
         *
         * @param ld The session handle.
         *
         * @param pMessage The search result, as obtained by a call to one of the synchronous
         *                 search routines or ldap_result.
         *
         * @return If the search returned valid results, this function returns a pointer to the first result entry.
         *         If no entry or reference exists in the result set, it returns NULL.
         *         This is the only error return; the session error parameter in the LDAP data structure
         *         is cleared to 0 in either case.
         *         You do not have to explicitly free the returned entry as it is freed when the message itself is freed.
         */
        Pointer ldap_first_entry(Pointer ld, Pointer pMessage);

        // LDAPMessage* ldap_next_entry( __in  LDAP *ld, __in  LDAPMessage *entry );
        /**
         * The ldap_next_entry function retrieves an entry from a search result chain.
         *
         * @param ld The session handle.
         *
         * @param pEntry The entry returned by a previous call to ldap_first_entry or ldap_next_entry.
         *
         * @return If the search returned valid results, this function returns a pointer to the next
         *         result entry in the results set.
         *         If no further entries or references exist in the result set, it returns NULL.
         *         This is the only error return; the session error parameter in the LDAP data
         *         structure is cleared to 0 in either case.
         *         You are not required to explicitly free the returned entry because it is freed
         *         when the message itself is freed.
         */
        Pointer ldap_next_entry(Pointer ld, Pointer pEntry);

        // ULONG ldap_count_entries( _In_  LDAP *ld, _In_  LDAPMessage *res );
        /**
         * The ldap_count_entries function counts the number of search entries that a server returned.
         * @param ld The session handle.
         * @param pMessage The search result obtained by a call to one of the synchronous search routines
         *                 or to ldap_result.
         * @return If the function succeeds, it returns the number of entries.
                   If the function fails, the return value is -1 and the function sets
                   the session error parameters in the LDAP data structure.
         */
        int ldap_count_entries(Pointer ld, Pointer pMessage);

        // PCHAR ldap_first_attribute( __in   LDAP *ld, __in   LDAPMessage *entry, __out  BerElement **ptr );
        /**
         * For a given directory entry, the ldap_first_attribute function returns the first attribute.
         *
         * @param ld The session handle.
         *
         * @param pEntry The entry whose attributes are to be stepped through, as returned by ldap_first_entry
         *               or ldap_next_entry.
         *
         * @param ppBerElem The address of a pointer used internally to track the current position in the entry.
         *
         * @return A pointer to a null-terminated string.
         *         If the function succeeds, it returns a pointer to an allocated buffer that contains
         *         the current attribute name.
         *         When there are no more attributes to step through, it returns NULL.
         *         The session error parameter in the LDAP data structure is set to 0 in either case.
         *         If the function fails, it returns NULL and sets the session error parameter
         *         in the LDAP data structure to the LDAP error code.
         *
         *         A call to ldap_first_attribute allocates, and returns through the ptr parameter,
         *         a pointer to a BerElement structure. Pass this pointer to ldap_next_attribute to track the current
         *         position in the list of attributes. When you have finished stepping through a list of attributes
         *         and ptr is non-NULL, free the pointer by calling ber_free( ptr, 0 ).
         *         Be aware that you must pass the second parameter as 0 (zero) in this call.
         *
         *         Both ldap_first_attribute and ldap_next_attribute return a pointer to an allocated buffer
         *         containing the current attribute name. Free this buffer,
         *         when no longer required, by calling ldap_memfree.
         */
        Pointer ldap_first_attribute( Pointer ld, Pointer pEntry, PointerByReference ppBerElem);

        // PCHAR ldap_next_attribute( __in LDAP *ld, __in LDAPMessage *entry, __inout BerElement *ptr );
        /**
         * For a given entry, the ldap_next_attribute function returns the next attribute.
         *
         * @param ld The session handle.
         *
         * @param pEntry The entry whose attributes are to be stepped through,
         *               as returned by ldap_first_entry or ldap_next_entry.
         *
         * @param pBerElem The address of a pointer used internally to track the current position in the entry.
         *
         * @return If the function succeeds, it returns a pointer to a null-terminated string that contains
         *         the current attribute name. If there are no more attributes to step through, it returns NULL.
         *         The session error parameter in the LDAP data structure is set to 0 in either case.
         *         If the function fails, it returns NULL and sets the session error parameter
         *         in the LDAP data structure to the LDAP error code.
         *
         *         A call to ldap_next_attribute returns, through the ptr parameter, a pointer
         *         to a BerElement structure. Pass this pointer to the next call to ldap_next_attribute
         *         to track the current position in the list of attributes. When you have finished stepping
         *         through a list of attributes, and ptr is non-NULL, free the pointer
         *         by calling ber_free (ptr, 0).
         *         Be aware that you must pass the second parameter as 0 (zero) in this call.
         *
         *         The ldap_next_attribute function returns a pointer to an internally allocated buffer
         *         that contains the current attribute name. Free this buffer, when no longer required,
         *         by calling ldap_memfree.
         */
        Pointer ldap_next_attribute(Pointer ld, Pointer pEntry, Pointer pBerElem);

        // struct berval** ldap_get_values_len(
        // __in  LDAP *ExternalHandle, __in  LDAPMessage *Message, __in  PCHAR attr );
        /**
         * The ldap_get_values_len function retrieves the list of values for a given attribute.
         *
         * @param ld The session handle.
         *
         * @param pEntry Handle to the LDAPMessage structure.
         *
         * @param attrName A pointer to a null-terminated string that contains
         *                 the attribute whose values are to be retrieved.
         *
         * @return If the function succeeds, it returns a null-terminated list of pointers to
         *         berval structures that contain the values of the specified attribute.
         *         If no attribute values were found, it returns NULL.
         *         The session error parameter in the LDAP data structure is set to 0 in either case.
         *         If the function fails, it returns NULL and the session error parameter
         *         in the LDAP data structure is set to the LDAP error code.
         *
         *         Call ldap_value_free_len to release the returned value when it is no longer required.
         */
        Pointer ldap_get_values_len(Pointer ld, Pointer pEntry, String attrName);

        // ULONG ldap_count_values_len( __in  struct berval **vals );
        /**
         * The ldap_count_values_len function counts the number of values in a list.
         *
         * @param ppBerValues An array of values returned by ldap_get_values_len.
         *
         * @return This function returns the number of values in the array. There is no error return.
         *         If a NULL pointer is passed as the argument, 0 is returned.
         *         If an invalid argument is passed, the value returned is undefined.
         */
        int ldap_count_values_len(Pointer ppBerValues);

        // ULONG ldap_value_free_len( __in  struct berval **vals );
        /**
         * The ldap_value_free_len frees berval structures that were returned by ldap_get_values_len.
         *
         * @param ppBerValues The structure to free.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         */
        int ldap_value_free_len(Pointer ppBerValues);

        // ULONG ldap_unbind( __in  LDAP *ld );
        /**
         * The ldap_unbind function frees resources associated with an LDAP session.
         *
         * @param ld The session handle.
         *
         * @return If the function succeeds, the return value is LDAP_SUCCESS.
         *         If the function fails, it returns an error code.
         *
         *         Call ldap_unbind to unbind from the directory, close the connection, and dispose
         *         of the session handle. Call this function when you have finished with an LDAP
         *         connection structure, even if you have not called ldap_bind when opening the connection.
         *
         *         The ldap_unbind function is for the asynchronous set of APIs,
         *         but it completes synchronously. There is no server response to an unbind operation.
         */
        int ldap_unbind(Pointer ld);

        // ULONG ldap_msgfree( __in  LDAPMessage *res );
        /**
         * The ldap_msgfree function frees the results obtained from a previous call
         * to ldap_result, or to one of the synchronous search routines.
         *
         * @param msg The result, or chain of results, to free.
         *
         * @return Returns LDAP_SUCCESS.
         */
        int ldap_msgfree(Pointer msg);

        // VOID ldap_memfree( __in  PCHAR Block );
        /**
         * The ldap_memfree function frees memory allocated from the LDAP heap.
         *
         * Call ldap_memfree to free strings, such as the attribute names returned by ldap_*_attribute,
         * or distinguished names returned by ldap_get_dn.
         * Do not free the static buffers used by ldap_open, ldap_get_values, and others.
         *
         * @param mem A pointer to a null-terminated string that contains a pointer
         *            to memory allocated by the LDAP library.
         */
        void ldap_memfree(Pointer mem);

        // PCHAR ldap_err2string( __in  ULONG err );
        /**
         * The ldap_err2string function converts a numeric LDAP error code
         * into a null-terminated character string that describes the error.
         *
         * @param errCode An LDAP error code as returned by another LDAP function.
         *
         * @return If the function succeeds, a pointer to a null-terminated character string
         *         that describes the error, is returned.
         *         If the function fails, a pointer to NULL is returned.
         *
         *         Be aware that some of the asynchronous calls return -1. In this case,
         *         use LdapGetLastError to retrieve the LDAP error code, and then use ldap_err2string on that value.
         *
         *         The return value is a static pointer to the character string. Do not free this string.
         */
        String ldap_err2string(int errCode);

        // void ber_free( __in  BerElement *pBerElement, __in  INT fbuf );
        /**
         * The ber_free function frees a BerElement structure that was previously allocated
         * with ber_alloc_t, ber_init, or the ldap_first_attribute/ ldap_next_attribute search functions.
         *
         * @param berElement Pointer to the BerElement structure to be deallocated.
         *
         * @param freebuf Must be set to 0 if freeing structures allocated by ldap_first_attribute/ldap_next_attribute,
         *                otherwise it must be set to 1.
         */
        void ber_free( Pointer berElement, int freebuf );

        void ber_bvfree( Pointer berVal );

        //ULONG LdapGetLastError(void);
        /**
         * The LdapGetLastError function retrieves the last error code returned by an LDAP call.
         *
         * @return Last LDAP error code.
         */
        int LdapGetLastError();

        // ULONG ldap_create_page_control(__in  LDAP *ld, ULONG pageSize, __in  struct berval *Cookie, UCHAR isCrtical, __out LdapControl* control)
        /**
         * The ldap_create_paged_control function creates a basic control for paging results.
         */
        int
        ldap_create_page_control(
            Pointer ld,
            int pageSize,
            Pointer cookie,
            char isCritical,
            PointerByReference ppControl);

        // ULONG ldap_parse_result(__in LDAP* ld, __in __LdapMessage* resultMessage, __out __opt ULONG* returnCode, __out __opt PCHAR* matchedDNs, __out __opt PCHAR* errorMessage, __out __opt PCHAR* referrals, __out __opt LdapControl** ServerControls, __in Boolean freeit)
        /*
         * The ldap_parse_result parses response from the server and returns the appropriate fields.
         */
        int
        ldap_parse_result(
            Pointer ld,
            Pointer pMessageResult,
            PointerByReference errorCodep,
            String[] matchedDnp,
            String[] errMsagp,
            Pointer referralsp,
            PointerByReference serverCtrlsp,
            int freeit);

        // ULONG ldap_parse_page_control(__in LDAP* ld, __in LdapControl* serverControls, __out ULONG* totalCount, __out struct berval** Cookie)
        int
        ldap_parse_page_control(
            Pointer ld,
            Pointer sctrls,
            PointerByReference totalCountp,
            PointerByReference cookiep);

        // ULONG ldap_control_free( __in LdapControl* control)
        void
        ldap_control_free(Pointer ctrl);

        // ULONG ldap_controls_free( __in LdapControl** control)
        void
        ldap_controls_free(Pointer ctrls);
    }

    @Override
    public void ldap_sasl_srp_bind_s(Pointer ld, String upn, String password) {
        throw new UnsupportedOperationException("ldap_sasl_srp_bind_s is not supported in Windows Native Ldap InterOp");
    }
}

