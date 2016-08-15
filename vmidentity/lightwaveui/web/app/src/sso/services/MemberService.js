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
sso_service_module.factory('MemberService', MemberService);

MemberService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function MemberService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.Search = Search;
    return service;

    function Search(context, domain, type, searchBy, query) {
        var endpoint = Configuration.getAllUsersEndpoint(context.server, context.tenant);
        endpoint += "?domain=" + domain + "&type=" + type + "&searchBy=" + searchBy + "&query=" + query;
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }
}