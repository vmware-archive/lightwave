/*
 *  Copyright (c) 2012-2017 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

import {Injectable} from '@angular/core';
import {HttpHeaders} from '@angular/common/http';

@Injectable()
export class ConfigService{
    public currentUser:any;
    public errors:any;
    public currentSelection:string;
    // Server Uri
    getServerUri(server) {
        let port = server.port && server.port != '' ? ':' + server.port + '/' : '/';
        return server.protocol + '://' + server.host + port;
    }

    // Lightwave UI
    getLightwaveUri(server) {
        return 'https://' + server + '/lightwaveui';
    }

    // Open Id Connect login endpoint
    getLoginEndpoint(server, tenant) {
        let serverUri = this.getServerUri(server);
        return serverUri + 'openidconnect/token/' + tenant;
    }

    // IDM endpoint
    getIdmEndpoint(server) {
        let serverUri = this.getServerUri(server);
        return serverUri + 'idm';
    }

    getDirEndpoint(server) {
        let serverUri = this.getServerUri(server);
        return serverUri + 'vmdir';
    }

    // Tenants endpoint
    getDirTenantEndpoint(server, tenant) {
        let idmEndpoint = this.getDirEndpoint(server);
        return idmEndpoint + '/tenant/' + tenant;
    }
        getTenantsEndpoint(server) {
        let idmEndpoint = this.getIdmEndpoint(server);
        return idmEndpoint + '/tenant/';
    }

    // Tenant endpoint
    getTenantEndpoint(server, tenant) {
        let idmEndpoint = this.getIdmEndpoint(server);
        return idmEndpoint + '/tenant/' + tenant;
    }

    // Tenant cleanup endpoint
    getTenantCleanupEndpoint(server, tenant) {

        let lightwaveEndpoint = this.getLightwaveUri(server);
        return lightwaveEndpoint + '/CleanupTenant?tenant=' + tenant;
    }
    // Open Id Connect login arguments
    getLoginArgument(username, password) {
        username = username.replace('@', '%40');
        return 'grant_type=password&username=' + username + '&password=' + password + '&scope=openid+offline_access+id_groups+at_groups+rs_admin_server';
    }

    // Tenant configuration endpoint
    getConfigEndpoint(server, tenant, policies) {

        let type = '';
        if(policies && policies.length > 0)
        {
            type = '?';
            for(let i = 0; i < policies.length; i++) {
                type = type + 'type=' + policies[i] + '&';
            }
            type = type.substr(0, type.length - 1);
        }
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/config' + type;
    }

    // Tenant configuration endpoint
    getDirConfigEndpoint(server, tenant, policies) {

        let type = '';
        if(policies && policies.length > 0)
        {
            type = '?';
            for(let i = 0; i < policies.length; i++) {
                type = type + 'type=' + policies[i] + '&';
            }
            type = type.substr(0, type.length - 1);
        }
        let tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/config' + type;
    }

    // Member search endpoint
    getMemberSearchEndpoint(server, tenant) {
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/search';
    }

    // User endpoint
    getUserEndpoint(server, tenant, username) {
        let tenantEndpoint = this.getUsersEndpoint(server, tenant);
        return tenantEndpoint + '/' + username;
    }

    getIdmUserEndpoint(server, tenant, username){
        let tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        let usersEndpoint = tenantEndpoint + '/users';
        let userEndpoint =usersEndpoint + "/" + username;
        return userEndpoint;
    }
    // User password endpoint
    getUserPasswordEndpoint(server, tenant, username) {
        let userEndpoint = this.getIdmUserEndpoint(server, tenant, username);
        return userEndpoint + '/password';
    }

    // User groups
    getUserGroupsEndpoint(server, tenant, username) {
        let userEndpoint = this.getIdmUserEndpoint(server, tenant, username);
        return userEndpoint + '/groups';
    }

    // Users all endpoint
    getUsersEndpoint(server, tenant) {
        let tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/users';
    }

    // Solution User endpoint
    getSolutionUserEndpoint(server, tenant, username) {
        let tenantEndpoint = this.getSolutionUsersEndpoint(server, tenant);
        return  tenantEndpoint + '/' + username;
    }

    // Solution Users all endpoint
    getSolutionUsersEndpoint(server, tenant) {
        let tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/solutionusers';
    }

    // Group endpoint
    getGroupEndpoint(server, tenant, name) {
        let tenantEndpoint = this.getGroupsEndpoint(server, tenant);
        return  tenantEndpoint + '/' + name;
    }
    // Groups endpoint
    getGroupsEndpoint(server, tenant) {
        let tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/groups';
    }

    getIdmGroupEndpoint(server, tenant, name){
        let tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        let groupsEndpoint = tenantEndpoint + '/groups';
        return groupsEndpoint + "/" + name;
    }


    // Group membership endpoint
    getGroupMembership(server, tenant, upn, members, type){
        let groupEndpoint = this.getIdmGroupEndpoint(server, tenant, upn);
        return groupEndpoint + '/members?' + members + 'type=' + type;
    }

    // Identity sources endpoint
    getAllIdentitySourcesEndpoint(server, tenant) {
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/providers';
    }

    // Identity source endpoint
    getIdentitySourceEndpoint(server, tenant, name) {
        let identitySourcesEndpoint = this.getAllIdentitySourcesEndpoint(server, tenant);
        return identitySourcesEndpoint + '/' + name;
    }
   // Server computers endpoint
    getComputersEndpoint(server) {
        let idmEndpoint = this.getIdmEndpoint(server);
        return idmEndpoint + '/server/computers';
    }

    // Relying Party endpoint
    getRelyingPartyEndpoint(server, tenant, name) {
        let groups = this.getRelyingPartiesEndpoint(server, tenant);
        return  groups + '/' + name;
    }

    // Relying Parties endpoint
    getRelyingPartiesEndpoint(server, tenant) {
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/relyingparty';
    }

    // External Identity Provider endpoint
    getIdentityProviderEndpoint(server, tenant, name) {
        let idpEndpoint = this.getIdentityProvidersEndpoint(server, tenant);
        return  idpEndpoint + '/' + name;
    }

    // External Identity Providers endpoint
    getIdentityProvidersEndpoint(server, tenant) {
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/externalidp';
    }

   // Open ID Connect Client endpoint
    getOpenIdConnectClientEndpoint(server, tenant, name) {
        let groups = this.getOpenIdConnectClientsEndpoint(server, tenant);
        return  groups + '/' + name;
    }

    // Open ID Connect Clients endpoint
    getOpenIdConnectClientsEndpoint(server, tenant) {
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/oidcclient';
    }
    // Certificates endpoint
    getCertificateBaseEndpoint(server, tenant) {
        let tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/certificates';
    }

    // Certificates endpoint
    getCertificatesEndpoint(server, tenant, fingerprint) {
        let url = this.getCertificateBaseEndpoint(server, tenant);
        let query = '';
        if(fingerprint && fingerprint.length > 0)
        {
            query = '&fingerprint' + fingerprint;
        }
        return url + '?scope=TENANT' + query;
    }
   // Private Key endpoint
    getPrivateKeysEndpoint(server, tenant) {
        let certificatesEndpoint = this.getCertificateBaseEndpoint(server, tenant);
        return certificatesEndpoint + '/privatekey';
    }

    // Client Id
    addClientId(server, tenant, clientId){
        let serverUri = this.getServerUri(server);
        return serverUri + "lightwaveui/RegisterOidc?tenant=" + tenant + "&clientId=" + clientId;
    }

    getHeaders(token, contentType):any {
        let headers:HttpHeaders = new HttpHeaders();
        if(token){
            headers = headers.set("Authorization", token.token_type + " " + token.access_token);
        }
 	if(contentType){
            headers = headers.set("Content-Type", contentType);
        }
        return headers;
    }

}
