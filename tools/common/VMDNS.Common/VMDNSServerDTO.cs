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
using VMIdentity.CommonUtils;
using VMDNS.Client;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace VMDNS.Common
{
    public class VMDNSServerDTO:IServerDTO
    {
        #region IServerDTO implementation

        public string Server
        {
            get ;
            set ;
        }

        public string GUID
        {
            get ;
            set ;
        }

        public string UserName
        {
            get ;
            set ;
        }

        public string Password
        {
            get ;
            set ;
        }

        public string DomainName
        {
            get ;
            set ;
        }

        #endregion

        public bool IsLoggedIn { get; set; }

        private VmDnsClient _dnsClient;

        private VMDNSServerDTO()
        {
            IsLoggedIn = false;
        }

        public VmDnsClient DNSClient
        {
            get
            {
                if (_dnsClient == null)
                    try
                    {
                        _dnsClient = new VmDnsClient(Server, UserName, DomainName, Password);
                    }
                    catch (Exception e)
                    {
                        _dnsClient = null;
                    }
                return _dnsClient;
            }

            set
            {
                _dnsClient = value;
            }
        }

        public void DisposeDNSClient()
        {
            DNSClient.CloseConnection();
            _dnsClient = null;
        }

        public  static VMDNSServerDTO CreateInstance()
        {
            var dto = new VMDNSServerDTO { GUID = Guid.NewGuid().ToString() };
            return dto;
        }

        public async Task LoginToServer()
        {
            try
            {
                Task t = new Task(ResetUserPass);
                t.Start();
                if (await Task.WhenAny(t, Task.Delay(VMDNSConstants.DNSSERVER_TIMEOUT)) == t)
                {
                }
                else
                { 
                    DisposeDNSClient();
                    throw new Exception(CommonConstants.SERVER_TIMEOUT);
                }

            }
            catch (Exception e)
            {
                throw e;
            }
            return;
        }

        public void ResetUserPass()
        {
            if (DNSClient != null)
            {
                IList<VmDnsZone> zones;
                try
                {
                    zones = DNSClient.ListZones(VmDnsZoneType.FORWARD);
                }
                catch (Exception e)
                {
                    zones = null;
                    DisposeDNSClient();
                    throw;
                }
                if (zones != null)
                    IsLoggedIn = true;
            }
        }
    }
}

