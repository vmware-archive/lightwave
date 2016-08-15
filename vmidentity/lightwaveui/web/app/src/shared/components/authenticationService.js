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

var module = angular.module('lightwave.ui.shared.components');
module.factory('AuthenticationService', AuthenticationService);

AuthenticationService.$inject = ['$window', '$rootScope'];

function AuthenticationService($window, $rootScope) {

    var service = {};
    service.logout = logout;
    service.redirectToHome = redirectToHome;
    return service;

    function logout() {
        var logoutUri = "/lightwaveui/Logout?id_token="
            + $rootScope.globals.currentUser.token.id_token
            + "&state="
            + $rootScope.globals.currentUser.token.state
            + "&tenant="
            + $rootScope.globals.currentUser.tenant;
        $window.sessionStorage.currentUser = 'logout';
        $window.location.href = logoutUri;
    }

    function redirectToHome() {
        var homeUri = "/lightwaveui";
        $window.sessionStorage.currentUser = 'logout';
        $window.location.href = homeUri;
    }
}