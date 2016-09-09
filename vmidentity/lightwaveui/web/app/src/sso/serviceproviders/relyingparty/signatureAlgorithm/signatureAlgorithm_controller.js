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
module.controller('SignatureAlgorithmCntrl', [ '$scope', '$rootScope',
        function($scope, $rootScope) {

            $scope.vm.saveSignatureAlgorithm = saveSignatureAlgorithm;
            $scope.vm.isValidSignatureAlgorithm = isValidSignatureAlgorithm;

            init();

            function init() {
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
                $scope.vm.newSignatureAlgorithm = {};
            }

            function saveSignatureAlgorithm(algo) {

                if($scope.vm.isNewRelyingParty) {

                    if(!$scope.vm.newRelyingParty.signatureAlgorithms) {
                        $scope.vm.newRelyingParty.signatureAlgorithms = [];
                    }
                    $scope.vm.newRelyingParty.signatureAlgorithms.push(algo);
                } else {

                    if(!$scope.vm.selectedRelyingParty.signatureAlgorithms) {
                        $scope.vm.selectedRelyingParty.signatureAlgorithms = [];
                    }
                    $scope.vm.selectedRelyingParty.signatureAlgorithms.push(algo);
                }
                $scope.closeThisDialog('save');
            }

            function isValidSignatureAlgorithm(algo) {
                return (algo &&
                algo.priority &&
                algo.minKeySize &&
                algo.maxKeySize &&
                algo.minKeySize < algo.maxKeySize);
            }
        }]);