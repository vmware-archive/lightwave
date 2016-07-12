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

import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;

import org.apache.commons.lang.Validate;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.TypeMapper;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.NativeMemory;
import com.vmware.identity.interop.PlatformUtils;
import com.vmware.identity.interop.idm.IdmClientLibraryFactory;
import com.vmware.identity.interop.ldap.ssl.ASN1_String;
import com.vmware.identity.interop.ldap.ssl.X509CINFNative;
import com.vmware.identity.interop.ldap.ssl.X509Native101e;
import com.vmware.identity.interop.ldap.ssl.X509StoreCtxNative101e;
import com.vmware.identity.interop.ldap.ssl.X509_VAL;

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 1:22 PM
 * To change this template use File | Settings | File Templates.
 */
class OpenLdapClientLibrary extends NativeAdapter implements ILdapClientLibrary
{
   // see openLdap docs at : http://www.openldap.org/software/man.cgi?query=ldap

   private static final Log logger = LogFactory.getLog(OpenLdapClientLibrary.class);

   private interface SSLLibrary extends Library
   {
      ///lib64/libssl.so.1.0.2 on Linux, or ssleay32.dll on Windows
        SSLLibrary INSTANCE = Platform.isWindows() ?
                (SSLLibrary) Native.loadLibrary("ssleay32.dll", SSLLibrary.class) :
                (SSLLibrary) Native.loadLibrary("libssl.so.1.0.2", SSLLibrary.class);

      //#include <openssl/ssl.h>
      //int SSL_library_init(void);
      void SSL_library_init();

      //#include <openssl/ssl.h>
      //void SSL_load_error_strings(void);
      void SSL_load_error_strings();

      Pointer TLSv1_method();

      //#include <openssl/ssl.h>
      //SSL_CTX *SSL_CTX_new(const SSL_METHOD *method);
      Pointer SSL_CTX_new(Pointer method);

      //#include <openssl/ssl.h>
      //int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile, const char *CApath);
      int SSL_CTX_load_verify_locations(Pointer ctx, Pointer CAfile, Pointer CApath);

      //#include <openssl/ssl.h>
      //void SSL_CTX_set_verify(SSL_CTX *ctx, int mode, int (*verify_callback)(int, X509_STORE_CTX *))
      void SSL_CTX_set_verify(Pointer ctx, int mode, ISslCtxVerify ctxVerifyCallback);

      //#include <openssl/ssl.h>
      //void SSL_CTX_set_verify_depth(SSL_CTX *ctx,int depth);
      void SSL_CTX_set_verify_depth(Pointer ctx, int depth);

      //#include <openssl/ssl.h>
      //void SSL_CTX_set_cert_verify_callback(SSL_CTX *ctx, int (*callback)(X509_STORE_CTX *,void *), void *arg)
      void SSL_CTX_set_cert_verify_callback(Pointer ctx, ISslCertVerify certVerifyCallback, Pointer arg);

      //#include <openssl/ssl.h>
      //void SSL_CTX_free(SSL_CTX *ctx);
      void SSL_CTX_free(Pointer ctx);

      //#include <openssl/evp.h>
      //const EVP_MD *EVP_sha1(void);
      Pointer EVP_sha1();

      //#include <openssl/evp.h>
      //int X509_digest(const X509 *data,const EVP_MD *type, unsigned char *md, unsigned int *len);
      int X509_digest(Pointer data, Pointer type, Pointer md, IntByReference len);

      // include <openssl/x509.h>
      // int i2d_X509(X509 *x, unsigned char **out);
      int i2d_X509(Pointer x, PointerByReference out);
   }

   private interface ISslCtxVerify extends Callback
   {

       // The verify_callback function is used to control the behaviour when the
       // SSL_VERIFY_PEER flag is set. It must be supplied by the application and
       // receives two arguments: preverify_ok indicates, whether the
       // verification of the certificate in question was passed (preverify_ok=1)
       // or not (preverify_ok=0). x509_ctx is a pointer to the complete context
       // used for the certificate chain verification.
       //
       //int (*verify_callback)(int, X509_STORE_CTX *)
       int callback(int preverifyOK, Pointer pX509StoreCtx);
   }

   private static ISslCtxVerify _sslCtxVerify = new ISslCtxVerify() {
      @Override
      public int callback(int preverifyOK, Pointer pX509StoreCtx)
      {
         return preverifyOK;
      }
   };

   private interface ISslCertVerify extends Callback
   {

       // callback should return 1 to indicate verification success and 0 to
       // indicate verification failure. If SSL_VERIFY_PEER is set and callback
       // returns 0, the handshake will fail. As the verification procedure may
       // allow to continue the connection in case of failure (by always
       // returning 1) the verification result must be set in any case using the
       // error member of x509_store_ctx so that the calling application will be
       // informed about the detailed result of the verification procedure!
       //
       //
       // int (*callback)(X509_STORE_CTX *,void *), void *arg)
       int callback(Pointer pX509StoreCtx, Pointer arg);
   }

   private static class SslCertVerify implements ISslCertVerify
   {
      ISslX509VerificationCallback verificationCallback;

