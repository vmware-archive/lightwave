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
module.controller('ServerMgmtCntrl', ['$scope',  '$rootScope','ServerService', 'TenantService', 'Util', 'popupUtil',
    'AuthenticationService',
        function($scope, $rootScope, ServerService, TenantService, Util, popupUtil, AuthenticationService) {

            $scope.vm = this;
            $scope.vm.getcomputers = getcomputers;
            $scope.vm.gettenants = gettenants;
            $scope.vm.addTenant = addTenant;
            $scope.vm.deleteTenant = deleteTenant;

            init();

            function init() {

                $scope.vm.tenants = [];
                $scope.vm.filteredtenants = [];
                $scope.vm.newtenant = {
                    credentials: {
                        certificates: []
                    }
                };
            }

            function addTenant(){
                var template = 'sso/servermgmt/tenant/tenant.add.html';
                var controller = 'TenantCntrl';
                popupUtil.open($scope, template, controller);
            }

            function getcomputers(searchText) {
                if($rootScope.globals.currentUser.isSystemTenant) {
                    $rootScope.globals.errors = null;
                    $scope.vm.computersdataLoading = true;
                    ServerService
                        .Get($rootScope.globals.currentUser)
                        .then(function (res) {
                            if (res.status == 200) {
                                var comps = res.data;
                                if (!searchText || searchText == '') {
                                    $scope.vm.computers = comps;
                                } else if (comps != null) {
                                    $scope.vm.computers = [];
                                    for (var i = 0; i < comps.length; i++) {
                                        if (comps[i].hostname.indexOf(searchText) > -1) {
                                            $scope.vm.computers.push(comps[i]);
                                        }
                                    }
                                }
                            }
                            else {
                                $rootScope.globals.errors = res.data;
                            }
                            $scope.vm.computersdataLoading = false;
                        });
                }
            }

            function gettenants(searchText) {
                $rootScope.globals.errors = null;
                var tenants = $scope.vm.tenants;
                if (!searchText || searchText == '') {
                    $scope.vm.filteredtenants = tenants;
                } else if (tenants != null && tenants.length > 0) {
                    $scope.vm.filteredtenants = [];
                    for (var i = 0; i < $scope.vm.tenants.length; i++) {
                        if (tenants[i].name.indexOf(searchText) > -1) {
                            $scope.vm.filteredtenants.push(tenants[i]);
                        }
                    }
                }
            }

            function deleteTenant(){
                $rootScope.globals.errors = null;
                TenantService
                    .Delete($rootScope.globals.currentUser)
                    .then(function (res) {
                        if (res.status == 200 || res.status == 204) {
                            TenantService.Cleanup($rootScope.globals.currentUser);
                            AuthenticationService.redirectToHome();
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }
        }]);