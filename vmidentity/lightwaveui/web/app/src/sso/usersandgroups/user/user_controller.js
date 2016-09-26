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
module.controller('UserCntrl', [ '$scope', '$rootScope', 'UserService', 'Util',
        function($scope, $rootScope, UserService, Util) {

            $scope.saveUser = saveUser;
            $scope.updateUser = updateUser;
            $scope.updateUserPassword = updateUserPassword;
            $scope.numberToTime = Util.numberToTime;
            $scope.unixToDateText = Util.unixToDateText;
            $scope.isValid = isValid;
            init();

            function init(){
                $rootScope.globals.errors = null;
                $rootScope.globals.popup_errors = null;
                $scope.newuser = {};
            }

            function isValid(newPassword, confirmPassword){
                return (newPassword &&
                confirmPassword &&
                newPassword == confirmPassword);
            }

            function saveUser(user){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                user.details.upn = user.name + "@" + provider.name;
                user.alias = { name: user.name, domain: provider.name };
                user.domain = provider.name;

                if(!user.disabled) {
                    user.disabled = false;
                }
				user.locked = false;
                UserService
                    .Add($rootScope.globals.currentUser, user)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'User ' + user.details.upn + ' added successfully', success:true};
                            $scope.newuser = {};
                            $scope.addNewUser = true;
                            $scope.currentPassword = null;
                            $scope.getusers(provider, $scope.vm.search);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function updateUser(user){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                user.details.upn = user.name + "@" + provider.name;
                user.alias = { name: user.name, domain: provider.name };
                user.domain = provider.name;
                UserService
                    .Update($rootScope.globals.currentUser, user)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'User ' + user.details.upn + ' updated successfully', success:true};
                            $scope.getusers(provider, $scope.vm.search);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function updateUserPassword(user){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                user.details.upn = user.name + "@" + provider.name;
                user.alias = { name: user.name, domain: provider.name };
                user.domain = provider.name;

                UserService
                    .SetPassword($rootScope.globals.currentUser, user, user.passwordDetails)
                    .then(function(res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Password for user ' + user.details.upn + ' updated successfully', success:true};
                            user.passwordDetails.newPassword = '';
                            user.passwordDetails.confirmpassword = '';
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                });
            }
        }]);