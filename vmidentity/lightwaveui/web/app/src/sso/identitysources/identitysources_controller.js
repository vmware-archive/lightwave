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
module.controller('IdentitySourcesCntrl', ['$scope',  '$rootScope', 'popupUtil', 'IdentitySourceService', 'TenantService', 'Util',
        function($scope, $rootScope, popupUtil, IdentitySourceService, TenantService, Util) {

            $scope.vm = this;
            $scope.vm.getIdentitySources = getIdentitySources;
            $scope.vm.view = view;
            $scope.vm.edit = edit;
            $scope.vm.add = add;
            $scope.vm.deleteIdentitySource = deleteIdentitySource;
            $scope.vm.providerPolicy = {};

            init();

            function init(){
                getIdentitySources();
            }

            function add() {
                $scope.vm.isNew = true;
                $scope.vm.newIdentitySource = {
                        connectionStrings: [],
                        type: 'IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING'
                };
                $scope.vm.selectedIdentitysource = null;
                var template = 'sso/identitysources/identitysource/identitysource.add.html';
                var controller = 'IdentitySourceCntrl';
                popupUtil.open($scope, template, controller);
            }

            function edit(ids){
                if(ids && ids.domainType == 'EXTERNAL_DOMAIN'){
                    $scope.vm.isNew = false;
                    $scope.vm.selectedIdentitysource = ids;
                    var template = 'sso/identitysources/identitysource/identitysource.edit.html';
                    var controller = 'IdentitySourceCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }

            function view(ids){
                if(ids && ids.domainType == 'EXTERNAL_DOMAIN'){
                    $scope.vm.selectedIdentitysource = ids;
                    var template = 'sso/identitysources/identitysource/identitysource.view.html';
                    var controller = 'IdentitySourceCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }

            function getIdentitySources(searchText) {
                $rootScope.globals.errors = null;
                $scope.vm.idsdataLoading = true;
                IdentitySourceService
                    .GetAll($rootScope.globals.currentUser)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.identitySources = res.data;
                            getDefaultProvider();
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    })
                    .then(function (res) {
                        var identitySources = $scope.vm.identitySources;
                        $scope.vm.filteredIdentitySources = [];
                        if (identitySources != null && identitySources.length > 0) {
                            for (var i = 0; i < $scope.vm.identitySources.length; i++) {
                                if (!searchText ||
                                    searchText == '' ||
                                    identitySources[i].name.indexOf(searchText) > -1) {
                                    if (identitySources[i].domainType == 'EXTERNAL_DOMAIN') {

                                        if (identitySources[i].connectionStrings[0].startsWith('ldaps')) {
                                            identitySources[i].ssl = true;
                                        }

                                        for (var j = 0; j < identitySources[i].certificates.length; j++) {
                                            var encoded = identitySources[i].certificates[j].encoded;
                                            identitySources[i].certificates[j].metadata = Util.getCertificateDetails(encoded);
                                        }
                                    }
                                    $scope.vm.filteredIdentitySources.push(identitySources[i]);
                                }
                            }
                        }
                        $scope.vm.idsdataLoading = false;
                    });
            }

            function getDefaultProvider() {
                $rootScope.globals.errors = null;
                var configs = ['PROVIDER'];

                TenantService
                    .GetConfiguration($rootScope.globals.currentUser, configs)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.providerPolicy = res.data.providerPolicy;
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }

            function deleteIdentitySource(ids) {
                $rootScope.globals.errors = null;
                if(ids && ids.domainType == 'EXTERNAL_DOMAIN') {
                    IdentitySourceService
                        .Delete($rootScope.globals.currentUser, ids.name)
                        .then(function (res) {
                            if (res.status == 200 || res.status == 204) {
                                getIdentitySources();
                            }
                            else {
                                $rootScope.globals.errors = res.data;
                            }
                        });
                }
            }
        }]);