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
module.controller('AttributeConsumerServiceCntrl', [ '$scope', '$rootScope', 'popupUtil',
    function($scope, $rootScope, popupUtil) {

        $scope.vm.saveAttributeConsumerService = saveAttributeConsumerService;
        $scope.vm.isValidAttributeConsumerService = isValidAttributeConsumerService;
        $scope.vm.addAttribute = addAttribute;
        $scope.vm.removeAttribute = removeAttribute;
        init();

        function init() {
            $rootScope.globals.errors = '';
            $rootScope.globals.popup_errors = null;
            $scope.vm.newAttributeConsumerService = {};
        }

        function saveAttributeConsumerService(acs) {

            if($scope.vm.isNewRelyingParty) {

                if(!$scope.vm.newRelyingParty.attributeConsumerServices) {
                    $scope.vm.newRelyingParty.attributeConsumerServices = [];
                }
                $scope.vm.newRelyingParty.attributeConsumerServices.push(acs);
                if(acs.isDefault) {
                    $scope.vm.newRelyingParty.defaultAttributeConsumerService = acs.name;
                }
            } else {

                if(!$scope.vm.selectedRelyingParty.attributeConsumerServices) {
                    $scope.vm.selectedRelyingParty.attributeConsumerServices = [];
                }
                $scope.vm.selectedRelyingParty.attributeConsumerServices.push(acs);
                if(acs.isDefault) {
                    $scope.vm.selectedRelyingParty.defaultAttributeConsumerService = acs.name;
                }
            }

            $scope.closeThisDialog('save');
        }

        function isValidAttributeConsumerService(acs) {
            return (acs &&
            acs.name &&
            acs.index &&
            acs.attributes && acs.attributes.length > 0);
        }

        function addAttribute() {
            $rootScope.globals.popup_errors = null;
            var template = 'sso/serviceproviders/relyingparty/attributeConsumerService/attribute/attribute.add.html';
            var controller = 'AttributeCntrl';
            popupUtil.open($scope, template, controller);
        }


        function removeAttribute(attributes, attribute){

            if(attributes && attribute) {
                for (var i = 0; i < attributes.length; i++) {

                    if (attributes[i].name == attribute.name &&
                        attributes[i].friendlyName == attribute.friendlyName &&
                        attributes[i].nameFormat == attribute.nameFormat) {
                        attributes.splice(i, 1);
                        break;
                    }
                }
            }
        }
    }]);