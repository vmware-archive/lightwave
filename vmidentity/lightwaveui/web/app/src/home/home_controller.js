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
                  'IdentitySourceService',
        function($rootScope, $cookies, $location, $scope, $window, Configuration, Util,
                 IdentitySourceService) {

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

        function redirectToSsoHome(){
            var key = 'oidc_session_id-'+$rootScope.globals.currentUser.tenant;
            $rootScope.globals.sessionId = $cookies.get(key);
            $location.path('ssohome');
        }

        function setcontext(id_token, access_token, token_type, expires_in, state) {

            var decodedJwt = Util.decodeJWT(id_token);
            var decodedAccessJwt = Util.decodeJWT(access_token);
            $rootScope.globals = {
                currentUser: {
                    server: getserver(decodedJwt.header.iss),//$location.host(),
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

        function getserver(uri){

            var server_uri = uri.split('//')[1];
            var server_with_port = server_uri.split('/')[0];
           return server_with_port.split(':')[0];
        }

    }]);