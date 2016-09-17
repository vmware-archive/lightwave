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
sso_service_module.factory('OidcClientService', OidcClientService);

OidcClientService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function OidcClientService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.GetAll = GetAll;
    service.Get = Get;
    service.Add = Add;
    service.Create = Create;
    service.Delete = Delete;
    service.Update = Update;
    service.AddClientId = AddClientId;
    return service;

    function Get(context, name) {
        var endpoint = Configuration.getOpenIdConnectClientEndpoint(context.server, context.tenant, name);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function GetAll(context) {
        var endpoint = Configuration.getOpenIdConnectClientsEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Create(server, token, tenant, client) {
        var endpoint = Configuration.getOpenIdConnectClientsEndpoint(server, tenant);
        return HttpService
            .getResponse(endpoint, 'POST', token, client)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Add(context, client) {
        var endpoint = Configuration.getOpenIdConnectClientsEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'POST', context.token, client)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function AddClientId(server, token, tenant, clientId) {
        var endpoint = Configuration.addClientId(server, tenant, clientId);
        return HttpService
            .getResponse(endpoint, 'GET', token, clientId)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Delete(context, name) {
        var endpoint = Configuration.getOpenIdConnectClientEndpoint(context.server, context.tenant, name);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Update(context, client) {
        var endpoint = Configuration.getOpenIdConnectClientEndpoint(context.server, context.tenant, client.clientId);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, client.oidcclientMetadataDTO)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
}