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
module.controller('OidcClientCntrl', [ '$scope', '$rootScope', 'OidcClientService',
        function($scope, $rootScope, OidcClientService) {

                $scope.vm.addRedirectEditUri = addRedirectEditUri;
                $scope.vm.addRedirectAddUri = addRedirectAddUri;
                $scope.vm.addPRedirectEditUri = addPRedirectEditUri;
                $scope.vm.addPRedirectAddUri = addPRedirectAddUri;
                $scope.vm.removeUri = removeUri;
                $scope.vm.saveOidcClient = saveOidcClient;
                $scope.vm.updateOidcClient = updateOidcClient;
                $scope.vm.isValidOidcClient = isValidOidcClient;

                init();

                function init() {
                        $rootScope.globals.errors = '';
                        $rootScope.globals.popup_errors = null;

                        if($scope.vm.selectedOIDCClient) {
                                var jwt = ($scope.vm.selectedOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethod == "private_key_jwt");
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethodPrivateKeyJwt = jwt;
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethodNone = !jwt;
                        }
                        $scope.vm.newOIDCClient = {
                                oidcclientMetadataDTO : {
                                        tokenEndpointAuthMethod : "none",
                                        tokenEndpointAuthMethodNone: true,
                                        tokenEndpointAuthMethodPrivateKeyJwt: false

                        }};
                }

                function saveOidcClient(oidc){

                        if($scope.vm.newOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethodPrivateKeyJwt)
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethod = "private_key_jwt";
                        else
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethod = "none";

                        $rootScope.globals.errors = '';
                        OidcClientService
                            .Create($rootScope.globals.currentUser, oidc)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $rootScope.globals.errors = {details: 'OIDC Client ' + oidc.clientId + ' added successfully', success:true};
                                            $scope.newOIDCClient = {};
                                            $scope.getalloidcclient();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.popup_errors = res.data;
                                    }
                            });
                }

                function updateOidcClient(oidc){

                        if($scope.vm.newOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethodPrivateKeyJwt)
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethod = "private_key_jwt";
                        else
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.tokenEndpointAuthMethod = "none";

                        $rootScope.globals.errors = '';
                        OidcClientService
                            .Update($rootScope.globals.currentUser, oidc)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $rootScope.globals.errors = {details: 'OIDC Client ' + oidc.clientId + ' updated successfully', success:true};
                                            $scope.selectedOIDCClient = null;
                                            $scope.getalloidcclient();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.popup_errors = res.data;
                                    }
                            });
                }

                function removeUri(uris, uri) {

                        if(uris && uri){
                                for(var i=0;i<uris.length; i++){
                                        if(uris[i] == uri){
                                                uris.splice(i, 1);
                                        }

                                }
                        }
                }

                function addRedirectEditUri(uris,uri) {

                        if(!uris)
                        {
                                uris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                uris.push(uri);
                                console.log('pushed');
                                uri = '';
                        }
                }
                function addRedirectAddUri(uris,uri) {

                        if(!uris)
                        {
                                uris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                uris.push(uri);
                                console.log('pushed');
                                uri = '';
                        }
                }
                function addPRedirectEditUri(uris,uri) {

                        if(!uris)
                        {
                                uris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                uris.push(uri);
                                console.log('pushed');
                                uri = '';
                        }
                }
                function addPRedirectAddUri(uris,uri) {

                        if(!uris)
                        {
                                uris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                uris.push(uri);
                                console.log('pushed');
                                uri = '';
                        }
                }

                function isValidUri(uri) {
                        var str = uri.toString();
                        var expression = /[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/gi;
                        var regex = new RegExp(expression);
                        var status = str.match(regex);

                        if(!status){
                                $rootScope.globals.popup_errors = {details: 'Invalid URI'};
                                return false;
                        }
                        if(str.indexOf('https://') != 0)
                        {
                                $rootScope.globals.popup_errors = {details: 'URI should start with https://'};
                                return false;
                        }
                        return true;
                }

                function isValidOidcClient(oidcClient) {

                        if(oidcClient && oidcClient.oidcclientMetadataDTO) {
                                if(oidcClient.oidcclientMetadataDTO.tokenEndpointAuthMethodPrivateKeyJwt && !oidcClient.oidcclientMetadataDTO.certSubjectDN) {
                                        $rootScope.globals.popup_errors = {details: 'No subject DN provided for Private Key JWT token method.'};
                                        return false;
                                }

                                if(oidcClient.oidcclientMetadataDTO.logoutUri.toString().indexOf('https://') != 0) {
                                        $rootScope.globals.popup_errors = {details: 'Logout URI should start with https://'};
                                        return false;
                                }
                        }
                        return true;
                }
        }]);