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
module.controller('GroupMembersCntrl', [ '$scope', '$rootScope', 'GroupService', 'MemberService',
        function($scope, $rootScope, GroupService, MemberService) {

            $scope.vm.saveMembers = saveMembers;
            $scope.vm.deleteSelectedMembers = deleteSelectedMembers;
            $scope.vm.getAllMembers = getAllMembers;
            $scope.vm.clearMemberSearch = clearMemberSearch;
            $scope.vm.searchMembers = searchMembers;
            $scope.vm.addMembersToGroup = addMembersToGroup;
            $scope.vm.searchSelectedMembers = searchSelectedMembers;

            init();

            function init() {
                $rootScope.globals.errors = '';
                var group = $scope.vm.selectedGroup;
                $scope.vm.selectedmembersearch = '';
                getMembers(group, '');

                if($scope.vm.idsmembersearch) {
                    getAllMembers($scope.vm.idsmembersearch);
                }
                clearMemberSearch();
            }

            function searchSelectedMembers(){
                $rootScope.globals.errors = '';
                var group = $scope.vm.selectedGroup;
                var text = $scope.vm.selectedmembersearch;
                getMembers(group, text);
            }
            function getMembers(group, searchText) {
                $rootScope.globals.errors = '';
                $scope.vm.searchSelectedMembersLoading = true;
                $scope.currentgroup = {};
                var upn = group.name + "@" + group.domain;
                GroupService
                    .Get($rootScope.globals.currentUser, upn)
                    .then(function (res) {
                        if(res.status == 200){
                            $scope.currentgroup = res.data;
                            $scope.currentgroup.members = {};
                        }
                        else {
                            $rootScope.globals.errors = res.data;
                        }
                    })
                    .then(function (res) {
                        if ($scope.currentgroup) {
                            GroupService
                                .GetMembership($rootScope.globals.currentUser, upn, "USER")
                                .then(function (res) {
                                    if (res.status == 200) {
                                        $scope.currentgroup.members.users = [];
                                        if (res.data.users) {
                                            for (var i = 0; i < res.data.users.length; i++) {
                                                if (!search || searchText == '' || res.data.users[i].name.indexOf(searchText) > -1) {
                                                    $scope.currentgroup.members.users.push(res.data.users[i]);
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        $rootScope.globals.errors = res.data;
                                    }
                                });
                        }
                    })
                    .then(function (res) {
                        if ($scope.currentgroup) {
                            GroupService
                                .GetMembership($rootScope.globals.currentUser, upn, "SOLUTIONUSER")
                                .then(function (res) {
                                    if(res.status == 200){
                                        $scope.currentgroup.members.solutionusers = [];
                                        if(res.data.solutionUsers)
                                        {
                                            for(var i=0;i<res.data.solutionUsers.length; i++){
                                                if(!searchText || searchText == '' || res.data.solutionUsers[i].name.indexOf(searchText) > -1)
                                                {
                                                    $scope.currentgroup.members.solutionusers.push(res.data.solutionUsers[i]);
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        $rootScope.globals.errors = res.data;
                                    }
                                });
                        }
                    })
                    .then(function (res) {
                        if ($scope.currentgroup) {
                            GroupService
                                .GetMembership($rootScope.globals.currentUser, upn, "GROUP")
                                .then(function (res) {
                                    if (res.status == 200) {
                                        $scope.currentgroup.members.groups = [];
                                        if (res.data.groups) {
                                            for (var i = 0; i < res.data.groups.length; i++) {
                                                if (!searchText || searchText == '' || res.data.groups[i].name.indexOf(searchText) > -1) {
                                                    $scope.currentgroup.members.groups.push(res.data.groups[i]);
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        $rootScope.globals.errors = res.data;
                                    }
                                    $scope.vm.searchSelectedMembersLoading = false;
                                });
                        }
                    })
            }

            function saveMembers(){
                $scope.vm.isSaving = true;
                $rootScope.globals.errors = '';
                var group = $scope.currentgroup;
                var upn = group.name + '%40' + group.domain;

                if(group.members) {

                    if (group.members.groups) {

                        var deletedgroups = '';
                        for (var i = group.members.groups.length - 1; i >= 0; i--) {
                            if (group.members.groups[i].state == 2) {
                                deletedgroups += 'members=' + group.members.groups[i].name + '@' + group.members.groups[i].domain + '&';
                            }
                        }

                        var addedgroups = '';
                        for (var i = group.members.groups.length - 1; i >= 0; i--) {
                            if (group.members.groups[i].state == 1) {
                                addedgroups += 'members=' + group.members.groups[i].name + '@' + group.members.groups[i].domain + '&';
                            }
                        }

                        if (addedgroups != '') {
                            GroupService
                                .UpdateMembers($rootScope.globals.currentUser, upn, addedgroups, 'GROUP')
                                .then(function (res) {
                                    if (!res.status == 200) {
                                        $rootScope.globals.errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }

                        if (deletedgroups != '') {
                            GroupService
                                .DeleteMembers($rootScope.globals.currentUser, upn, deletedgroups, 'GROUP')
                                .then(function (res) {
                                    if (!res.status == 200) {
                                        $rootScope.globals.errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }
                    }

                    if (group.members.users) {

                        var deletedusers = '';
                        for (var i = group.members.users.length - 1; i >= 0; i--) {
                            if (group.members.users[i].state == 2) {
                                deletedusers += 'members=' + group.members.users[i].name + '@' + group.members.users[i].domain + '&';
                            }
                        }

                        var addedusers = '';
                        for (var i = group.members.users.length - 1; i >= 0; i--) {
                            if (group.members.users[i].state == 1) {
                                addedusers += 'members=' + group.members.users[i].name + '@' + group.members.users[i].domain + '&';
                            }
                        }

                        if (addedusers != '') {
                            GroupService
                                .UpdateMembers($rootScope.globals.currentUser, upn, addedusers, 'USER')
                                .then(function (res) {
                                    if (!res.status == 200) {
                                        $rootScope.globals.errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }
                        if (deletedusers != '') {
                            GroupService
                                .DeleteMembers($rootScope.globals.currentUser, upn, deletedusers, 'USER')
                                .then(function (res) {
                                    if (!res.status == 200) {
                                        $rootScope.globals.errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }
                    }
                }
            }

            function getAllMembers(provider){
                $scope.membersearchtype = { name:"all" };
                searchMembers('', provider, $scope.membersearchtype);
            }

            function clearMemberSearch()
            {
                $scope.vm.membersearch = '';
                $scope.vm.membersearchresult = {};
            }

            function searchMembers(searchvalue, provider, type) {

                var typename = type.name;
                $scope.vm.membersearchresult = {};
                $scope.vm.searchMembersLoading = true;
                if(type.name == "user")
                {
                    MemberService
                        .Search($rootScope.globals.currentUser, provider.name, "USER", "NAME", searchvalue)
                        .then(function (res) {
                            if (res.status == 200) {
                                // console.log("Users: " + JSON.stringify(res.data));
                                $scope.vm.membersearchresult.users = res.data.users;
                                $scope.vm.searchMembersLoading = false;
                            }
                            else {
                                $rootScope.globals.errors = res.data;
                            }
                        });

                } else if(type.name == "group")
                {
                    MemberService
                        .Search($rootScope.globals.currentUser, provider.name, "GROUP", "NAME", searchvalue)
                        .then(function (res) {
                            if (res.status == 200) {
                                $scope.vm.membersearchresult.groups = res.data.groups;
                                $scope.vm.searchMembersLoading = false;
                            }
                            else {
                                $rootScope.globals.errors = res.data;
                            }
                        });
                } else if(type.name == "all")
                {
                    MemberService
                        .Search($rootScope.globals.currentUser, provider.name, "ALL", "NAME", searchvalue)
                        .then(function (res) {
                            if (res.status == 200) {
                                $scope.vm.membersearchresult.users = res.data.users;
                                $scope.vm.membersearchresult.groups = res.data.groups;
                                $scope.vm.searchMembersLoading = false;
                            }
                            else {
                                $rootScope.globals.errors = res.data;
                            }
                        })
                }
            }

            function addMembersToGroup(){
                console.log('addMembersToGroup');
                if($scope.vm.membersearchresult) {
                    addUserMembers();
                    addGroupMembers();
                }
            }

            function addUserMembers(){
                var group = $scope.currentgroup;

                if(group.members == null)
                    group.members = {};
                if(group.members.users == null)
                    group.members.users = [];

                console.log('addUserMembers');

                /* check duplicate */
                var dup = false;
                var user = null;

                if($scope.vm.membersearchresult.users) {
                    for (var j = 0; j < $scope.vm.membersearchresult.users.length; j++) {
                        user = $scope.vm.membersearchresult.users[j];
                        if (user.markedForAdd) {
                            for (var i = 0; i < group.members.users.length; i++) {
                                if (group.members.users[i].name == user.name &&
                                    group.members.users[i].domain == user.domain) {
                                    if (group.members.users[i].state == 2) {
                                        group.members.users[i].state = 0;
                                    }
                                    dup = true;
                                    break;
                                }
                            }
                            if (!dup && user) {
                                user.state = 1;
                                group.members.users.push(user);
                                user.markedForAdd = false;
                            }
                        }
                    }
                }
            }

            function addGroupMembers(){
                var group = $scope.currentgroup;

                if(group.members == null)
                    group.members = {};
                if(group.members.groups == null)
                    group.members.groups = [];

                /* check duplicate */
                var dup = false;
                var member = null;

                if($scope.vm.membersearchresult.groups) {
                    for (var j = 0; j < $scope.vm.membersearchresult.groups.length; j++) {
                        member = $scope.vm.membersearchresult.groups[j];
                        if (member.markedForAdd) {
                            for (var i = 0; i < group.members.groups.length; i++) {
                                if (group.members.groups[i].name == member.name &&
                                    group.members.groups[i].domain == member.domain) {
                                    if (group.members.groups[i].state == 2) {
                                        group.members.groups[i].state = 0;
                                    }
                                    dup = true;
                                    break;
                                }
                            }
                            if (!dup && member) {
                                member.state = 1;
                                group.members.groups.push(member);
                            }
                        }
                    }
                }
            }


            function deleteSelectedMembers(){

                console.log('deleteSelectedMembers');
                deleteUserMembers();
                deleteGroupMembers();
            }

            function deleteUserMembers() {

                var group = $scope.currentgroup;
                if(group.members == null || group.members.users == null)
                    return;

                console.log('deleteUserMembers: ALL USERS: ' + JSON.stringify(group.members.users));

                /* check duplicate */
                var found = false;
                for(var i=group.members.users.length-1; i>= 0 ; i--) {
                    if(group.members.users[i].markedForDelete)
                    {
                        console.log('markedForDelete: USER: ' + JSON.stringify(group.members.users[i]));
                        found = true;

                        if(group.members.users[i].state == 1) {
                            group.members.users.splice(i, 1);
                        }
                        else
                        {
                            group.members.users[i].state = 2;
                        }
                    }
                }
            }

            function deleteGroupMembers(){

                var group = $scope.currentgroup;

                if(group.members == null || group.members.groups == null)
                    return;

                /* check duplicate */
                var found = false;
                for(var i=group.members.groups.length-1; i>= 0 ; i--) {
                    if(group.members.groups[i].markedForDelete)
                    {
                        found = true;

                        if(group.members.groups[i].state == 1) {
                            group.members.groups.splice(i, 1);
                        }
                        else
                        {
                            group.members.groups[i].state = 2;
                        }
                    }
                }
            }
        }]);