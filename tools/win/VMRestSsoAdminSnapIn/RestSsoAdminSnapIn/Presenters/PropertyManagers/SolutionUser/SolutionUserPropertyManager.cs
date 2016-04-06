﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.SolutionUser
{
    public class SolutionUserPropertyManager : IPropertyDataManager
    {
        private readonly ServiceGateway _service;
        private readonly string _tenantName;
        private readonly ServerDto _serverDto;
        readonly SolutionUserDto _dtoOriginal;
        public SolutionUserPropertyManager(ServiceGateway service, SolutionUserDto dto, ServerDto serverDto, string tenantName)
        {
            _tenantName = tenantName;
            _dtoOriginal = dto;
            _service = service;
            _serverDto = serverDto;
        }

        public object GetData()
        {
            return _dtoOriginal.DeepCopy();
        }

        public bool Apply(object obj)
        {
            var dto = obj as SolutionUserDto;

            if (_dtoOriginal.Description != dto.Description || _dtoOriginal.Disabled != dto.Disabled ||
                _dtoOriginal.Certificate.Encoded != dto.Certificate.Encoded)
            {
                var result = false;
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
                ActionHelper.Execute(delegate()
                {
                    _service.SolutionUser.Update(_serverDto, _tenantName, dto, auth.Token);
                    result = true;
                }, auth);
                return result;
            }
            else
            {
                return true;
            }
        }
    }
}
