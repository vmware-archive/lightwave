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
module.controller('CertificateCntrl', ['$scope', '$rootScope', 'CertificateService', 'Util', 'popupUtil',
        function($scope, $rootScope, CertificateService, Util, popupUtil) {

            $scope.vm = this;
            $scope.showUpdateChain = showUpdateChain;
            $scope.vm.getchains = getchains;
            $scope.vm.viewcertificate = viewcertificate;
            $scope.certIndex = 1;

            init();

            function init() {
                $scope.vm.addchain = true;
                $scope.vm.chains = [];
                $scope.vm.currentCert = null;
                getchains('');
            }

            function showUpdateChain() {
                console.log('showUpdateChain clicked ..');
                var template = 'sso/certificates/chain.update.html';
                var controller = 'ChainCntrl';
                popupUtil.open($scope, template, controller);
            }

            function getchains(searchText) {
                $rootScope.globals.errors = null;
                $scope.vm.certsdataLoading = true;
                CertificateService
                    .GetCertificateChain($rootScope.globals.currentUser)
                    .then(function (res) {
                        if (res.status == 200) {
                            var chains = res.data;
                            $scope.vm.chains = [];
                            if (chains && chains.length > 0) {
                                for (var i = 0; i < chains.length; i++) {
                                    var chain = chains[chains.length - 1 - i];
                                    chain.active = (i == 0);
                                    $scope.vm.chains.push(chain);
                                }
                            }
                            populatemetadata($scope.vm.chains);
                            $scope.vm.certsdataLoading = false;
                            $scope.vm.currentCert = null;
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }

            function populatemetadata(chains) {

                if (chains) {
                    for (var i = 0; i < chains.length; i++) {
                        chains[i].name = "Chain " + (i + 1).toString();
                        for (var j = 0; j < chains[i].certificates.length; j++) {

                            // console.log('getting certificate metadata for chain: ' + chains[i].name + ' certificate: ' + j.toString());
                            var encoded = chains[i].certificates[j].encoded;
                            chains[i].certificates[j].metadata = Util.getCertificateDetails(encoded);
                        }
                    }
                }
            }

            function viewcertificate() {
                $rootScope.globals.errors = null;

                if ($scope.vm.currentCert) {
                    var template = 'shared/components/certificate/certificate.view.html';
                    var controller = 'CertificateViewerCntrl';
                    Util.viewCertificate($scope, $scope.vm.currentCert.encoded, template, controller);
                }
            }
        }]);