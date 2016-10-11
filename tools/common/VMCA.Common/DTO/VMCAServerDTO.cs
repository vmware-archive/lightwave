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
using System.Xml.Serialization;
using System.Collections.Generic;
using VMCA;
using System.Threading.Tasks;
using System.Security.Cryptography.X509Certificates;
using VMIdentity.CommonUtils;

namespace VMCASnapIn.DTO
{
    public class VMCAServerDTO
    {
        protected VMCAClient _vmcaClient;

        public string Server { get; set; }

        public string GUID { get; set; }

        public string UserName { get; set; }

        [XmlIgnore]
        public string Password { get; set; }

        public string DomainName { get; set; }

        public List<PrivateCertificateDTO> PrivateCertificates { get; set; }

        public List<SigningRequestDTO> SigningRequests { get; set; }

        public List<KeyPairDTO> KeyPairs { get; set; }

        [XmlIgnore]
        public bool IsLoggedIn { get; set; }

        [XmlIgnore]
        public VMCAClient VMCAClient {
            get {
                if (_vmcaClient == null)
                    _vmcaClient = new VMCAClient (Server, UserName, Password, DomainName);
                return _vmcaClient;
            }
        }

        public void Cleanup()
        {
            VMCAClient.CloseServer();
            _vmcaClient = null;
            IsLoggedIn = false;
            Password = null;
        }

        public static VMCAServerDTO CreateInstance ()
        {
            var dto = new VMCAServerDTO { GUID = Guid.NewGuid ().ToString () };
            dto.PrivateCertificates = new List<PrivateCertificateDTO> ();
            dto.SigningRequests = new List<SigningRequestDTO> ();
            dto.KeyPairs = new List<KeyPairDTO> ();
            dto.IsLoggedIn = false;
            return dto;
        }

        public async Task LogintoServer (string user, string pass, string domain)
        {
            try {
                UserName = user;
                Password = pass;
                DomainName = domain;

                Task t = new Task (ServerConnect);
                t.Start ();
                if (await Task.WhenAny (t, Task.Delay (CommonConstants.TEN_SEC)) == t) {
                    await t;
                } else {
                    throw new Exception(CommonConstants.SERVER_TIMEOUT);
                }
            } catch (Exception e) {
                throw e;
            }
        }


        public void ServerConnect ()
        {
            try {
                VMCAClient.RefreshServerContext (UserName, Password);
                //Added this get root ca call to test is the server is reachable.
                X509Certificate2 testCert = VMCAClient.GetRootCertificate ();
                if (testCert != null)
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

