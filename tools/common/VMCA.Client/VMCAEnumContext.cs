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
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;

namespace VMCA.Client
{
    public class VMCAEnumContext : IDisposable
    {
        IntPtr _context;

        public VMCAClient Client { get; protected set; }

        public CertificateState Filter { get; protected set; }

        Int32 _currentIndex = 0;

        public VMCAEnumContext (VMCAClient client, CertificateState filter)
        {
            Client = client;
            Filter = filter;
        }

        public void Dispose ()
        {
            if (_context != null) {
                VMCAAdaptor.VMCACloseEnumContext (_context);
            }
        }

        public void OpenEnumContext ()
        {
            if (_context.ToInt64 () == 0) {
                UInt32 error = VMCAAdaptor.VMCAOpenEnumContextHA (Client.ServerContext, Client.ServerName, (int)Filter, out _context);
                VMCAError.Check (error);
            }
        }

        public IEnumerable<X509Certificate2> GetCertificates ()
        {
            Int32 enumStatus = 0;
            OpenEnumContext ();

            while (enumStatus != (int)VMCAEnum.End) {
                IntPtr pCert = new IntPtr ();

                UInt32 error = VMCAAdaptor.VMCAGetNextCertificate (_context, out pCert, out _currentIndex, out enumStatus);
                VMCAError.Check (error);

                if ((VMCAEnum)enumStatus != VMCAEnum.Success)
                    yield break;

                string certString = Marshal.PtrToStringAnsi (pCert);
                VMCAAdaptor.VMCAFreeCertificate (pCert);

                var cert = new X509Certificate2 (ASCIIEncoding.ASCII.GetBytes (certString));
                yield return cert;
            }
        }
    }
}
