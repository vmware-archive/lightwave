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

package com.vmware.identity.interop.idm;

import com.sun.jna.Library;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.win32.W32APIOptions;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.Validate;

public class WindowsIdmNativeAdapter extends NativeAdapter implements IIdmClientLibrary
{
    public interface IdmNativeLibrary extends Library
    {
        IdmNativeLibrary INSTANCE =
                (IdmNativeLibrary) Native.loadLibrary(
                                                "idm.dll",
                                                IdmNativeLibrary.class,
                                                W32APIOptions.UNICODE_OPTIONS);

        int
        IDMGetComputerName(
            PointerByReference ppszName
            );

        int
        IDMAuthenticateUser(
            String            pszUserName,
            String            pszDomainName,
            String            pszPassword,
            PointerByReference ppIdmUserInformation /* PIDM_USER_INFO* */
            );

        int
        IDMAuthenticate2(
            Pointer            pAuthContext, /* PIDM_AUTH_CONTEXT */
            Pointer            pInputBuffer,
            int                dwInputBufferSize,
            PointerByReference ppOutputBuffer,
            IntByReference     pdwOutputBufferSize,
            IntByReference     pfDone
            );

        int
        IDMGetUserInformationFromAuthContext(
            Pointer            pAuthContext, /* PIDM_AUTH_CONTEXT */
            PointerByReference ppIdmUserInformation /* PIDM_USER_INFO* */
            );

        void
        IDMFreeUserInfo(
            Pointer pIdmUserInformation /* PIDM_USER_INFO */
            );

        void
        IDMFreeAuthContext(
            Pointer pAuthContext /* PIDM_AUTH_CONTEXT */
            );

        int
        IDMCreateAuthContext(
            String            pszPackageName,
            PointerByReference ppAuthContext /* PIDM_AUTH_CONTEXT* */
            );

        int
        IDMAllocateMemory(
            int                dwSize,
            PointerByReference ppMemory
            );


        void
        IDMFreeMemory(
            Pointer pMemory
            );
    }

    private static final WindowsIdmNativeAdapter _instance = new WindowsIdmNativeAdapter();

    private WindowsIdmNativeAdapter()
    {
    }

    public static WindowsIdmNativeAdapter getInstance()
    {
        return _instance;
    }

    public IdmNativeLibrary getIdmClientNativeLibrary()
    {
        return IdmNativeLibrary.INSTANCE;
    }

    @Override
    public
    String getComputerName()
    {
        PointerByReference ppName = new PointerByReference();

        try
        {
            int errCode = IdmNativeLibrary.INSTANCE.IDMGetComputerName(ppName);

            if (errCode != 0)
            {
                throw new Win32Exception(errCode);
            }

            return ppName.getValue().getString(0, true);
        }
        finally
        {
            if (ppName.getValue() != Pointer.NULL)
            {
                IdmNativeLibrary.INSTANCE.IDMFreeMemory(ppName.getValue());
                ppName.setValue(Pointer.NULL);
            }
        }
    }

    @Override
    public
    UserInfo
    AuthenticateByPassword(
        String pszUserName,
        String pszDomainName,
        String pszPassword
        )
    {
        Validate.validateNotEmpty(pszUserName, "User name");
        Validate.validateNotEmpty(pszDomainName, "Domain Name");
        Validate.validateNotNull(pszPassword, "Password");

        PointerByReference ppUserInfo = new PointerByReference();

        try
        {
            int errCode = IdmNativeLibrary.INSTANCE.IDMAuthenticateUser(
                                                        pszUserName,
                                                        pszDomainName,
                                                        pszPassword,
                                                        ppUserInfo);
            if (errCode != 0)
            {
                throw new Win32Exception(errCode);
            }

            UserInfoNative info = new UserInfoNative(ppUserInfo.getValue());

            return info.getUserInfo();
        }
        finally
        {
            if (ppUserInfo.getValue() != Pointer.NULL)
            {
                IdmNativeLibrary.INSTANCE.IDMFreeUserInfo(
                        ppUserInfo.getValue());
                ppUserInfo.setValue(Pointer.NULL);
            }
        }
    }

