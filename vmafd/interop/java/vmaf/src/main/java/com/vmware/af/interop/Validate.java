/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.af.interop;

import java.lang.reflect.Array;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Provides validate methods for checking the value of given argument. If
 * validation passes the methods will return silently, otherwise -
 * {@link IllegalArgumentException} should be thrown.
 */
public final class Validate {

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

   private static void logAndThrow(String msg) {
      LogFactory.getLog(Validate.class).error(msg);
      throw new IllegalArgumentException(msg);
   }

   private static Log getLog() {
      return LogFactory.getLog(Validate.class);
   }

   private Validate() {
      // prevent instantiation
   }
}
