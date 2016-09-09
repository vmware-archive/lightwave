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

                        $scope.vm.newOIDCClient = {
                                oidcclientMetadataDTO : {
                                        tokenEndpointAuthMethod : "none"
                        }};
                }

                function saveOidcClient(oidc){
                        $rootScope.globals.errors = '';
                        OidcClientService
                            .Add($rootScope.globals.currentUser, oidc.oidcclientMetadataDTO)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $rootScope.globals.errors = {details: 'OIDC Client added successfully', success:true};
                                            $scope.newOIDCClient = {};
                                            $scope.vm.getalloidcclient();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.popup_errors = res.data;
                                    }
                            });
                }

                function updateOidcClient(oidc){
                        $rootScope.globals.errors = '';
                        OidcClientService
                            .Update($rootScope.globals.currentUser, oidc)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $rootScope.globals.errors = {details: 'OIDC Client ' + oidc.clientId + ' updated successfully', success:true};
                                            $scope.selectedOIDCClient = null;
                                            $scope.vm.getalloidcclient();
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

                function addPRedirectEditUri(uri) {

                        if(!$scope.vm.selectedOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUris)
                        {
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUris.push(uri);
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUri = '';
                        }
                }
                function addPRedirectAddUri(uri) {

                        if(!$scope.vm.newOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUris)
                        {
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUris.push(uri);
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.postLogoutRedirectUri = '';
                        }
                }
                function addRedirectEditUri(uri) {

                        if(!$scope.vm.selectedOIDCClient.oidcclientMetadataDTO.redirectUris)
                        {
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.redirectUris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.redirectUris.push(uri);
                                $scope.vm.selectedOIDCClient.oidcclientMetadataDTO.redirectUri = '';
                        }
                }
                function addRedirectAddUri(uri) {

                        if(!$scope.vm.newOIDCClient.oidcclientMetadataDTO.redirectUris)
                        {
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.redirectUris = [];
                        }
                        if(uri && isValidUri(uri)) {
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.redirectUris.push(uri);
                                $scope.vm.newOIDCClient.oidcclientMetadataDTO.redirectUri = '';
                        }
                }

                function isValidUri(uri) {

                        if(uri) {
                                var str = uri.toString();
                                if (str.indexOf('https://') != 0) {
                                        $rootScope.globals.popup_errors = {details: 'URI should start with https://'};
                                        return false;
                                }
                                if (str.length < 9) {
                                        $rootScope.globals.popup_errors = {details: 'Invalid URI'};
                                        return false;
                                }
                        }
                        return true;
                }

                function isValidOidcClient(oidcClient) {

                        if(oidcClient && oidcClient.oidcclientMetadataDTO) {
                                if (oidcClient.oidcclientMetadataDTO.tokenEndpointAuthMethod == 'private_key_jwt'
                                    && !oidcClient.oidcclientMetadataDTO.certSubjectDN) {
                                        $rootScope.globals.popup_errors = {details: 'No subject DN provided for Private Key JWT token method.'};
                                        return false;
                                }

                                if (oidcClient.oidcclientMetadataDTO.logoutUri && oidcClient.oidcclientMetadataDTO.logoutUri.toString().toLowerCase().indexOf('https://') != 0) {
                                        $rootScope.globals.popup_errors = {details: 'Logout URI should start with https://'};
                                        return false;
                                }


                                return oidcClient.oidcclientMetadataDTO.tokenEndpointAuthMethod &&
                                    oidcClient.oidcclientMetadataDTO.logoutUri &&
                                    oidcClient.oidcclientMetadataDTO.postLogoutRedirectUris &&
                                    oidcClient.oidcclientMetadataDTO.postLogoutRedirectUris.length > 0 &&
                                    oidcClient.oidcclientMetadataDTO.redirectUris &&
                                    oidcClient.oidcclientMetadataDTO.redirectUris.length > 0;
                        }
                        return false;
                }
        }]);