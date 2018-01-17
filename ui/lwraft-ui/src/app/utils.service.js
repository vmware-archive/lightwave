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
var Rx_1 = require("rxjs/Rx");
var config_service_1 = require("./config.service");
var UtilsService = (function () {
    function UtilsService(configService) {
        this.configService = configService;
        this.API_PORT = 7477;
    }
    UtilsService.prototype.decodeJWT = function (jwt) {
        var sJWS = jwt;
        var components = sJWS.split(".");
        var uHeader = atob(components[1]).toString();
        var decodedJWT = {
            header: JSON.parse(uHeader)
            //claims: JSON.parse(uClaim)
        };
        return decodedJWT;
    };
    UtilsService.prototype.setIcon = function (cnObj) {
        console.log(cnObj);
        if (cnObj.displayName.indexOf("Users") != -1 || cnObj.displayName.indexOf("Administrators") != -1 || cnObj.displayName.indexOf("Operators") != -1) {
            cnObj.iconShape = "user";
            cnObj.iconClass = "is-solid is-info";
        }
        else if (cnObj.displayName.indexOf("Computers") != -1) {
            cnObj.iconShape = "computer";
            cnObj.iconClass = "is-solid is-info";
        }
        else if (cnObj.displayName.indexOf("Security") != -1) {
            cnObj.iconShape = "shield";
            cnObj.iconClass = "is-solid is-info";
        }
        else if (cnObj.displayName.indexOf("Component") != -1) {
            cnObj.iconShape = "plugin";
            cnObj.iconClass = "is-solid is-info";
        }
        else if (cnObj.displayName.indexOf("Principals") != -1) {
            cnObj.iconShape = "list";
            cnObj.iconClass = "is-info";
        }
        else if (cnObj.displayName.indexOf("Services") != -1) {
            cnObj.iconShape = "cog";
            cnObj.iconClass = "is-solid is-info";
        }
        else if (cnObj.displayName.indexOf("Domain Controller") != -1) {
            cnObj.iconShape = "cluster";
            cnObj.iconClass = "is-info is-solid";
        }
        else if (cnObj.displayName.indexOf("Configuration") != -1) {
            cnObj.iconShape = "file-settings";
            cnObj.iconClass = "is-solid is-info";
        }
        else if (cnObj.displayName.indexOf("Service Accounts") != -1) {
            cnObj.iconShape = "users";
            cnObj.iconClass = "is-info is-solid";
        }
        else if (cnObj.displayName.indexOf("Zones") != -1) {
            cnObj.iconShape = "network-globe";
            cnObj.iconClass = "is-info";
        }
        else if (cnObj.displayName.indexOf("Builtin") != -1) {
            cnObj.iconShape = "connect";
            cnObj.iconClass = "is-info is-solid";
        }
        else if (cnObj.displayName.indexOf("Deleted") != -1) {
            cnObj.iconShape = "trash";
            cnObj.iconClass = "is-info is-solid";
            cnObj.active = 'true';
        }
        else if (cnObj.displayName.indexOf("policy") != -1) {
            cnObj.iconShape = "tasks";
            cnObj.iconClass = "is-info";
        }
        else {
            cnObj.iconShape = "folder";
            cnObj.iconClass = "is-solid is-info";
        }
    };
    UtilsService.prototype.extractName = function (dirList, doSetIcon) {
        for (var _i = 0, dirList_1 = dirList; _i < dirList_1.length; _i++) {
            var cn = dirList_1[_i];
            var name_1 = cn.dn;
            var id = name_1.indexOf(',');
            name_1 = name_1.substr(0, id);
            id = name_1.indexOf('=');
            name_1 = name_1.substr(id + 1);
            console.log(name_1);
            cn.displayName = name_1;
            cn.active = 'false';
            if (doSetIcon) {
                this.setIcon(cn);
            }
            cn.encodedDN = encodeURIComponent(cn.dn);
        }
    };
    UtilsService.prototype.strInsert = function (str, toInsert, position) {
        str = [str.slice(0, position), toInsert, str.slice(position)].join('');
        return str;
    };
    UtilsService.prototype.toValidTimeStr = function (dateInput) {
        dateInput = this.strInsert(dateInput, '-', 4);
        dateInput = this.strInsert(dateInput, '-', 7);
        dateInput = this.strInsert(dateInput, 'T', 10);
        dateInput = this.strInsert(dateInput, ':', 13);
        dateInput = this.strInsert(dateInput, ':', 16);
        dateInput = dateInput.replace('.0', '');
        return dateInput;
    };
    UtilsService.prototype.getRootDnQuery = function (dmn) {
        var index = dmn.indexOf('.', 0);
        var finalStr = '';
        var prevIndex = -1;
        while (-1 != index) {
            var dm1 = dmn.substr(0, index);
            finalStr += ('dc=' + dm1 + ',');
            prevIndex = index;
            index = dmn.indexOf('.', index + 1);
        }
        var dm2 = dmn.substr(prevIndex + 1);
        finalStr += ('dc=' + dm2);
        return encodeURIComponent(finalStr);
    };
    UtilsService.prototype.constructLogoutUrl = function (idpHost) {
        var tenant = this.configService.currentUser.tenant;
        var postLogoutUrl = "https://" + idpHost + "/lightwaveui";
        var logoutUrl = "https://" + idpHost + "/openidconnect/logout/" + tenant;
        var token = this.configService.currentUser.idToken;
        var state = this.configService.currentUser.state;
        var args = "?id_token_hint=" + token +
            "&post_logout_redirect_uri=" + postLogoutUrl + "&state=" + state;
        return logoutUrl + args;
    };
    UtilsService.prototype.handleError = function (error) {
        console.log(error);
        if ((error) &&
            (error.status == 401 || (error.error && error.error.error == 'invalid_token'))) {
            var redirectUri = '/lightwaveui/Login?tenant=' + this.configService.currentUser.tenant;
            sessionStorage.currentUser = 'logout';
            window.location.href = redirectUri;
        }
        else {
            return Rx_1.Observable.throw(error);
        }
    };
    return UtilsService;
}());
UtilsService = __decorate([
    core_1.Injectable(),
    __metadata("design:paramtypes", [config_service_1.ConfigService])
], UtilsService);
exports.UtilsService = UtilsService;
//# sourceMappingURL=utils.service.js.map