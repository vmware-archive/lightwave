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
module.controller('AuthenticationPolicyCntrl', [ '$scope', '$rootScope', 'Util', 'TenantService',
        function($scope, $rootScope, Util, TenantService) {

            $scope.updateAuthenticationPolicy = updateAuthenticationPolicy;
            $scope.addPolicyOid = addPolicyOid;
            $scope.removePolicyOid = removePolicyOid;
            $scope.addCertificate = addCertificate;
            $scope.removeCertificate = removeCertificate;

            init();

            function init(){
                $rootScope.globals.errors = null;
                $rootScope.globals.popup_errors = null;
            }

            function addPolicyOid(){

                var certPolicyOIDs = $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.certcertPolicyOIDs;
                if(!certPolicyOIDs)
                {
                    certPolicyOIDs = [];
                }
                var policyOid = $scope.vm.PolicyOID;
                if(policyOid){
                    certPolicyOIDs.push(policyOid);
                }
                $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.certPolicyOIDs = certPolicyOIDs;
            }
            function removePolicyOid(policyOid) {
                var certPolicyOIDs = $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.certPolicyOIDs;
                if(certPolicyOIDs) {
                    for(var i=0; i < certPolicyOIDs.length;i++) {
                        if(certPolicyOIDs[i] == policyOid){
                            certPolicyOIDs.splice(i,1);
                            break;
                        }
                    }
                }
                $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.certPolicyOIDs = certPolicyOIDs;
            }

            function updateAuthenticationPolicy(authenticationPolicy) {
                $rootScope.globals.errors = null;
                var policy = {
                    authenticationPolicy: authenticationPolicy
                };

                TenantService
                    .UpdateConfiguration($rootScope.globals.currentUser, policy)
                    .then(function (res) {
                        if (res.status == 200) {
                            $rootScope.globals.errors = {details: 'Authentication policy updated successfully', success:true};
                            $scope.refresh();
                            $scope.closeThisDialog('save');
                        }
                        else {
                            $rootScope.globals.popup_errors = res.data;
                        }
                    });
            }

            function addCertificate(certificates, contents) {

                if(!certificates) {
                    certificates = [];
                }

                var metadata = Util.getCertificateDetails(contents);
                var certificate = {
                    encoded : contents,
                    metadata : metadata
                };
                certificates.push(certificate);
                $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.trustedCACertificates = certificates;
            }

            function removeCertificate(certificates, certificate){

                if(certificates) {
                    for(var i=0; i<certificates.length;i++){
                        if(certificates[i].encoded == certificate.encoded) {
                            certificates.splice(i, 1);
                        }
                    }
                    $scope.vm.policies.authenticationPolicy.clientCertificatePolicy.trustedCACertificates = certificates;
                }
            }
        }]);