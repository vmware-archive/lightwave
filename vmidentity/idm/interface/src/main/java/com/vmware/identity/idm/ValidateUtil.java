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
package com.vmware.identity.idm;

import java.lang.reflect.Array;
import java.security.cert.CertificateException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Provides validate methods for checking the value of given argument. If
 * validation passes the methods will return silently, otherwise -
 * {@link IllegalArgumentException} should be thrown.
 */
public final class ValidateUtil {

    public final static char UPN_SEPARATOR = '@';
    public final static char NETBIOS_SEPARATOR = '\\';
    public final static String spn_prefix = "STS/";

   /**
    * Validate the the given value is a positive number.
    *
    * @param fieldValue
    *           field value to validate
    * @param fieldName
    *           field name
    *
    * @throws IllegalArgumentException
    *            on validation failure
    */
   public static void validatePositiveNumber(long fieldValue, String fieldName) {

      if (fieldValue <= 0) {
         logAndThrow(String.format("%s should be a positive number: %d",
            fieldName, fieldValue));
      }
   }

    /**
     * Validate the the given value is a non-negative number.
     *
     * @param fieldValue
     *           field value to validate
     * @param fieldName
     *           field name
     *
     * @throws IllegalArgumentException
     *            on validation failure
     */
    public static void validateNonNegativeNumber(long fieldValue, String fieldName) {

        if (fieldValue < 0) {
            logAndThrow(String.format("%s should be a non-negative number: %d",
                    fieldName, fieldValue));
        }
    }

   /**
    * Same as {@link #validateNotNull(Object, String)} but just for {@code
    * null} value
    *
    * @throws IllegalArgumentException
    *            on validation failure
    */
   public static void validateNotNull(Object fieldValue, String fieldName) {

      if (fieldValue == null) {
         logAndThrow(String.format("'%s' value should not be NULL", fieldName));
      }
   }

    /**
     * Validates that given value is <code>null</code>.
     *
     * @throws IllegalArgumentException
     *            on validation failure
     */
    public static void validateNull(Object fieldValue, String fieldName) {

        if (fieldValue != null) {
            logAndThrow(String.format("'%s' value should be NULL", fieldName));
        }
    }

    /**
     * Validates that given certificate is <code>valid</code>.
     * clockTolerance - value of current clock tolerance in milliseconds
     * @throws IllegalArgumentException
     *            on validation failure
     */
    public static void validateSolutionDetail(SolutionDetail fieldValue, String fieldName, long clockTolerance) {

        X509Certificate cert = fieldValue.getCertificate();
        ValidateUtil.validateNotNull(cert, "Solution user certificate");
        try
        {
            cert.checkValidity();
        }
        catch(CertificateException ex)
        {
            if (ex instanceof CertificateNotYetValidException)
            {
                // Check to see whether certificate is within clock tolerance
                // if so do not throw, cert passes the validation
                if (cert.getNotBefore().getTime() <= System.currentTimeMillis() + clockTolerance)
                {
                    return;
                }
            }

            if (ex instanceof CertificateExpiredException)
            {
                // Check to see whether certificate is within clock tolerance
                // if so do not throw, cert passes the validation
                if (cert.getNotAfter().getTime() >= System.currentTimeMillis() - clockTolerance)
                {
                    return;
                }
            }

            logAndThrow(String.format("'%s' certificate is invalid - " +
                        "certificateException %s", fieldName, ex.toString()));
        }
    }

    /**
     * Validates that given certificate type is <code>valid</code>.
     * @throws IllegalArgumentException
     *            on validation failure
     */
    public static void validateCertType(CertificateType fieldValue, String fieldName)
    {
        if (fieldValue != CertificateType.LDAP_TRUSTED_CERT && fieldValue != CertificateType.STS_TRUST_CERT)
        {
            logAndThrow(String.format("'%s' value is invalid certificate type", fieldName));
        }
    }

    /**
     * Checks validity of the given certificate.
     * @throws IllegalArgumentException
     *            on validation failure
     */
    public static void validateCertificate(X509Certificate cert)
    {
        try {
            cert.checkValidity();
        } catch (Exception e) {
            logAndThrow(String.format("Certificate is not valid: %s", e.getMessage()));
        }
    }

   /**
    * Check whether given object value is empty. Depending on argument runtime
    * type <i>empty</i> means:
    * <ul>
    * <li>for java.lang.String type - {@code null} value or empty string</li>
    * <li>for array type - {@code null} value or zero length array</li>
    * <li>for any other type - {@code null} value</li>
    * </ul>
    * <p>
    * Note that java.lang.String values should be handled by
    * {@link #isEmpty(String)}
    * </p>
    *
    * @param obj
    *           any object or {@code null}
    *
    * @return {@code true} when the object value is {@code null} or empty array
    */
   public static boolean isEmpty(Object obj) {

      if (obj == null) {
         return true;
      }

      if (obj.getClass().equals(String.class)) {
         return ((String) obj).isEmpty();
      }

      if (obj.getClass().isArray()) {
         return Array.getLength(obj) == 0;
      }

      if (obj instanceof java.util.Collection<?>) {
         return ((java.util.Collection<?>) obj).isEmpty();
      }

      final String message = "String, java.lang.Array or java.util.Collection "
         + "expected but " + obj.getClass().getName() + " was found ";

      getLog().error(message);
      throw new IllegalArgumentException(message);
   }

