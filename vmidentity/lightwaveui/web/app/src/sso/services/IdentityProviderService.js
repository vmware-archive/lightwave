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
sso_service_module.factory('IdentityProviderService', IdentityProviderService);

IdentityProviderService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function IdentityProviderService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.GetAll = GetAll;
    service.Get = Get;
    service.Delete = Delete;
    service.Create = Create;
    return service;

    function Get(context, name) {
        var endpoint = Configuration.getIdentityProviderEndpoint(context.server, context.tenant, name);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function GetAll(context) {
        var endpoint = Configuration.getIdentityProvidersEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Delete(context, name) {
        var endpoint = Configuration.getIdentityProviderEndpoint(context.server, context.tenant, name);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Create(context, idp) {
        var endpoint = Configuration.getIdentityProvidersEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'POST', context.token, idp)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
}