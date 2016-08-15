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
sso_service_module.factory('SolutionUserService', SolutionUserService);

SolutionUserService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function SolutionUserService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.Get = Get;
    service.Add = Add;
    service.Delete = Delete;
    service.Update = Update;
    return service;

    function Get(context, username) {
        var endpoint = Configuration.getSolutionUserEndpoint(context.server, context.tenant, username);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Add(context, user) {
        var endpoint = Configuration.getSolutionUsersEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'POST', context.token, user)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Update(context, user) {
        var endpoint = Configuration.getSolutionUserEndpoint(context.server, context.tenant, user.name);
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, user)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function Delete(context, username) {
        var endpoint = Configuration.getSolutionUserEndpoint(context.server, context.tenant, username);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

}