      private SslCertVerify(ISslX509VerificationCallback callback)
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
      public int callback(Pointer pX509StoreCtx, Pointer arg) {

         try {
            logger.debug("In SslCertVerify callback");
            Pointer pCert           = null;
            X509CINFNative x509CInf = null;

            X509StoreCtxNative101e storeCtx = new X509StoreCtxNative101e(pX509StoreCtx);
            pCert = storeCtx.cert;
            X509Native101e x509 = new X509Native101e(pCert);
            x509CInf = new X509CINFNative(x509.cert_info);

            if (!checkValidity(new X509_VAL(x509CInf.validity)))
            {
               return 0;  //invalid certificate
            }

            int certLen = SSLLibrary.INSTANCE.i2d_X509(pCert, null);
            if (certLen < 0) {
               throw new Exception(
                  String.format("Native call i2d_X509() failed: %d", certLen));
            }
            byte[] certBytes = null;
            PointerByReference prCert = new PointerByReference();
            try (NativeMemory certBuf = new NativeMemory(certLen)) {
               prCert.setValue(certBuf);
               int res = SSLLibrary.INSTANCE.i2d_X509(pCert, prCert);
               if (res < 0) {
                  throw new Exception(
                     String.format("Native call i2d_X509() failed: %d", res));
               }
               certBytes = new byte[certLen];
               certBuf.read(0, certBytes, 0, certLen);
            }
            if (!verificationCallback.isTrustedCertificate(
                   SslUtil.decodeCertificate(certBytes))) {
               logger.error(
                  String.format("Server SSL certificate not trusted; bytes: %s",
                                Arrays.toString(certBytes)));
               return 0;
            }
         } catch (Throwable t) {
            logger.error("Error during callback: " + t.getMessage(), t);
            return 0;
         }
         return 1;
      }

      private boolean checkValidity(X509_VAL x509Val)
      {
         final Date now = new GregorianCalendar().getTime();
         SimpleDateFormat dateF = new SimpleDateFormat("yyMMddHHmmssX");
            ASN1_String notBefore = new ASN1_String(x509Val.notBefore);

         try {
            final Date notBeforeD = dateF.parse(new String(notBefore.data.getByteArray(0, notBefore.length)));
            if (now.before(notBeforeD))
            {
               logger.error(String.format("certificate is not valid until [%s]", notBeforeD));
               return false;
            }
         } catch (ParseException e) {
            logger.error("error when trying to get notBefore date", e);
            return false;
         }

         ASN1_String notAfter = new ASN1_String(x509Val.notAfter);
         try {
            final Date notAfterD = dateF.parse(new String(notAfter.data.getByteArray(0, notAfter.length)));
            if (now.after(notAfterD))
            {
               logger.error(String.format("certificate expired at [%s]", notAfterD));
               return false;
            }
         } catch (ParseException e) {
            logger.error("error when trying to get notAfter date", e);
            return false;
         }
         return true;
      }
   }

   static
   {
      SSLLibrary.INSTANCE.SSL_library_init();
      SSLLibrary.INSTANCE.SSL_load_error_strings();
      logger.info("SSL library initialized successfully");
   }

   private interface LBerLibrary extends Library
   {
       LBerLibrary INSTANCE = Platform.isWindows() ?
               (LBerLibrary) Native.loadLibrary("liblber.dll", LBerLibrary.class) :
               (LBerLibrary) Native.loadLibrary("liblber.so", LBerLibrary.class);

       void
       ber_free( Pointer berElement, int freebuf );

       void
       ber_bvfree( Pointer berVal );
   }

   private interface LdapClientLibrary extends Library
   {
      LdapClientLibrary INSTANCE =
            Platform.isWindows()?
            (LdapClientLibrary) Native.loadLibrary("libldap_r.dll", LdapClientLibrary.class):
            (LdapClientLibrary) Native.loadLibrary("ldap_r", LdapClientLibrary.class);

      int
      ldap_initialize(PointerByReference prLd, String url);

      Pointer
      ldap_open(String hostName, int  portNumber);

      int
      ldap_set_option(Pointer ld, int option, Pointer value);

      int
      ldap_get_option(Pointer ld, int option, Pointer outValue);

      int
      ldap_get_option(Pointer ld, int option, PointerByReference prOutValue);

      int
      ldap_bind_s(Pointer ld, Pointer dn, Pointer cred, int method);

      int
      ldap_sasl_bind_s(Pointer ld, String dn, Pointer mechs,
              Pointer cred, Pointer[] sctrls, Pointer[] cctrls,
              Pointer[] svrcreds);

      int
      ldap_sasl_interactive_bind_s(
          Pointer ld,
          String dn,
          String mechs,
          Pointer[] sctrls,
          Pointer[] cctrls,
          int flags,
          LdapSaslSRPInteractFunc interact,
          Pointer defaults
          );

      int
      ldap_add_s(Pointer ld, Pointer dn, Pointer[] attributes);

      int
      ldap_modify_s(Pointer ld, Pointer dn, Pointer[] attributes);

      int
      ldap_search_s(
            Pointer            ld,
            Pointer            baseDn,
            int                scope,
            String             filter,
            Pointer[]           attrs,
            int                attrsonly,
            PointerByReference res
            );

