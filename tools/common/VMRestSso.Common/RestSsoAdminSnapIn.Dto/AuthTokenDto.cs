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

using System.Security.Claims;
using System.Xml.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    public class AuthTokenDto : IDataContext
    {
        private System.Func<ServerDto, LoginDto, Token, Token> _refreshAction;
        public AuthTokenDto()
        {
        }
        public AuthTokenDto(System.Func<ServerDto, LoginDto, Token, Token> refreshAction)
        {
            _refreshAction = refreshAction;
        }
        public ServerDto ServerDto { get; set; }
        [XmlIgnore]
        public Token Token { get; set; }
        [XmlIgnore]
        public ClaimsPrincipal ClaimsPrincipal { get; set; }
        [XmlIgnore]
        public LoginDto Login { get; set; }
        public bool Refresh()
        {
            var refreshed = false;
            if (_refreshAction != null)
            {
                Token = _refreshAction.Invoke(ServerDto, Login, Token);
                refreshed = true;
            }
            return refreshed;
        }
    }
}