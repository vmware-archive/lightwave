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
module.controller('IdentitySourceCntrl', ['$scope',  '$rootScope', 'popupUtil', 'IdentitySourceService', 'TenantService', 'Util',
        function($scope, $rootScope, popupUtil, IdentitySourceService, TenantService, Util) {

                $scope.vm.viewCertificate = viewCertificate;
                $scope.vm.isSecure = isSecure;
                $scope.vm.isNewSecure = isNewSecure;
                $scope.vm.showBasicTab = showBasicTab;
                $scope.vm.showCertificateTab = showCertificateTab;
                $scope.vm.showCredentialsTab = showCredentialsTab;
                $scope.vm.showSummaryTab = showSummaryTab;
                $scope.vm.addCertificate = addCertificate;
                $scope.vm.addNewCertificate = addNewCertificate;
                $scope.vm.updateIdentitySource = updateIdentitySource;
                $scope.vm.showPreviousTab = showPreviousTab;
                $scope.vm.showNextTab = showNextTab;
                $scope.vm.removeCertificate = removeCertificate;
                $scope.vm.testConnection = testConnection;
                $scope.vm.setIdentitySourceType = setIdentitySourceType;
                $scope.vm.addIdentitySource = addIdentitySource;

                init();

                function init() {
                        $scope.vm.idsTab = $scope.vm.isNew ? 0 : 1;
                        $scope.vm.testConnectionStatus = null;
                        $scope.vm.isTestingConnection = false;

                        if($scope.vm.newIdentitySource) {
                                $scope.vm.newIdentitySource.type = 'IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING';
                        }
                }

                function setIdentitySourceType(type){

                        if(type == 1){
                                $scope.vm.newIdentitySource.type = 'IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING';
                        }
                        else if(type == 2) {
                                $scope.vm.newIdentitySource.type = 'IDENTITY_STORE_TYPE_LDAP';
                        }
                }
                function viewCertificate(certificate) {

                        if (certificate) {
                                var template = 'shared/components/certificate/certificate.view.html';
                                var controller = 'CertificateViewerCntrl';
                                Util.viewCertificate($scope, certificate.encoded, template, controller);
                        }
                }

                function isSecure() {
                        return $scope.vm.selectedIdentitysource &&
                            $scope.vm.selectedIdentitysource.connectionStrings &&
                            (($scope.vm.selectedIdentitysource.connectionStrings.length > 0
                            && $scope.vm.selectedIdentitysource.connectionStrings[0].indexOf("ldaps") > -1) ||
                            ($scope.vm.selectedIdentitysource.connectionStrings.length > 1
                            && $scope.vm.selectedIdentitysource.connectionStrings[1].indexOf("ldaps") > -1));
                }

                function isNewSecure() {
                        return $scope.vm.newIdentitySource &&
                            $scope.vm.newIdentitySource.connectionStrings &&
                            (($scope.vm.newIdentitySource.connectionStrings.length > 0
                            && $scope.vm.newIdentitySource.connectionStrings[0].indexOf("ldaps") > -1) ||
                            ($scope.vm.newIdentitySource.connectionStrings.length > 1
                            && $scope.vm.newIdentitySource.connectionStrings[1].indexOf("ldaps") > -1));
                }

                function showBasicTab() {
                        $scope.vm.idsTab = 1;
                }

                function showCertificateTab() {
                        $scope.vm.idsTab = 2;
                }

                function showCredentialsTab() {
                        $scope.vm.idsTab = 3;
                }

                function showSummaryTab() {
                        $scope.vm.idsTab = 4;
                }

                function showIdentityChoice(){
                        $scope.vm.idsTab = 0;
                }
                function addCertificate(contents) {
                        console.log('add certificate');
                        if (!$scope.vm.selectedIdentitysource.certificatescertificates) {
                                $scope.vm.selectedIdentitysource.certificates = [];
                        }

                        var metadata = Util.getCertificateDetails(contents);

                        console.log('Metadata: ' + JSON.stringify(metadata));
                        var certificate = {
                                encoded: contents,
                                metadata: metadata
                        };
                        $scope.vm.selectedIdentitysource.certificates.push(certificate);
                }

                function addNewCertificate(contents) {
                        console.log('add certificate');
                        if (!$scope.vm.newIdentitySource.certificates) {
                                $scope.vm.newIdentitySource.certificates = [];
                        }

                        var metadata = Util.getCertificateDetails(contents);

                        console.log('Metadata: ' + JSON.stringify(metadata));
                        var certificate = {
                                encoded: contents,
                                metadata: metadata
                        };
                        $scope.vm.newIdentitySource.certificates.push(certificate);
                }

                function updateIdentitySource(identitySource) {
                        $scope.vm.isSaving = true;
                        $rootScope.globals.errors = null;
                            IdentitySourceService
                            .Update($rootScope.globals.currentUser, identitySource)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $scope.vm.getIdentitySources();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.errors = res.data;
                                    }
                                    $scope.vm.isSaving = false;
                            });
                }

                function addIdentitySource(identitySource) {
                        $scope.vm.isSaving = true;
                        $rootScope.globals.errors = null;
                        IdentitySourceService
                            .Add($rootScope.globals.currentUser, identitySource)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $scpe.vm.getIdentitySources();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.errors = res.data;
                                    }
                                    $scope.vm.isSaving = false;
                            });
                }

                function showNextTab(){
                        $scope.vm.testConnectionStatus = null;
                        if($scope.vm.idsTab == 1) {
                                if($scope.vm.isSecure() || $scope.vm.isNewSecure())
                                        $scope.vm.idsTab += 1;
                                else {
                                        if($scope.vm.selectedIdentitysource)
                                                $scope.vm.selectedIdentitysource.certificates = [];
                                        if($scope.vm.selectedIdentitysource)
                                                $scope.vm.newIdentitySource.certificates = [];
                                        $scope.vm.idsTab += 2;
                                }
                        }
                        else
                        {
                                $scope.vm.idsTab += 1;
                        }
                        showRightPane();
                }

                function showPreviousTab(){
                        $scope.vm.testConnectionStatus = null;
                        if($scope.vm.idsTab == 3){
                                if($scope.vm.isSecure() || $scope.vm.isSecure())
                                        $scope.vm.idsTab -= 1;
                                else
                                        $scope.vm.idsTab -= 2;
                        }
                        else
                        {
                                $scope.vm.idsTab -= 1;
                        }
                        showRightPane();
                }

                function showRightPane(){

                        if($scope.vm.idsTab == 1){
                                $scope.vm.showBasicTab();
                        } else if($scope.vm.idsTab == 2){
                                $scope.vm.showCertificateTab();
                        } else if($scope.vm.idsTab == 3){
                                $scope.vm.showCredentialsTab();
                        } else if($scope.vm.idsTab == 4){
                                $scope.vm.showSummaryTab();
                        } else if($scope.vm.idsTab == 0){
                                $scope.vm.showIdentityChoice();
                        }
                }

                function removeCertificate(certificates, certificate){

                        for(var i = 0; i < certificates.length; i++){
                                if(certificates[i].encoded == certificate.encoded){
                                        certificates.splice(i,1);
                                        break;
                                }
                        }
                }

                function testConnection(identitySource){
                        $scope.vm.isTestingConnection = true;
                        $scope.vm.testConnectionStatus = null;
                        identitySource.authenticationType = "PASSWORD";
                        IdentitySourceService
                            .TestConnectivity($rootScope.globals.currentUser, identitySource)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $scope.vm.testConnectionStatus =
                                            {
                                                    status: "Test Connection successful.",
                                                    success: true,
                                                    css: "success-display"
                                            };
                                    }
                                    else {
                                            var details = res.data.details == undefined ? '' : res.data.details;
                                            var cause = res.data.cause == undefined ? '' : res.data.cause;
                                            $scope.vm.testConnectionStatus =
                                            {
                                                    status: "Test Connection failed. - " + details + ' ' + cause,
                                                    success: false,
                                                    css: "error-display"
                                            };
                                    }
                                    $scope.vm.isTestingConnection = false;
                            });

                }
        }]);