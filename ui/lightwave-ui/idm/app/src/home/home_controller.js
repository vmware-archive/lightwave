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

var module = angular.module('lightwave.ui.home');
module.controller('HomeCntrl', ['$rootScope', '$cookies', '$location', '$scope', '$window', 'Configuration', 'Util',
    'IdentitySourceService', 'RelyingPartyService', 'IdentityProviderService', 'MemberService',
    function($rootScope, $cookies, $location, $scope, $window, Configuration, Util,
             IdentitySourceService, RelyingPartyService, IdentityProviderService, MemberService ) {

        $scope.vm = this;
        $scope.vm.summary = {};
        init();
        function init() {
            var errAlert = document.getElementById("errorAlert")
            errAlert.style.display = 'none'
            var qs = $location.search();
            var hash = $location.hash();
            var access_token, id_token, state, token_type, expires_in;
            var queryParams = {};
            if(hash.length > 0){
                var paramsArr = hash.split('&');
                for (var i = 0;i < paramsArr.length; i ++){
                    var key = paramsArr[i].split('=')[0];
                    var val = paramsArr[i].split('=')[1];
                    queryParams[key] = val;
                }
                access_token = queryParams['access_token'];
                id_token = queryParams['id_token'];
                state = queryParams['state'];
                token_type = queryParams['token_type'];
                expires_in = queryParams['expires_in'];
            }
            var tenant = qs.tenant;
            var server = qs.server;
            var localLogin = qs.local;
            if(id_token === undefined && access_token === undefined){
                if(server === undefined && tenant === undefined && $window.sessionStorage.currentUser === undefined){
                    window.location.href = "/";
                    return;
                }else{
                    if($window.sessionStorage.currentUser === undefined || $window.sessionStorage.currentUser == "logout"){
                        authenticateUser(tenant, server, localLogin)
                        return
                    }
                    else{
                        var curUserObj = JSON.parse($window.sessionStorage.currentUser);
                        if(server && tenant && curUserObj.server.host != server && curUserObj.tenant != tenant){
                            authenticateUser(tenant, server, localLogin)
                            return
                        }
                    }
                }
            }
            if('logout' in queryParams)
            {
                $rootScope.globals.currentUser = null;
            }
            else
            {
                var loggedIn = $window.sessionStorage.currentUser;
                if (loggedIn === undefined || loggedIn == null) {
                    setcontext(id_token, access_token, token_type, expires_in, state);
                }
                else {
                    if (loggedIn == 'logout') {
                        setcontext(id_token, access_token, token_type, expires_in, state);
                    }
                    else {
                        $rootScope.globals.currentUser = JSON.parse($window.sessionStorage.currentUser);
                        getRestEndPoint();
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

        function getRestEndPoint() {
            var jsonData;
            readConfigFile("config/lightwaveui.json", function(data){
                jsonData = JSON.parse(data);
                for (var i = 0; i < jsonData.knownServers.length; i ++){
                    if (jsonData.knownServers[i].server.toUpperCase() === $rootScope.globals.currentUser.server.host.toUpperCase()){
                        if(typeof jsonData.knownServers[i].restEndPoint === undefined){
                            $rootScope.globals.currentUser.rest_server = jsonData.knownServers[i].server;
                        }else{
                            $rootScope.globals.currentUser.rest_server = jsonData.knownServers[i].restEndPoint;
                        }
                        $window.sessionStorage.currentUser = JSON.stringify($rootScope.globals.currentUser)
                        return;
                    }
                }
            })
        }

        function authenticateUser(tenant, server, localLogin) {
            var jsonData
            $rootScope.globals.currentUser = null;
            if(server === undefined || server.length == 0 || tenant === undefined || tenant.length == 0){
                //Redirect to home
                window.location.href = "/";
            }
            readConfigFile("config/lightwaveui.json", function(data){
                    jsonData = JSON.parse(data);
                var errAlert = document.getElementById("errorAlert")
                if(jsonData.knownServers === undefined){
                    alert("Cannot open lightwaveui for server " + server + " and tenant " + tenant);
                    window.location.href = "/";
                    return;
                }
                for (var i = 0; i < jsonData.knownServers.length; i ++){
                    if (jsonData.knownServers[i].server === server ||
                       (typeof jsonData.knownServers[i].tenant === undefined || (jsonData.knownServers[i].tenant && jsonData.knownServers[i].tenant === tenant))){
			           var OIDCClientID = jsonData.knownServers[i].oidcClientId;
                        redirectToAuthorizeUrl(server, OIDCClientID, tenant, localLogin);
                        return;
                    }
                }
                // If we are unable to locate the server/tenant from the config file.
                alert("Cannot open lightwaveui: Incorrect configuration");
                window.location.href = "/";
            }
            );
        }

        function readConfigFile(file, callback) {
            var rawFile = new XMLHttpRequest();
            rawFile.overrideMimeType("application/json");
            rawFile.open("GET", file, true);
            rawFile.onreadystatechange = function() {
                if (rawFile.status == "200") {
                    if(rawFile.readyState === 4){
                        callback(rawFile.responseText);
                    }
                }else{
                     var errAlert = document.getElementById("errorAlert")
                     errAlert.style.display = 'block'
                     errAlert.innerHTML = "<b>Error!</b> Unable to open lightwaveui. Missing configuration"
                }
            }
            rawFile.send(null);
        }

        function getHostName(url){
            var parts = url.split('://');
            var protocol = parts[0];
            var server_uri = parts[1]
            var server_with_port = server_uri.split('/')[0];
            return server_with_port;
        }

        function redirectToAuthorizeUrl(server, OIDCClientID, tenantName, localLogin){
            var hostName = getHostName(location.href);
            if(hostName === undefined || hostName.length == 0){
                // throw some error
            }
            var redirectURI = "https://" + hostName + "/lightwaveui/idm";
            var openIdConnectURI = "https://" + server + "/openidconnect/oidc/authorize/" + tenantName
            var args = "?response_type=id_token%20token&response_mode=fragment&client_id=" +
                                                  OIDCClientID +
                                                  "&redirect_uri=" + redirectURI +"&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server%20rs_vmdir";

            if(localLogin != undefined){
                if(localLogin === "true"){
                    var issuer = "https://" + server + "/openidconnect/" + tenantName;
                    var localLoginArg = "&login_hint=" + issuer;
                    args = args + localLoginArg;
                }
            }
            var authorizeUrl = openIdConnectURI + args;
            window.location = authorizeUrl;
        }

        function readParams() {
            var queryParams = $window.location.search
            if(queryParams[0] == '?'){
                queryParams = queryParams.slice(1)
            }else{
                return []
            }
            var params = queryParams.split("&")
            if(params.length != 2){
                return []
            }
            var tenant = params[0].split("=")[1]
            var server = params[1].split("=")[1]
            return [tenant,server]
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
            if(window.location.href.indexOf('#') != -1){
                window.location.href = window.location.href.split('#')[0];
                return;
            }
            initSummary();
        }

        function setcontext(id_token, access_token, token_type, expires_in, state) {

            var decodedJwt = Util.decodeJWT(id_token);
            var decodedAccessJwt = Util.decodeJWT(access_token);
            var serverHostName = getHostName(decodedJwt.header.iss);
            var clientHostName = getHostName(location.href);

            $rootScope.globals = {
                currentUser: {
                    server:  { host: serverHostName, port: $location.port(), protocol: $location.protocol() },
                    client:  { host: clientHostName },
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
