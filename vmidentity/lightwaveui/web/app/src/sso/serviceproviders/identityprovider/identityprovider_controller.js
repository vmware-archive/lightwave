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
                $scope.vm.setNewCertificate = setNewCertificate;
                $scope.vm.setEditCertificate = setEditCertificate;
                $scope.vm.removeCertificate = removeCertificate;

                $scope.vm.addNameIdFormat = addNameIdFormat;
                $scope.vm.updateNameIdFormat = updateNameIdFormat;
                $scope.vm.removeNameIdFormat = removeNameIdFormat;

                $scope.vm.addSubjectFormat = addSubjectFormat;
                $scope.vm.removeSubjectFormat = removeSubjectFormat;

                $scope.vm.addSso = addSso;
                $scope.vm.removeSso = removeSso;

                $scope.vm.addSlo = addSlo;
                $scope.vm.removeSlo = removeSlo;

                $scope.vm.addIdentityProvider = addIdentityProvider;
                $scope.vm.isValidIdentityProvider = isValidIdentityProvider;

                init();

                function init(){
                        $rootScope.globals.errors = '';
                        $rootScope.globals.popup_errors = null;

                        if($scope.vm.addNewIdentityProvider) {
                                $scope.vm.newIdentityProvider = {};
                        }
                }


                function viewCertificate(certificate){
                        if(certificate) {
                                var template = 'shared/components/certificate/certificate.view.html';
                                var controller = 'CertificateViewerCntrl';
                                Util.viewCertificate($scope, certificate.encoded, template, controller);
                        }
                }

                function setNewCertificate(chain, contents){

                        if(!$scope.vm.newIdentityProvider.signingCertificates) {
                                $scope.vm.newIdentityProvider.signingCertificates = { certificates : []};
                        }

                        if(!$scope.vm.newIdentityProvider.signingCertificates.certificates) {
                                $scope.vm.newIdentityProvider.signingCertificates.certificates = [];
                        }
                        var metadata = Util.getCertificateDetails(contents);
                        var certificate = {
                                encoded: contents,
                                metadata: metadata
                        };
                        $scope.vm.newIdentityProvider.signingCertificates.certificates.push(certificate);
                }

                function setEditCertificate(chain, contents){

                        if(!$scope.vm.selectedIdentityProvider.signingCertificates) {
                                $scope.vm.selectedIdentityProvider.signingCertificates = { certificates : []};
                        }

                        if(!$scope.vm.selectedIdentityProvider.signingCertificates.certificates) {
                                $scope.vm.selectedIdentityProvider.signingCertificates.certificates = [];
                        }
                        var metadata = Util.getCertificateDetails(contents);
                        var certificate = {
                                encoded: contents,
                                metadata: metadata
                        };
                        $scope.vm.selectedIdentityProvider.signingCertificates.certificates.push(certificate);
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

                                if(!$scope.vm.newIdentityProvider.nameIDFormats){
                                        $scope.vm.newIdentityProvider.nameIDFormats = [];
                                }
                                $scope.vm.newIdentityProvider.nameIDFormats.push(format);
                                $scope.vm.nameIDFormat = '';
                        }
                }

                function updateNameIdFormat(format){
                        if(format){
                                if(!$scope.vm.selectedIdentityProvider.nameIDFormats){
                                        $scope.vm.selectedIdentityProvider.nameIDFormats = [];
                                }
                                $scope.vm.selectedIdentityProvider.nameIDFormats.push(format);
                                $scope.vm.selectedNameIDFormat = '';
                        }
                }

                function removeNameIdFormat(formats, format){

                        if(formats && format){

                                for(var i=0;i<formats.length; i++) {
                                        if(formats[i] == format){
                                                formats.splice(i, 1);
                                        }
                                }
                        }
                }

                function addSubjectFormat() {
                        $rootScope.globals.popup_errors = null;
                        var template = 'sso/serviceproviders/identityprovider/subjectFormat/subjectFormat.add.html';
                        var controller = 'SubjectFormatCntrl';
                        popupUtil.open($scope, template, controller);
                }

                function removeSubjectFormat(formats, key){

                        if(formats && key) {
                                delete formats[key];
                        }
                }

                function addSso(){
                        $rootScope.globals.errors = '';
                        $rootScope.globals.popup_errors = null;
                        var template = 'sso/serviceproviders/identityprovider/ssoService/ssoService.add.html';
                        var controller = 'SsoServiceCntrl';
                        popupUtil.open($scope, template, controller);
                }

                function removeSso(ssos, sso){

                        if(ssos && sso) {

                                for(var i=0;i<ssos.length; i++) {
                                        if(ssos[i].name == sso.name){
                                                ssos.splice(i, 1);
                                        }
                                }
                        }
                }

                function addSlo(){
                        $rootScope.globals.popup_errors = null;
                        var template = 'sso/serviceproviders/identityprovider/sloService/sloService.add.html';
                        var controller = 'SloServiceCntrl';
                        popupUtil.open($scope, template, controller);
                }

                function removeSlo(slos, slo){

                        if(slos && slo) {

                                for(var i=0;i<slos.length; i++) {
                                        if(slos[i].name == slo.name){
                                                slos.splice(i, 1);
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
                                            $scope.vm.selectedIdentityProvider = null;
                                            $scope.vm.newIdentityProvider = null;
                                            $scope.vm.getallidentityprovider();
                                            $scope.closeThisDialog('save');
                                    }
                                    else {
                                            $rootScope.globals.popup_errors = res.data;
                                    }
                            });
                }

                function isValidIdentityProvider(idp){
                        return (idp &&
                                idp.entityID && idp.subjectFormats && idp.alias &&
                                idp.nameIDFormats && idp.nameIDFormats.length > 0 &&
                                idp.signingCertificates && idp.signingCertificates.certificates && idp.signingCertificates.certificates.length > 0 &&
                                idp.sloServices && idp.sloServices.length > 0 &&
                                idp.ssoServices && idp.ssoServices.length > 0);
                }
        }]);