      int
      ldap_search_ext_s(
            Pointer            ld,
            Pointer            baseDn,
            int                scope,
            String             filter,
            Pointer[]           attrs,
            int                attrsonly,
            Pointer[]          sctrls,
            Pointer[]          cctrls,
            Pointer            timeout,
            int                sizelimit,
            PointerByReference res
            );

      int
      ldap_delete_s(Pointer ld, Pointer dn);

      Pointer
      ldap_get_dn(Pointer ld, Pointer msg);

      Pointer
      ldap_first_entry(Pointer ld, Pointer pMessage);

      Pointer
      ldap_next_entry(Pointer ld, Pointer pEntry);

      int
      ldap_count_entries(Pointer ld, Pointer pMessage);

      Pointer
      ldap_first_attribute(
            Pointer            ld,
            Pointer            pEntry,
            PointerByReference ppBerElem);

      Pointer
      ldap_next_attribute(Pointer ld, Pointer pEntry, Pointer pBerElem);

      /**
       * Use the following function only when the attribute is known to have
       * a string value.
       *
       * @param ld        LDAP connection handle
       * @param pEntry    Handle to entry in LDAP search result
       * @param attrName  Name of attribute whose values are sought
       * @return          Array of string values
       */
      //        Pointer
      //        ldap_get_values(Pointer ld, Pointer pEntry, String attrName);

      /**
       * Use this only in conjunction with ldap_get_values(...)
       *
       * @param ppszValues Values returned by a call to ldap_get_values(...)
       * @return           Count of string values passed in
       */
      //        int
      //        ldap_count_values(Pointer ppszValues);

      /**
       * Use the following function when the attribute may not be a string
       *
       * @param ld        LDAP connection handle
       * @param pEntry    Handle to entry in LDAP search result
       * @param attrName  Name of attribute whose values are sought
       * @return          Array of BerVal structures
       */
      Pointer
      ldap_get_values_len(Pointer ld, Pointer pEntry, String attrName);

      /**
       * Use this in conjunction with the call to ldap_get_values_len(...)
       *
       * @param ppBerValues Array of BerVal Structures
       * @return            Count of BerVal structures in array passed in
       */
      int
      ldap_count_values_len(Pointer ppBerValues);

      //        void
      //        ldap_value_free(Pointer ppszValues);

      // LDAP_F( void ) ldap_value_free_len LDAP_P((struct berval **vals ));
      void
      ldap_value_free_len(Pointer ppBerValues);

      int
      ldap_unbind_s(Pointer ld);

      int
      ldap_msgfree(Pointer msg);

      void
      ldap_memfree(Pointer mem);

      String
      ldap_err2string(int errCode);

      /**
       * Create a paged results control used when paging search results
       */
      int
      ldap_create_page_control(
          Pointer ld,
          int pageSize,
          Pointer cookie,
          char isCritical,
          PointerByReference ppControl);

      int
      ldap_parse_result(
          Pointer ld,
          Pointer pMessageResult,
          PointerByReference errorCodep,
          Pointer[] matchedDnp,
          Pointer[] errMsagp,
          Pointer referralsp,
          PointerByReference serverCtrlsp,
          int freeit);

      int
      ldap_parse_page_control(
          Pointer ld,
          Pointer sctrls,
          PointerByReference totalCountp,
          PointerByReference cookiep);

      void
      ldap_control_free(Pointer ctrl);

      void
      ldap_controls_free(Pointer ctrls);
   }

   private static final OpenLdapClientLibrary _instance = new OpenLdapClientLibrary();

   public static OpenLdapClientLibrary getInstance() { return _instance; }

   @Override
   public LdapConnectionCtx ldap_initializeWithUri(URI uri, List<LdapSetting> connOptions)
   {
      Validate.notNull(connOptions);
      PointerByReference prConnection = new PointerByReference();
      prConnection.setValue(Pointer.NULL);

      if (logger.isDebugEnabled()){
         logger.debug("start initializing client: " + uri);
      }

      OpenLdapClientLibrary.CheckError(
            LdapClientLibrary.INSTANCE.ldap_initialize(prConnection, uri.toString()));
      LdapConnectionCtx connectionCtx = ldap_set_options(prConnection, connOptions);

      Pointer pConnection = prConnection.getValue();
      if ( (pConnection == null) || (pConnection == Pointer.NULL) )
      {
         logger.error(" cannot establish connection: " + uri);
         OpenLdapClientLibrary.CheckError(LdapErrors.LDAP_OpenLdap_CONNECT_ERROR.getCode());
      }

      if (logger.isDebugEnabled()){
         logger.debug("client initialized");
      }
      return connectionCtx;
   }

