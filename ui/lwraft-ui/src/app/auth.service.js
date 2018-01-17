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
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = require("@angular/core");
var http_1 = require("@angular/common/http");
var config_service_1 = require("./config.service");
var Rx_1 = require("rxjs/Rx");
require("./rxjs-operators");
var utils_service_1 = require("./utils.service");
var AuthService = (function () {
    function AuthService(utilsService, httpClient, configService) {
        this.utilsService = utilsService;
        this.httpClient = httpClient;
        this.configService = configService;
    }
    AuthService.prototype.getAuthHeader = function () {
        if (this.header) {
            return this.header;
        }
        else {
            this.constructAuthHeader(this.configService.currentUser.server.host, this.configService.currentUser.token.access_token);
            this.rootDN = this.configService.currentUser.tenant;
            this.rootDNQuery = this.utilsService.getRootDnQuery(this.rootDN);
            return this.header;
        }
    };
    AuthService.prototype.getSTSLoginUrl = function () {
        var protocol = window.location.protocol;
        var host = window.location.host;
        var path = '/v1/post/idp';
        var queryURL = protocol + '//' + host + ':' + this.configService.API_PORT + path;
        var loginURL, tokenStr;
        var resObj;
        return this.httpClient.get(queryURL)
            .share()
            .map(function (res) { return res; })
            .catch(this.handleError);
    };
    AuthService.prototype.getDomain = function () {
        if (this.domain) {
            return this.domain;
        }
        else {
            return this.configService.currentUser.server.host;
        }
    };
    AuthService.prototype.getRootDnQuery = function () {
        return this.rootDNQuery;
    };
    AuthService.prototype.getRootDN = function () {
        return this.rootDN;
    };
    AuthService.prototype.logout = function (idpHost) {
        var logoutUrl = this.utilsService.constructLogoutUrl(idpHost);
        window.sessionStorage.currentUser = 'logout';
        window.location.href = logoutUrl;
    };
    AuthService.prototype.constructRootDNQuery = function (username) {
        var index = username.indexOf("@");
        var dmn = username.substr(index + 1);
        this.rootDN = dmn;
        this.rootDNQuery = this.utilsService.getRootDnQuery(dmn);
        console.log(this.rootDNQuery);
    };
    AuthService.prototype.constructAuthHeader = function (tenant, token) {
        this.domain = tenant;
        this.header = new http_1.HttpHeaders({ 'Authorization': 'Bearer ' + token });
    };
    AuthService.prototype.handleError = function (error) {
        var errMsg = (error.message) ? error.message :
            error.status ? error.status + " - " + error.statusText : 'Server error';
        console.log(error);
        console.error(errMsg);
        return Rx_1.Observable.throw(errMsg);
    };
    return AuthService;
}());
AuthService = __decorate([
    core_1.Injectable(),
    __metadata("design:paramtypes", [utils_service_1.UtilsService, http_1.HttpClient, config_service_1.ConfigService])
], AuthService);
exports.AuthService = AuthService;
//# sourceMappingURL=auth.service.js.map