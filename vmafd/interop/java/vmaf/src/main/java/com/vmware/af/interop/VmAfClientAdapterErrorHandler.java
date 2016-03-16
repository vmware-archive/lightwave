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

import java.util.HashMap;
import java.util.Map;

import com.vmware.af.VmAfClientNativeException;

class VmAfClientAdapterErrorHandler {

   private interface Handler {
      public void handleError(int errorCode) throws VmAfClientNativeException;
   }

   private static Map<Integer, Handler> errorCodeToHandler =
         new HashMap<Integer, Handler>();

   static {
      errorCodeToHandler.put(VmafErrors.ERROR_ACCESS_DENIED.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfAccessDeniedException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.ERROR_NOT_SUPPORTED.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfNotSupportedException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.ERROR_INVALID_PARAMETER.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfInvalidParameterException(errorCode);

               }
            });

      errorCodeToHandler.put(
            VmafErrors.ERROR_INVALID_COMPUTERNAME.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfInvalidComputerNameException(errorCode);

               }
            });

      errorCodeToHandler.put(
            VmafErrors.ERROR_NO_SUCH_LOGON_SESSION.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode) throws VmAfClientNativeException {
                  throw new VmAfNoSuchLogonSessionException(errorCode);

               }
            });

      errorCodeToHandler.put(
            VmafErrors.ERROR_WRONG_PASSWORD.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode) throws VmAfClientNativeException {
                  throw new VmAfWrongPasswordException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.ERROR_NO_SUCH_DOMAIN.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfNoSuchDomainException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.NERR_UNKNOWNSERVER.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int vmafErrorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfUnknownServerException(vmafErrorCode);
               }
            });


      errorCodeToHandler.put(VmafErrors.NERR_SETUPALREADYJOINED.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int vmafErrorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfAlreadyJoinedException(vmafErrorCode);
               }
            });

      errorCodeToHandler.put(VmafErrors.NERR_SETUPNOTJOINED.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int vmafErrorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfNotJoinedException(vmafErrorCode);
               }
            });

      errorCodeToHandler.put(VmafErrors.ERROR_BAD_PACKET.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfBadPacketException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.LW_ERROR_PASSWORD_MISMATCH.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfWrongPasswordException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.LW_ERROR_LDAP_NO_SUCH_OBJECT.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfLdapNoSuchObjectException(errorCode);

               }
            });

      errorCodeToHandler.put(VmafErrors.LW_ERROR_KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN.getErrorCode(),
            new Handler() {

               @Override
               public void handleError(int errorCode)
                     throws VmAfClientNativeException {
                  throw new VmAfNoSuchLogonSessionException(errorCode);

               }
            });
   }

   private VmAfClientAdapterErrorHandler() {
      throw new UnsupportedOperationException(
            "not instantiable, use the static handleError method");
   }

   public static void handleErrorCode(int errorCode) {
      if (errorCode != 0) {
         Handler handler =
               errorCodeToHandler.get(errorCode);
         if (handler != null) {
            handler.handleError(errorCode);
         } else {
            throw new VmAfClientNativeException(errorCode);
         }

      }
   }
}
