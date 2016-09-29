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
using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Cache;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn
{
    public class AuthTokenManager
    {
        private SimpleCache<AuthTokenDto> _localData;
        public AuthTokenManager()
        {
            _localData = new SimpleCache<AuthTokenDto>();
        }
        public IList<AuthTokenDto> GetAuthTokens(ServerDto serverDto)
        {
            var authTokens = new List<AuthTokenDto>();
            var tokens = GetAllAuthTokens();
            foreach (var token in tokens)
            {
                if (token.ServerDto.ServerName == serverDto.ServerName)
                {
                    if (token.Login != null)
                    {
                        var authToken = GetAuthToken(serverDto, token.Login.TenantName);
                        authTokens.Add(authToken);
                    }
                }
            }
            return authTokens;
        }
        public AuthTokenDto GetAuthToken(ServerDto serverDto, string tenant)
        {
            var key = string.Format("{0}|{1}", serverDto.ServerName, tenant);
            var authToken = _localData.Get(key);            
            return authToken;
        }
        public void SetAuthToken(AuthTokenDto token)
        {
            var key = string.Format("{0}|{1}", token.ServerDto.ServerName, token.Login.TenantName);
            var authToken = _localData.Get(key);
            if (authToken != null)
                _localData.Remove(key);
            _localData.Add(key, token);
        }
        public void RemoveAuthToken(AuthTokenDto token)
        {
            var key = string.Format("{0}|{1}", token.ServerDto.ServerName, token.Login == null ? string.Empty : token.Login.TenantName);
            var authToken = _localData.Get(key);
            if (authToken != null)
                _localData.Remove(key);
        }

        public void RemoveAuthTokens(string serverName)
        {
            var keysForDeletion = new List<string>();
            foreach(var key in _localData.GetKeys())
            {
                if (key == serverName || key.StartsWith(serverName + "|"))
                    keysForDeletion.Add(key);                    
            }

            foreach(var key in keysForDeletion)
            {
                _localData.Remove(key);
            }
        }
        public ICollection<AuthTokenDto> GetAllAuthTokens()
        {
            var tokens = _localData.GetAll();
            if (tokens == null)
                tokens = new List<AuthTokenDto>();
            return tokens;
        }
        public void Clear()
        {
            _localData.Clear();
        }
        public void CacheAuthTokens(SimpleCache<AuthTokenDto> localData)
        {
            if (localData != null)
                _localData = localData;
        }
        public SimpleCache<AuthTokenDto> GetAuthTokenCache()
        {
            return _localData;
        }       
    }
}
