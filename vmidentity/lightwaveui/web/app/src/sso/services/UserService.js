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

var module = angular.module('lightwave.ui.sso.services');
module.factory('UserService', UserService);

UserService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function UserService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.Get = Get;
    service.Add = Add;
    service.Delete = Delete;
    service.Update = Update;
    service.GetGroups = GetGroups;
    service.SetPassword = SetPassword;
    return service;

    function GetGroups(context, upn) {
        var endpoint = Configuration.getUserGroupsEndpoint(context.server, context.tenant, upn);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Get(context, username) {
        var endpoint = Configuration.getUserEndpoint(context.server, context.tenant, username);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Add(context, user) {
        var endpoint = Configuration.getUsersEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'POST', context.token, user)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Update(context, user) {
        var endpoint = Configuration.getUserEndpoint(context.server, context.tenant, user.details.upn);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, user)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Delete(context, username) {
        var endpoint = Configuration.getUserEndpoint(context.server, context.tenant, username);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function SetPassword(context, user, password){
        var endpoint = Configuration.getUserPasswordEndpoint(context.server, context.tenant, user.details.upn);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, password)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
}