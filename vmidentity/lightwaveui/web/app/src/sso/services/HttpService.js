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
sso_service_module.factory('HttpService', HttpService);

HttpService.$inject = ['$http','HttpConfiguration','AuthenticationService', '$window', '$q', '$rootScope'];

function HttpService($http, HttpConfiguration, AuthenticationService, $window, $q, $rootScope) {

    var service = {};
    service.getResponse = getResponse;
    return service;

    function getResponse(endpoint, verb, token, postData, isText, contentType) {

        var cType = contentType ? contentType : isText ? 'application/x-www-form-urlencoded; charset=utf-8' : 'application/json';

        return $http({
            url: endpoint,
            dataType: 'json',
            method: verb,
            data: postData != null && postData!= undefined ? postData : 'data',
            headers: HttpConfiguration.getHeaders(token, cType)
        }).then(handleSuccess, handleFailure);
    }

    function handleSuccess(response){
        return response;
    }

    function handleFailure(response){
        if(response.status == 401 && response.data.error == 'invalid_token') {
            var redirectUri = '/lightwaveui/Login?tenant=' + $rootScope.globals.currentUser.tenant;
            $window.sessionStorage.currentUser = 'logout';
            response.data.error = null;
            response.status = 200;
            $window.location.href = redirectUri;
            /*AuthenticationService.logout();*/
            return $q.reject(response);
        }
        return response;
    }
}