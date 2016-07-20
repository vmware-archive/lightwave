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
module.controller('TenantCntrl', ['$scope', '$rootScope', 'TenantService', 'Util',
        function($scope, $rootScope, TenantService, Util) {

            $scope.addtenant = addtenant;
            $scope.setprivatekeycontent = setprivatekeycontent;
            $scope.setcertificatecontent = setcertificatecontent;
            $scope.removecertificate = removecertificate;
            $scope.viewcertificate = viewcertificate;
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

                var newTenant = {
                    name: tenant.name,
                    username: tenant.username + '@' + tenant.name,
                    password: tenant.password,
                    credentials: newchain
                };

                TenantService
                    .Create($rootScope.globals.currentUser, newTenant)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.vm.tenants.push(tenant);
                            $scope.newtenant = {
                                credentials: {
                                    certificates: [],
                                    privateKey: {}
                                }
                            };
                            $scope.vm.gettenants('');
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                        $scope.vm.isSaving = false;
                    });
            }

            function setprivatekeycontent(tenant, contents) {

                tenant.credentials.privateKey = {
                    algorithm: 'RSA',
                    encoded: contents
                };
            }

            function setcertificatecontent(tenant, contents) {

                console.log('setcertificatecontent');
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
    }]);