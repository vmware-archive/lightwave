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
module.controller('ChainCntrl', ['$scope', '$rootScope', 'CertificateService', 'Util',
        function($scope, $rootScope, CertificateService, Util) {

        $scope.newchain = { certificates : []};
        $scope.updatechain = updatechain;
        $scope.removecertificate = removecertificate;
        $scope.viewcertificate = viewcertificate;
        $scope.setprivatekeycontent = setprivatekeycontent;
        $scope.setcertificatecontent = setcertificatecontent;

        function setprivatekeycontent(chain, contents){

            if(!chain) {
                chain = {};
            }

            chain.privateKey = {
                algorithm : 'RSA',
                encoded : contents
            };
        }

        function setcertificatecontent(chain, contents) {

            if(!chain) {
                chain = {};
            }

            if(!chain.certificates) {
                chain.certificates = [];
            }

            var metadata = Util.getCertificateDetails(contents);
            var certificate = {
                encoded : contents,
                metadata : metadata
            };
            chain.certificates.push(certificate);
        }

        function removecertificate(chain, certificate){

            if(chain) {
                for(var i=0; i<chain.certificates.length;i++){
                    if(chain.certificates[i].encoded == certificate.encoded) {
                        chain.certificates.splice(i, 1);
                    }
                }
            }
        }

        function updatechain(chain) {
            $rootScope.globals.errors = null;
            var newchain = {
                privateKey: {
                    algorithm: chain.privateKey.algorithm,
                    encoded: Util.extractBase64Encoded(chain.privateKey.encoded)
                },
                certificates: []
            };

            for(var i=0; i< chain.certificates.length; i++) {
                var certificate = { encoded: chain.certificates[i].encoded };
                newchain.certificates.push(certificate);
            }

            CertificateService
                .SetTenantCredentials($rootScope.globals.currentUser, newchain)
                .then(function (res) {
                    if (res.status == 200) {
                        $scope.vm.addchain = true;
                        getchains();
                    }
                    else {
                        $rootScope.globals.errors = res.data;
                    }

                });
        }

        function viewcertificate(certificate){

            if(certificate) {
                var template = 'shared/components/certificate/certificate.view.html';
                var controller = 'CertificateViewerCntrl';
                Util.viewCertificate($scope, certificate.encoded, template, controller);
            }
        }
    }]);