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
var config_service_1 = require("./config.service");
var utils_service_1 = require("./utils.service");
var http_1 = require("@angular/common/http");
var IdentitySourceService = (function () {
    function IdentitySourceService(httpClient, configService, utilsService) {
        this.httpClient = httpClient;
        this.configService = configService;
        this.utilsService = utilsService;
    }
    IdentitySourceService.prototype.GetAll = function (context) {
        var _this = this;
        var res;
        var endpoint = this.configService.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        var cType = 'application/json';
        var headers = this.configService.getHeaders(context.token, cType);
        return this.httpClient.get(endpoint, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    IdentitySourceService.prototype.Add = function (context, identitySource) {
        var _this = this;
        var endpoint = this.configService.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        var cType = 'application/json';
        var headers = this.configService.getHeaders(context.token, cType);
        return this.httpClient.post(endpoint, identitySource, { headers: headers })
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    IdentitySourceService.prototype.Update = function (context, identitySource) {
        var _this = this;
        var endpoint = this.configService.getIdentitySourceEndpoint(context.server, context.tenant, identitySource.name);
        var cType = 'application/json';
        var headers = this.configService.getHeaders(context.token, cType);
        return this.httpClient.put(endpoint, identitySource, { headers: headers })
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    IdentitySourceService.prototype.Delete = function (context, name) {
        var _this = this;
        var endpoint = this.configService.getIdentitySourceEndpoint(context.server, context.tenant, name);
        var cType = 'application/json';
        var headers = this.configService.getHeaders(context.token, cType);
        return this.httpClient.delete(endpoint, { headers: headers })
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    IdentitySourceService.prototype.TestConnectivity = function (context, identitySource) {
        var _this = this;
        var endpoint = this.configService.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        var cType = 'application/json';
        var headers = this.configService.getHeaders(context.token, cType);
        endpoint = endpoint + "?probe=true";
        return this.httpClient.post(endpoint, identitySource, { headers: headers })
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    return IdentitySourceService;
}());
IdentitySourceService = __decorate([
    core_1.Injectable(),
    __metadata("design:paramtypes", [http_1.HttpClient, config_service_1.ConfigService, utils_service_1.UtilsService])
], IdentitySourceService);
exports.IdentitySourceService = IdentitySourceService;
//# sourceMappingURL=identitysources.service.js.map