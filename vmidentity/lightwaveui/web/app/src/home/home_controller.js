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

var module = angular.module('lightwave.ui.home');
module.controller('HomeCntrl', ['$rootScope', '$cookies', '$location', '$scope', '$window', 'Configuration', 'Util',
    'IdentitySourceService', 'RelyingPartyService', 'IdentityProviderService', 'MemberService',
    function($rootScope, $cookies, $location, $scope, $window, Configuration, Util,
             IdentitySourceService, RelyingPartyService, IdentityProviderService, MemberService ) {

        $scope.vm = this;
        $scope.vm.summary = {};

        init();


        function init() {

            $rootScope.globals.errors = '';

            var qs = $location.search();
            var state = qs.state;

            var id_token = qs.id_token;
            var access_token = qs.access_token;
            var token_type = qs.token_type;
            var expires_in = qs.expires_in;
            if('logout' in qs)
            {
                $rootScope.globals.currentUser = null;
            }
            else
            {
                var loggedIn = $window.sessionStorage.currentUser;
                if (!loggedIn) {
                    setcontext(id_token, access_token, token_type, expires_in, state);
                }
                else {
                    if (loggedIn == 'logout') {
                        setcontext(id_token, access_token, token_type, expires_in, state);
                    }
                    else {
                        $rootScope.globals.currentUser = JSON.parse($window.sessionStorage.currentUser);
                        redirectToSsoHome();
                    }
                }
            }
        }

        function initSummary() {
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

        function redirectToSsoHome(){
            initSummary();
        }

        function setcontext(id_token, access_token, token_type, expires_in, state) {

            var decodedJwt = Util.decodeJWT(id_token);
            var decodedAccessJwt = Util.decodeJWT(access_token);
            $rootScope.globals = {
                currentUser: {
                    server:  { host: $location.host(), port: $location.port(), protocol: $location.protocol() },
                    tenant: decodedJwt.header.tenant,
                    username: decodedJwt.header.sub,
                    first_name: decodedJwt.header.given_name,
                    last_name: decodedJwt.header.family_name,
                    role: decodedAccessJwt.header.admin_server_role,
                    token: {
                        id_token: id_token,
                        token_type: token_type,
                        access_token: access_token,
                        expires_in: expires_in,
                        state: state
                    }
                }
            };
            checkForSystemTenant();
        }

        function checkForSystemTenant() {

            $rootScope.globals.currentUser.isSystemTenant = false;
            IdentitySourceService
                .GetAll($rootScope.globals.currentUser)
                .then(function (res) {
                    if (res.status == 200) {
                        var identitySources = res.data;
                        for (var i = 0; i < identitySources.length; i++) {
                            if (identitySources[i].domainType == 'LOCAL_OS_DOMAIN') {
                                $rootScope.globals.currentUser.isSystemTenant = true;
                                break;
                            }
                        }
                    }
                    else {
                        $rootScope.globals.errors = res.data;
                    }
                    $window.sessionStorage.currentUser = JSON.stringify($rootScope.globals.currentUser);
                    redirectToSsoHome();
                });
        }

    }]);