   /**
    * Validates that given value is not empty (as defined by
    * {@link #isEmpty(Object)} contract). If validation check fails -
    * {@link IllegalArgumentException} will be thrown.
    * <p>
    * Useful for validation of required input arguments passed by end user at
    * public methods, etc.
    *
    * @param fieldValue
    *           field value to validate
    * @param fieldName
    *           field name
    *
    * @throws IllegalArgumentException
    *            on validation failure
    */
   public static void validateNotEmpty(Object fieldValue, String fieldName) {

      if (isEmpty(fieldValue)) {
         logAndThrow(String.format("'%s' value should not be empty", fieldName));
      }
   }

   public static void validateIdsDomainName(String domainName, DomainType domainType)
   {
       validateNotEmpty(domainName, "ids domainName");
   }

   /**
    * Validate IdentityStoreData attributes. Includes:
    *  domainName format validation
    *  domainAliase format validation
    *
    *
    * @param idsData
    */
   public static void validateIdsDomainNameNAlias(IIdentityStoreData idsData)
   {
      if (idsData.getDomainType().equals(DomainType.LOCAL_OS_DOMAIN))
         return;

      validateIdsDomainName(idsData.getName(), idsData.getDomainType());

      if (null != idsData.getExtendedIdentityStoreData())
      {
          String domainAlias = idsData.getExtendedIdentityStoreData().getAlias();
          if (domainAlias != null)
          {
              validateNotEmpty(domainAlias, "ids domainAlias");
          }
      }
   }

   public static String normalizeIdsKrbUserName(String krbUpn)
   {
       char[] validUpnSeparator = {UPN_SEPARATOR};
       int idxSep = getValidIdxAccountNameSeparator(krbUpn, validUpnSeparator);
       // if no '@' or multiple '@' or leading '@' exception is thrown
       if (idxSep == -1 || idxSep == 0)
       {
           logAndThrow(String.format("userkrbPrincipal [%s] is not in valid UPN format ", krbUpn));
       }
       String userName = krbUpn.substring(0, idxSep);
       validateNotEmpty(userName, "user account name in UPN");
       String domain   = krbUpn.substring(idxSep+1);
       validateNotEmpty(domain, "user REALM in UPN");

       return String.format("%s%c%s", userName, UPN_SEPARATOR, domain);
   }

   public static void validateUpn(String upn, String fieldName)
   {
       char[] validUpnSeparator = {UPN_SEPARATOR};
       int idxSep = getValidIdxAccountNameSeparator(upn, validUpnSeparator);
       // if no '@' or multiple '@' or leading '@' exception is thrown
       if (idxSep == -1 || idxSep == 0)
       {
           logAndThrow(String.format("%s=[%s] is invalid: not a valid UPN format ", fieldName, upn));
       }
       String userName = upn.substring(0, idxSep);
       validateNotEmpty(userName, "%s=[%s] is invalid: user name in UPN cannot be empty.");
       String domain   = upn.substring(idxSep+1);
       validateNotEmpty(domain, "%s=[%s] is invalid: domain suffix in UPN cannot be empty.");
   }

   public static void validateNetBios(String netbiosName, String fieldName)
   {
       char[] validUpnSeparator = {NETBIOS_SEPARATOR};
       int idxSep = getValidIdxAccountNameSeparator(netbiosName, validUpnSeparator);
       // if no '\\' or multiple '\\' or leading '\\' exception is thrown
       if (idxSep == -1 || idxSep == 0)
       {
           logAndThrow(String.format("%s=[%s] is invalid: not a valid netbios name format ", fieldName, netbiosName));
       }
       String userName = netbiosName.substring(0, idxSep);
       validateNotEmpty(userName, "%s=[%s] is invalid: user name in netbios name cannot be empty.");
       String domain   = netbiosName.substring(idxSep+1);
       validateNotEmpty(domain, "%s=[%s] is invalid: domain suffix in netbios name canot be empty.");
   }

