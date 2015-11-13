/* **********************************************************************
 * Copyright 2011-2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.impl.util;

import java.lang.reflect.Array;
import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Provides validate methods for checking the value of given argument. If
 * validation passes the methods will return silently, otherwise -
 * {@link IllegalArgumentException} should be thrown.
 */
public final class ValidateUtil {

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
         logAndThrow(String.format("%s should be a positive number but was: %d",
            fieldName, fieldValue));
      }
   }

   /**
    * Validate the the given value is not a negative number (accepting zero).
    *
    * @param fieldValue
    *           field value to validate
    * @param fieldName
    *           field name
    *
    * @throws IllegalArgumentException
    *            on validation failure
    */
   public static void validateNotNegative(long fieldValue, String fieldName) {

      if (fieldValue < 0) {
         logAndThrow(String.format("%s should not be a negative number but was: %d",
            fieldName, fieldValue));
      }
   }

   /**
    * Validate the the given value is an inclusive range.
    *
    * @param fieldValue
    *           field value to validate
    * @param fieldName
    *           field name
    *
    * @throws IllegalArgumentException
    *            on validation failure
    */
   public static void validateRange(long fieldValue, String fieldName,
         long lower, long upper) {

      if (fieldValue < lower || fieldValue > upper) {
         logAndThrow(String.format("%s should be in [%d-%d] but was %d",
            fieldName, lower, upper, fieldValue));
      }
   }

   /**
    * Same as {@link #validateNotEmpty(Object, String)} but just for {@code
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
    * Check whether given object value is empty. Depending on argument runtime
    * type <i>empty</i> means:
    * <ul>
    * <li>for java.lang.String type - {@code null} value or empty string</li>
    * <li>for array type - {@code null} value or zero length array</li>
    * <li>for any other type - {@code null} value</li>
    * </ul>
    * <p>
    * Note that java.lang.String values should be handled by
    * {@link #isEmpty(Object)}
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

   private static void logAndThrow(String msg) {
      LogFactory.getLog(ValidateUtil.class).error(msg);
      throw new IllegalArgumentException(msg);
   }

   private static Log getLog() {
      return LogFactory.getLog(ValidateUtil.class);
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
    *           array which {@code null} elements to remove; requires {@code
    *           non-null} value
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

      if (LogFactory.getLog(ValidateUtil.class).isDebugEnabled()) {
         LogFactory.getLog(ValidateUtil.class).debug(
            "Null elements stripped from the array of " + clazz.getSimpleName()
               + ": " + Arrays.deepToString(source));
      }

      return trimmed;
   }

   @SuppressWarnings("unchecked")
   private static <T> T[] newArray(Class<T> clazz, int length) {
      return (T[]) Array.newInstance(clazz, length);
   }
}
