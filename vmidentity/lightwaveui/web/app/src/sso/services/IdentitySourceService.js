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
sso_service_module.factory('IdentitySourceService', IdentitySourceService);

    IdentitySourceService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function IdentitySourceService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.GetAll = GetAll;
    service.Add = Add;
    service.Update = Update;
    service.Delete = Delete;
    service.TestConnectivity = TestConnectivity;
    return service;

    function GetAll(context) {
        var endpoint = Configuration.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Add(context, identitySource) {
        var endpoint = Configuration.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'POST', context.token, identitySource)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure)
    }

    function Update(context, identitySource) {
        var endpoint = Configuration.getIdentitySourceEndpoint(context.server, context.tenant, identitySource.name);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, identitySource)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure)
    }

    function Delete(context, name) {
        var endpoint = Configuration.getIdentitySourceEndpoint(context.server, context.tenant, name);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure)
    }

    function TestConnectivity(context, identitySource){
        var endpoint = Configuration.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        endpoint = endpoint + "?probe=true";
        return HttpService
            .getResponse(endpoint, 'POST', context.token, identitySource)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure)
    }
}