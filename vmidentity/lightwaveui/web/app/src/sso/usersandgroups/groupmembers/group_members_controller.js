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
            $scope.vm.canDeleteSelectedMembers = canDeleteSelectedMembers;
            $scope.vm.getAllMembers = getAllMembers;
            $scope.vm.clearMemberSearch = clearMemberSearch;
            $scope.vm.searchMembers = searchMembers;
            $scope.vm.addMembersToGroup = addMembersToGroup;
            $scope.vm.searchSelectedMembers = searchSelectedMembers;
            $scope.vm.disable = disable;

            init();

            function init() {
                $rootScope.globals.errors = null;
                $rootScope.globals.popup_errors = null;
                var group = $scope.vm.selectedGroup;
                $scope.vm.selectedmembersearch = '';
                getMembers(group, '');

                if ($scope.vm.identitysource) {
                    $scope.vm.idsmembersearch = $scope.vm.identitysource;
                    getAllMembers($scope.vm.idsmembersearch);
                }
                clearMemberSearch();
            }

            function searchSelectedMembers() {
                $rootScope.globals.popup_errors = null;
                var group = $scope.vm.selectedGroup;
                var text = $scope.vm.selectedmembersearch;
                getMembers(group, text);
            }

            function getMembers(group, searchText) {
                $rootScope.globals.popup_errors = null;
                $scope.vm.searchSelectedMembersLoading = true;
                $scope.currentgroup = {};
                var upn = group.name + "@" + group.domain;
                GroupService
                    .Get($rootScope.globals.currentUser, upn)
                    .then(function (res) {
                        if (res.status == 200) {
                            $scope.currentgroup = res.data;
                            $scope.currentgroup.members = {};
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
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
                                        $rootScope.globals.popup_errors = res.data;
                                    }
                                });
                        }
                    })
                    .then(function (res) {
                        if ($scope.currentgroup) {
                            GroupService
                                .GetMembership($rootScope.globals.currentUser, upn, "SOLUTIONUSER")
                                .then(function (res) {
                                    if (res.status == 200) {
                                        $scope.currentgroup.members.solutionusers = [];
                                        if (res.data.solutionUsers) {
                                            for (var i = 0; i < res.data.solutionUsers.length; i++) {
                                                if (!searchText || searchText == '' || res.data.solutionUsers[i].name.indexOf(searchText) > -1) {
                                                    $scope.currentgroup.members.solutionusers.push(res.data.solutionUsers[i]);
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        $rootScope.globals.popup_errors = res.data;
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
                                        $rootScope.globals.popup_errors = res.data;
                                    }
                                    $scope.vm.searchSelectedMembersLoading = false;
                                });
                        }
                    })
            }

            function disable() {

                var group = $scope.currentgroup;
                if (group.members) {
                    if (group.members.groups) {
                        for (var j = group.members.groups.length - 1; j >= 0; j--) {
                            if (group.members.groups[j].state == 2 ||
                                group.members.groups[j].state == 1) {
                                return false;
                            }
                        }
                    }

                    if (group.members.users) {
                        for (var i = group.members.users.length - 1; i >= 0; i--) {
                            if (group.members.users[i].state == 2 ||
                                group.members.users[i].state == 1) {
                                return false;
                            }
                        }
                    }
                }

                return true;
            }

            function saveMembers() {

                $rootScope.globals.popup_errors = null;
                var group = $scope.currentgroup;
                var upn = group.name + '%40' + group.domain;

                if (group.members) {

                    $scope.vm.isSaving = true;

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
                                        $rootScope.globals.popup_errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }

                        if (deletedgroups != '') {
                            GroupService
                                .DeleteMembers($rootScope.globals.currentUser, upn, deletedgroups, 'GROUP')
                                .then(function (res) {
                                    if (!res.status == 200) {
                                        $rootScope.globals.popup_errors += res.data;
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
                                        $rootScope.globals.popup_errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }
                        if (deletedusers != '') {
                            GroupService
                                .DeleteMembers($rootScope.globals.currentUser, upn, deletedusers, 'USER')
                                .then(function (res) {
                                    if (!res.status == 200) {
                                        $rootScope.globals.popup_errors += res.data;
                                    }
                                    $scope.vm.isSaving = false;
                                });
                        }
                    }
                }
                else {
                    $scope.vm.isSaving = false;
                }
            }

            function getAllMembers(provider) {
                $scope.membersearchtype = $scope.membersearchtypes[2];
                searchMembers('', provider, $scope.membersearchtype);
            }

            function clearMemberSearch() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.membersearch = '';
                $scope.vm.membersearchresult = {};
            }

            function searchMembers(searchvalue, provider, type) {

                if (provider && type) {
                    $rootScope.globals.popup_errors = null;
                    var typename = type.name;
                    $scope.vm.membersearchresult = {};
                    $scope.vm.searchMembersLoading = true;
                    if (type.name == "user") {
                        MemberService
                            .Search($rootScope.globals.currentUser, provider.name, "USER", "NAME", searchvalue)
                            .then(function (res) {
                                if (res.status == 200) {
                                    $scope.vm.membersearchresult.users = res.data.users;
                                    $scope.vm.searchMembersLoading = false;
                                }
                                else {
                                    $rootScope.globals.popup_errors = res.data;
                                }
                            });

                    } else if (type.name == "group") {
                        MemberService
                            .Search($rootScope.globals.currentUser, provider.name, "GROUP", "NAME", searchvalue)
                            .then(function (res) {
                                if (res.status == 200) {
                                    $scope.vm.membersearchresult.groups = res.data.groups;
                                    $scope.vm.searchMembersLoading = false;
                                }
                                else {
                                    $rootScope.globals.popup_errors = res.data;
                                }
                            });
                    } else if (type.name == "all") {
                        MemberService
                            .Search($rootScope.globals.currentUser, provider.name, "ALL", "NAME", searchvalue)
                            .then(function (res) {
                                if (res.status == 200) {
                                    $scope.vm.membersearchresult.users = res.data.users;
                                    $scope.vm.membersearchresult.groups = res.data.groups;
                                    $scope.vm.searchMembersLoading = false;
                                }
                                else {
                                    $rootScope.globals.popup_errors = res.data;
                                }
                            })
                    }
                }
            }


            function addMembersToGroup() {
                $rootScope.globals.popup_errors = null;
                if ($scope.vm.membersearchresult) {
                    var dupUsers = addUserMembers();
                    var dupGroups = addGroupMembers();

                    var duplicate = dupUsers + dupGroups;
                    if(duplicate != ''){
                        $rootScope.globals.popup_errors = { details : "Duplicate: Member(s) already exist - " + duplicate };
                    }
                }
            }

            function addUserMembers() {
                var group = $scope.currentgroup;
                var duplicate = '';
                if (group.members == null)
                    group.members = {};
                if (group.members.users == null)
                    group.members.users = [];
                /* check duplicate */
                var dup = false;
                var user = null;

                if ($scope.vm.membersearchresult.users) {
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

                            if(dup)
                            {
                                duplicate = duplicate + " " + user.name + "@" + user.domain + " ";
                            }
                        }
                    }
                }
                return duplicate;
            }

            function addGroupMembers() {
                var group = $scope.currentgroup;
                var duplicate = '';
                if (group.members == null)
                    group.members = {};
                if (group.members.groups == null)
                    group.members.groups = [];

                /* check duplicate */
                var dup = false;
                var member = null;

                if ($scope.vm.membersearchresult.groups) {
                    for (var j = 0; j < $scope.vm.membersearchresult.groups.length; j++) {
                        member = $scope.vm.membersearchresult.groups[j];
                        if (member.markedForAdd) {

                            if (member.name != group.name || member.domain != group.domain) {

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
                                if(dup)
                                {
                                    duplicate = duplicate + " " + member.name + "@" + member.domain + " ";
                                }
                            }
                            else {
                                member.markedForAdd = false;
                                $rootScope.globals.popup_errors = {details: 'Cannot add a group as member to itself.'};
                            }
                        }
                    }
                }
                return duplicate;
            }

            function canDeleteSelectedMembers() {

                var group = $scope.currentgroup;
                if (group.members) {
                    if (group.members.users) {
                        for (var i = group.members.users.length - 1; i >= 0; i--) {
                            if (group.members.users[i].markedForDelete) {
                                return true;
                            }
                        }
                    }

                    if (group.members.groups) {
                        for (var i = group.members.groups.length - 1; i >= 0; i--) {
                            if (group.members.groups[i].markedForDelete) {
                                return true;
                            }
                        }
                    }
                }
                return false;
            }


            function deleteSelectedMembers() {

                $rootScope.globals.popup_errors = null;
                deleteUserMembers();
                deleteGroupMembers();
            }

            function deleteUserMembers() {

                var group = $scope.currentgroup;
                if (group.members == null || group.members.users == null)
                    return;
                /* check duplicate */
                var found = false;
                for (var i = group.members.users.length - 1; i >= 0; i--) {
                    if (group.members.users[i].markedForDelete) {
                        found = true;

                        if (group.members.users[i].state == 1) {
                            group.members.users.splice(i, 1);
                        }
                        else {
                            group.members.users[i].state = 2;
                        }
                    }
                }
            }

            function deleteGroupMembers() {

                var group = $scope.currentgroup;

                if (group.members == null || group.members.groups == null)
                    return;

                /* check duplicate */
                var found = false;
                for (var i = group.members.groups.length - 1; i >= 0; i--) {
                    if (group.members.groups[i].markedForDelete) {
                        found = true;

                        if (group.members.groups[i].state == 1) {
                            group.members.groups.splice(i, 1);
                        }
                        else {
                            group.members.groups[i].state = 2;
                        }
                    }
                }
            }
        }]);