    @Override
    public AuthenticationContext CreateAuthenticationContext()
    {
        String pszPackageName = new String("Negotiate");
        PointerByReference ppAuthContext = new PointerByReference();

        int errCode = IdmNativeLibrary.INSTANCE.IDMCreateAuthContext(
                                                        pszPackageName,
                                                        ppAuthContext);
        if (errCode != 0)
        {
            throw new Win32Exception(errCode);
        }

        return new AuthenticationContext(ppAuthContext.getValue());
    }

    @Override
    public
    AuthResult
    AuthenticateBySSPI(AuthenticationContext context, byte[] gssBLOB) {

        Validate.validateNotNull(context, "Authentication context");
        Validate.validateNotEmpty(gssBLOB, "GSSAPI Blob");

        PointerByReference ppOutBuffer = new PointerByReference();
        IntByReference pdwOutBufLen = new IntByReference();
        IntByReference pfDone = new IntByReference();

        try
        {
            Memory pInputBuffer = new Memory(gssBLOB.length);

            pInputBuffer.write(0, gssBLOB, 0, gssBLOB.length);

            int errCode = IdmNativeLibrary.INSTANCE.IDMAuthenticate2(
                                                    context.getNativeContext(),
                                                    pInputBuffer,
                                                    gssBLOB.length,
                                                    ppOutBuffer,
                                                    pdwOutBufLen,
                                                    pfDone);
            if (errCode != 0)
            {
                context.setAuthStatus(SSPI_AUTH_STATUS.ERROR);

                throw new Win32Exception(errCode);
            }

            if (pfDone.getValue() == 1)
            {
                context.setAuthStatus(SSPI_AUTH_STATUS.COMPLETE);

                return new AuthResult(SSPI_AUTH_STATUS.COMPLETE, null);
            }
            else
            {
                context.setAuthStatus(SSPI_AUTH_STATUS.CONTINUE_NEEDED);

                if (pdwOutBufLen.getValue() <= 0)
                {
                    throw new IllegalStateException("Invalid SSPI Negotiation result length");
                }

                return new AuthResult(
                                SSPI_AUTH_STATUS.CONTINUE_NEEDED,
                                ppOutBuffer.getValue().getByteArray(
                                                            0,
                                                            pdwOutBufLen.getValue()));
            }
        }
        finally
        {
            if (ppOutBuffer.getValue() != Pointer.NULL)
            {
                IdmNativeLibrary.INSTANCE.IDMFreeMemory(ppOutBuffer.getValue());
                ppOutBuffer.setValue(Pointer.NULL);
            }
        }
    }

    @Override
    public UserInfo getUserInfo(AuthenticationContext context)
    {
        Validate.validateNotNull(context, "Authentication context");

        if (context.getAuthStatus() != SSPI_AUTH_STATUS.COMPLETE)
        {
            throw new IllegalStateException(
                        "An incomplete authentication context was provided");
        }

        PointerByReference ppUserInfo = new PointerByReference();

        try
        {
            int errCode =
                    IdmNativeLibrary.INSTANCE.IDMGetUserInformationFromAuthContext(
                            context.getNativeContext(),
                            ppUserInfo);

            if (errCode != 0)
            {
                throw new Win32Exception(errCode);
            }

            UserInfoNative info = new UserInfoNative(ppUserInfo.getValue());

            return info.getUserInfo();
        }
        finally
        {
            if (ppUserInfo.getValue() != Pointer.NULL)
            {
                IdmNativeLibrary.INSTANCE.IDMFreeUserInfo(
                        ppUserInfo.getValue());
                ppUserInfo.setValue(Pointer.NULL);
            }
        }
    }

    @Override
    public boolean LdapSaslBind(Pointer pLdapConnection, String userName, String domainName, String password)
    {
        throw new UnsupportedOperationException(
                    "LDAP SASL Bind is unsupported");
    }

    @Override
    public void FreeAuthenticationContext(Pointer pContext)
    {
        if (pContext != Pointer.NULL)
        {
            IdmNativeLibrary.INSTANCE.IDMFreeAuthContext(pContext);
        }
    }
}
