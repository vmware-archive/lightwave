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
using VMCertStore.Utilities;
using System.Security.Cryptography.X509Certificates;
using System.IO;
using VMCertStore.Client;

namespace Vecs
{
    public class VecsStoreSession : IDisposable
    {
        string _storeName = null;
        string _storePass = null;
        IntPtr _hStore = IntPtr.Zero;

        public VecsClient Client { get; protected set; }

        public IntPtr StoreHandle {
            get {
                try {
                    if (_hStore == IntPtr.Zero) {
                        var result = VecsAdaptor.VecsOpenCertStoreHA (
                                         Client.ServerContext,
                                         _storeName,
                                         _storePass,
                                         out _hStore);
                        VecsError.Check (result);
                    }
                    return _hStore;
                } catch (Exception e) {
                    throw e;
                }
            }
        }

        public VecsStoreSession (VecsClient client, string storeName, string storePass)
        {
            Client = client;
            _storeName = storeName;
            _storePass = storePass;
        }

        public void AddCertificateEntry (string alias, string privateKey, string password, string certificate)
        {
            ErrorHelper.CatchAndThrow (delegate() {
                String cert = File.ReadAllText (certificate);
                var result = VecsAdaptor.VecsAddEntryA (
                                 StoreHandle,
                                 VecsAdaptor.CertEntryType.TrustedCert,
                                 alias,
                                 cert,
                                 privateKey,
                                 password,
                                 false);
                VecsError.Check (result);
            });
        }

        public void AddPrivateKeyEntry (string alias, string privateKey, string password, string certificate)
        {
            ErrorHelper.CatchAndThrow (delegate() {
                String privatekey = File.ReadAllText (privateKey);
                String cert = File.ReadAllText (certificate);
                if (string.IsNullOrEmpty (password))
                    password = null;
                var result = VecsAdaptor.VecsAddEntryA (
                                 StoreHandle,
                                 VecsAdaptor.CertEntryType.PrivateKey,
                                 alias,
                                 cert,
                                 privatekey,
                                 password,
                                 false);
                VecsError.Check (result);
            });
        }

        public void AddSecretKeyEntry (string alias, string secretKey, string password, X509Certificate2 cert)
        {
            ErrorHelper.CatchAndThrow (delegate() {
                var result = VecsAdaptor.VecsAddEntryA (
                                 StoreHandle,
                                 VecsAdaptor.CertEntryType.SecretKey,
                                 alias,
                                 null,
                                 secretKey,
                                 password,
                                 false);
                VecsError.Check (result);
            });
        }

        public void DeleteCertificate (string alias)
        {
            ErrorHelper.CatchAndThrow (delegate() {
                var result = VecsAdaptor.VecsDeleteEntryA (
                                 StoreHandle,
                                 alias);
                VecsError.Check (result);
            });
        }

        public IEnumerable<CertDTO> GetCertificates ()
        {
            try {
                return GetStoreEntries (VecsAdaptor.CertEntryType.TrustedCert);
            } catch (Exception e) {
                throw e;
            }
        }

        public IEnumerable<CertDTO> GetPrivateKeys ()
        {
            try {
                return GetStoreEntries (VecsAdaptor.CertEntryType.PrivateKey);
            } catch (Exception e) {
                throw e;
            }
        }

        public IEnumerable<CertDTO> GetEncryptedPrivateKeys ()
        {
            try {
                return GetStoreEntries (VecsAdaptor.CertEntryType.EncryptedPrivateKey);
            } catch (Exception e) {
                throw e;
            }
        }

        public IEnumerable<CertDTO> GetSecretKeys ()
        {
            try {
                return GetStoreEntries (VecsAdaptor.CertEntryType.SecretKey);
            } catch (Exception e) {
                throw e;
            }
        }

        public IEnumerable<CertDTO> GetStoreEntries (VecsAdaptor.CertEntryType entryType)
        {
            var lst = new List<CertDTO> ();
            UInt32 certCount = 0;
            IntPtr ptrCerts = IntPtr.Zero;
            var hEnumContext = new IntPtr ();

            try {
                var result = VecsAdaptor.VecsGetEntryCount (StoreHandle, out certCount);
                VecsError.Check (result);

           
                result = VecsAdaptor.VecsBeginEnumEntries (
                    _hStore,
                    certCount,
                    VecsAdaptor.EntryInfoLevel.Level2,
                    out hEnumContext);
                VecsError.Check (result);

                if (hEnumContext != null) {
                    result = VecsAdaptor.VecsEnumEntriesA (
                        hEnumContext,
                        out ptrCerts,
                        out certCount);
                    VecsError.Check (result);

                    int sz = Marshal.SizeOf (typeof(VecsAdaptor.VECS_CERT_ENTRY));
                    var certArray = new VecsAdaptor.VECS_CERT_ENTRY[certCount];

                    for (UInt32 i = 0; i < certCount; i++) {
                        certArray [i] =
                            (VecsAdaptor.VECS_CERT_ENTRY)Marshal.PtrToStructure (
                            new IntPtr (ptrCerts.ToInt64 () + (sz * i)),
                            typeof(VecsAdaptor.VECS_CERT_ENTRY));
                        var certString = Marshal.PtrToStringAnsi (
                                             certArray [i].pszCertificate);
                        var aliasString = Marshal.PtrToStringAnsi (
                                              certArray [i].pszAlias);
                        var passwordString = Marshal.PtrToStringAnsi (
                                                 certArray [i].pszPassword);

                        //if(!string.IsNullOrEmpty(aliasString))
                        //    File.WriteAllText("c:\\temp\\" + aliasString, certString);

                        if (certArray [i].entryType != (int)entryType) {
                            continue;
                        }
                        var dto = new CertDTO {
                            Alias = aliasString != null ? aliasString : ""
                        };
                        lst.Add (dto);
                        try {
                            if (!string.IsNullOrEmpty (certString)) {
                                dto.Cert = certString.GetX509Certificate2FromString ();
                            }
                            if (!string.IsNullOrEmpty (passwordString)) {
                                dto.Password = passwordString;
                            }
                        } catch (Exception) {
                        }
                    }
                }
            } catch (Exception e) {
                throw e;
            } finally {
                if (hEnumContext != IntPtr.Zero) {
                    VecsAdaptor.VecsEndEnumEntries (hEnumContext);
                }
                if (ptrCerts != IntPtr.Zero) {
                    VecsAdaptor.VecsFreeCertEntryArrayA (ptrCerts, certCount);
                }
            }

            return lst;
        }

        public void Dispose ()
        {
            try {
                if (_hStore != IntPtr.Zero)
                    Close ();
            } catch (Exception e) {
                throw e;
            }
        }

        public void Close ()
        {
            try {
                VecsAdaptor.VecsCloseCertStore (_hStore);
            } catch (Exception e) {
                throw e;
            }
        }
    }
}
