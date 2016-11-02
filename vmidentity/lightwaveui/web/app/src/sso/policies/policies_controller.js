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
module.controller('PolicyCntrl', ['$scope',  '$rootScope', 'popupUtil', 'TenantService', 'Util',
    function($scope, $rootScope, popupUtil, TenantService, Util) {

        $scope.vm = this;
        $scope.refresh = refresh;
        $scope.getConfig = getConfig;
        $scope.showEditLockoutPolicy = showEditLockoutPolicy;
        $scope.showEditTokenPolicy = showEditTokenPolicy;
        $scope.showEditPasswordPolicy = showEditPasswordPolicy;
        $scope.showEditBrandPolicy = showEditBrandPolicy;
        $scope.showEditAuthenticationPolicy = showEditAuthenticationPolicy;
        $scope.showlockoutpolicy = showlockoutpolicy;
        $scope.showtokenpolicy = showtokenpolicy;
        $scope.showpasswordpolicy = showpasswordpolicy;
        $scope.viewcertificate = viewcertificate;

        refresh();

        function refresh() {
            if($rootScope.globals.currentUser.isSystemTenant) {
                getConfig();

            } else {
                getConfigPartial();
            }
            $scope.vm.policyTab = 1;
        }

        function showlockoutpolicy() {
            $scope.vm.policyTab = 1;
        }

        function showtokenpolicy() {
            $scope.vm.policyTab = 2;
        }

        function showpasswordpolicy() {
            $scope.vm.policyTab = 3;
        }

        function getConfig() {
            $rootScope.globals.errors = null;
            TenantService
                .GetConfiguration($rootScope.globals.currentUser)
                .then(function (res) {
                    if (res.status == 200) {
                        $scope.vm.policies = res.data;

                        if($scope.vm.policies.authenticationPolicy
                            && $scope.vm.policies.authenticationPolicy.clientCertificatePolicy
                            && $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.trustedCACertificates) {
                            var certs = $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.trustedCACertificates;
                            for (var i = 0; i < certs.length; i++) {
                                    certs[i].metadata = Util.getCertificateDetails(certs[i].encoded);
                            }
                        }
                    }
                    else {
                        $rootScope.globals.errors = res.data;
                    }
                });
        }

        function getConfigPartial() {
            $rootScope.globals.errors = null;
            var configs = ['LOCKOUT', 'PASSWORD', 'TOKEN', 'BRAND', 'AUTHENTICATION'];
            TenantService
                .GetConfiguration($rootScope.globals.currentUser, configs)
                .then(function (res) {
                    if (res.status == 200) {
                        $scope.vm.policies = res.data;

                        if($scope.vm.policies.authenticationPolicy
                            && $scope.vm.policies.authenticationPolicy.clientCertificatePolicy
                            && $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.trustedCACertificates) {
                            var certs = $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.trustedCACertificates;
                            for (var i = 0; i < certs.length; i++) {
                                certs[i].metadata = Util.getCertificateDetails(certs[i].encoded);
                            }
                        }
                    }
                    else {
                        $rootScope.globals.errors = res.data;
                    }
                });
        }

        function showEditLockoutPolicy() {
            var template = 'sso/policies/lockout/lockout.edit.html';
            var controller = 'LockoutPolicyCntrl';
            popupUtil.open($scope, template, controller);
        }

        function showEditTokenPolicy() {
            var template = 'sso/policies/token/token.edit.html';
            var controller = 'TokenPolicyCntrl';
            popupUtil.open($scope, template, controller);
        }

        function showEditPasswordPolicy() {
            var template = 'sso/policies/password/password.edit.html';
            var controller = 'PasswordPolicyCntrl';
            popupUtil.open($scope, template, controller);
        }
        function showEditBrandPolicy() {
            var template = 'sso/policies/banner/banner.edit.html';
            var controller = 'BannerPolicyCntrl';
            popupUtil.open($scope, template, controller);
        }
        function showEditAuthenticationPolicy() {
            var template = 'sso/policies/authentication/authentication.edit.html';
            var controller = 'AuthenticationPolicyCntrl';
            popupUtil.open($scope, template, controller);
        }

        function viewcertificate(certificate){

            if(certificate) {
                var template = 'shared/components/certificate/certificate.view.html';
                var controller = 'CertificateViewerCntrl';
                Util.viewCertificate($scope, certificate.encoded, template, controller);
            }
        }
    }]);