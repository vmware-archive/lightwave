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
module.controller('TenantCntrl', ['$scope', '$rootScope', 'TenantService', 'OidcClientService',
    'OpenIdConnectService', 'Util', 'Configuration',
    function($scope, $rootScope, TenantService, OidcClientService,
             OpenIdConnectService, Util, Configuration) {

        $scope.addtenant = addtenant;
        $scope.setprivatekeycontent = setprivatekeycontent;
        $scope.setcertificatecontent = setcertificatecontent;
        $scope.removecertificate = removecertificate;
        $scope.viewcertificate = viewcertificate;
        $scope.isValid = isValid;

        init();


        function viewcertificate(certificate) {
            if (certificate) {
                var template = 'shared/components/certificate/certificate.view.html';
                var controller = 'CertificateViewerCntrl';
                Util.viewCertificate($scope, certificate.encoded, template, controller);

            }
        }

        function init(){
            $scope.newtenant = {
                credentials: {
                    certificates: [],
                    privateKey:{}
                }
            };
        }
        function addtenant(tenant) {
            $scope.vm.isSaving = true;
            $rootScope.globals.errors = null;
            var newchain = {
                privateKey: {
                    algorithm: tenant.credentials.privateKey.algorithm,
                    encoded: Util.extractBase64Encoded(tenant.credentials.privateKey.encoded)
                },
                certificates: []
            };

            for (var i = 0; i < tenant.credentials.certificates.length; i++) {
                var certificate = {encoded: tenant.credentials.certificates[i].encoded};
                newchain.certificates.push(certificate);
            }

            if(newchain.certificates.length < 2)
            {
                $rootScope.globals.errors = { details : 'At-least 2 certificates needs to be present.'};
                return;
            }

            var newTenant = {
                name: tenant.name,
                username: tenant.username + '@' + tenant.name,
                password: tenant.password,
                credentials: newchain
            };

            TenantService
                .Create($rootScope.globals.currentUser, newTenant)
                .then(function (res4) {
                    if (res4.status == 200) {
                        OpenIdConnectService
                            .GetToken($rootScope.globals.currentUser.server, newTenant.name, newTenant.username, newTenant.password)
                            .then(function (res1) {
                                if (res1.status == 200) {

                                    var serverConfig = $rootScope.globals.currentUser.server;
                                    var serverUri = Configuration.getServerUri(serverConfig);
                                    var uri = serverUri + "lightwaveui";
                                    var client = {
                                        "redirectUris": [uri + "/Home"],
                                        "tokenEndpointAuthMethod": "none",
                                        "postLogoutRedirectUris": [uri],
                                        "logoutUri": uri
                                    };
                                    var token = res1.data;
                                    OidcClientService
                                        .Create($rootScope.globals.currentUser.server, token , tenant.name, client)
                                        .then(function (res2) {
                                            if (res2.status == 200) {
                                                var clientId = res2.data.clientId;
                                                OidcClientService
                                                    .AddClientId($rootScope.globals.currentUser.server, token , tenant.name, clientId)
                                                    .then(function (res3) {
                                                        if (res3.status == 200) {
                                                            $scope.vm.tenants.push(tenant);
                                                            $scope.newtenant = {
                                                                credentials: {
                                                                    certificates: [],
                                                                    privateKey: {}
                                                                }
                                                            };
                                                            $rootScope.globals.errors = {
                                                                details: 'Tenant ' + tenant.name + ' added successfully',
                                                                success: true
                                                            };
                                                            $scope.vm.gettenants('');
                                                            $scope.closeThisDialog('save');
                                                        }
                                                        else {
                                                            $rootScope.globals.popup_errors = {details: "OIDC client ID could not be saved"};
                                                        }
                                                    });
                                            }
                                            else {
                                                $rootScope.globals.popup_errors = res2.data;
                                            }
                                        });
                                }
                                else {
                                    $rootScope.globals.popup_errors = res1.data;
                                }
                            });
                    }
                    else {
                        $rootScope.globals.popup_errors = res4.data;
                    }
                    $scope.vm.isSaving = false;
                });
        }

        function setprivatekeycontent(tenant, contents) {

            //tenant.privateKeyFilePath = "(selected)";
            tenant.credentials.privateKey = {
                algorithm: 'RSA',
                encoded: contents
            };
        }

        function setcertificatecontent(tenant, contents) {

            tenant.certFilePath = "(selected)";
            var metadata = Util.getCertificateDetails(contents);
            var certificate = {
                encoded: contents,
                metadata: metadata
            };
            tenant.credentials.certificates.push(certificate);
        }

        function removecertificate(chain, certificate)
        {
            for (var i = 0; i < chain.length; i++) {
                if (chain[i].encoded == certificate.encoded) {
                    chain.splice(i,1);
                    break;
                }
            }
        }

        function isValid() {
            return ($scope.newtenant.credentials
            && $scope.newtenant.credentials.certificates
            && $scope.newtenant.credentials.certificates.length >= 2
            && $scope.newtenant.credentials.privateKey);
        }
    }]);