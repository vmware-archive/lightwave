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
module.controller('RelyingPartyCntrl', [ '$scope', '$rootScope', 'Util',
        function($scope, $rootScope, Util) {

            $scope.relyingpartyvm = {};
            $scope.relyingpartyvm.viewcertificate = viewcertificate;

            init();

            function init(){
                $rootScope.globals.errors = '';
                $rootScope.globals.popup_errors = null;
            }


            function viewcertificate(){
                console.log('view certificate');

                if($scope.vm.selectedRelyingParty.certificate) {
                    var template = 'shared/components/certificate/certificate.view.html';
                    var controller = 'CertificateViewerCntrl';
                    Util.viewCertificate($scope, $scope.vm.selectedRelyingParty.certificate.encoded, template, controller);
                }
            }
        }]);