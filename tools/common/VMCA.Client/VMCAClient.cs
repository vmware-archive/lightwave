/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

using System;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using VMCA.Client;
using System.IO;
using System.Reflection;
using VMCA.Utilities;

namespace VMCA
{
    public class VMCAClient
    {
        IntPtr _serverContext = IntPtr.Zero;

        public string ServerName { get; protected set; }

        public string UserName { get; set; }

        public string Password { get; set; }

        public string DomainName { get; set; }

        public IntPtr ServerContext {
            get {
                if (_serverContext == IntPtr.Zero) {
                    EnsureServerContext ();
                }
                return _serverContext;
            }
        }

        public bool NeedAuth {
            get {
                return string.IsNullOrEmpty (UserName) ||
                string.IsNullOrEmpty (Password);
            }
        }
        public void CloseServer()
        {
            try
            {
                if (_serverContext != IntPtr.Zero)
                    VMCAAdaptor.VMCACloseServer(_serverContext);
            }
            catch (Exception e) 
            {
                throw e;
            }
        }

        void EnsureServerContext ()
        {
            try {
                if (_serverContext != IntPtr.Zero)
                    return;
                UInt32 result = VMCAAdaptor.VMCAOpenServerA (
                                    ServerName,
                                    UserName,
                                    DomainName,
                                    Password,
                                    0,
                                    IntPtr.Zero,
                                    out _serverContext);
                VMCAError.Check (result);
            } catch (Exception) {
                UserName = "";
                Password = "";
                throw;
            }
        }

        public void RefreshServerContext (string user, string pass)
        {
            try {
                _serverContext = IntPtr.Zero;
                UserName = user;
                Password = pass;
                EnsureServerContext ();
            } catch {
                throw;
            }
        }

        public VMCAClient (string serverName, string userName, string password, string domainName)
        {
            ServerName = serverName;
            UserName = userName;
            Password = password;
            DomainName = domainName;
        }

        public string GetServerVersion ()
        {
             
            var serverVersionPtr = new IntPtr (); 
            UInt32 dwError = VMCAAdaptor.VMCAGetServerVersionHA (
                                 _serverContext,
                                 ServerName, out serverVersionPtr);
            VMCAError.Check (dwError);
            string version = Marshal.PtrToStringAnsi (serverVersionPtr);
            VMCAAdaptor.VMCAFreeVersion (serverVersionPtr);
            return version;
        }

        public X509Certificate2 GetRootCertificate ()
        {
            IntPtr pCert = new IntPtr ();

            UInt32 dwError = VMCAAdaptor.VMCAGetRootCACertificateHA (
                                 _serverContext,
                                 ServerName,
                                 out pCert);
            VMCAError.Check (dwError);

            string certString = Marshal.PtrToStringAnsi (pCert);
            VMCAAdaptor.VMCAFreeCertificate (pCert);

            return new X509Certificate2 (ASCIIEncoding.ASCII.GetBytes (certString));
        }
        public X509Certificate2 GetVMCASignedCertificate(VMCAAdaptor.VMCA_PKCS_10_REQ_DATA certRequest, string privateKey, DateTime notBefore, DateTime notAfter)
        {
            IntPtr pCert = new IntPtr();
            long nBefore = notBefore.ToTime_t();
            long nAfter = notAfter.ToTime_t();
            UInt32 dwError = VMCAAdaptor.VMCAGetSignedCertificateHA(_serverContext,
                                 ServerName, certRequest, privateKey, "", nBefore, nAfter, out pCert);
            VMCAError.Check(dwError);
            string certString = Marshal.PtrToStringAnsi(pCert);
            VMCAAdaptor.VMCAFreeCertificate(pCert);
            return new X509Certificate2(ASCIIEncoding.ASCII.GetBytes(certString));
        }
        public void AddRootCertificate (string rootCert, string passPhrase, string privateKey)
        {
            UInt32 dwError = VMCAAdaptor.VMCAAddRootCertificate (ServerName, rootCert, passPhrase, privateKey);
            VMCAError.Check (dwError);
        }
    }
}
