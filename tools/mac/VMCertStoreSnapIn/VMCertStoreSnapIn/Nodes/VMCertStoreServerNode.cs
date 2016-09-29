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
using VmIdentity.UI.Common.Utilities;
using VMCertStore.Common.DTO;
using Vecs;
using System.Linq;
using System.Collections.Generic;

namespace VMCertStoreSnapIn.Nodes
{
    public class VMCertStoreServerNode
    {
        public class CertBagItem
        {
            public List<CertDTO> PrivateKeys { get; set; }

            public List<CertDTO> SecretKeys { get; set; }

            public List<CertDTO> Certs { get; set; }

        }

        public bool IsLoggedIn { get; set; }

        public int NoPrivateKeys{ get; set; }

        public int NoSecretKeys { get; set; }

        public int NoCertificates { get; set; }

        public int NoStores { get; set; }

        public bool IsDetailsLoaded { get; set; }

        public Dictionary<string, CertBagItem> StoresInfo { get; set; }

        public VMCertStoreServerDTO ServerDTO { get; set; }


        public VMCertStoreServerNode (VMCertStoreServerDTO dto)
        {
            ServerDTO = dto;
            IsLoggedIn = false;
            IsDetailsLoaded = false;
            StoresInfo = new Dictionary<string, CertBagItem> ();
        }

        public async void  FillServerInfo ()
        {
            try {
                var stores = ServerDTO.VecsClient.GetStores ();
                IsDetailsLoaded = true;
                StoresInfo.Clear ();
                foreach (var store in stores) {
                    var storePass = "";
                    using (var session = new VecsStoreSession (ServerDTO.VecsClient, store, storePass)) {
                        //store add
                        StoresInfo.Add (store, new CertBagItem () {
                            PrivateKeys = session.GetPrivateKeys ().Concat (session.GetEncryptedPrivateKeys ()).ToList (),
                            SecretKeys = session.GetSecretKeys ().ToList (),
                            Certs = session.GetCertificates ().ToList ()
                        });
                    }
                }
                CalculateKeyInfo ();
            } catch (Exception e) {
                // do nothing in async task.
            }
        }

        void CalculateKeyInfo ()
        {
            NoStores = StoresInfo.Count;
            NoPrivateKeys = StoresInfo.Values.Sum (x => x.PrivateKeys.Count ());
            NoSecretKeys = StoresInfo.Values.Sum (x => x.SecretKeys.Count ());
            NoCertificates = StoresInfo.Values.Sum (x => x.Certs.Count ());
        }

        public void RemoveStoreInfo (string storename)
        {
            UIErrorHelper.CheckedExec (delegate() {
                StoresInfo.Remove (storename);
                CalculateKeyInfo ();
            });
        }

        public void UpdateStoreInfo (string storename)
        {
            UIErrorHelper.CheckedExec (delegate() {
                if (storename != null) {
                    string storePass = "";
                    using (var session = new VecsStoreSession (ServerDTO.VecsClient, storename, storePass)) {
                        // update store info 
                        if (StoresInfo.ContainsKey (storename)) {
                            StoresInfo [storename] = new CertBagItem () {
                                PrivateKeys = session.GetPrivateKeys ().Concat (session.GetEncryptedPrivateKeys ()).ToList (),
                                SecretKeys = session.GetSecretKeys ().ToList (),
                                Certs = session.GetCertificates ().ToList ()
                            };
                            CalculateKeyInfo ();
                        } else {
                            StoresInfo.Add (storename, new CertBagItem () {
                                PrivateKeys = session.GetPrivateKeys ().Concat (session.GetEncryptedPrivateKeys ()).ToList (),
                                SecretKeys = session.GetSecretKeys ().ToList (),
                                Certs = session.GetCertificates ().ToList ()
                            });
                            NoStores++;
                        }
                    }
                }

            });
        }
    }
}

