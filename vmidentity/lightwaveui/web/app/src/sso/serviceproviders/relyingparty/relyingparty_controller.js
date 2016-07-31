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
module.controller('RelyingPartyCntrl', [ '$scope', '$rootScope', 'Util', 'popupUtil',
        function($scope, $rootScope, Util, popupUtil) {

            $scope.relyingpartyvm = {};
            $scope.relyingpartyvm.viewcertificate = viewcertificate;
            $scope.relyingpartyvm.viewSlo = viewSlo;
            $scope.relyingpartyvm.viewAttributeConsumerServices = viewAttributeConsumerServices;
            $scope.relyingpartyvm.viewAssertionConsumerServices = viewAssertionConsumerServices;

            init();

            function init(){
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
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
                    var template = 'sso/serviceproviders/relyingparty/singleLogoutService.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
            function viewAttributeConsumerServices(acs) {
                $rootScope.globals.popup_errors = null;
                if(acs) {
                    $scope.vm.selectedAttributeConsumerService = acs;
                    var template = 'sso/serviceproviders/relyingparty/attributeConsumerService.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }
            function viewAssertionConsumerServices(acs) {
                $rootScope.globals.popup_errors = null;
                if(acs) {
                    $scope.vm.selectedAssertionConsumerService = acs;
                    var template = 'sso/serviceproviders/relyingparty/assertConsumerService.view.html';
                    var controller = 'RelyingPartyCntrl';
                    popupUtil.open($scope, template, controller);
                }
            }

        }]);