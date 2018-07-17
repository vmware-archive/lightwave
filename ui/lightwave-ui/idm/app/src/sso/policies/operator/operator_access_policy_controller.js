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

var module = angular.module('lightwave.ui.sso');
module.controller('OperatorPolicyCntrl', [ '$scope', '$rootScope', 'Util', 'TenantService',
        function($scope, $rootScope, Util, TenantService) {

            $scope.updateOperatorsAccessPolicy = updateOperatorsAccessPolicy;

            init();

            function init(){
                $rootScope.globals.errors = null;
                $rootScope.globals.popup_errors = null;
                $scope.vm.operatorsAccessPolicyUnderEdit = {}
                if ($scope.vm.policies.operatorsAccessPolicy) {
                    $scope.vm.operatorsAccessPolicyUnderEdit.enabled = $scope.vm.policies.operatorsAccessPolicy.enabled;
                    $scope.vm.operatorsAccessPolicyUnderEdit.userBaseDn = $scope.vm.policies.operatorsAccessPolicy.userBaseDn;
                    $scope.vm.operatorsAccessPolicyUnderEdit.groupBaseDn = $scope.vm.policies.operatorsAccessPolicy.groupBaseDn;
                } else {
                    $scope.vm.operatorsAccessPolicyUnderEdit.enabled = false;
                }
            }

            function updateOperatorsAccessPolicy(policy) {
                $rootScope.globals.errors = null;
                var config = {
                    operatorsAccessPolicy: policy
                };

                TenantService
                    .UpdateConfiguration($rootScope.globals.currentUser, config)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Operators Access Policy updated successfully', success:true};
                            $scope.vm.policies.operatorsAccessPolicy = res.data.operatorsAccessPolicy
                            $scope.refresh();
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }
        }]);