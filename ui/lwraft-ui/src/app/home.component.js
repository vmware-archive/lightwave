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
var __param = (this && this.__param) || function (paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); }
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = require("@angular/core");
var router_1 = require("@angular/router");
var config_service_1 = require("./config.service");
var auth_service_1 = require("./auth.service");
var utils_service_1 = require("./utils.service");
var vmdir_service_1 = require("./vmdir.service");
var platform_browser_1 = require("@angular/platform-browser");
var identitysources_service_1 = require("./identitysources.service");
var HomeComponent = (function () {
    function HomeComponent(document, authService, vmdirService, identitySourceService, utilsService, configService, activatedRoute) {
        this.document = document;
        this.authService = authService;
        this.vmdirService = vmdirService;
        this.identitySourceService = identitySourceService;
        this.utilsService = utilsService;
        this.configService = configService;
        this.activatedRoute = activatedRoute;
        this.error = '';
        this.signedInUser = '';
        this.tenantName = '';
        this.role = '';
        this.signedIn = false;
        this.showError = false;
        this.loginURL = '';
        this.displayComponents = false;
        this.isSystemTenant = false;
        this.rightMenuOpen = false;
    }
    HomeComponent.prototype.ngOnInit = function () {
        var _this = this;
        console.log('sessionStorage:');
        console.log(window.sessionStorage);
        if (window.sessionStorage.currentUser && window.sessionStorage.currentUser != 'logout') {
            console.log(window.sessionStorage.currentUser);
            this.displayComponents = true;
            this.loadFromSession();
        }
        else {
            this.activatedRoute.fragment.subscribe(function (params) {
                console.log(params);
                _this.readQueryParams(params);
            });
        }
        this.containerHeight = window.innerHeight * 90 / 100;
        console.log(window.innerHeight);
        console.log(this.containerHeight);
    };
    HomeComponent.prototype.openRightMenu = function () {
        var rightMenu = this.document.getElementById("rightMenu");
        rightMenu.style.display = "block";
        this.rightMenuOpen = true;
    };
    HomeComponent.prototype.closeRightMenu = function () {
        this.document.getElementById("rightMenu").style.display = "none";
        this.rightMenuOpen = false;
    };
    HomeComponent.prototype.closeSideBar = function ($event) {
        if ($event.target.id != 'slideOut') {
            var rightMenu = this.document.getElementById("rightMenu");
            if (this.rightMenuOpen && $event.target.id != 'rightMenu') {
                this.closeRightMenu();
            }
        }
    };
    HomeComponent.prototype.openIDM = function () {
        var idmUri = "/lightwaveui/app/#/ssohome";
        window.location.href = idmUri;
    };
    HomeComponent.prototype.readQueryParams = function (params) {
        var curUser = sessionStorage.getItem('currentUser');
        var paramsArr, idToken, accessToken, state, tokenType, expiresIn;
        if (params) {
            var paramsArr_1 = params.split('&');
            idToken = paramsArr_1[1].split('=')[1];
            accessToken = paramsArr_1[0].split('=')[1];
            state = paramsArr_1[2].split('=')[1];
            tokenType = paramsArr_1[3].split('=')[1];
            expiresIn = paramsArr_1[4].split('=')[1];
        }
        if (curUser == 'logout') {
            this.configService.currentUser = null;
            if (accessToken && idToken) {
                this.displayComponents = true;
                this.setcontext(idToken, accessToken, tokenType, expiresIn, state);
            }
            else {
                this.redirectToSTSLogin();
            }
        }
        else {
            if (curUser) {
                curUser = JSON.parse(curUser);
                this.displayComponents = true;
                this.configService.currentUser = curUser;
                this.loadFromSession();
            }
            else {
                if (accessToken && idToken) {
                    this.displayComponents = true;
                    this.setcontext(idToken, accessToken, tokenType, expiresIn, state);
                }
                else {
                    this.redirectToSTSLogin();
                }
            }
        }
    };
    // Fetch the STS login URL & redirect there
    HomeComponent.prototype.redirectToSTSLogin = function () {
        var _this = this;
        var resObj;
        var resStr;
        this.authService.getSTSLoginUrl()
            .subscribe(function (result) {
            resStr = JSON.stringify(result);
            resObj = JSON.parse(resStr);
            console.log(resObj.result);
            _this.loginURL = resObj.result;
            window.location.href = _this.loginURL;
        }, function (error) {
            _this.configService.errors = error.data;
            _this.showError = true;
        });
    };
    HomeComponent.prototype.loadFromSession = function () {
        var curSession = JSON.parse(window.sessionStorage.currentUser);
        this.signedInUser = curSession.first_name;
        this.role = curSession.role;
        this.tenantName = curSession.tenant;
        this.configService.currentUser = curSession;
        if (window.location.href.indexOf('#') != -1) {
            window.location.href = window.location.href.split('#')[0];
        }
        this.signedIn = true;
    };
    HomeComponent.prototype.setcontext = function (id_token, access_token, token_type, expires_in, state) {
        var decodedJwt = this.utilsService.decodeJWT(id_token);
        var decodedAccessJwt = this.utilsService.decodeJWT(access_token);
        var currentUser = {};
        currentUser.server = {};
        currentUser.server.host = window.location.host;
        currentUser.server.port = window.location.port;
        currentUser.server.protocol = window.location.protocol;
        currentUser.tenant = decodedJwt.header.tenant;
        currentUser.username = decodedJwt.header.sub;
        currentUser.first_name = decodedJwt.header.given_name;
        currentUser.last_name = decodedJwt.header.family_name;
        currentUser.role = decodedAccessJwt.header.admin_server_role;
        currentUser.token = {};
        currentUser.token.id_token = id_token;
        currentUser.token.access_token = access_token;
        currentUser.token.expires_in = expires_in;
        currentUser.token.token_type = token_type;
        currentUser.isSystemTenant = false;
        currentUser.token.state = state;
        this.configService.currentUser = currentUser;
        window.sessionStorage.currentUser = JSON.stringify(this.configService.currentUser);
        this.loadFromSession();
    };
    HomeComponent.prototype.getAllTenants = function () {
        var _this = this;
        var tlist;
        var listingObj;
        this.vmdirService.getAllTenants(this.configService.currentUser.tenant)
            .subscribe(function (listing) {
            tlist = JSON.stringify(listing);
            listingObj = JSON.parse(tlist);
            _this.utilsService.extractName(listingObj.result, false);
            _this.allTenants = listingObj;
            console.log(listingObj);
        }, function (error) { return _this.error = error; });
    };
    HomeComponent.prototype.handleLogout = function () {
        var _this = this;
        // get the IDP host
        var resObj;
        var resStr;
        var idpHost;
        this.authService.getSTSLoginUrl()
            .subscribe(function (result) {
            resStr = JSON.stringify(result);
            resObj = JSON.parse(resStr);
            idpHost = resObj.idp_host;
            _this.authService.logout(idpHost);
        }, function (error) {
            _this.configService.errors = error.data;
            _this.showError = true;
        });
    };
    return HomeComponent;
}());
HomeComponent = __decorate([
    core_1.Component({
        moduleId: module.id,
        selector: 'home',
        templateUrl: './home.component.html',
    }),
    __param(0, core_1.Inject(platform_browser_1.DOCUMENT)),
    __metadata("design:paramtypes", [Object, auth_service_1.AuthService, vmdir_service_1.VmdirService, identitysources_service_1.IdentitySourceService, utils_service_1.UtilsService, config_service_1.ConfigService, router_1.ActivatedRoute])
], HomeComponent);
exports.HomeComponent = HomeComponent;
//# sourceMappingURL=home.component.js.map