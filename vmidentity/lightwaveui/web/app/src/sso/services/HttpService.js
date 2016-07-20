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

HttpService.$inject = ['$http','HttpConfiguration','$window', '$q', '$rootScope'];

function HttpService($http, HttpConfiguration, $window, $q, $rootScope) {

    var service = {};
    service.getResponse = getResponse;
    return service;

    function getResponse(endpoint, verb, token, postData) {

        var contentType = 'application/json';

        return $http({
            url: endpoint,
            dataType: 'json',
            method: verb,
            data: postData != null && postData!= undefined ? postData : 'data',
            headers: HttpConfiguration.getHeaders(token, contentType)
        }).then(handleSuccess, handleFailure);
    }

    function handleSuccess(response){
        //console.log('Success: ' + JSON.stringify(response));
        return response;
    }

    function handleFailure(response){
        //console.log('Error: ' + JSON.stringify(response));
        //console.log('Response status: ' + response.status);
        //console.log('Response response.data.error: ' + response.data.error);
        if(response.status == 401 && response.data.error == 'invalid_token') {
            console.log('UnAuthorized ... re-login');
            var redirectUri = '/lightwaveui/Login?tenant=' + $rootScope.globals.currentUser.tenant;
            $window.location.href = redirectUri;
            return $q.reject(response);
        }
        return response;
    }
}