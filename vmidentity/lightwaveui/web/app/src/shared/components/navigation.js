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
module.controller('NavigationCntrl', [ '$scope', '$rootScope',
        function($scope, $rootScope) {
            $scope.vm = this;
            $scope.vm.select = select;
            $scope.vm.getItemStyle = getItemStyle;
            $scope.vm.exists = exists;

            init();

            function init() {
                $scope.vm.menus = [
                    {
                        name: "Single Sign-On",
                        image: "sso.png",
                        header: true,
                        href: "#ssohome",
                        selected: true,
                        roles: ['Administrator', 'RegularUser','GuestUser']

                    },
                    {
                        name: "Users & Groups",
                        image: "group.png",
                        header: false,
                        href: "#usersandgroups",
                        roles: ['Administrator', 'RegularUser']
                    },
                    {
                        name: "Identity Sources",
                        image: "tenant.png",
                        header: false,
                        href: "#identitysources",
                        roles: ['Administrator', 'RegularUser']
                    },
                    {
                        name: "Service Providers",
                        image: "serviceprovider.png",
                        header: false,
                        href: "#serviceproviders",
                        roles: ['Administrator']
                    },
                    {
                        name: "Certificates",
                        image: "certificate.png",
                        header: false,
                        href: "#ssocertificate",
                        roles: ['Administrator', 'RegularUser', 'GuestUser']
                    },
                    {
                        name: "Policies & Configuration",
                        image: "configuration.png",
                        header: false,
                        href: "#ssopolicies",
                        roles: ['Administrator', 'RegularUser']
                    },
                    {
                        name: "Computers & Tenants",
                        image: "computer.png",
                        header: false,
                        href: "#ssoservermgmt",
                        roles: ['Administrator']
                    }
                ];
            }

            function exists(roles, role){

                var contains = false;
                for(var i=0;i<roles.length;i++){
                    if(roles[i] == role){
                        contains = true;
                        break;
                    }
                }
                return contains;
            }

            function select(menu){
                for (var i=0; i<$scope.vm.menus.length; i++) {
                    $scope.vm.menus[i].selected = false;
                }
                menu.selected = true;
            }

            function getItemStyle(menu){
                var style = 'navigation-item';
                var selected = menu.selected ? '-selected' : '';
                var header = menu.header ? '-header' : '';
                return style + header + selected;
            }
        }]);