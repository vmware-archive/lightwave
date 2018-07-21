/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.wstrust.test.util;

import java.util.Collection;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Assert {
  private static final Logger log = LoggerFactory.getLogger(Assert.class);

  private Assert() {
  }

  /**
   * Checks whether the boolean expression is true. If it is not true
   * {@link AssertException} is thrown
   *
   * @param expression   an expression to be checked whether it is true
   * @param errorMessage a message to be logged if expression is false
   * @throws AssertException when the expression is false after logging of the
   *                         error message
   */
  public static void assertTrue(boolean expression,
                                String errorMessage)
      throws AssertException {
    assertTrue(expression, null, errorMessage);
  }

  /**
   * Checks whether the boolean expression is true. If it is not true
   * {@link AssertException} is thrown
   *
   * @param expression   an expression to be checked whether it is true
   * @param message      a message to be logged if the expression is true
   * @param errorMessage a message to be logged if the expression is false
   * @throws AssertException when the expression is false after logging of the
   *                         error message
   */
  public static void assertTrue(boolean expression,
                                String message,
                                String errorMessage)
      throws AssertException {
    assertBoolean(expression, message, errorMessage);
  }

  /**
   * Checks whether the boolean expression is false. If the expression is true
   * {@link AssertException} will be thrown
   *
   * @param expression   an expression to be checked whether it is false or not
   * @param errorMessage a message to be logged if the expression is true
   * @throws AssertException when the expression is true after logging of the
   *                         error message
   */
  public static void assertFalse(boolean expression,
                                 String errorMessage)
      throws AssertException {
    assertFalse(expression, null, errorMessage);
  }

  /**
   * Checks whether the boolean expression is false. If the expression is true
   * {@link AssertException} will be thrown
   *
   * @param expression   an expression to be checked whether it is false
   * @param message      a message to be logged if the expression is false
   * @param errorMessage the message to be logged if the expression is true
   * @throws AssertException when the expression is true after logging of the
   *                         error message
   */
  public static void assertFalse(boolean expression,
                                 String message,
                                 String errorMessage)
      throws AssertException {
    assertBoolean(!expression, message, errorMessage);
  }

  /**
   * Checks whether the object is not null. If the object is null
   * {@link AssertException} is thrown
   *
   * @param notNullObject an object to be checked whether it is not null
   * @param errorMessage  a message to be logged if the object is null
   * @throws AssertException when the object is null after logging of the error
   *                         message
   */
  public static void assertNotNull(Object notNullObject,
                                   String errorMessage)
      throws AssertException {
    assertNotNull(notNullObject, null, errorMessage);
  }

  /**
   * Checks whether the object is not null. If the object is null
   * {@link AssertException} is
   * thrown
   *
   * @param notNullObject an object to be checked whether it is not null
   * @param message       a message to be logged if the object is not null.
   * @param errorMessage  a message to be logged if the object is null
   * @throws AssertException when the object is null after logging of the error
   *                         message
   */
  public static void assertNotNull(Object notNullObject,
                                   String message,
                                   String errorMessage)
      throws AssertException {
    assertFalse(notNullObject == null, message, errorMessage);
  }

  /**
   * Checks whether the object is null. If the object is not null
   * {@link AssertException} is thrown
   *
   * @param nullObject   an object to be checked whether it is null
   * @param errorMessage a message to be logged if the object is not null
   * @throws AssertException when the object is not null after logging of the
   *                         error message
   */
  public static void assertNull(Object nullObject,
                                String errorMessage)
      throws AssertException {
    assertNull(nullObject, null, errorMessage);
  }

  /**
   * Checks whether the object is null. If the object is not null
   * {@link AssertException} is thrown
   *
   * @param nullObject   an object to be checked whether it is null
   * @param message      a message to be logged if the object is null.
   * @param errorMessage a message to be logged if the object is not null
   * @throws AssertException when the object is not null after logging of the
   *                         error message
   */
  public static void assertNull(Object nullObject,
                                String message,
                                String errorMessage)
      throws AssertException {
    assertTrue(nullObject == null, message, errorMessage);
  }

  /**
   * Checks that the Collection is not null and that it contains at least one
   * item.
   *
   * @param col          Any Collection object
   * @param errorMessage a message to be logged if the Collection is empty.
   * @throws AssertException when the Collection is empty after logging the
   *                         error message
   */
  public static void assertNotEmpty(Collection<?> col,
                                    String errorMessage)
      throws AssertException {
    assertNotNull(col, errorMessage);
    assertFalse(col.isEmpty(), errorMessage);
  }

  /**
   * Checks that the Collection is not null and that it contains at least one
   * item.
   *
   * @param col          Any Collection object
   * @param message      a message to be logged if the Collection is not empty.
   * @param errorMessage a message to be logged if the Collection is empty.
   * @throws AssertException when the Collection is empty after logging the
   *                         error message
   */
  public static void assertNotEmpty(Collection<?> col,
                                    String message,
                                    String errorMessage)
      throws AssertException {
    assertNotEmpty(col, errorMessage);
    if (message != null) {
      log.info(message);
    }
  }

  /**
   * Checks that the array is not null and that it contains at least one item.
   *
   * @param arr          Any array
   * @param errorMessage a message to be logged if the array is empty.
   * @throws AssertException when the array is empty after logging the error
   *                         message
   */
  public static void assertNotEmpty(Object[] arr,
                                    String errorMessage)
      throws AssertException {
    assertNotNull(arr, errorMessage);
    assertTrue(arr.length > 0, errorMessage);
  }

  /**
   * Checks that the array is not null and that it contains at least one item.
   *
   * @param arr          Any array
   * @param message      a message to be logged if the array is not empty.
   * @param errorMessage a message to be logged if the array is empty.
   * @throws AssertException when the array is empty after logging the error
   *                         message
   */
  public static void assertNotEmpty(Object[] arr,
                                    String message,
                                    String errorMessage)
      throws AssertException {
    assertNotEmpty(arr, errorMessage);
    if (message != null) {
      log.info(message);
    }
  }

  private static void assertBoolean(boolean expression,
                                    String message,
                                    String errorMessage)
      throws AssertException {
    if (expression) {
      if (message != null) {
        log.info(message);
      }
    } else {
      log.error(errorMessage == null ? "Assertion failed" : errorMessage);
      throw new AssertException(errorMessage);
    }
  }

}
