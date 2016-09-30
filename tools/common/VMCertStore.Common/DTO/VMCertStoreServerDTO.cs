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
using VMCertStore.Client;
using System.Xml.Serialization;
using System.Threading.Tasks;
using VMIdentity.CommonUtils;

namespace VMCertStore.Common.DTO
{
    public class LotusDomainJoinInfo
    {
        public bool IsDomainMember;
        public string ComputerName;
        public string DomainName;
    }

    public class WindowsADDomainJoinInfo
    {
        public bool IsDomainMember;
        public string ComputerName;
        public string DomainName;
    }

    public class VMCertStoreServerDTO
    {
        [XmlIgnore]
        protected VMCertStoreClient _certStoreClient;
        [XmlIgnore]
        protected VMAfdClient _afdClient;
        [XmlIgnore]
        protected Vecs.VecsClient _vecsClient;

        [XmlIgnore]
        public bool IsLoggedIn { get; set; }

        public LotusDomainJoinInfo _lotusDomainInfo;
        public WindowsADDomainJoinInfo _windowsDomainInfo;

        public string Server { get; set; }

        public string GUID { get; set; }

        [XmlIgnore]
        public string UserName { get; set; }

        [XmlIgnore]
        public string Password { get; set; }

        [XmlIgnore]
        public string DomainName { get; set; }

        [XmlIgnore]
        public VMCertStoreClient VMCertStoreClient {
            get {
                if (_certStoreClient == null)
                    _certStoreClient = new VMCertStoreClient (Server);
                return _certStoreClient;
            }
        }

        [XmlIgnore]
        public VMAfdClient VMAfdClient {
            get {
                if (_afdClient == null)
                    _afdClient = new VMAfdClient (Server);
                return _afdClient;
            }
        }

        [XmlIgnore]
        public Vecs.VecsClient VecsClient {
            get {
                if (_vecsClient == null)
                    _vecsClient = new Vecs.VecsClient (Server, UserName, Password);
                return _vecsClient;
            }
        }

        public VMCertStoreServerDTO ()
        {
            this._lotusDomainInfo = new LotusDomainJoinInfo {
                IsDomainMember = false,
                ComputerName = "",
                DomainName = ""
            };
            this._windowsDomainInfo = new WindowsADDomainJoinInfo {
                IsDomainMember = false,
                ComputerName = "",
                DomainName = ""
            };
        }

        public static VMCertStoreServerDTO CreateInstance ()
        {
            var dto = new VMCertStoreServerDTO{ GUID = Guid.NewGuid ().ToString () };
            return dto;
        }

        public void Cleanup()
        {
            _vecsClient.CloseServer();
            _certStoreClient = null;
            _afdClient = null;
            _vecsClient = null;
            IsLoggedIn = false;
            Password = null;
        }

        public async Task LogintoServer (string user, string pass, string domain)
        {
            UserName = user + "@" + domain;
            Password = pass;
            DomainName = domain;

            try {
                Task t = new Task (ResetUserPass);
                t.Start ();
                if (await Task.WhenAny (t, Task.Delay (2*CommonConstants.TEN_SEC)) == t) {
                    await t;
                } else { 
                    throw new Exception (CommonConstants.SERVER_TIMEOUT);
                }

            } catch (Exception e) {
                throw e;
            }
        }

        public void ResetUserPass ()
        {
            try {
                VecsClient.RefreshServerContext (UserName, Password);
                string[] stores = VecsClient.GetStores ();
                if (stores != null)
                    IsLoggedIn = true;
            } catch (Exception) {
                IsLoggedIn = false;
                throw;
            }
        }

        public bool CanLogin ()
        {
            return !string.IsNullOrEmpty (UserName) && !string.IsNullOrEmpty (Password);
        }
    }
}
