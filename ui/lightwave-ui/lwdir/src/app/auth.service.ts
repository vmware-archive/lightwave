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

import { Injectable, Inject } from '@angular/core';
import { Response } from '@angular/http';
import { HttpClient, HttpHeaders } from '@angular/common/http';
import { ConfigService } from './config.service';
import { Observable } from "rxjs/Rx";
import './rxjs-operators';
import { UtilsService } from './utils.service';
@Injectable()
export class AuthService {
    private server:string;
    private header: HttpHeaders;
    private rootDNQuery: string;
    private rootDN: string;

    constructor(private utilsService:UtilsService, private httpClient:HttpClient, private configService: ConfigService) {}

    getAuthHeader():any {
        if(this.header){
            return this.header;
        }else{
            this.constructAuthHeader(this.configService.currentUser.server.host, this.configService.currentUser.token.access_token);
            this.rootDN = this.configService.currentUser.tenant
            this.rootDNQuery = this.utilsService.getRootDnQuery(this.rootDN);
            return this.header;
        }
    }
    
    getServer() {
        if(this.server){
            return this.server;
        }else{
            this.server = this.configService.currentUser.rest_server;
            return this.configService.currentUser.rest_server;
        }
    }
    
    getOIDCServer() {
        return this.configService.currentUser.server.host;
    }

    getRootDnQuery() {
       return this.rootDNQuery;
    }

    getRootDN() {
       return this.rootDN;
    }

    invalidateLocalSession(){
        this.configService.currentUser = 'logout';
        window.sessionStorage.currentUser = this.configService.currentUser;
    }

    getLogoutUri():string{
        let clientHost = this.configService.currentUser.client.host;
        let postLogoutRedirectUri =  "https://" + clientHost + "/lightwaveui";
        let openIdConnectUri =  "https://" + this.configService.currentUser.server.host + "/openidconnect/logout/" + this.configService.currentUser.tenant;
        let args = "?id_token_hint=" + this.configService.currentUser.token.id_token + "&post_logout_redirect_uri=" + postLogoutRedirectUri + 
        "&state=" + this.configService.currentUser.token.id_token.state;
        return openIdConnectUri + args;
    }

    logout() {
        let logoutUri = this.getLogoutUri();
        this.invalidateLocalSession();
        window.location.href = logoutUri;
    }

    constructRootDNQuery(username: string) {
        var index = username.indexOf( "@" );
        let dmn = username.substr(index+1);
        this.rootDN = dmn;
        this.rootDNQuery = this.utilsService.getRootDnQuery(dmn);
        console.log(this.rootDNQuery);
    }

    constructAuthHeader(tenant:string, token:string) {
        this.server = tenant;
        this.header = new HttpHeaders({ 'Authorization': 'Bearer ' + token });
    }

    private handleError(error:any) {
        let errMsg = (error.message) ? error.message :
            error.status ? `${error.status} - ${error.statusText}` : 'Server error';
        console.log(error);
        console.error(errMsg); // log to console instead
        return Observable.throw(errMsg);
    }
}
