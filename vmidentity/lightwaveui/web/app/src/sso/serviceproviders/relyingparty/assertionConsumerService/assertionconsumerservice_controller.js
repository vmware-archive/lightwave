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
module.controller('AssertionConsumerServiceCntrl', [ '$scope', '$rootScope',
        function($scope, $rootScope) {

            $scope.vm.saveAssertionConsumerService = saveAssertionConsumerService;
            $scope.vm.isValidAssertionConsumerService = isValidAssertionConsumerService;

            init();

            function init() {
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
                $scope.vm.newAssertionConsumerService = {};
            }

            function saveAssertionConsumerService(acs) {

                if($scope.vm.isNewRelyingParty) {

                    if(!$scope.vm.newRelyingParty.assertionConsumerServices) {
                        $scope.vm.newRelyingParty.assertionConsumerServices = [];
                    }
                    $scope.vm.newRelyingParty.assertionConsumerServices.push(acs);
                    if(acs.isDefault) {
                        $scope.vm.newRelyingParty.defaultAssertionConsumerService = acs.name;
                    }
                } else {

                    if(!$scope.vm.selectedRelyingParty.assertionConsumerServices) {
                        $scope.vm.selectedRelyingParty.assertionConsumerServices = [];
                    }
                    $scope.vm.selectedRelyingParty.assertionConsumerServices.push(acs);
                    if(acs.isDefault) {
                        $scope.vm.selectedRelyingParty.defaultAssertionConsumerService = acs.name;
                    }
                }

                $scope.closeThisDialog('save');
            }

            function isValidAssertionConsumerService(acs) {
                return (acs &&
                acs.name &&
                acs.endpoint &&
                acs.binding &&
                acs.index);
            }
        }]);