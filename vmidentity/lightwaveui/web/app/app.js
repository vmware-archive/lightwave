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

var app = angular.module('lightwave.ui', [
    'ngRoute',
    'ngCookies',
    'ngDialog',
    'ui.bootstrap',
    'lightwave.ui.sso.services',
    'lightwave.ui.sso',
    'lightwave.ui.home',
    'lightwave.ui.shared.components',
    'lightwave.ui.shared.directives',
    'lightwave.ui.shared.utils',
    'lightwave.ui.modules'
]);

app.run(run);

run.$inject = ['$rootScope', '$location', '$cookieStore', '$window'];

function run($rootScope, $location, $cookieStore, $window) {
    $rootScope.globals = $cookieStore.get('globals') || {};
    $rootScope.globals.errors = '';
    $rootScope.globals.crumbs = [];
    $rootScope.globals.vm = {};

    $rootScope.$on('$locationChangeStart', function (event, next, current) {

        $rootScope.globals.errors = '';
        $rootScope.appLoad = false;
        var loggedIn = $window.sessionStorage.currentUser;
        if (!loggedIn) {
            $rootScope.appLoad = true;
            $location.path('/home');
        }
        else {
            if (loggedIn == 'logout') {
                $rootScope.appLoad = true;
                $location.path('/home');
            }
            else {
                $rootScope.globals.currentUser = JSON.parse($window.sessionStorage.currentUser);
            }
        }
    });
}