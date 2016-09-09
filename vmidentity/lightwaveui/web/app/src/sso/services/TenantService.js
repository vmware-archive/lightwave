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
sso_service_module.factory('TenantService', TenantService);

TenantService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function TenantService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.Create = Create;
    service.Delete = Delete;
    service.Cleanup = Cleanup;
    service.GetConfiguration = GetConfiguration;
    service.UpdateConfiguration = UpdateConfiguration;
    service.UpdateDirConfiguration = UpdateDirConfiguration;
    return service;

    function GetConfiguration(context, policies) {
        var endpoint = Configuration.getConfigEndpoint(context.server, context.tenant, policies);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function UpdateConfiguration(context, policy) {
        var endpoint = Configuration.getConfigEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, policy)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function UpdateDirConfiguration(context, policy) {
        var endpoint = Configuration.getDirConfigEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, policy)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Create(context, tenant) {
        var endpoint = Configuration.getTenantsEndpoint(context.server);
        return HttpService
            .getResponse(endpoint, 'POST', context.token, tenant)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Delete(context) {
        var endpoint = Configuration.getTenantEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
    function Cleanup(context) {
        var endpoint = Configuration.getTenantCleanupEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'GET', null, false, "text/html")
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
}