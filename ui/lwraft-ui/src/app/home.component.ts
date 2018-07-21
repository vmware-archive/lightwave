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

import { Inject,Component, OnInit } from '@angular/core';
import { Router, ActivatedRoute, Params } from '@angular/router';
import { ConfigService } from './config.service';
import { AuthService } from './auth.service';
import { UtilsService } from './utils.service';
import { VmdirService } from './vmdir.service';
import { DOCUMENT } from '@angular/platform-browser';
import { IdentitySourceService } from './identitysources.service';

@Component({
  moduleId: module.id,
  selector: 'home',
  templateUrl: './home.component.html',
})
export class HomeComponent  {
    private error:any = '';
    signedInUser:string = '';
    tenantName:string = '';
    role:string = '';
    containerHeight;
    signedIn:boolean = false;
    showError:boolean = false;
    loginURL:string = '';
    displayComponents:boolean = false;
    allTenants:any;
    isSystemTenant:boolean = false;
    rightMenuOpen:boolean = false;
    constructor(@Inject(DOCUMENT) private document: any, private authService:AuthService, private vmdirService:VmdirService, private identitySourceService:IdentitySourceService, private utilsService: UtilsService, private configService: ConfigService, private activatedRoute: ActivatedRoute){
    }
    ngOnInit(){
       console.log('sessionStorage:');
       console.log(window.sessionStorage);
       this.authenticateUser()
       this.containerHeight = window.innerHeight * 90 / 100;
       console.log(window.innerHeight);
       console.log(this.containerHeight);
    }

    openRightMenu(){
        let rightMenu = this.document.getElementById("rightMenu");
        rightMenu.style.display = "block";
        this.rightMenuOpen = true;
    }

    closeRightMenu() {
        this.document.getElementById("rightMenu").style.display = "none";
        this.rightMenuOpen = false;
    }

    closeSideBar($event:any){
        if($event.target.id != 'slideOut'){
            let rightMenu = this.document.getElementById("rightMenu");
            if(this.rightMenuOpen && $event.target.id != 'rightMenu' ){
                this.closeRightMenu();
            }
        }
    }

    openIDM(){
        let idmUri = "/lightwaveui/idm"
        window.location.href = idmUri;
    }

    redirectToAuthorizeUrl(server:string, OIDCClientID:string, tenantName:string, obj?:any){
        let clientHost:string[];
        if(obj){
            clientHost = obj.getHostName(location.href);
        }else{
            clientHost = this.getHostName(location.href);
        }
        let hostName = clientHost[0];
        let redirectURI = "https://" + hostName + "/lwraftui";
        let openIdConnectURI = "https://" + server + "/openidconnect/oidc/authorize/" + tenantName
        let args = "?response_type=id_token%20token&response_mode=fragment&client_id=" +
                                              OIDCClientID +
                                              "&redirect_uri=" + redirectURI +"&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server%20rs_post";

        let authorizeUrl = openIdConnectURI + args;
        window.location.href = authorizeUrl;
    }

    readConfigFile(file, object, callback) {
        let rawFile = new XMLHttpRequest();
        rawFile.overrideMimeType("application/json");
        rawFile.open("GET", file, true);
        rawFile.onreadystatechange = function() {
            if (rawFile.status == 200) {
                if(rawFile.readyState === 4){
                    callback(rawFile.responseText, object);
                }
            }else{
                 let errAlert = document.getElementById("errorAlert")
                 errAlert.style.display = 'block'
                 errAlert.innerHTML = "<b>Error!</b> Unable to open lwraft-ui. Missing configuration"
            }
        }
        rawFile.send(null);
    }

