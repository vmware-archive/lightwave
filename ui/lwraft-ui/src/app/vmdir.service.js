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
var VmdirService = (function () {
    function VmdirService(utilsService, configService, authService, httpClient) {
        this.utilsService = utilsService;
        this.configService = configService;
        this.authService = authService;
        this.httpClient = httpClient;
    }
    VmdirService.prototype.getDirListing = function (rootDn) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        if (rootDn == null) {
            rootDn = this.authService.getRootDnQuery();
        }
        console.log("root DN:" + rootDn);
        this.getUrl += '?scope=one&dn=' + rootDn + '&attrs=dn';
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .do(function (listing) { return _this.listing = listing; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getAllTenants = function (tenantName) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var rootDn = encodeURIComponent('cn=Tenants,cn=IdentityManager,cn=Services,') + this.utilsService.getRootDnQuery(tenantName);
        console.log("root DN:" + rootDn);
        this.getUrl += '?scope=one&dn=' + rootDn + '&attrs=dn';
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .do(function (listing) { return _this.listing = listing; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getAttributes = function (rootDn) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        if (rootDn == null) {
            rootDn = this.authService.getRootDnQuery();
        }
        console.log("root DN:" + rootDn);
        this.getUrl += '?scope=base&dn=' + rootDn + '&attrs=' + encodeURIComponent('*,+');
        return this.httpClient.get(this.getUrl, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .do(function (listing) { return _this.listing = listing; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getACLString = function (rootDn) {
        var _this = this;
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var url = this.getUrl + '?scope=base&dn=' + rootDn + '&attrs=vmwaclstring';
        console.log(url);
        return this.httpClient.get(url, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getJsonObjEntry = function (operation, attrType, attrValue) {
        var jsonObjEntry = {};
        jsonObjEntry.operation = operation;
        jsonObjEntry.attribute = {};
        jsonObjEntry.attribute.type = attrType;
        jsonObjEntry.attribute.value = [attrValue];
        return jsonObjEntry;
    };
    VmdirService.prototype.constructJsonBody = function (initialValMap, attrArr, attrMap, schemaMap) {
        var jsonOut;
        var jsonArrObj = [];
        for (var _i = 0, attrArr_1 = attrArr; _i < attrArr_1.length; _i++) {
            var attrEntry = attrArr_1[_i];
            var attrType = attrEntry; //has the key
            var updatedVal = attrMap[attrType];
            var initVal = initialValMap[attrType];
            if (initialValMap[attrType] && schemaMap[attrType]['isSingleValued'][0] == 'FALSE') {
                //create a map of Existing and new values
                var newMap = new Map();
                for (var p = 0; p < updatedVal.value.length; p++) {
                    newMap[updatedVal.value[p]] = false;
                }
                for (var i = 0; i < initVal.length; i++) {
                    if (undefined == newMap[initVal[i]]) {
                        jsonArrObj.push(this.getJsonObjEntry('delete', attrType, initVal[i]));
                    }
                    else {
                        newMap[initVal[i]] = true; // new list[i] = old list[i]
                    }
                }
                for (var i = 0; i < updatedVal.value.length; i++) {
                    if (!newMap[updatedVal.value[i]]) {
                        jsonArrObj.push(this.getJsonObjEntry('add', attrType, updatedVal.value[i]));
                    }
                }
            }
            else if (!initialValMap[attrType] && schemaMap[attrType]['isSingleValued'][0] == 'FALSE') {
                for (var k = 0; k < updatedVal.value.length; k++) {
                    jsonArrObj.push(this.getJsonObjEntry('add', attrType, updatedVal.value[k]));
                }
            }
            else {
                var operation = void 0;
                if (!initVal || !initVal.length) {
                    operation = 'add';
                }
                else {
                    operation = 'replace';
                }
                jsonArrObj.push(this.getJsonObjEntry(operation, attrType, updatedVal.value));
            }
        }
        return JSON.stringify(jsonArrObj);
    };
    VmdirService.prototype.updateAttributes = function (rootDn, originalValMap, attribsArr, modifiedAttributesMap, schemaMap) {
        var _this = this;
        var headers = this.authService.getAuthHeader();
        var body = this.constructJsonBody(originalValMap, attribsArr, modifiedAttributesMap, schemaMap);
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var updateUrl = this.getUrl + '?dn=' + rootDn;
        console.log(updateUrl);
        return this.httpClient.patch(updateUrl, body, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.constructSDJsonBody = function (aclString) {
        var jsonOut = "[{\
\"operation\": \"" + "replace" + "\",\
\"attribute\": {\
\"type\":\"" + 'vmwaclstring' + "\",\
\"value\": [\"" + aclString + "\"]\
}\
}]";
        console.log(JSON.parse(jsonOut));
        return jsonOut;
    };
    VmdirService.prototype.createAttrObject = function (attr, value) {
        var obj = {};
        obj.type = attr;
        obj.value = value.split(',');
        return obj;
    };
    VmdirService.prototype.createAttrObjArray = function (attrMap, attrsArr, jsonObjAttrs) {
        for (var i = 0; i < attrsArr.length; i++) {
            if (attrMap[attrsArr[i]].length) {
                jsonObjAttrs.push(this.createAttrObject(attrsArr[i], attrMap[attrsArr[i]]));
            }
        }
    };
    VmdirService.prototype.constructObjectAddJsonBody = function (mustAttrMap, mayAttrMap) {
        var jsonObjAttrs = [];
        var mustAttrsArr = Object.keys(mustAttrMap);
        var mayAttrsArr = Object.keys(mayAttrMap);
        this.createAttrObjArray(mustAttrMap, mustAttrsArr, jsonObjAttrs);
        this.createAttrObjArray(mayAttrMap, mayAttrsArr, jsonObjAttrs);
        //        jsonObjAttrs.push(this.createAttrObject('nTSecurityDescriptor', ''));
        return jsonObjAttrs;
    };
    VmdirService.prototype.addNewObject = function (rootDn, mustAttrMap, mayAttrMap) {
        var _this = this;
        var headers = this.authService.getAuthHeader();
        if (mustAttrMap['cn'] && mustAttrMap['cn'].length) {
            mayAttrMap['cn'] = '';
        }
        var jsonObj = {};
        jsonObj.dn = rootDn;
        jsonObj.attributes = this.constructObjectAddJsonBody(mustAttrMap, mayAttrMap);
        var body = JSON.stringify(jsonObj);
        var addUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        console.log(addUrl);
        return this.httpClient.put(addUrl, body, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.updateAclString = function (rootDn, aclStrVal) {
        var _this = this;
        var headers = this.authService.getAuthHeader();
        var body = this.constructSDJsonBody(aclStrVal);
        var url = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        url += '?dn=' + encodeURIComponent(rootDn);
        console.log(url);
        return this.httpClient.patch(url, body, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.delete = function (rootDn) {
        var _this = this;
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var deleteUrl = this.getUrl + '?dn=' + rootDn;
        console.log(deleteUrl);
        return this.httpClient.delete(deleteUrl, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getObjectBySID = function (objectSid) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var url = this.getUrl + '?scope=sub&dn=' + this.authService.getRootDnQuery() + '&filter=' + encodeURIComponent('objectsid=' + objectSid);
        console.log(url);
        return this.httpClient.get(url, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getAllStrObjectClasses = function () {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var url = this.getUrl + '?scope=one&dn=' + encodeURIComponent('cn=schemacontext') + '&filter=' + encodeURIComponent('objectclass=classschema') + '&attrs=' + encodeURIComponent('subClassOf,cn,systemMustContain,systemMayContain,auxiliaryClass,mayContain,mustContain,objectClassCategory');
        console.log(url);
        return this.httpClient.get(url, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getAuxObjectClass = function (objectClass) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var url = this.getUrl + '?scope=base&dn=' + encodeURIComponent('cn=' + objectClass + ',cn=schemacontext') + '&attrs=auxiliaryClass';
        console.log(url);
        return this.httpClient.get(url, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getAllUsersAndGroups = function () {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var url = this.getUrl + '?scope=sub&dn=' + this.authService.getRootDnQuery() + '&filter=' + encodeURIComponent('objectsid=*') +
            '&attrs=objectsid,cn,objectclass';
        console.log(url);
        return this.httpClient.get(url, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    VmdirService.prototype.getObjectByGUID = function (objectGuid) {
        var _this = this;
        this.domain = this.authService.getDomain();
        var headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/post/ldap';
        var url = this.getUrl + '?scope=sub&dn=' + this.authService.getRootDnQuery() + '&filter=' + encodeURIComponent('objectguid=' + objectGuid);
        console.log(url);
        return this.httpClient.get(url, { headers: headers })
            .share()
            .map(function (res) { return res; })
            .catch(function (err) { return _this.utilsService.handleError(err); });
    };
    return VmdirService;
}());
VmdirService = __decorate([
    core_1.Injectable(),
    __metadata("design:paramtypes", [utils_service_1.UtilsService, config_service_1.ConfigService, auth_service_1.AuthService, http_1.HttpClient])
], VmdirService);
exports.VmdirService = VmdirService;
//# sourceMappingURL=vmdir.service.js.map