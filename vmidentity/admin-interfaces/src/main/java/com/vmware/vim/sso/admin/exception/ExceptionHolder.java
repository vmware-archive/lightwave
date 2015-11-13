/*
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
 */

package com.vmware.vim.sso.admin.exception;

import java.io.PrintWriter;
import java.io.StringWriter;

/**
 * Temporary class to ease transition to our exceptions code convention
 *
 * Wrap checked into unchecked exceptions while preserving the original
 * exception and the original stack trace. Partial implementation of this class
 * is taken from http://www.mindview.net/Etc/Discussions/CheckedExceptions
 */
@SuppressWarnings("deprecation")
public final class ExceptionHolder extends SystemException {

   /**
    * Construct the exception with a cause.
    *
    * @param t
    *           The original {@link Throwable}.
    */
   public ExceptionHolder(Throwable t) {
      super(t.getMessage(), t);
      originalThrowable = t;
   }

   /**
    * Construct the exception with a cause and message.
    *
    * @param message
    * @param t
    *           The original {@link Throwable}.
    */
   public ExceptionHolder(String message, Throwable t) {
      super(message, t);
      originalThrowable = t;
   }

   @Override
   public void printStackTrace() {
      printStackTrace(System.err);
   }

   @Override
   public void printStackTrace(java.io.PrintStream s) {

      String stackTrace = getStackTraceAsString();

      synchronized (s) {
         s.print(getClass().getName() + ": ");
         s.print(stackTrace);
      }
   }

   /**
    * @return
    */
   private String getStackTraceAsString() {
      StringWriter sw = new StringWriter();
      originalThrowable.printStackTrace(new PrintWriter(sw));
      String stackTrace = sw.toString();
      return stackTrace;
   }

   @Override
   public void printStackTrace(java.io.PrintWriter s) {
      synchronized (s) {
         s.print(getClass().getName() + ": ");
         s.print(getStackTraceAsString());
      }
   }

   /**
    * Throw the original exception.
    */
   public void rethrow() throws Throwable {
      throw getCause();
   }

   /*   public static RuntimeServiceException asRuntimeServiceException(Throwable t) {
         if (t instanceof RuntimeServiceException) {
            return (RuntimeServiceException) t;
         } else {
            return new ExceptionHolder(t);
         }
      }

      public static RuntimeException asRuntimeException(Throwable t) {
         if (t instanceof RuntimeException) {
            return (RuntimeException) t;
         } else {
            return new ExceptionHolder(t);
         }
      }

   */
   private final Throwable originalThrowable;
   private static final long serialVersionUID = 1L;

}
