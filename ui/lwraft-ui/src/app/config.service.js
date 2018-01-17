"use strict";
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
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = require("@angular/core");
var http_1 = require("@angular/common/http");
var ConfigService = (function () {
    function ConfigService() {
        this.API_PORT = 7578;
    }
    // Server Uri
    ConfigService.prototype.getServerUri = function (server) {
        var port = server.port && server.port != '' ? ':' + server.port + '/' : '/';
        return server.protocol + '://' + server.host + port;
    };
    // Lightwave UI
    ConfigService.prototype.getLightwaveUri = function (server) {
        return 'https://' + server + '/lightwaveui';
    };
    // Open Id Connect login endpoint
    ConfigService.prototype.getLoginEndpoint = function (server, tenant) {
        var serverUri = this.getServerUri(server);
        return serverUri + 'openidconnect/token/' + tenant;
    };
    // IDM endpoint
    ConfigService.prototype.getIdmEndpoint = function (server) {
        var serverUri = this.getServerUri(server);
        return serverUri + 'idm';
    };
    ConfigService.prototype.getDirEndpoint = function (server) {
        var serverUri = this.getServerUri(server);
        return serverUri + 'vmdir';
    };
    // Tenants endpoint
    ConfigService.prototype.getDirTenantEndpoint = function (server, tenant) {
        var idmEndpoint = this.getDirEndpoint(server);
        return idmEndpoint + '/tenant/' + tenant;
    };
    ConfigService.prototype.getTenantsEndpoint = function (server) {
        var idmEndpoint = this.getIdmEndpoint(server);
        return idmEndpoint + '/tenant/';
    };
    // Tenant endpoint
    ConfigService.prototype.getTenantEndpoint = function (server, tenant) {
        var idmEndpoint = this.getIdmEndpoint(server);
        return idmEndpoint + '/tenant/' + tenant;
    };
    // Tenant cleanup endpoint
    ConfigService.prototype.getTenantCleanupEndpoint = function (server, tenant) {
        var lightwaveEndpoint = this.getLightwaveUri(server);
        return lightwaveEndpoint + '/CleanupTenant?tenant=' + tenant;
    };
    // Open Id Connect login arguments
    ConfigService.prototype.getLoginArgument = function (username, password) {
        username = username.replace('@', '%40');
        return 'grant_type=password&username=' + username + '&password=' + password + '&scope=openid+offline_access+id_groups+at_groups+rs_admin_server';
    };
    // Tenant configuration endpoint
    ConfigService.prototype.getConfigEndpoint = function (server, tenant, policies) {
        var type = '';
        if (policies && policies.length > 0) {
            type = '?';
            for (var i = 0; i < policies.length; i++) {
                type = type + 'type=' + policies[i] + '&';
            }
            type = type.substr(0, type.length - 1);
        }
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/config' + type;
    };
    // Tenant configuration endpoint
    ConfigService.prototype.getDirConfigEndpoint = function (server, tenant, policies) {
        var type = '';
        if (policies && policies.length > 0) {
            type = '?';
            for (var i = 0; i < policies.length; i++) {
                type = type + 'type=' + policies[i] + '&';
            }
            type = type.substr(0, type.length - 1);
        }
        var tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/config' + type;
    };
    // Member search endpoint
    ConfigService.prototype.getMemberSearchEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/search';
    };
    // User endpoint
    ConfigService.prototype.getUserEndpoint = function (server, tenant, username) {
        var tenantEndpoint = this.getUsersEndpoint(server, tenant);
        return tenantEndpoint + '/' + username;
    };
    ConfigService.prototype.getIdmUserEndpoint = function (server, tenant, username) {
        var tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        var usersEndpoint = tenantEndpoint + '/users';
        var userEndpoint = usersEndpoint + "/" + username;
        return userEndpoint;
    };
    // User password endpoint
    ConfigService.prototype.getUserPasswordEndpoint = function (server, tenant, username) {
        var userEndpoint = this.getIdmUserEndpoint(server, tenant, username);
        return userEndpoint + '/password';
    };
    // User groups
    ConfigService.prototype.getUserGroupsEndpoint = function (server, tenant, username) {
        var userEndpoint = this.getIdmUserEndpoint(server, tenant, username);
        return userEndpoint + '/groups';
    };
    // Users all endpoint
    ConfigService.prototype.getUsersEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/users';
    };
    // Solution User endpoint
    ConfigService.prototype.getSolutionUserEndpoint = function (server, tenant, username) {
        var tenantEndpoint = this.getSolutionUsersEndpoint(server, tenant);
        return tenantEndpoint + '/' + username;
    };
    // Solution Users all endpoint
    ConfigService.prototype.getSolutionUsersEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/solutionusers';
    };
    // Group endpoint
    ConfigService.prototype.getGroupEndpoint = function (server, tenant, name) {
        var tenantEndpoint = this.getGroupsEndpoint(server, tenant);
        return tenantEndpoint + '/' + name;
    };
    // Groups endpoint
    ConfigService.prototype.getGroupsEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        return tenantEndpoint + '/groups';
    };
    ConfigService.prototype.getIdmGroupEndpoint = function (server, tenant, name) {
        var tenantEndpoint = this.getDirTenantEndpoint(server, tenant);
        var groupsEndpoint = tenantEndpoint + '/groups';
        return groupsEndpoint + "/" + name;
    };
    // Group membership endpoint
    ConfigService.prototype.getGroupMembership = function (server, tenant, upn, members, type) {
        var groupEndpoint = this.getIdmGroupEndpoint(server, tenant, upn);
        return groupEndpoint + '/members?' + members + 'type=' + type;
    };
    // Identity sources endpoint
    ConfigService.prototype.getAllIdentitySourcesEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/providers';
    };
    // Identity source endpoint
    ConfigService.prototype.getIdentitySourceEndpoint = function (server, tenant, name) {
        var identitySourcesEndpoint = this.getAllIdentitySourcesEndpoint(server, tenant);
        return identitySourcesEndpoint + '/' + name;
    };
    // Server computers endpoint
    ConfigService.prototype.getComputersEndpoint = function (server) {
        var idmEndpoint = this.getIdmEndpoint(server);
        return idmEndpoint + '/server/computers';
    };
    // Relying Party endpoint
    ConfigService.prototype.getRelyingPartyEndpoint = function (server, tenant, name) {
        var groups = this.getRelyingPartiesEndpoint(server, tenant);
        return groups + '/' + name;
    };
    // Relying Parties endpoint
    ConfigService.prototype.getRelyingPartiesEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/relyingparty';
    };
    // External Identity Provider endpoint
    ConfigService.prototype.getIdentityProviderEndpoint = function (server, tenant, name) {
        var idpEndpoint = this.getIdentityProvidersEndpoint(server, tenant);
        return idpEndpoint + '/' + name;
    };
    // External Identity Providers endpoint
    ConfigService.prototype.getIdentityProvidersEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/externalidp';
    };
    // Open ID Connect Client endpoint
    ConfigService.prototype.getOpenIdConnectClientEndpoint = function (server, tenant, name) {
        var groups = this.getOpenIdConnectClientsEndpoint(server, tenant);
        return groups + '/' + name;
    };
    // Open ID Connect Clients endpoint
    ConfigService.prototype.getOpenIdConnectClientsEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/oidcclient';
    };
    // Certificates endpoint
    ConfigService.prototype.getCertificateBaseEndpoint = function (server, tenant) {
        var tenantEndpoint = this.getTenantEndpoint(server, tenant);
        return tenantEndpoint + '/certificates';
    };
    // Certificates endpoint
    ConfigService.prototype.getCertificatesEndpoint = function (server, tenant, fingerprint) {
        var url = this.getCertificateBaseEndpoint(server, tenant);
        var query = '';
        if (fingerprint && fingerprint.length > 0) {
            query = '&fingerprint' + fingerprint;
        }
        return url + '?scope=TENANT' + query;
    };
    // Private Key endpoint
    ConfigService.prototype.getPrivateKeysEndpoint = function (server, tenant) {
        var certificatesEndpoint = this.getCertificateBaseEndpoint(server, tenant);
        return certificatesEndpoint + '/privatekey';
    };
    // Client Id
    ConfigService.prototype.addClientId = function (server, tenant, clientId) {
        var serverUri = this.getServerUri(server);
        return serverUri + "lightwaveui/RegisterOidc?tenant=" + tenant + "&clientId=" + clientId;
    };
    ConfigService.prototype.getHeaders = function (token, contentType) {
        var headers = new http_1.HttpHeaders();
        if (token) {
            headers = headers.set("Authorization", token.token_type + " " + token.access_token);
        }
        if (contentType) {
            headers = headers.set("Content-Type", contentType);
        }
        return headers;
    };
    return ConfigService;
}());
ConfigService = __decorate([
    core_1.Injectable()
], ConfigService);
exports.ConfigService = ConfigService;
//# sourceMappingURL=config.service.js.map