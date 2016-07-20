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
module.controller('HomeCntrl', ['$rootScope', '$cookies', '$location', '$scope', 'Configuration', 'Util',
        function($rootScope, $cookies, $location, $scope, Configuration, Util) {

        init();

        function init() {

            $rootScope.globals.errors = '';

            // console.log('home controller Logout:' + $location.search.target);
            var qs = $location.search();
            var state = qs.state;
            var id_token = qs.id_token;
            console.log('Extracted id_token: ' + id_token);
            var access_token = qs.access_token;
            // console.log('Extracted id_token: ' + access_token);
            var token_type = qs.token_type;
            // console.log('Extracted id_token: ' + token_type);
            var expires_in = qs.expires_in;
            // console.log('Extracted id_token: ' + expires_in);
            if('logout' in qs)
            {
                // console.log('logout ..');
                $rootScope.globals.currentUser = null;
            }
            else
            {
                setcontext(id_token, access_token, token_type, expires_in, state);

                var key = 'oidc_session_id-'+$rootScope.globals.currentUser.tenant;
                var sessionId = $cookies.get(key);

                // console.log('$cookies oidc_session_id: key - ' + key.toString());

                if(sessionId != undefined) {
                    // console.log('$cookies oidc_session_id: value - ' + sessionId.toString());
                }

                $rootScope.globals.sessionId = sessionId;

                $location.path('ssohome');
            }
        }

        function setcontext(id_token, access_token, token_type, expires_in, state) {

            var decodedJwt = Util.decodeJWT(id_token);
            var decodedAccessJwt = Util.decodeJWT(access_token);

            // console.log('Decoded JWT:' + JSON.stringify(decodedJwt));
            //// console.log('Decoded AccessJWT:' + JSON.stringify(decodedAccessJwt));

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

            //console.log(JSON.stringify($rootScope.globals));
            //var expiration = $rootScope.globals.currentUser.token.expires_in;
            //var redirectUri = 'https://'
            //    + $rootScope.globals.currentUser.server
            //    + '/webui/Login?tenant='
            //    + $rootScope.globals.currentUser.tenant;
            //var loc = "console.log('inside timeout'); window.location = '" + redirectUri + "' ;";
            // console.log('timeout set to: ' + loc);
            //setTimeout(loc, expiration * 1000);
        }

        function getserver(uri){

            var server_uri = uri.split('//')[1];
            var server_with_port = server_uri.split('/')[0];
            var server_name = server_with_port.split(':')[0];
            return server_name;
        }

    }]);