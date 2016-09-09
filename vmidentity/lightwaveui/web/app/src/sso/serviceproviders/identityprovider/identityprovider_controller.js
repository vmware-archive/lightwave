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
module.controller('IdentityProviderCntrl', [ '$scope', '$rootScope', 'Util', 'popupUtil', 'IdentityProviderService',
        function($scope, $rootScope, Util, popupUtil, IdentityProviderService) {

                $scope.vm.viewCertificate = viewCertificate;
                $scope.vm.setCertificate = setCertificate;
                $scope.vm.removeCertificate = removeCertificate;

                $scope.vm.addNameIdFormat = addNameIdFormat;
                $scope.vm.removeNameIdFormat = removeNameIdFormat;

                $scope.vm.addSubjectFormat = addSubjectFormat;
                $scope.vm.removeSubjectFormat = removeSubjectFormat;

                $scope.vm.addSso = addSso;
                $scope.vm.removeSso = removeSso;

                $scope.vm.addSlo = addSlo;
                $scope.vm.removeSlo = removeSlo;

                $scope.vm.addIdentityProvider = addIdentityProvider;

                init();

                function init(){
                        $rootScope.globals.errors = '';
                        $rootScope.globals.popup_errors = null;
                }


                function viewCertificate(certificate){
                        if(certificate) {
                                var template = 'shared/components/certificate/certificate.view.html';
                                var controller = 'CertificateViewerCntrl';
                                Util.viewCertificate($scope, certificate.encoded, template, controller);
                        }
                }

                function setCertificate(chain, contents){

                        if(!chain) {
                                chain = [];
                        }
                        var metadata = Util.getCertificateDetails(contents);
                        var certificate = {
                                encoded: contents,
                                metadata: metadata
                        };
                        chain.push(certificate);
                }

                function removeCertificate(chain, certificate){

                        for (var i = 0; i < chain.length; i++) {
                                if (chain[i].encoded == certificate.encoded) {
                                        chain.splice(i,1);
                                        break;
                                }
                        }
                }

                function addNameIdFormat(format){

                        if(format){
                                vm.selectedIdentityProvider.nameIDFormats.push(format);
                        }
                }

                function removeNameIdFormat(format){

                        if(format){

                                for(var i=0;i<vm.selectedIdentityProvider.nameIDFormats.length; i++) {
                                        if(vm.selectedIdentityProvider.nameIDFormats[i] == format){
                                                vm.selectedIdentityProvider.nameIDFormats.splice(i, 1);
                                        }
                                }
                        }
                }

                function addSubjectFormat(format){

                        if(format) {
                                $rootScope.globals.popup_errors = null;
                                $scope.vm.selectedSubjectFormat = format;
                                var template = 'sso/serviceproviders/identityprovider/subjectFormat.add.html';
                                var controller = 'IdentityProviderCntrl';
                                popupUtil.open($scope, template, controller);
                        }
                }

                function removeSubjectFormat(format){

                        if(format) {

                                for(var i=0;i<vm.selectedIdentityProvider.subjectFormats.length; i++) {
                                        if(vm.selectedIdentityProvider.subjectFormats[i] == format){
                                                vm.selectedIdentityProvider.subjectFormats.splice(i, 1);
                                        }
                                }
                        }
                }

                function addSso(sso){

                        if(sso) {
                                $rootScope.globals.popup_errors = null;
                                $scope.vm.ssoServices = sso;
                                var template = 'sso/serviceproviders/identityprovider/ssoService.add.html';
                                var controller = 'IdentityProviderCntrl';
                                popupUtil.open($scope, template, controller);
                        }
                }

                function removeSso(sso){

                        if(sso) {

                                for(var i=0;i<vm.selectedIdentityProvider.ssoServices.length; i++) {
                                        if(vm.selectedIdentityProvider.ssoServices[i].name == sso.name){
                                                vm.selectedIdentityProvider.ssoServices.splice(i, 1);
                                        }
                                }
                        }
                }

                function addSlo(slo){

                        if(slo) {
                                $rootScope.globals.popup_errors = null;
                                $scope.vm.sloServices = sso;
                                var template = 'sso/serviceproviders/identityprovider/sloService.add.html';
                                var controller = 'IdentityProviderCntrl';
                                popupUtil.open($scope, template, controller);
                        }
                }

                function removeSlo(slo){

                        if(slo) {

                                for(var i=0;i<vm.selectedIdentityProvider.sloServices.length; i++) {
                                        if(vm.selectedIdentityProvider.sloServices[i].name == slo.name){
                                                vm.selectedIdentityProvider.sloServices.splice(i, 1);
                                        }
                                }
                        }
                }

                function addIdentityProvider(idp){

                        $rootScope.globals.errors = '';
                        IdentityProviderService
                            .Create($rootScope.globals.currentUser, idp)
                            .then(function (res) {
                                    if (res.status == 200) {
                                            $rootScope.globals.errors = {details: 'Identity Provider updated successfully', success:true};
                                            $scope.selectedOIDCClient = null;
                                            $scope.getalloidcclient();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.popup_errors = res.data;
                                    }
                            });
                }
        }]);