   public static String validateIdsUserName(String userName, IdentityStoreType idsType, AuthenticationType authType)
   {
       // userName is used to bind to IDP
       // (1) Doing a kerberos bind, userName is in upn@REALM format
       // (2) Doing a simple bind to OpenLdap and vmware directory userName is in DN format
       // (3) Doing a simple bind to AD over LDAP userName can be in DN format, upn or NetBios\\userName
       if (authType == AuthenticationType.PASSWORD)
       {
           if (isValidDNFormat(userName))
           {
               return userName;
           }
           else if (idsType != IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING)
           {
               logAndThrow(String.format("userName [%s] is not in valid DN format ", userName));
           }

           // validate userName to be UPN format
           try
           {
               validateUpn(userName, "userName in UPN format");
           }
           catch(Exception e)
           {
               // validate userName to be Netbios\\userName
               validateNetBios(userName, "userName in Netbios format");
           }
           return userName;
       }
       else if (authType == AuthenticationType.USE_KERBEROS)
       {
           try
           {
               return normalizeIdsKrbUserName(userName);
           } catch (Exception e) {
               logAndThrow(String.format("userName [%s] is not in valid kerberos UPN format ", userName));
           }
       }
       else
       {
           logAndThrow(String.format("Unsupported IDS authentication type"));
       }

       return null;
   }

   // Simple validation of DN format. True:  succeeded in validation.
   // This method is not enforcing non-emptiness.
   public static void validateDNFormat(String dnString) {
       if (!isValidDNFormat(dnString)) {
           throw new IllegalArgumentException(String.format("dnString [%s] is not in valid DN format ", dnString));
       }
   }

   // RFC 4512: keystring(descr), numericoid
   private static Pattern attributeTypePattern = Pattern.compile("(([a-zA-Z]([a-zA-Z0-9\\x2D])*)|(([0-9]|([1-9]([0-9])+))(\\x2E([0-9]|([1-9]([0-9])+)))+))([ ])*\\=");

   // we will not validate the whole dn format syntax at the moment, but will check that the string comtain at least one instance of
   // <attributeType>= sequence
   public static boolean isValidDNFormat(String dnString)
   {
       boolean isValid = false;
       if (dnString != null && !dnString.isEmpty() )
       {
            Matcher m = attributeTypePattern.matcher(dnString);
            isValid = m.find();
       }
       return isValid;
   }

   /**
    * Validate extended IdentityStoreData attributes that should be in DN format.
    *
    * The attributes are:
    *  userName format validation
    *  userBaseDN format validation
    *  groupBaseDN format validation
    *
    * @param idsData
    */
   public static void validateIdsUserNameAndBaseDN(IIdentityStoreData idsData)
   {
      if (idsData.getDomainType().equals(DomainType.LOCAL_OS_DOMAIN))
         return;

      IIdentityStoreDataEx idsEx = idsData.getExtendedIdentityStoreData();
      if (null == idsEx) {
          return;
      }

      if (!idsData.getExtendedIdentityStoreData().useMachineAccount())
      {
          validateIdsUserName(idsEx.getUserName(),
                              idsData.getExtendedIdentityStoreData().getProviderType(),
                              idsData.getExtendedIdentityStoreData().getAuthenticationType());
      }

      // for AD IWA, skip groupBaseDN and userBaseDN check.
      if ( !(idsData.getExtendedIdentityStoreData().getProviderType()
                == IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY &&
             idsData.getExtendedIdentityStoreData().getAuthenticationType()
                == AuthenticationType.USE_KERBEROS
            ))
      {
          String groupBaseDN = idsEx.getGroupBaseDn();
          try {
              validateDNFormat(groupBaseDN);
          } catch (Exception e) {
              logAndThrow(String.format("groupBaseDN [%s] is not in valid DN format ", groupBaseDN));
          }

          String userBaseDN = idsEx.getUserBaseDn();
          try {
              validateDNFormat(userBaseDN);
          } catch (Exception e) {
              logAndThrow(String.format("userBaseDN [%s] is not in valid DN format ", userBaseDN));
          }
      }
   }

   public static int getValidIdxAccountNameSeparator(String userPrincipal, char[] validAccountNameSep)
   {
       // separator scan
       // validate there is no more than one separator is used
       int latestResult = -1;   // not found
       int totalSepCount = 0;
       for (char sep : validAccountNameSep)
       {
          int count = StringUtils.countMatches(userPrincipal, String.valueOf(sep));
          totalSepCount += count;
          if (totalSepCount > 1)
          {
              logAndThrow(String.format(
                "Invalid user principal format [%s]: only one separator of '@' or '\' is allowed.",
                userPrincipal));
          }
          if (count == 1)
          {
             latestResult = userPrincipal.indexOf(sep);
          }
       }

       return latestResult;
   }

   public static String getCanonicalUpnSuffix(String upnSuffix)
   {
       String resSuffix = null;
       if ( upnSuffix != null )
       {
           resSuffix = upnSuffix.toUpperCase();
       }
       return resSuffix;
   }

   private static void logAndThrow(String msg) {
      DiagnosticsLoggerFactory.getLogger(ValidateUtil.class).error(msg);
      throw new IllegalArgumentException(msg);
   }

   private static IDiagnosticsLogger getLog() {
      return DiagnosticsLoggerFactory.getLogger(ValidateUtil.class);
   }

   private ValidateUtil() {
      // prevent instantiation
   }
}
