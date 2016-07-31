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
module.controller('GroupCntrl', [ '$scope', '$rootScope', 'GroupService',
        function($scope, $rootScope, GroupService) {

            $scope.saveGroup = saveGroup;
            $scope.updateGroup = updateGroup;

            init();

            function init(){
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
                $scope.newGroup = {};
            }

            function saveGroup(group){
                $rootScope.globals.errors = '';
                var provider_name = $scope.vm.identitysource.name;
                group.upn = group.name + "@" + provider_name;
                group.domain = provider_name;
                GroupService
                    .Add($rootScope.globals.currentUser, group)
                    .then(function (res) {
                        if(res.status == 200) {
                            $rootScope.globals.errors = {details: 'Group ' + group.upn + ' added successfully', success:true};
                            $scope.newgroup = {};
                            $scope.getgroups($scope.vm.identitysources, $scope.vm.groupsearch);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function updateGroup(group){
                $rootScope.globals.errors = '';
                var newgroup = {
                    upn: group.name + "@" + group.domain,
                    name: null,
                    domain: null,
                    details: {
                        description: group.details.description
                    },
                    objectId: null
                };
                GroupService
                    .Update($rootScope.globals.currentUser, newgroup)
                    .then(function (res) {
                        if(res.status == 200) {
                            $rootScope.globals.errors = {details: 'Group ' + newgroup.upn + ' updated successfully', success:true};
                            $scope.getgroups($scope.vm.identitysources, $scope.vm.groupsearch);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }
        }]);