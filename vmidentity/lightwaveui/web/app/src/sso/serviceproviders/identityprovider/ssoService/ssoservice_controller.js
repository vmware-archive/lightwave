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
module.controller('SsoServiceCntrl', [ '$scope', '$rootScope',
    function($scope, $rootScope) {

        $scope.vm.saveSso = saveSso;
        $scope.vm.isValidSso = isValidSso;

        init();

        function init() {
            $rootScope.globals.errors = '';
            $rootScope.globals.popup_errors = null;
            $scope.vm.newSso = {};
        }

        function saveSso(sso) {

            if($scope.vm.isNewIdentityProvider) {
                if(!$scope.vm.newIdentityProvider.ssoServices) {
                    $scope.vm.newIdentityProvider.ssoServices = [];
                }
                $scope.vm.newIdentityProvider.ssoServices.push(sso);
            } else {
                if(!$scope.vm.selectedIdentityProvider.ssoServices) {
                    $scope.vm.selectedIdentityProvider.ssoServices = [];
                }
                $scope.vm.selectedIdentityProvider.ssoServices.push(sso);
            }
            $scope.closeThisDialog('save');
        }

        function isValidSso(sso) {
            return (sso &&
            sso.name &&
            sso.endpoint &&
            sso.binding);
        }
    }]);