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
module.controller('UserCntrl', [ '$scope', '$rootScope', 'UserService',
        function($scope, $rootScope, UserService) {

            $scope.saveUser = saveUser;
            $scope.updateUser = updateUser;
            $scope.updateUserPassword = updateUserPassword;

            init();

            function init(){
                $scope.newUser = {};
            }

            function saveUser(user){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                user.details.upn = user.name + "@" + provider.name;
                user.alias = { name: user.name, domain: provider.name };
                user.domain = provider.name;
                console.log('inside saveUser: ' + JSON.stringify(user));
                UserService
                    .Add($rootScope.globals.currentUser, user)
                    .then(function (res) {
                        if (res.status == 200) {
                            console.log('Save response: ' + JSON.stringify(res));
                            $scope.newuser = {};
                            $scope.addNewUser = true;
                            $scope.currentPassword = null;
                            $scope.getusers(provider, $scope.vm.search);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }

            function updateUser(user){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                user.details.upn = user.name + "@" + provider.name;
                user.alias = { name: user.name, domain: provider.name };
                user.domain = provider.name;
                console.log('inside savecurrentuser: ' + JSON.stringify(user));
                UserService
                    .Update($rootScope.globals.currentUser, user)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.getusers(provider, $scope.vm.search);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    });
            }

            function updateUserPassword(user){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                user.details.upn = user.name + "@" + provider.name;
                user.alias = { name: user.name, domain: provider.name };
                user.domain = provider.name;
                console.log('inside updateUserPassword: ' + JSON.stringify(user));

                UserService
                    .SetPassword($rootScope.globals.currentUser, user, user.passwordDetails)
                    .then(function(res) {
                        if (res.status == 200) {
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                });
            }
        }]);