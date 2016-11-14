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
sso_service_module.factory('Configuration', Configuration);

function Configuration() {

    // Server Uri
    function getServerUri(server) {
        var port = server.port && server.port != '' ? ':' + server.port + '/' : '/';
        return server.protocol + '://' + server.host + port;
    }

    // Lightwave UI
    function getLightwaveUri(server) {
        return 'https://' + server + '/lightwaveui';
    }

    // Open Id Connect login endpoint
    function getLoginEndpoint(server, tenant) {
        var serverUri = getServerUri(server);
        return serverUri + 'openidconnect/token/' + tenant;
    }

    // IDM endpoint
    function getIdmEndpoint(server) {
        var serverUri = getServerUri(server);
        return serverUri + 'idm';
    }

    function getDirEndpoint(server) {
        var serverUri = getServerUri(server);
        return serverUri + 'vmdir';
    }

    // Tenants endpoint
    function getDirTenantEndpoint(server, tenant) {
        var idmEndpoint = getDirEndpoint(server);
        return idmEndpoint + '/tenant/' + tenant;
    }

    function getTenantsEndpoint(server) {
        var idmEndpoint = getIdmEndpoint(server);
        return idmEndpoint + '/tenant/';
    }

    // Tenant endpoint
    function getTenantEndpoint(server, tenant) {
        var idmEndpoint = getIdmEndpoint(server);
        return idmEndpoint + '/tenant/' + tenant;
    }

    // Tenant cleanup endpoint
    function getTenantCleanupEndpoint(server, tenant) {

        var lightwaveEndpoint = getLightwaveUri(server);
        return lightwaveEndpoint + '/CleanupTenant?tenant=' + tenant;
    }
    // Open Id Connect login arguments
    function getLoginArgument(username, password) {
        username = username.replace('@', '%40');
        return 'grant_type=password&username=' + username + '&password=' + password + '&scope=openid+offline_access+id_groups+at_groups+rs_admin_server';
    }

    // Tenant configuration endpoint
    function getConfigEndpoint(server, tenant, policies) {

        var type = '';
        if(policies && policies.length > 0)
        {
            type = '?';
            for(var i = 0; i < policies.length; i++) {
                type = type + 'type=' + policies[i] + '&';
            }
            type = type.substr(0, type.length - 1);
        }
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/config' + type;
    }

    // Tenant configuration endpoint
    function getDirConfigEndpoint(server, tenant, policies) {

        var type = '';
        if(policies && policies.length > 0)
        {
            type = '?';
            for(var i = 0; i < policies.length; i++) {
                type = type + 'type=' + policies[i] + '&';
            }
            type = type.substr(0, type.length - 1);
        }
        var tenantEndpoint = getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/config' + type;
    }

    // Member search endpoint
    function getMemberSearchEndpoint(server, tenant) {
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/search';
    }

    // User endpoint
    function getUserEndpoint(server, tenant, username) {
        var tenantEndpoint = getUsersEndpoint(server, tenant);
        return tenantEndpoint + '/' + username;
    }

    function getIdmUserEndpoint(server, tenant, username){
        var tenantEndpoint = getDirTenantEndpoint(server, tenant);
        var usersEndpoint = tenantEndpoint + '/users';
        var userEndpoint =usersEndpoint + "/" + username;
        return userEndpoint;
    }

    // User password endpoint
    function getUserPasswordEndpoint(server, tenant, username) {
        var userEndpoint = getIdmUserEndpoint(server, tenant, username);
        return userEndpoint + '/password';
    }

    // User groups
    function getUserGroupsEndpoint(server, tenant, username) {
        var userEndpoint = getIdmUserEndpoint(server, tenant, username);
        return userEndpoint + '/groups';
    }

    // Users all endpoint
    function getUsersEndpoint(server, tenant) {
        var tenantEndpoint = getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/users';
    }

    // Solution User endpoint
    function getSolutionUserEndpoint(server, tenant, username) {
        var tenantEndpoint = getSolutionUsersEndpoint(server, tenant);
        return  tenantEndpoint + '/' + username;
    }

    // Solution Users all endpoint
    function getSolutionUsersEndpoint(server, tenant) {
        var tenantEndpoint = getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/solutionusers';
    }

    // Group endpoint
    function getGroupEndpoint(server, tenant, name) {
        var tenantEndpoint = getGroupsEndpoint(server, tenant);
        return  tenantEndpoint + '/' + name;
    }

    // Groups endpoint
    function getGroupsEndpoint(server, tenant) {
        var tenantEndpoint = getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/groups';
    }

    function getIdmGroupEndpoint(server, tenant, name){
        var tenantEndpoint = getDirTenantEndpoint(server, tenant);
        var groupsEndpoint = tenantEndpoint + '/groups';
        return groupsEndpoint + "/" + name;
    }


    // Group membership endpoint
    function getGroupMembership(server, tenant, upn, members, type){
        var groupEndpoint = getIdmGroupEndpoint(server, tenant, upn);
        return groupEndpoint + '/members?' + members + 'type=' + type;
    }

    // Identity sources endpoint
    function getAllIdentitySourcesEndpoint(server, tenant) {
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/providers';
    }

    // Identity source endpoint
    function getIdentitySourceEndpoint(server, tenant, name) {
        var identitySourcesEndpoint = getAllIdentitySourcesEndpoint(server, tenant);
        return identitySourcesEndpoint + '/' + name;
    }

    // Server computers endpoint
    function getComputersEndpoint(server) {
        var idmEndpoint = getIdmEndpoint(server);
        return idmEndpoint + '/server/computers';
    }

    // Relying Party endpoint
    function getRelyingPartyEndpoint(server, tenant, name) {
        var groups = getRelyingPartiesEndpoint(server, tenant);
        return  groups + '/' + name;
    }

    // Relying Parties endpoint
    function getRelyingPartiesEndpoint(server, tenant) {
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/relyingparty';
    }

    // External Identity Provider endpoint
    function getIdentityProviderEndpoint(server, tenant, name) {
        var idpEndpoint = getIdentityProvidersEndpoint(server, tenant);
        return  idpEndpoint + '/' + name;
    }

    // External Identity Providers endpoint
    function getIdentityProvidersEndpoint(server, tenant) {
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/externalidp';
    }

    // Open ID Connect Client endpoint
    function getOpenIdConnectClientEndpoint(server, tenant, name) {
        var groups = getOpenIdConnectClientsEndpoint(server, tenant);
        return  groups + '/' + name;
    }

    // Open ID Connect Clients endpoint
    function getOpenIdConnectClientsEndpoint(server, tenant) {
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/oidcclient';
    }
    // Certificates endpoint
    function getCertificateBaseEndpoint(server, tenant) {
        var tenantEndpoint = getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/certificates';
    }

    // Certificates endpoint
    function getCertificatesEndpoint(server, tenant, fingerprint) {
        var url = getCertificateBaseEndpoint(server, tenant);
        var query = '';
        if(fingerprint && fingerprint.length > 0)
        {
            query = '&fingerprint' + fingerprint;
        }
        return url + '?scope=TENANT' + query;
    }

    // Private Key endpoint
    function getPrivateKeysEndpoint(server, tenant) {
        var certificatesEndpoint = getCertificateBaseEndpoint(server, tenant);
        return certificatesEndpoint + '/privatekey';
    }

    // Client Id
    function addClientId(server, tenant, clientId){
        var serverUri = getServerUri(server);
        return serverUri + "lightwaveui/RegisterOidc?tenant=" + tenant + "&clientId=" + clientId;
    }

    var service = {};
    service.getLoginEndpoint = getLoginEndpoint;
    service.getLoginArgument = getLoginArgument;
    service.getTenantsEndpoint = getTenantsEndpoint;
    service.getTenantEndpoint = getTenantEndpoint;
    service.getAllIdentitySourcesEndpoint = getAllIdentitySourcesEndpoint;
    service.getIdentitySourceEndpoint = getIdentitySourceEndpoint;
    service.getAllUsersEndpoint = getMemberSearchEndpoint;
    service.getSolutionUserEndpoint = getSolutionUserEndpoint;
    service.getSolutionUsersEndpoint = getSolutionUsersEndpoint;
    service.getUserEndpoint = getUserEndpoint;
    service.getUsersEndpoint = getUsersEndpoint;
    service.getGroupEndpoint = getGroupEndpoint;
    service.getGroupsEndpoint = getGroupsEndpoint;
    service.getConfigEndpoint = getConfigEndpoint;
    service.getComputersEndpoint = getComputersEndpoint;
    service.getUserGroupsEndpoint = getUserGroupsEndpoint;
    service.getGroupMembership = getGroupMembership;
    service.getRelyingPartyEndpoint = getRelyingPartyEndpoint;
    service.getRelyingPartiesEndpoint = getRelyingPartiesEndpoint;
    service.getIdentityProviderEndpoint = getIdentityProviderEndpoint;
    service.getIdentityProvidersEndpoint = getIdentityProvidersEndpoint;
    service.getOpenIdConnectClientEndpoint = getOpenIdConnectClientEndpoint;
    service.getOpenIdConnectClientsEndpoint = getOpenIdConnectClientsEndpoint;
    service.getPrivateKeysEndpoint = getPrivateKeysEndpoint;
    service.getCertificatesEndpoint = getCertificatesEndpoint;
    service.getUserPasswordEndpoint = getUserPasswordEndpoint;
    service.getDirConfigEndpoint = getDirConfigEndpoint;
    service.addClientId = addClientId;
    service.getTenantCleanupEndpoint = getTenantCleanupEndpoint;
    service.getServerUri = getServerUri;
    return service;
}