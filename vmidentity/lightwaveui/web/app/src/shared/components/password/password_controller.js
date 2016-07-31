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
module.controller('PasswordCntrl', [ '$scope', '$rootScope', 'UserService',
    function($scope, $rootScope, UserService) {
        $scope.vm = this;
        $scope.vm.updatePassword = updatePassword;
        $scope.vm.isValid = isValid;

        init();

        function init(){
            $rootScope.globals.errors = null;
            $rootScope.globals.popup_errors = null;
        }
        function updatePassword() {

            $rootScope.globals.errors = '';
            var password = {
                newPassword : $scope.newPassword,
                currentPassword : $scope.currentPassword
            };
            var provider = $rootScope.globals.currentUser.tenant;
            var username = $rootScope.globals.currentUser.username;
            var user = {
                details: { upn: username },
                alias: {
                    name: username,
                    domain: provider
                },
                domain: provider,
                passwordDetails: password
            };
            UserService
                .SetPassword($rootScope.globals.currentUser, user, user.passwordDetails)
                .then(function(res) {
                    if (res.status == 200) {
                        $rootScope.globals.errors = {details: 'Password for user ' + user.details.upn + ' updated successfully', success:true};
                        $scope.newPassword = '';
                        $scope.currentPassword = '';
                        $scope.closeThisDialog('save');
                    }
                    else {
                        $rootScope.globals.popup_errors = res.data;
                    }
                });
        }

        function isValid(){
            return ($scope.newPassword &&
            $scope.confirmpassword &&
            $scope.currentPassword &&
            $scope.confirmpassword == $scope.newPassword);
        }
    }]);