   @Override
   public Pointer ldap_init(String hostName, int portNumber)
   {
      if (logger.isTraceEnabled()){
         logger.trace(
               String.format("start initializing connection with hostname:port = [%s:%d]", hostName, portNumber));
      }
      Pointer connection = LdapClientLibrary.INSTANCE.ldap_open( hostName, portNumber );
      if ( (connection == null) || (connection == Pointer.NULL) )
      {
         // get proper error code from errno ....
         /*
            OpenLdap:
                If an error occurs, ldap_open() and ldap_init() will  return  NULL  and
                errno should be set appropriately.
          */
         OpenLdapClientLibrary.CheckError( LdapErrors.LDAP_OpenLdap_CONNECT_ERROR.getCode() );
      }
      if (logger.isTraceEnabled()){
         logger.trace("connection initialized");
      }
      return connection;
   }

   private static LdapConnectionCtx ldap_set_options(PointerByReference prConnection, List<LdapSetting> connOptions)
   {
      Pointer ld = prConnection.getValue();
      PointerByReference prSslContext = new PointerByReference(Pointer.NULL);

      // Callback object (from platform) to client, where it is set
      ISslX509VerificationCallback callback = null;
      // This is the OpenSSL callback during the process of SSL handshake.
      ISslCertVerify sslCertVerifyObj = null;
      boolean tlsDemand = false;
      int sslMinimumProtocol = LdapSSLProtocols.getDefaultMinProtocol().getCode();

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
            case LDAP_OPT_X_SASL_NOCANON:
                Validate.notNull(val);
                IntByReference rIntSaslNoCanon = new IntByReference(((Integer)val).intValue());
                CheckSetOptionError(option, ld,
                    LdapClientLibrary.INSTANCE.ldap_set_option(
                        ld,
                        option.getCode(),
                        rIntSaslNoCanon.getPointer())
                );
                break;
            case LDAP_OPT_PROTOCOL_VERSION:
               Validate.notNull(val);
               IntByReference rIntVer = new IntByReference(((Integer)val).intValue());
               CheckSetOptionError(option, ld,
                   LdapClientLibrary.INSTANCE.ldap_set_option(
                       ld,
                       option.getCode(),
                       rIntVer.getPointer())
               );
               break;
            case LDAP_OPT_X_CLIENT_TRUSTED_FP_CALLBACK:
               Validate.notNull(val);
               Validate.isTrue(val instanceof ISslX509VerificationCallback);

               callback = (ISslX509VerificationCallback)val;
               Validate.notNull(callback);
               sslCertVerifyObj = new SslCertVerify(callback);
               break;
            case LDAP_OPT_X_TLS_REQUIRE_CERT:
               Validate.notNull(val);
               IntByReference rIntVal = new IntByReference(((Integer)val).intValue());
               if (rIntVal.getValue() == LdapConstants.LDAP_OPT_X_TLS_NEVER)
               {
                  CheckSetOptionError(option, ld,
                      LdapClientLibrary.INSTANCE.ldap_set_option(
                          ld,
                          option.getCode(),
                          rIntVal.getPointer())
                      );

                  CheckSetSSLOptionError(SSLOption.LDAP_OPT_X_TLS_NEWCTX, ld,
                        LdapClientLibrary.INSTANCE.ldap_set_option(ld,
                        SSLOption.LDAP_OPT_X_TLS_NEWCTX.getCode(),
                        new IntByReference(LdapConstants.LDAP_OPT_OFF).getPointer()));
                  if (logger.isDebugEnabled()){
                     logger.debug("LDAP_OPT_X_TLS_REQUIRE_CERT set successfully: LDAP_OPT_X_TLS_NEVER");
                  }
               }
               else if (rIntVal.getValue() == LdapConstants.LDAP_OPT_X_TLS_DEMAND)
               {
                  tlsDemand = true;
               }
               else
               {
                  throw new IllegalStateException("Only TLS_NEVER or TLS_DEMAND should be used");
               }
               break;
            case LDAP_OPT_REFERRALS:
               Validate.notNull(val);
               Pointer pVal = Pointer.NULL;
               if (((Boolean)val).equals(Boolean.TRUE))
               {
                  pVal = new IntByReference(1).getPointer();
               }
               CheckSetOptionError(option, ld,
                   LdapClientLibrary.INSTANCE.ldap_set_option(
                       ld,
                       option.getCode(),
                       pVal)
               );
               break;
            case LDAP_OPT_NETWORK_TIMEOUT:
                Validate.notNull(val);
                TimevalNative rTimeoutVal = new
                    TimevalNative(((Long)val).longValue(), new Long(0).longValue());
                CheckSetOptionError(option, ld,
                    LdapClientLibrary.INSTANCE.ldap_set_option(
                        ld,
                        option.getCode(),
                        rTimeoutVal.getPointer())
                );
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
               String msg = String.format("unsupport options: [%s, %s]", option, val);
               logger.warn(msg);
               throw new IllegalArgumentException(msg);
            }
      }

      if (tlsDemand) {
          //setup the sub-options for TLS-DEMAND
          IntByReference rIntVal = new IntByReference(LdapConstants.LDAP_OPT_X_TLS_DEMAND);
          CheckSetOptionError(LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT, ld,
                LdapClientLibrary.INSTANCE.ldap_set_option(
                            ld,
                            LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT.getCode(),
                            rIntVal.getPointer()));

          IntByReference rSsl3 = new IntByReference(getOpenLdapProtocol(sslMinimumProtocol));
          CheckSetSSLOptionError(SSLOption.LDAP_OPT_X_TLS_PROTOCOL_MIN, ld,
                LdapClientLibrary.INSTANCE.ldap_set_option(
                            ld,
                            SSLOption.LDAP_OPT_X_TLS_PROTOCOL_MIN.getCode(),
                            rSsl3.getPointer()));

          CheckSetSSLOptionError(SSLOption.LDAP_OPT_X_TLS_NEWCTX, ld,
                LdapClientLibrary.INSTANCE.ldap_set_option(
                            ld,
                            SSLOption.LDAP_OPT_X_TLS_NEWCTX.getCode(),
                            new IntByReference(LdapConstants.LDAP_OPT_OFF).getPointer()));
          //query the standard TLS library context
          CheckGetSSLOptionError(SSLOption.LDAP_OPT_X_TLS_CTX, ld,
                LdapClientLibrary.INSTANCE.ldap_get_option(ld,
                SSLOption.LDAP_OPT_X_TLS_CTX.getCode(), prSslContext));
          Pointer pSslContext = prSslContext.getValue();
          assert (pSslContext != null);

          //set mode (don't need to set the CA cert)
          SSLLibrary.INSTANCE.SSL_CTX_set_verify(pSslContext, OpenLdapSSLConstants.SSL_VERIFY_PEER, null);

          Validate.notNull(sslCertVerifyObj);
          //use default peer certificate verification with call back
          SSLLibrary.INSTANCE.SSL_CTX_set_cert_verify_callback(pSslContext, sslCertVerifyObj, Pointer.NULL);
          SSLLibrary.INSTANCE.SSL_CTX_set_verify_depth(pSslContext, 0); //unlimited

          CheckSetSSLOptionError(SSLOption.LDAP_OPT_X_TLS_CTX, ld,
                LdapClientLibrary.INSTANCE.ldap_set_option(
                   ld, SSLOption.LDAP_OPT_X_TLS_CTX.getCode(), pSslContext));
          if (logger.isDebugEnabled()){
             logger.debug("LDAP_OPT_X_TLS_REQUIRE_CERT set successfully: LDAP_OPT_X_TLS_DEMAND");
          }
      }
      //return the connection context object to the caller.
      return new LdapConnectionCtx(ld, sslCertVerifyObj);
   }

   @Override
   public void ldap_set_option(Pointer ld, int option, Pointer value)
   {
      OpenLdapClientLibrary.CheckError(ld, LdapClientLibrary.INSTANCE.ldap_set_option(ld, option, value));
   }

   @Override
   public void ldap_bind_s(LdapConnectionCtx ctx, String dn, String cred, int method) {
       Validate.notNull(dn, "dn");
       Validate.notNull(cred, "cred");

       try(NativeMemory pDn = getNativeMemoryFromString(dn);
           NativeMemory pCred = getNativeMemoryFromString(cred))
       {
           OpenLdapClientLibrary.CheckError(ctx.getConnection(), LdapClientLibrary.INSTANCE.ldap_bind_s(ctx.getConnection(), pDn, pCred, method));
       }
   }

   @Override
   public void ldap_sasl_bind_s(Pointer ld, String userName, String domainName, String password)
   {
       IdmClientLibraryFactory.getInstance().getLibrary().LdapSaslBind(ld, userName, domainName, password);
   }

   final static LdapSaslSRPInteractFunc ldapSaslSrpInteractFunc = new LdapSaslSRPInteractFunc() {
       final int SASL_CB_LIST_END = 0;         /* end of list */
       final int SASL_CB_AUTHNAME = 0x4002;    /* client authentication name */
       final int SASL_CB_PASS = 0x4004;        /* client passphrase-based secret */

       @Override
       public int invoke(Pointer ld, int flags, Pointer pDefaults, Pointer pIn){
            try (SaslInputStructNative input = new SaslInputStructNative(
                    pDefaults)) {
               int offset = 0;
               while (true) {
                   SaslInteractStructNative interact = new SaslInteractStructNative(pIn,
                           offset);
                   interact.defResult = interact.result = Pointer.NULL;
                   interact.val = 0;
                   if (interact.id == SASL_CB_LIST_END) {
                       break;
                   } else if (interact.id == SASL_CB_AUTHNAME) {
                       interact.defResult = interact.result = input.authName;
                       interact.val = input.authNameLength;
                   } else if (interact.id == SASL_CB_PASS) {
                       interact.defResult = interact.result = input.password;
                       interact.val = input.passwordLength;
                   }
                   interact.writeField("defResult");
                   interact.writeField("result");
                   interact.writeField("val");

                   offset += interact.size();
               }
           }

           return LdapErrors.LDAP_SUCCESS.getCode();
       }
   };

    @Override
    public void ldap_sasl_srp_bind_s(Pointer ld, String userName, String password) {
        final int LDAP_SASL_QUIET = 2; /* Quiet: never prompt */

        try (SaslInputStructNative input = new SaslInputStructNative(userName,
                password)) {
            OpenLdapClientLibrary.CheckError(ld, LdapClientLibrary.INSTANCE.ldap_sasl_interactive_bind_s(
                    ld, null, "SRP", null, null, LDAP_SASL_QUIET, ldapSaslSrpInteractFunc, input.getPointer()));
        }
    }

   @Override
   public void ldap_add_s(Pointer ld, String dn, Pointer[] attributes)
   {
       Validate.notNull(dn, "dn");

       try(NativeMemory pDn = getNativeMemoryFromString(dn))
       {
           OpenLdapClientLibrary.CheckError(ld, LdapClientLibrary.INSTANCE.ldap_add_s(ld, pDn, attributes));
       }
   }

   @Override
   public void ldap_modify_s(Pointer ld, String dn, Pointer[] attributes)
   {
       Validate.notNull(dn, "dn");

       try(NativeMemory pDn = getNativeMemoryFromString(dn))
       {
           OpenLdapClientLibrary.CheckError(ld, LdapClientLibrary.INSTANCE.ldap_modify_s(ld, pDn, attributes));
       }
   }

   @Override
   public Pointer ldap_search_s(
         Pointer ld, String baseDn, int scope, String filter,
         String[] attrs, int attrsonly
         )
   {
      PointerByReference values = new PointerByReference();
      values.setValue(Pointer.NULL);
      NativeMemory pBaseDn = getNativeMemoryFromString(baseDn);
      Pointer[] attrArray = getNativeMemoryArray(attrs);

      try
      {
          OpenLdapClientLibrary.CheckError(ld,
                                LdapClientLibrary.INSTANCE.ldap_search_s(
                                            ld, pBaseDn, scope, filter, attrArray, attrsonly, values
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

         String ctx =
               String.format("base=%s, scope=%d, filter=%s, attrs=%s, attrsonly=%d",
                     baseDn==null? "null":baseDn,
                     scope,
                     filter==null?"null":filter,
                     attrs==null?"null":attrs.toString(),
                     attrsonly);
         logger.error("Exception when calling ldap_search_s: " + ctx, ex);
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
          if(pBaseDn != null) {
              pBaseDn.close();
          }
          freeNativeMemoryArray(attrArray);
      }
   }

   @Override
   public Pointer ldap_search_ext_s(
         Pointer ld, String baseDn, int scope, String filter,
         String[] attrs, int attrsonly, Pointer[] sctrls, Pointer[] cctrls, Pointer timeout, int sizelimit
         )
   {
      PointerByReference values = new PointerByReference();
      values.setValue(Pointer.NULL);
      NativeMemory pBaseDn = getNativeMemoryFromString(baseDn);
      Pointer[] attrArray = getNativeMemoryArray(attrs);

      try
      {
          int retVal =
              LdapClientLibrary.INSTANCE.ldap_search_ext_s(
                  ld, pBaseDn, scope, filter, attrArray, attrsonly, sctrls, cctrls, timeout, sizelimit, values
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
              OpenLdapClientLibrary.CheckError(ld, retVal);
          }
          else
          {
              // if the number of returned entries does not match our limit
              // propagate error
              int entries = LdapClientLibrary.INSTANCE.ldap_count_entries(ld, values.getValue());
              if ( ( entries == -1 ) || ( entries != sizelimit ) )
              {
                  OpenLdapClientLibrary.CheckError(ld, retVal);
              }
              else
              {
                  // openldap library does not reset this (errno from the current call carries over to the next call)
                  OpenLdapClientLibrary.clearErrNo(ld);
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

         String ctx =
               String.format("base=%s, scope=%d, filter=%s, attrs=%s, attrsonly=%d, sizelimit=%d",
                     baseDn==null?"null":baseDn,
                     scope,
                     filter==null?"null":filter,
                     attrs==null?"null":attrs.toString(),
                     attrsonly,
                     sizelimit);
         logger.error("Exception when calling ldap_search_ext_s: " + ctx, ex);
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
          if(pBaseDn != null) {
              pBaseDn.close();
          }
          freeNativeMemoryArray(attrArray);
      }
   }

   @Override
   public LdapPagedSearchResultPtr ldap_one_paged_search(
           Pointer ld,
           String baseDn,
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

       NativeMemory pBaseDn = getNativeMemoryFromString(baseDn);

       Pointer[] attrArray = getNativeMemoryArray(attrs);

       try
       {
           OpenLdapClientLibrary.CheckError(ld,
                    LdapClientLibrary.INSTANCE.ldap_create_page_control(ld,
                               pageSize,
                               pBerCookie,
                               pagingCriticality,
                               pPageControl)
                    );

           ppInputControls[0] = pPageControl.getValue();

           OpenLdapClientLibrary.CheckError(ld,
                    LdapClientLibrary.INSTANCE.ldap_search_ext_s(
                            ld, pBaseDn, scope, filter, attrArray, 0, ppInputControls, null, null, 0, pMessage)
                    );

           OpenLdapClientLibrary.CheckError(ld,
                    LdapClientLibrary.INSTANCE.ldap_parse_result(ld, pMessage.getValue(), errorcodep, null, null, null, ppReturnedControls, 0)
                    );

           OpenLdapClientLibrary.CheckError(ld,
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
               LBerLibrary.INSTANCE.ber_bvfree(pNewBerCookie.getValue());
               pNewBerCookie = new PointerByReference();
               pNewBerCookie.setValue(Pointer.NULL);
           }

           String ctx =
                   String.format("base=%s, scope=%d, filter=%s, attrs=%s, attrsonly=0, sizelimit=0",
                         baseDn==null?"null":baseDn,
                         scope,
                         filter==null?"null":filter,
                         attrs==null?"null":attrs.toString());
           logger.error("Exception when calling ldap_one_paged_search: " + ctx, ex);

           if (ex instanceof LdapException)
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

           if(pBaseDn != null) {
               pBaseDn.close();
           }

           freeNativeMemoryArray(attrArray);
       }
   }

   @Override
   public void ldap_delete_s(Pointer ld, String dn) {
       Validate.notNull(dn, "dn");

       try (NativeMemory pDn = getNativeMemoryFromString(dn))
       {
           OpenLdapClientLibrary.CheckError(ld, LdapClientLibrary.INSTANCE.ldap_delete_s(ld, pDn));
       }
   }

   @Override
   public Pointer ldap_get_dn(Pointer ld, Pointer msg)
   {
      Pointer p = LdapClientLibrary.INSTANCE.ldap_get_dn( ld, msg );
      OpenLdapClientLibrary.CheckError(ld, p);
      return p;
   }

   @Override
   public Pointer ldap_first_entry(Pointer ld, Pointer pMessage)
   {
      Pointer p = LdapClientLibrary.INSTANCE.ldap_first_entry( ld, pMessage );
      OpenLdapClientLibrary.CheckError(ld, p);
      return p;
   }

   @Override
   public Pointer ldap_next_entry(Pointer ld, Pointer pEntry)
   {
      Pointer p = LdapClientLibrary.INSTANCE.ldap_next_entry( ld, pEntry );
      OpenLdapClientLibrary.CheckError(ld, p);
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
      OpenLdapClientLibrary.CheckError(ld, p);
      return p;
   }

   @Override
   public Pointer ldap_next_attribute(Pointer ld, Pointer pEntry, Pointer pBerElem)
   {
       Pointer p = LdapClientLibrary.INSTANCE.ldap_next_attribute(ld, pEntry, pBerElem);
       OpenLdapClientLibrary.CheckError(ld, p);

       return p;
   }

   @Override
   public
   Pointer ldap_get_values_len(Pointer ld, Pointer pEntry, String attrName)
   {
      return LdapClientLibrary.INSTANCE.ldap_get_values_len(
            ld,
            pEntry,
            attrName);
   }

   @Override
   public int ldap_count_values_len(Pointer ppBerValues)
   {
      return LdapClientLibrary.INSTANCE.ldap_count_values_len(ppBerValues);
   }

   @Override
   public void ldap_value_free_len(Pointer ppBerValues)
   {
      LdapClientLibrary.INSTANCE.ldap_value_free_len(ppBerValues);
   }

   @Override
   public void ldap_unbind(Pointer ld)
   {
      OpenLdapClientLibrary.CheckError(LdapClientLibrary.INSTANCE.ldap_unbind_s(ld));

      if (logger.isDebugEnabled()){
         logger.debug("connection closed by client");
      }
   }

   @Override
   public void ldap_msgfree(Pointer msg)
   {
      /*
        OpenLdap:
            The ldap_msgfree() routine is used to free  the    memory    allocated  for
            result(s)  by  ldap_result()  or  ldap_search_ext_s(3) and friends.  It
            takes a pointer to the result or result chain to be freed  and  returns
            the  type  of the last message in the chain.  If the parameter is NULL,
            the function does nothing and returns zero.
       */
      LdapClientLibrary.INSTANCE.ldap_msgfree(msg);
   }

   @Override
   public void ldap_memfree(Pointer mem)
   {
      LdapClientLibrary.INSTANCE.ldap_memfree(mem);
   }

    @Override
    public void ber_free(Pointer berElement, int freebuf) {
        LBerLibrary.INSTANCE.ber_free(berElement, freebuf);
    }

    @Override
    public void ber_bvfree(Pointer berVal) {
        LBerLibrary.INSTANCE.ber_bvfree(berVal);
    }

   @Override
   public String ldap_err2string(int errorCode)
   {
      return LdapClientLibrary.INSTANCE.ldap_err2string(errorCode);
   }

   @Override
   public String getString(Pointer p) {
       String str = null;
       if( ( p != null ) &&( p != Pointer.NULL ) )
       {
           if(!Platform.isWindows()) {
               str = p.getString(0);
           } else {
               int bufLen = -1;
               for (int i = 0; i < Integer.MAX_VALUE; i++)
               {
                   if (p.getByte(i) == 0) // check end of string
                   {
                       bufLen = i;
                       break;
                   }
               }
               if ( ( bufLen == -1 ) || (bufLen > Integer.MAX_VALUE) )
               {
                   throw new RuntimeException("Invalid native string.");
               }
               else if(bufLen == 0)
               {
                   str = "";
               }
               else
               {
                   str = new String(p.getByteArray( 0 , bufLen ), PlatformUtils.getLdapServerCharSet());
               }
           }
       }
       return str;
   }

   @Override
   public TypeMapper getTypeMapper(){
       return null;
   }

   // private ctor
   private OpenLdapClientLibrary() {}

   private static void CheckError(int errorCode)
   {
      LdapErrorChecker.CheckError( errorCode, _instance );
   }

   private static void CheckError(Pointer ld, int errorCode) throws LdapException
   {
      try
      {
          LdapErrorChecker.CheckError( errorCode, _instance );
      }
      catch(LdapException ex)
      {
          // set errno on this connection 'ld' to 0
          // openldap library does not reset this (errno from the current call carries over to the next call)
          if (errorCode != LdapErrors.LDAP_SUCCESS.getCode())
          {
              OpenLdapClientLibrary.clearErrNo(ld);
          }

          // throw the original ldap exception
          throw ex;
      }
   }

   private static void CheckSetOptionError(LdapOption opt, Pointer ld, int errorCode) throws LdapSetOptionException
   {
      if (errorCode != LdapErrors.LDAP_SUCCESS.getCode())
      {
         OpenLdapClientLibrary.clearErrNo(ld);
         logger.error(String.format("ldap_set_option failed: %s", opt));
         throw new LdapSetOptionException(errorCode, opt);
      }
   }

   private static void CheckSetSSLOptionError(SSLOption opt, Pointer ld, int errorCode) throws LdapSetSSLOptionException
   {
      if (errorCode != LdapErrors.LDAP_SUCCESS.getCode())
      {
         OpenLdapClientLibrary.clearErrNo(ld);
         logger.error(String.format("ldap_set_option failed: %s", opt));
         throw new LdapSetSSLOptionException(errorCode, opt);
      }
   }

   private static void CheckGetSSLOptionError(SSLOption opt, Pointer ld, int errorCode) throws LdapGetSSLOptionException
   {
      if (errorCode != LdapErrors.LDAP_SUCCESS.getCode())
      {
         OpenLdapClientLibrary.clearErrNo(ld);
         logger.error(String.format("ldap_get_option failed: %s", opt));
         throw new LdapGetSSLOptionException(errorCode, opt);
      }
   }

   private static void clearErrNo(Pointer ld)
   {
       IntByReference rErrorVal = new IntByReference(LdapErrors.LDAP_SUCCESS.getCode());
       LdapClientLibrary.INSTANCE.ldap_set_option(
               ld,
               LdapOption.LDAP_OPT_RESULT_CODE.getCode(),
               rErrorVal.getPointer());
   }

   private static void CheckError(Pointer ld, Pointer pointer)
   {
      if (pointer == Pointer.NULL)
      {
          checkSessionError( ld );
      }
   }

   private static void CheckErrorOnIntResult(Pointer ld, int res)
   {
      if (res == -1)
      {
          checkSessionError( ld );
      }
   }

   private static void checkSessionError(Pointer ld)
   {
       IntByReference errorNumber = new IntByReference();
       errorNumber.setValue(LdapErrors.LDAP_SUCCESS.getCode());

       LdapClientLibrary.INSTANCE.ldap_get_option(
             ld, LdapOption.LDAP_OPT_RESULT_CODE.getCode(), errorNumber.getPointer()
             );

       OpenLdapClientLibrary.CheckError(ld, errorNumber.getValue());
   }

    private static int getOpenLdapProtocol(int code) {
        LdapSSLProtocols sslMinProtocol = LdapSSLProtocols.getProtocolByCode(code);
        switch (sslMinProtocol) {
        case SSLv3:
            return OpenLdapSSLConstants.LDAP_OPT_X_TLS_PROTOCOL_SSL3;
        case TLSv1_0:
            return OpenLdapSSLConstants.LDAP_OPT_X_TLS_PROTOCOL_TLS1_0;
        case TLSv1_1:
            return OpenLdapSSLConstants.LDAP_OPT_X_TLS_PROTOCOL_TLS1_1;
        case TLSv1_2:
            return OpenLdapSSLConstants.LDAP_OPT_X_TLS_PROTOCOL_TLS1_2;
        default:
            return OpenLdapSSLConstants.LDAP_OPT_X_TLS_PROTOCOL_TLS1_0;
        }
   }

    private static NativeMemory getNativeMemoryFromString(String s)
    {
        NativeMemory res = null;
        if( s != null )
        {
            byte[] array = null;
            array = s.getBytes(PlatformUtils.getLdapServerCharSet());
            res = new NativeMemory(array.length + 1); // + null terminator
            res.write(0, array, 0, array.length);
            res.setByte( array.length, (byte)0x0 );
        }

        return res;
    }

    private static Pointer[] getNativeMemoryArray(String[] a)
    {
        Pointer[] res = null;
        if( a != null )
        {
            res = new Pointer[a.length];
            for(int i=0; i<res.length; i++) {
                res[i] = getNativeMemoryFromString(a[i]);
            }
        }

        return res;
    }

    private static void freeNativeMemoryArray(Pointer[] array) {
        if(array != null) {
            for(Pointer pAttr: array) {
                if(pAttr != null && pAttr instanceof NativeMemory)
                   ((NativeMemory) pAttr).close();
            }
        }
    }
}
