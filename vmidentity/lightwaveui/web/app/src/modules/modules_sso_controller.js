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

var module = angular.module('lightwave.ui.modules');
module.controller('SsoComponentCntrl', ['$scope', '$rootScope',
        'IdentitySourceService', 'RelyingPartyService', 'IdentityProviderService', 'MemberService',
        function($scope, $rootScope, IdentitySourceService, RelyingPartyService, IdentityProviderService, MemberService ) {

        $scope.vm = this;
        $scope.vm.summary = {};

        init();

        function init() {
            $scope.vm.summarydataLoading = true;
            getIdentitySources();
            getRelyingParties();
            getIdentityProviders();
        }

        function getIdentitySources() {

            if($rootScope.globals.currentUser.role != 'GuestUser') {
                IdentitySourceService
                    .GetAll($rootScope.globals.currentUser)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.summary.identitySources = 0;
                            if (res.data)
                                $scope.vm.summary.identitySources = res.data.length;
                            var ids = res.data;
                            var identitySource = null;
                            if (ids && ids.length > 0) {

                                for (var i = 0; i < ids.length; i++) {

                                    if (ids[i].domainType == 'SYSTEM_DOMAIN') {
                                        identitySource = ids[i];
                                        break;
                                    }
                                }

                                if (identitySource) {
                                    getSolutionUsers(identitySource.name);
                                    getUsers(identitySource.name);
                                    getGroups(identitySource.name);
                                }
                                else {
                                    $scope.vm.summary.solutionUsers = 0;
                                    $scope.vm.summary.users = 0;
                                    $scope.vm.summary.groups = 0;
                                }
                                $scope.vm.summarydataLoading = false;
                            }
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }
        }

        function getRelyingParties() {
            if($rootScope.globals.currentUser.role == 'Administrator') {
                RelyingPartyService
                    .GetAll($rootScope.globals.currentUser, '')
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.summary.relyingParties = 0;
                            if (res.data)
                                $scope.vm.summary.relyingParties = res.data.length;
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }
        }

        function getIdentityProviders(){
            if($rootScope.globals.currentUser.role == 'Administrator') {
                IdentityProviderService
                    .GetAll($rootScope.globals.currentUser, '')
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.summary.identityProviders = 0;
                            if (res.data)
                                $scope.vm.summary.identityProviders = res.data.length;
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }
        }

        function getSolutionUsers(provider_name){
                MemberService
                    .Search($rootScope.globals.currentUser, provider_name, "SOLUTIONUSER", "NAME", '')
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.summary.solutionUsers = 0;
                            if (res.data && res.data.solutionUsers)
                                $scope.vm.summary.solutionUsers = res.data.solutionUsers.length;
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }

        function getGroups(provider_name){
            MemberService
                .Search($rootScope.globals.currentUser, provider_name, "GROUP", "NAME", '')
                .then(function (res) {
                    if (res.status == 200) {
                        $scope.vm.summary.groups = 0;
                        if (res.data && res.data.groups)
                            $scope.vm.summary.groups = res.data. groups.length;
                    }
                    else {
                        $rootScope.globals.errors = res.data;
                    }
                });
        }

        function getUsers(provider_name){
            MemberService
                .Search($rootScope.globals.currentUser, provider_name, "USER", "NAME", '')
                .then(function (res) {
                    if (res.status == 200) {
                        $scope.vm.summary.users = 0;
                        if (res.data && res.data.users)
                            $scope.vm.summary.users = res.data.users.length;
                    }
                    else {
                        $rootScope.globals.errors = res.data;
                    }
                });
        }
    }]);