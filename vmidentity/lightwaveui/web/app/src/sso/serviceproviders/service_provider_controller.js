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
module.controller('ServiceProviderCntrl', ['$scope',  '$rootScope', 'popupUtil', 'Util', 'RelyingPartyService',
        'IdentityProviderService','OidcClientService',
        function($scope, $rootScope, popupUtil, Util, RelyingPartyService, IdentityProviderService, OidcClientService) {
            $scope.vm = this;
            $scope.vm.currentTab = 0;

            $scope.vm.tokenauthmethods = [{name:'none'}, {name:'private-key-jwt'}];

            /* Relying Party */
            $scope.vm.relyingparties = [];
            $scope.vm.addrelyingparty = true;
            $scope.currentrelyingparty = {};
            $scope.vm.newrelyingparty = {};

            $scope.vm.getallrelyingparty = getallrelyingparty;
            $scope.vm.viewrelyingparty = viewrelyingparty;

            /* Identity Providers */
            $scope.vm.identityproviders = [];
            $scope.vm.addidentityprovider = true;
            $scope.currentidentityprovider = {};
            $scope.vm.newidentityprovider = {};

            $scope.vm.getallidentityprovider = getallidentityprovider;
            $scope.vm.viewidentityprovider = viewidentityprovider;

            /* OIDC Clients */
            $scope.vm.oidcclients = [];
            $scope.vm.addoidcclient = true;
            $scope.currentoidcclient = {};
            $scope.vm.newoidcclient = {};

            $scope.vm.getalloidcclient = getalloidcclient;
            $scope.vm.viewoidcclient = viewoidcclient;

            init();

            function init() {
                getallrelyingparty();
            }

            function getallrelyingparty(searchText) {

                $rootScope.globals.errors = null;
                $scope.vm.selectedRelyingparty = null;
                $scope.vm.rpdataLoading = true;
                $scope.vm.relyingparties = [];
                $scope.vm.currentTab = 0;
                $scope.currentrelyingparty = {};
                RelyingPartyService
                    .GetAll($rootScope.globals.currentUser, searchText)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.relyingparties = [];
                            var rps = res.data;
                            if (rps != null) {
                                for (var i = 0; i < rps.length; i++) {
                                    if ((!searchText || searchText == '') ||
                                        rps[i].name.indexOf(searchText) > -1) {
                                        rps[i].cert = Util.getCertificateDetails(rps[i].certificate.encoded);
                                        $scope.vm.relyingparties.push(rps[i]);
                                    }
                                }
                            }
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                        $scope.vm.rpdataLoading = false;
                    });
            }

            function viewrelyingparty() {

                $rootScope.globals.errors = null;
                if($scope.vm.selectedRelyingParty) {
                    var template = 'sso/serviceproviders/relyingparty/relyingparty.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }

            function getallidentityprovider(searchText) {
                $rootScope.globals.errors = null;
                $scope.vm.selectedIdentityProvider = null;
                $scope.vm.idpdataLoading = true;
                $scope.vm.identityproviders = [];
                $scope.vm.currentTab = 1;
                $scope.currentidentityprovider = {};
                IdentityProviderService
                    .GetAll($rootScope.globals.currentUser, searchText)
                    .then(function (res) {
                        if (res.status == 200) {
                            var rps = res.data;
                            if (rps != null) {
                                for (var i = 0; i < rps.length; i++) {
                                    if (!searchText || searchText == '' || rps[i].entityId.indexOf(searchText) > -1) {

                                        if (rps[i].signingCertificates && rps[i].signingCertificates.certificates) {
                                            for (var j = 0; j < rps[i].signingCertificates.certificates.length; j++) {
                                                var cert = rps[i].signingCertificates.certificates[j];
                                                rps[i].signingCertificates.certificates[j].metadata = Util.getCertificateDetails(cert.encoded);
                                            }
                                        }
                                        $scope.vm.identityproviders.push(rps[i]);

                                    }
                                }
                            }
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                        $scope.vm.idpdataLoading = false;
                    });
            }

            function viewidentityprovider(){
                $rootScope.globals.errors = null;

                if($scope.vm.selectedIdentityProvider) {
                    var template = 'sso/serviceproviders/identityprovider/identityprovider.view.html';
                    var controller = 'IdentityProviderCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }

            function getalloidcclient(searchText) {
                $rootScope.globals.errors = null;
                $scope.vm.selectedOIDCClient = null;
                $scope.vm.oidcdataLoading = true;
                $scope.vm.oidcclients = [];
                $scope.vm.currentTab = 2;
                $scope.currentoidcclient = {};
                OidcClientService
                    .GetAll($rootScope.globals.currentUser, searchText)
                    .then(function (res) {
                        if (res.status == 200) {
                            var rps = res.data;
                            if (!searchText || searchText == '') {
                                $scope.vm.oidcclients = rps;
                            } else if (rps != null) {
                                for (var i = 0; i < rps.length; i++) {
                                    if (rps[i].clientId.indexOf(searchText) > -1) {
                                        rps[i].oidcclientMetadataDTO.authMethod =
                                        {
                                            name: rps[i].oidcclientMetadataDTO.tokenEndpointAuthMethod
                                        };
                                        $scope.vm.oidcclients.push(rps[i]);
                                    }
                                }
                            }
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                        $scope.vm.oidcdataLoading = false;
                    });
            }

            function viewoidcclient(){
                $rootScope.globals.errors = null;

                if($scope.vm.selectedOIDCClient) {
                    var template = 'sso/serviceproviders/oidcclient/oidcclient.view.html';
                    var controller = 'OidcClientCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
        }]);