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
require("./rxjs-operators");
var config_service_1 = require("./config.service");
var auth_service_1 = require("./auth.service");
var utils_service_1 = require("./utils.service");
var VmdirSchemaService = (function () {
    function VmdirSchemaService(utilsService, configService, authService, httpClient) {
        this.utilsService = utilsService;
        this.configService = configService;
        this.authService = authService;
        this.httpClient = httpClient;
        this.domain = '';
        this.schema = '';
    }
    VmdirSchemaService.prototype.getSchema = function (rootDn) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        console.log("root DN:" + rootDn);
        this.getUrl += '?dn=' + encodeURIComponent('cn=' + rootDn + ',cn=schemacontext');
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .do(function (listing) { return _this.listing = listing; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    return VmdirSchemaService;
}());
VmdirSchemaService = __decorate([
    core_1.Injectable(),
    __metadata("design:paramtypes", [utils_service_1.UtilsService, config_service_1.ConfigService, auth_service_1.AuthService, http_1.HttpClient])
], VmdirSchemaService);
exports.VmdirSchemaService = VmdirSchemaService;
//# sourceMappingURL=vmdirschema.service.js.map