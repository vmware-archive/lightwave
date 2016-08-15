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

var module = angular.module('lightwave.ui.shared.components');
module.controller('ProfileCntrl', [ '$scope', 'popupUtil', 'AuthenticationService',
    function($scope, popupUtil, AuthenticationService) {
        $scope.vm = this;
        $scope.vm.changePassword = changePassword;
        $scope.vm.logout = AuthenticationService.logout;
        $scope.vm.showProfile = showProfile;
        $scope.vm.hideProfile = hideProfile;

        init();

        function init(){
            $scope.vm.profileState = false;
        }

        function hideProfile(){
            $scope.vm.profileState = false;
        }

        function showProfile(){
            $scope.vm.profileState = true;
        }

        function changePassword() {
            var template = 'shared/components/password/password.change.html';
            var controller = 'PasswordCntrl';
            popupUtil.open($scope, template, controller);
        }

    }]);