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
module.controller('RelyingPartyCntrl', [ '$scope', '$rootScope', 'Util', 'popupUtil', 'RelyingPartyService',
        function($scope, $rootScope, Util, popupUtil, RelyingPartyService) {

            $scope.relyingpartyvm = {};
            $scope.relyingpartyvm.viewcertificate = viewcertificate;
            $scope.relyingpartyvm.setCertificate = setCertificate;
            $scope.relyingpartyvm.removeCertificate = removeCertificate;

            $scope.relyingpartyvm.addSignatureAlgorithm = addSignatureAlgorithm;
            $scope.relyingpartyvm.removeSignatureAlgorithm = removeSignatureAlgorithm;

            $scope.relyingpartyvm.viewSlo = viewSlo;
            $scope.relyingpartyvm.addSlo = addSlo;
            $scope.relyingpartyvm.removeSlo = removeSlo;

            $scope.relyingpartyvm.viewAttributeConsumerServices = viewAttributeConsumerServices;
            $scope.relyingpartyvm.addAttributeConsumerService = addAttributeConsumerService;
            $scope.relyingpartyvm.removeAttributeConsumerService = removeAttributeConsumerService;

            $scope.relyingpartyvm.viewAssertionConsumerServices = viewAssertionConsumerServices;
            $scope.relyingpartyvm.addAssertionConsumerService = addAssertionConsumerService;
            $scope.relyingpartyvm.removeAssertionConsumerService = removeAssertionConsumerService;

            $scope.relyingpartyvm.saveRelyingParty = saveRelyingParty;
            $scope.relyingpartyvm.updateRelyingParty = updateRelyingParty;

            init();

            function init(){
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
            }

            function setCertificate(rp, contents){

                var metadata = Util.getCertificateDetails(contents);
                rp.certificate = {
                    encoded: contents,
                    metadata: metadata
                };
            }

            function removeCertificate(rp) {

                rp.certificate = null
            }

            function addSlo() {
                $rootScope.globals.popup_errors = null;
                var template = 'sso/serviceproviders/relyingparty/singleLogoutService/singleLogoutService.add.html';
                var controller = 'RelyingPartyCntrl';
                popupUtil.open($scope, template, controller);
            }
            function addAttributeConsumerService() {
                $rootScope.globals.popup_errors = null;
                var template = 'sso/serviceproviders/relyingparty/attributeConsumerService/attributeConsumerService.add.html';
                var controller = 'RelyingPartyCntrl';
                popupUtil.open($scope, template, controller);
            }
            function addAssertionConsumerService() {
                $rootScope.globals.popup_errors = null;
                var template = 'sso/serviceproviders/relyingparty/assertionConsumerService/assertConsumerService.add.html';
                var controller = 'RelyingPartyCntrl';
                popupUtil.open($scope, template, controller);
            }

            function removeSlo(slos, slo){

                if(slos && slo) {
                    for (var i = 0; i < slos.length; i++) {

                        if (slos[i].name == slo.name) {
                            slos.splice(i, 1);
                            break;
                        }
                    }
                }
            }
            function removeAttributeConsumerService(slos, slo){

                if(slos && slo) {
                    for (var i = 0; i < slos.length; i++) {

                        if (slos[i].name == slo.name) {
                            slos.splice(i, 1);
                            break;
                        }
                    }
                }
            }
            function removeAssertionConsumerService(slos, slo){

                if(slos && slo) {
                    for (var i = 0; i < slos.length; i++) {

                        if (slos[i].name == slo.name) {
                            slos.splice(i, 1);
                            break;
                        }
                    }
                }
            }

            function addSignatureAlgorithm(signs){

                if(signs && sign) {
                    for (var i = 0; i < signs.length; i++) {

                        if (signs[i].priority == sign.priority &&
                            signs[i].minKeySize == sign.minKeySize &&
                            signs[i].maxKeySize == sign.maxKeySize) {
                            signs.splice(i, 1);
                            break;
                        }
                    }
                }
            }

            function removeSignatureAlgorithm(signs, sign){

                if(signs && sign) {
                    for (var i = 0; i < signs.length; i++) {

                        if (signs[i].priority == sign.priority &&
                            signs[i].minKeySize == sign.minKeySize &&
                            signs[i].maxKeySize == sign.maxKeySize) {
                            signs.splice(i, 1);
                            break;
                        }
                    }
                }
            }


            function viewcertificate(){
                $rootScope.globals.popup_errors = null;
                if($scope.vm.selectedRelyingParty.certificate) {
                    var template = 'shared/components/certificate/certificate.view.html';
                    var controller = 'CertificateViewerCntrl';
                    Util.viewCertificate($scope, $scope.vm.selectedRelyingParty.certificate.encoded, template, controller);
                }
            }

            function viewSlo(slo) {
                $rootScope.globals.popup_errors = null;
                if(slo) {
                    $scope.vm.selectedSlo = slo;
                    var template = 'sso/serviceproviders/relyingparty/singleLogoutService/singleLogoutService.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
            function viewAttributeConsumerServices(acs) {
                $rootScope.globals.popup_errors = null;
                if(acs) {
                    $scope.vm.selectedAttributeConsumerService = acs;
                    var template = 'sso/serviceproviders/relyingparty/attributeConsumerService/attributeConsumerService.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
            function viewAssertionConsumerServices(acs) {
                $rootScope.globals.popup_errors = null;
                if(acs) {
                    $scope.vm.selectedAssertionConsumerService = acs;
                    var template = 'sso/serviceproviders/relyingparty/assertionConsumerService/assertConsumerService.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }

            function updateRelyingParty(rp){
                $rootScope.globals.errors = '';
                RelyingPartyService
                    .Update($rootScope.globals.currentUser, rp)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Relying Party ' + rp.name + ' added successfully', success:true};
                            $scope.selectedRelyingParty = {};
                            $scope.getallrelyingparty();
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function saveRelyingParty(rp){
                $rootScope.globals.errors = '';
                RelyingPartyService
                    .Add($rootScope.globals.currentUser, oidc)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Relying Party ' + rp.name + ' added successfully', success:true};
                            $scope.newRelyingParty = {};
                            $scope.getallrelyingparty();
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

        }]);