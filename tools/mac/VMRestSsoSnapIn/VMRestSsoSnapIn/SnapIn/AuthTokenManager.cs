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

        public AuthTokenDto GetAuthToken(string key)
        {
            var authToken = _localData.Get(key);
            //authToken.Refresh();
            return authToken;
        }
		public AuthTokenDto GetReadOnlyAuthToken(string key)
		{
			var authToken = _localData.Get(key);
			return authToken;
		}
        public void SetAuthToken(AuthTokenDto token, string key)
        {
            var authToken = _localData.Get(key);
            if (authToken != null)
                _localData.Remove(key);
            _localData.Add(key, token);
        }
        public void RemoveAuthToken(string key)
        {
            var authToken = _localData.Get(key);
            if (authToken != null)
                _localData.Remove(key);
        }
        public ICollection<AuthTokenDto> GetAllAuthTokens()
        {
            var tokens = _localData.GetAll();
            if (tokens == null)
                tokens = new List<AuthTokenDto>();
            return tokens;
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