    authenticateUser(){
        let jsonData
        this.readConfigFile("config/lwraftui.json", this, function(data, obj){
            let jsonData:any = {};
            jsonData = JSON.parse(data);
            let lightwaveServer="", post:any = {}, tenant="", oidcId="" ;
            if(jsonData.LightwaveServer && jsonData.LightwaveServer.length > 0){
                lightwaveServer = jsonData.LightwaveServer
            }
            if(jsonData.PostServer){
                if(jsonData.PostServer.host && jsonData.PostServer.host.length > 0){
                    post.host = jsonData.PostServer.host
                }
                if(jsonData.PostServer.port){
                    post.port = jsonData.PostServer.port
                }
            }
            if(jsonData.Tenant && jsonData.Tenant.length > 0)
            {
                tenant = jsonData.Tenant
            }
            if(jsonData.OIDCClientID && jsonData.OIDCClientID.length > 0)
            {
                oidcId = jsonData.OIDCClientID
            }
            if(oidcId.length === 0 || post.host.length === 0 || lightwaveServer.length === 0 || tenant.length === 0){
                 var errAlert = document.getElementById("errorAlert")
                 errAlert.style.display = 'block'
                 errAlert.innerHTML = "<b>Error!</b> Unable to open lightwaveui. Missing/incorrect configuration"
            }
            let hash:string = window.location.hash;
            let access_token, id_token, state, token_type, expires_in;
            let curUser:any = sessionStorage.getItem('currentUser');
            var queryParams = {};
            if(hash.length > 0){
                hash = hash.substr(1);
                var paramsArr = hash.split('&');
                for (var i = 0;i < paramsArr.length; i ++){
                    var key = paramsArr[i].split('=')[0];
                    var val = paramsArr[i].split('=')[1];
                    queryParams[key] = val;
                }
                access_token = queryParams['access_token'];
                id_token = queryParams['id_token'];
                state = queryParams['state'];
                token_type = queryParams['token_type'];
                expires_in = queryParams['expires_in'];
            }
            // If redirected after successful login, set context and load the UI.
            if(hash.length && access_token && id_token && state && token_type && expires_in){
                window.sessionStorage.currentUser = 'logout';
                obj.configService.currentUser = null;
                obj.displayComponents = true;
                obj.setcontext(id_token, access_token, token_type, expires_in, state, post);
                return;
            }
            // else , is a valid session available ? If yes, load the UI else redirect to authorize URL to login to server.
            if (window.sessionStorage.currentUser && window.sessionStorage.currentUser != 'logout'){
                let curUserObj = JSON.parse(curUser);
                if(curUserObj && curUserObj.server && curUserObj.server.host === post.host && curUserObj.tenant === tenant){
                    obj.configService = curUserObj;
                    obj.displayComponents = true;
                    obj.loadFromSession();
                }else{
                    window.sessionStorage.currentUser = 'logout';
                    obj.configService = null;
                    obj.redirectToAuthorizeUrl(lightwaveServer, oidcId, tenant, obj);
                }
            }else{
                obj.redirectToAuthorizeUrl(lightwaveServer, oidcId, tenant, obj);
            }

        });
    }

    getHostName(url:string){
        let parts = url.split('://');
        let protocol = parts[0];
        let server_uri = parts[1]
        let server_with_port = server_uri.split('/')[0];
        let serverArr = server_with_port.split(':');
        let server, port = "443";
        if(serverArr.length > 1){
            port = serverArr[1];
        }
        server = serverArr[0];
        return [server,port];
    }

    readQueryParams(params:string, post:any):Boolean{
       let curUser:any = sessionStorage.getItem('currentUser');
       let paramsArr, idToken, accessToken, state, tokenType, expiresIn;
       if(params){
           let paramsArr = params.split('&');
           idToken = paramsArr[1].split('=')[1];
           accessToken = paramsArr[0].split('=')[1];
           state = paramsArr[2].split('=')[1];
           tokenType = paramsArr[3].split('=')[1];
           expiresIn = paramsArr[4].split('=')[1];
        }
        if(curUser == 'logout'){
            this.configService.currentUser = null;
            if(accessToken && idToken){
                this.displayComponents = true;
                this.setcontext(idToken, accessToken, tokenType, expiresIn, state, post);
            }else{
                return false
            }
        }else{
            if(curUser){// session is valid, load from session.
                curUser = JSON.parse(curUser);
                this.displayComponents = true;
                this.configService.currentUser = curUser;
                this.loadFromSession();
            }else{ // No session in progress
                if(accessToken && idToken){ // load from query parameters if available
                    this.displayComponents = true;
                    this.setcontext(idToken, accessToken, tokenType, expiresIn, state, post);
                }else{ // No fragments present either.
                    return false;
                }
            }
        }
        return true;
    }

    loadFromSession(){
        let curSession = JSON.parse(window.sessionStorage.currentUser);
        this.signedInUser = curSession.first_name;
        this.role = curSession.role;
        this.tenantName = curSession.tenant;
        this.configService.currentUser = curSession;
        if(window.location.href.indexOf('#') != -1){
            window.location.href = window.location.href.split('#')[0];
        }
        this.signedIn = true;
    }

    setcontext(id_token, access_token, token_type, expires_in, state, post:any) {
            let decodedJwt:any = this.utilsService.decodeJWT(id_token);
            let decodedAccessJwt:any = this.utilsService.decodeJWT(access_token);
            let server = this.getHostName(decodedJwt.header.iss);
            let currentUser:any =  {};
            let client = this.getHostName(location.href);
            currentUser.server = {};
            currentUser.server.host = post.host;
            currentUser.server.port = post.port;
            currentUser.lightwave_server = server[0].concat(":", server[1])
            currentUser.client = {};
            currentUser.client.host = client[0].concat(":", client[1]);
            currentUser.server.protocol = "https";
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
        }

        handleLogout(){
            this.authService.logout(this.configService.currentUser.lightwave_server);
        }
}
