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

            $scope.vm.viewCertificate = viewCertificate;
            $scope.vm.setAddCertificate = setAddCertificate;
            $scope.vm.setEditCertificate = setEditCertificate;
            $scope.vm.removeCertificate = removeCertificate;

            $scope.vm.addSignatureAlgorithm = addSignatureAlgorithm;
            $scope.vm.editSignatureAlgorithm = editSignatureAlgorithm;
            $scope.vm.removeSignatureAlgorithm = removeSignatureAlgorithm;

            $scope.vm.viewSlo = viewSlo;
            $scope.vm.addSlo = addSlo;
            $scope.vm.editSlo = editSlo;
            $scope.vm.removeSlo = removeSlo;

            $scope.vm.viewAttributeConsumerServices = viewAttributeConsumerServices;
            $scope.vm.addAttributeConsumerService = addAttributeConsumerService;
            $scope.vm.editAttributeConsumerService = editAttributeConsumerService;
            $scope.vm.removeAttributeConsumerService = removeAttributeConsumerService;

            $scope.vm.viewAssertionConsumerServices = viewAssertionConsumerServices;
            $scope.vm.addAssertionConsumerService = addAssertionConsumerService;
            $scope.vm.editAssertionConsumerService = editAssertionConsumerService;
            $scope.vm.removeAssertionConsumerService = removeAssertionConsumerService;

            $scope.vm.saveRelyingParty = saveRelyingParty;
            $scope.vm.updateRelyingParty = updateRelyingParty;
            $scope.vm.isValidRelyingParty = isValidRelyingParty;

            init();

            function init(){
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
                $scope.vm.newRelyingParty = {};

                if($scope.vm.selectedRelyingParty && $scope.vm.selectedRelyingParty.certificate)
                $scope.vm.selectedRelyingParty.certificate.isValid = true;
            }

            function setAddCertificate(rp, contents){

                var metadata = Util.getCertificateDetails(contents);
                $scope.vm.newRelyingParty.certificate = {
                    encoded: contents,
                    metadata: metadata
                };

                if($scope.vm.newRelyingParty.certificate.metadata.subject != "DC=" &&
                    $scope.vm.newRelyingParty.certificate.metadata.subject.indexOf('undefined') == -1)
                {
                    $scope.vm.newRelyingParty.certificate.isValid = true;
                }
                else {
                    $scope.vm.newRelyingParty.certificate.isValid = false;
                }
            }

            function setEditCertificate(rp, contents){

                var metadata = Util.getCertificateDetails(contents);
                $scope.vm.selectedRelyingParty.certificate = {
                    encoded: contents,
                    metadata: metadata
                };
                if($scope.vm.selectedRelyingParty.certificate.metadata.subject != "DC=" &&
                    $scope.vm.selectedRelyingParty.certificate.metadata.subject.indexOf('undefined') == -1)
                {
                    $scope.vm.selectedRelyingParty.certificate.isValid = true;
                }
                else {
                    $scope.vm.selectedRelyingParty.certificate.isValid = false;
                }
            }

            function removeCertificate(rp) {

                rp.certificate = null
            }

            function addSignatureAlgorithm() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = true;
                var template = 'sso/serviceproviders/relyingparty/signatureAlgorithm/signatureAlgorithm.add.html';
                var controller = 'SignatureAlgorithmCntrl';
                popupUtil.open($scope, template, controller);
            }

            function addSlo() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = true;
                var template = 'sso/serviceproviders/relyingparty/singleLogoutService/singleLogoutService.add.html';
                var controller = 'SingleLogoutCntrl';
                popupUtil.open($scope, template, controller);
            }

            function addAttributeConsumerService() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = true;
                var template = 'sso/serviceproviders/relyingparty/attributeConsumerService/attributeConsumerService.add.html';
                var controller = 'AttributeConsumerServiceCntrl';
                popupUtil.open($scope, template, controller);
            }

            function addAssertionConsumerService() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = true;
                var template = 'sso/serviceproviders/relyingparty/assertionConsumerService/assertConsumerService.add.html';
                var controller = 'AssertionConsumerServiceCntrl';
                popupUtil.open($scope, template, controller);
            }

            function editSignatureAlgorithm() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = false;
                var template = 'sso/serviceproviders/relyingparty/signatureAlgorithm/signatureAlgorithm.add.html';
                var controller = 'SignatureAlgorithmCntrl';
                popupUtil.open($scope, template, controller);
            }

            function editSlo() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = false;
                var template = 'sso/serviceproviders/relyingparty/singleLogoutService/singleLogoutService.add.html';
                var controller = 'SingleLogoutCntrl';
                popupUtil.open($scope, template, controller);
            }

            function editAttributeConsumerService() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = false;
                var template = 'sso/serviceproviders/relyingparty/attributeConsumerService/attributeConsumerService.add.html';
                var controller = 'AttributeConsumerServiceCntrl';
                popupUtil.open($scope, template, controller);
            }

            function editAssertionConsumerService() {
                $rootScope.globals.popup_errors = null;
                $scope.vm.isAddRelyingParty = false;
                var template = 'sso/serviceproviders/relyingparty/assertionConsumerService/assertConsumerService.add.html';
                var controller = 'AssertionConsumerServiceCntrl';
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

            function viewCertificate(rp){
                $rootScope.globals.popup_errors = null;
                if(rp.certificate) {
                    var template = 'shared/components/certificate/certificate.view.html';
                    var controller = 'CertificateViewerCntrl';
                    Util.viewCertificate($scope, rp.certificate.encoded, template, controller);
                }
            }

            function viewSlo(slo) {
                $rootScope.globals.popup_errors = null;
                if(slo) {
                    $scope.vm.selectedSlo = slo;
                    var template = 'sso/serviceproviders/relyingparty/singleLogoutService/singleLogoutService.view.html';
                    var controller = 'SingleLogoutCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
            function viewAttributeConsumerServices(acs) {
                $rootScope.globals.popup_errors = null;
                if(acs) {
                    $scope.vm.selectedAttributeConsumerService = acs;
                    var template = 'sso/serviceproviders/relyingparty/attributeConsumerService/attributeConsumerService.view.html';
                    var controller = 'AttributeConsumerServiceCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
            function viewAssertionConsumerServices(acs) {
                $rootScope.globals.popup_errors = null;
                if(acs) {
                    $scope.vm.selectedAssertionConsumerService = acs;
                    var template = 'sso/serviceproviders/relyingparty/assertionConsumerService/assertConsumerService.view.html';
                    var controller = 'AssertionConsumerServiceCntrl';
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
                            $scope.vm.getallrelyingparty();
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
                    .Create($rootScope.globals.currentUser, rp)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Relying Party ' + rp.name + ' added successfully', success:true};
                            $scope.newRelyingParty = {};
                            $scope.vm.getallrelyingparty();
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function isValidRelyingParty(rp) {

                return (rp &&
                    rp.name && rp.url && rp.certificate &&
                    rp.signatureAlgorithms && rp.signatureAlgorithms.length > 0 &&
                    rp.singleLogoutServices && rp.singleLogoutServices.length > 0 &&
                    rp.assertionConsumerServices && rp.assertionConsumerServices.length > 0 &&
                    rp.attributeConsumerServices && rp.attributeConsumerServices.length > 0 &&
                    rp.certificate.metadata.subject != "DC=" &&
                    rp.certificate.metadata.subject.indexOf('undefined') == -1);
            }

        }]);