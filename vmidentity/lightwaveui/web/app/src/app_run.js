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

/*
'use strict';

var module = angular.module('lightwave.ui');
module.run(run);

run.$inject = ['$rootScope', '$location', '$cookieStore'];

function run($rootScope, $location, $cookieStore) {
    $rootScope.globals = $cookieStore.get('globals') || {};
    $rootScope.globals.errors = '';
    $rootScope.globals.crumbs = [];
    $rootScope.globals.vm = {};
    $rootScope.globals.vm.menus = [];

    $rootScope.$on('$locationChangeStart', function (event, next, current) {
        $rootScope.globals.errors = '';
        var isHomePageWithToken = $location.path().indexOf('/home') >= 0;
        var loggedIn = $rootScope.globals.currentUser;
        $rootScope.appLoad = false;
        if (!isHomePageWithToken && !loggedIn) {
            $rootScope.appLoad = true;
            $location.path('/home');
        }
    });
}*/
