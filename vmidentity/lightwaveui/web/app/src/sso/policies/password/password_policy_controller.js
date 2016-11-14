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
module.controller('PasswordPolicyCntrl', [ '$scope', '$rootScope', 'TenantService',
        function($scope, $rootScope, TenantService) {

            $scope.updatePasswordPolicy = updatePasswordPolicy;

            init();

            function init(){
                $rootScope.globals.errors = null;
                $rootScope.globals.popup_errors = null;
            }

            function updatePasswordPolicy(passwordPolicy) {
                $rootScope.globals.errors = null;
                var policy = {
                    passwordPolicy: passwordPolicy
                };

                TenantService
                    .UpdateDirConfiguration($rootScope.globals.currentUser, policy)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Password policy updated successfully', success:true};
                            $scope.refresh();
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }
        }]);