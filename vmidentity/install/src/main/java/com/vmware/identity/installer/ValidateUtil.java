/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.installer;

import java.lang.reflect.Array;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.util.Arrays;

import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;

/**
 * Provides validate methods for checking the value of given argument. If
 * validation passes the methods will return silently, otherwise -
 * {@link IllegalArgumentException} should be thrown.
 */
public final class ValidateUtil {

   public static final int SERVICE_VERSION_MAX_LENGTH = 255;
   public static final int SERVICE_TYPE_MAX_LENGTH = 1024;
   public static final int VI_SITE_MAX_LENGTH = 255;
   public static final int SERVICE_OWNER_MAX_LENGTH = 255;
   public static final int SERVICE_DESCR_MAX_LENGTH = 2000;
   public static final int SERVICE_NAME_MAX_LENGTH = 255;
   public static final int PRODUCT_ID_MAX_LENGTH = 255;
   public static final int SERVICE_ID_MAX_LENGTH = 300;
   public static final int ENDPOINT_MAX_LENGTH = 2048;

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(ValidateUtil.class);

   private static final String INVALID_CERT_MSG = "Invalid certificate";

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
    * Same as {@link #validateNotEmpty(Object, String)} but just for
    * {@code null} value
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

      log.error(message);
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

   /**
    * Returns {@code non-null} array which is constructed from the given source
    * arrays stripped from all {@code null} elements.
    *
    * @param <T>
    *           elements type
    * @param sources
    *           accepts {@code null} everywhere
    * @param clazz
    *           the class type of source elements
    *
    * @return a valid instance representing the value of given source
    */
   public static <T> T[] notNullValueOf(Class<T> clazz, T[]... sources) {

      if (sources == null) {
         return newArray(clazz, 0);
      }

      // skip copying when sources has just one element
      if (sources.length == 1 && sources[0] != null) {
         return trimNulls(sources[0], clazz); // possibly return sources itself
      }

      // copy the sources array into one dimensional array

      int totalSize = 0;
      for (T[] src : sources) {
         if (src != null) {
            totalSize += src.length;
         }
      }

      T[] copyOfSources = newArray(clazz, totalSize);

      int copyPos = 0;
      for (T[] src : sources) {
         if (src != null) {
            System.arraycopy(src, 0, copyOfSources, copyPos, src.length);
            copyPos += src.length;
         }
      }

      return trimNulls(copyOfSources, clazz);
   }

   /**
    * Validates that if the fieldValue is not <code>null</code> it has at most
    * maxLength chars
    *
    * @param fieldValue
    * @param fieldName
    * @param maxLength
    */
   public static void validateStringMaxLength(String fieldValue,
      String fieldName, int maxLength) {
      if (fieldValue == null || fieldValue.length() <= maxLength) {
         return;
      }

      logAndThrow(fieldName + " value should not exceed " + maxLength
         + " chars");
   }

   public static void validateCertificate(X509Certificate cert) {
      try {
         cert.checkValidity();
      } catch (CertificateExpiredException e) {
         logAndThrow(INVALID_CERT_MSG, e);
      } catch (CertificateNotYetValidException e) {
         logAndThrow(INVALID_CERT_MSG, e);
      }
   }

   private static void logAndThrow(String msg) {
      log.error(msg);
      throw new IllegalArgumentException(msg);
   }

   private static void logAndThrow(String msg, Throwable e) {
      log.error(msg);
      throw new IllegalArgumentException(msg, e);
   }

   private ValidateUtil() {
      // prevent instantiation
   }

   /**
    * Trim {@code null} values from the source array. The same reference will be
    * returned when the source array does not contain {@code null} elements.
    *
    * @param <T>
    * @param source
    *           array which {@code null} elements to remove; requires
    *           {@code non-null} value
    * @param clazz
    *           the class type of source elements
    * @return {@code non-null} array stripped on {@code null} elements
    */
   private static <T> T[] trimNulls(T[] source, Class<T> clazz) {

      assert source != null;
      assert clazz != null;

      int nullsCount = 0;
      for (T data : source) {
         if (data == null) {
            nullsCount++;
         }
      }

      if (nullsCount == 0) {
         return source; // source array has no null elements
      }

      // create an array for not null elements
      T[] trimmed = newArray(clazz, source.length - nullsCount);

      // copy not null elements to the new array
      int i = 0;
      for (T data : source) {
         if (data != null) {
            trimmed[i++] = data;
         }
      }
      assert i == trimmed.length : i + " <> " + trimmed.length;

      if (log.isDebugEnabled()) {
         log.debug("Null elements stripped from the array of "
            + clazz.getSimpleName() + ": " + Arrays.deepToString(source));
      }

      return trimmed;
   }

   @SuppressWarnings("unchecked")
   private static <T> T[] newArray(Class<T> clazz, int length) {
      return (T[]) Array.newInstance(clazz, length);
   }
}
