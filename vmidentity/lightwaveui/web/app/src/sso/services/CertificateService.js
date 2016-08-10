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

var sso_service_module = angular.module('lightwave.ui.sso.services');
sso_service_module.factory('CertificateService', CertificateService);
CertificateService.$inject = ['Configuration', 'HttpService', 'HandleHttpResponse'];

function CertificateService(Configuration, HttpService, HandleHttpResponse) {

    var service = {};
    service.GetCertificateChain = GetCertificateChain;
    service.GetPrivateKey = GetPrivateKey;
    service.SetTenantCredentials = SetTenantCredentials;
    service.AddCertificate = AddCertificate;
    service.DeleteCertificateChain = DeleteCertificateChain;
    service.SetCertificateChain = SetCertificateChain;
    return service;

    function GetCertificateChain(context, username) {
        var endpoint = Configuration.getCertificatesEndpoint(context.server, context.tenant,'');
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function GetPrivateKey(context, username) {
        var endpoint = Configuration.getPrivateKeysEndpoint(context.server, context.tenant);
        return HttpService
            .getResponse(endpoint, 'GET', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure)
    }

    function AddCertificate(context, certificate, user) {
        var endpoint = Configuration.getCertificatesEndpoint(context.server, context.tenant, '');
        return HttpService
            .getResponse(endpoint, 'POST', context.token, user)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function SetTenantCredentials(context, tenantCredentials) {
        var endpoint = Configuration.getPrivateKeysEndpoint(context.server, context.tenant, '');
        return HttpService
            .getResponse(endpoint, 'PUT', context.token, tenantCredentials)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function SetCertificateChain(context, tenantCredentials) {
        var endpoint = Configuration.getPrivateKeysEndpoint(context.server, context.tenant, '');
        return HttpService
            .getResponse(endpoint, 'POST', context.token, tenantCredentials)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure);
    }

    function DeleteCertificateChain(context, fingerprint) {
        var endpoint = Configuration.getCertificatesEndpoint(context.server, context.tenant, fingerprint);
        return HttpService
            .getResponse(endpoint, 'DELETE', context.token)
            .then(HandleHttpResponse.Success, HandleHttpResponse.Failure)
    }
}