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
module.controller('SolutionUserCntrl', [ '$scope', '$rootScope', 'SolutionUserService', 'Util',
        function($scope, $rootScope, SolutionUserService, Util) {
            $scope.solnuservm = {};
            $scope.solnuservm.viewcertificate = viewcertificate;
            $scope.saveSolutionUser = saveSolutionUser;
            $scope.setCertificate = setCertificate;
            $scope.updateSolutionUser = updateSolutionUser;

            init();

            function init(){
                $rootScope.globals.errors = null;
                $rootScope.globals.popup_errors = null;
                $scope.newSolutionUser = {};
            }

            function viewcertificate(){
                console.log('view certificate');

                if($scope.vm.selectedSolutionUser.certificate) {
                    var template = 'shared/components/certificate/certificate.view.html';
                    var controller = 'CertificateViewerCntrl';
                    Util.viewCertificate($scope, $scope.vm.selectedSolutionUser.certificate.encoded, template, controller);
                }
            }

            function saveSolutionUser(solutionuser){
                $rootScope.globals.errors = '';
                var provider = $scope.vm.identitysource;
                solutionuser.upn = solutionuser.name + "@" + provider.name;
                solutionuser.alias = { name: solutionuser.name, domain: provider.name };
                solutionuser.domain = provider.name;
                console.log('inside saveSolutionUser: ' + JSON.stringify(solutionuser));
                SolutionUserService
                    .Add($rootScope.globals.currentUser, solutionuser)
                    .then(function (res) {
                            if (res.status == 200) {
                                $rootScope.globals.errors = {details: 'Solution User ' + solutionuser.upn + ' added successfully', success:true};
                                $scope.newsolutionuser = {};
                                $scope.getsolutionusers($scope.vm.identitysources, $scope.vm.solutionusersearch);
                                $scope.closeThisDialog('save');
                            }
                            else {
                                $rootScope.globals.popup_errors = res.data;
                            }

                    });
            }

            function updateSolutionUser(solutionUser){
                $rootScope.globals.errors = '';
                solutionUser.upn = solutionUser.name + "@" + solutionUser.domain;
                solutionUser.alias = {name: solutionUser.name, domain: solutionUser.domain};
                SolutionUserService
                    .Update($rootScope.globals.currentUser, solutionUser)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Solution User ' + solutionUser.upn + ' updated successfully', success:true};
                            $scope.getsolutionusers($scope.vm.identitysources, $scope.vm.solutionusersearch);
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function setCertificate(solutionuser, contents){
                $rootScope.globals.errors = '';
                if(!solutionuser.certificate)
                {
                    solutionuser.certificate = {};
                }
                solutionuser.certificate.encoded = contents;
            }
        }]);