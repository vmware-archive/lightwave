/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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

var module = angular.module('lightwave.ui.modules');
module.config(['$routeProvider', function($routeProvider) {
    $routeProvider
        .when('/idm/ssohome', {
            templateUrl: 'idm/modules/sso.summary.view.html',
            controller: 'SsoComponentCntrl'
        })
        .when('/idm/directory', {
            templateUrl: 'idm/modules/directory.summary.view.html',
            controller: 'ComponentCntrl'
        })
        .when('/idm/psc', {
            templateUrl: 'idm/modules/psc.summary.view.html',
            controller: 'PscComponentCntrl'
        })
        .when('/idm/certificate', {
            templateUrl: 'idm/modules/certificate.summary.view.html',
            controller: 'ComponentCntrl'
        });
}]);