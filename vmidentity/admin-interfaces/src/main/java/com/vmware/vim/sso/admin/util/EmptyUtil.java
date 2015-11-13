/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.util;

import java.lang.reflect.Array;
import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Utility methods
 */
@Deprecated
public final class EmptyUtil {

   private static final Log log = LogFactory.getLog(EmptyUtil.class);

   /**
    * Check whether given string value is empty
    * 
    * @param str
    *           any string or {@code null}
    * @return {@code true} when the string value is {@code null} or empty string
    */
   public static boolean isEmpty(String str) {
      return str == null || str.isEmpty();
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

      if (obj.getClass() == String.class) {
         return isEmpty((String) obj);
      }

      if (obj.getClass().isArray()) {
         return Array.getLength(obj) == 0;
      }

      if (obj instanceof java.util.Collection<?>) {
         return ((java.util.Collection<?>) obj).isEmpty();
      }

      if (obj instanceof javax.naming.Name) {
         return ((javax.naming.Name) obj).getAll().hasMoreElements();
      }

      throw new IllegalArgumentException(
         "Object or java.lang.Array expected but " + obj.getClass().getName()
            + " was found ");
   }

   /**
    * Same as {@link #validateNotEmpty(Object, String)} but just for {@code
    * null} value
    */
   public static void validateNotNull(Object fieldValue, String fieldName)
      throws IllegalArgumentException {

      if (fieldValue == null) {
         throw new IllegalArgumentException(String.format(
            "'%s' value should not be NULL", fieldName));
      }
   }

   /**
    * Validates that given value is not empty (as defined by
    * {@link #isEmpty(Object)} contract). If validation check fails -
    * {@link IllegalArgumentException} will be thrown.
    * <p>
    * Useful for validation of required input arguments passed by end user at
    * public methods, etc.
    * </p>
    * 
    * @param fieldValue
    *           field value to validate
    * @param fieldName
    *           field name
    * 
    * @throws IllegalArgumentException
    *            when field value is {@code null} or empty
    */
   public static void validateNotEmpty(Object fieldValue, String fieldName)
      throws IllegalArgumentException {

      if (isEmpty(fieldValue)) {
         throw new IllegalArgumentException(String.format(
            "'%s' value should not be empty", fieldName));
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
    * Default constructor
    */
   private EmptyUtil() {
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
