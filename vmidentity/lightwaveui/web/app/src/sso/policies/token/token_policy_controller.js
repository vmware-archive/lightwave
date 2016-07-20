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

var module = angular.module('lightwave.ui.sso');
module.controller('TokenPolicyCntrl', [ '$scope', '$rootScope', 'TenantService',
        function($scope, $rootScope, TenantService) {

            $scope.updateTokenPolicy = updateTokenPolicy;

            function updateTokenPolicy(tokenPolicy) {
                var policy = {
                    tokenPolicy: tokenPolicy
                };

                TenantService
                    .UpdateConfiguration($rootScope.globals.currentUser, policy)
                    .then(function (res) {
                        console.log("Tenant Policy update: " + JSON.stringify(res));
                        $scope.getConfig();
                        $scope.closeThisDialog('save');
                    });
            }
        }]);