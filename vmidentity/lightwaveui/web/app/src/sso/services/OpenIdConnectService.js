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

'use strict';

var sso_service_module = angular.module('lightwave.ui.sso.services');
sso_service_module.factory('OpenIdConnectService', OpenIdConnectService);

OpenIdConnectService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function OpenIdConnectService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.GetToken = GetToken;
    return service;

    function GetToken(server, tenant, username, password) {
        var endpoint = Configuration.getLoginEndpoint(server, tenant);
        var args = Configuration.getLoginArgument(username, password);
        return HttpService
            .getResponse(endpoint, 'POST', null, args, true)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
}