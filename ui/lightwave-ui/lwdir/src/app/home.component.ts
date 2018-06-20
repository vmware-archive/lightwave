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
    isSysTenantAdmin:boolean = false;
    displayComponents:boolean = false;
    allTenants:any;
    isSystemTenant:boolean = false;
    showOtherTenants:boolean = false;
    showPostNodes:boolean = false;
    rightMenuOpen:boolean = false;
    postOIDCs:any[] = [];
    constructor(@Inject(DOCUMENT) private document: any, private authService:AuthService, private vmdirService:VmdirService, private identitySourceService:IdentitySourceService, private utilsService: UtilsService, private configService: ConfigService, private activatedRoute: ActivatedRoute){
    }

    ngOnInit(){
       console.log('sessionStorage:');
       console.log(window.sessionStorage);
       if(window.sessionStorage.currentUser && window.sessionStorage.currentUser != 'logout'){
            console.log(window.sessionStorage.currentUser);
            this.displayComponents = true;
            this.loadFromSession();
       }else{
           this.activatedRoute.queryParams.subscribe((params: Params) => {
              console.log(params);
              this.readQueryParams(params);
           });
       }
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

    toggleTenants(){
        this.showOtherTenants = !this.showOtherTenants;
    }

    togglePostNodes(){
        this.showPostNodes = !this.showPostNodes;
    }

    readQueryParams(params:any){
       let idToken = params['id_token'];
       let accessToken = params['access_token'];
       let tokenType = params['token_type'];
       let expiresIn = params['expires_in'];
       let state = params['state'];
       let curUser:any = sessionStorage.getItem('currentUser');
       if(params['logout']){
          this.configService.currentUser = null;
       }else{
          if(curUser){
             if(curUser == 'logout'){  // session invalid, load from query paramters
               if(accessToken && idToken){
                   this.displayComponents = true;
                   this.setcontext(idToken, accessToken, tokenType, expiresIn, state);
               }else{ // query parameters unavailable, redirect to home page
                   this.utilsService.redirectToLandingPage();
               }
             }else{ // session is valid, load from session.
                curUser = JSON.parse(curUser);
                this.displayComponents = true;
                this.configService.currentUser = curUser;
                this.loadFromSession();
             }
          }else{ // No session in progress
               if(accessToken && idToken){ // load from query parameters if available
                   this.displayComponents = true;
                   this.setcontext(idToken, accessToken, tokenType, expiresIn, state);
               }else{ // query parameters unavailable, redirect to home page.
                   this.utilsService.redirectToLandingPage();
               }
          }
       }
    }

    openOtherTenant(tenant:any){
        let decodedJwt:any = this.utilsService.decodeJWT(this.configService.currentUser.token.id_token);
        let clientId:string = decodedJwt.header.client_id;
        let redirectUri:string = "https://" + this.configService.currentUser.server.host + "/lightwaveui/Home";
        let loginUrl:string = "https://" + this.configService.currentUser.server.host + "/openidconnect/oidc/authorize/" + tenant.displayName;
        let args:string = "?response_type=id_token%20token&response_mode=form_post&"
                               + "client_id=" + clientId + "&redirect_uri=" + redirectUri +
                                "&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server%20rs_vmdir";
        let completeUrl = loginUrl + args;
        let logoutUri = this.authService.getLogoutUri();
        this.authService.invalidateLocalSession();
        window.open(completeUrl, '_blank');
        window.location.href = logoutUri;
    }

    openLwPost(node:any){
        let authorizeUrl:string = "https://" + this.configService.currentUser.server.host + "/openidconnect/oidc/authorize/" + this.tenantName +
                                  "?response_type=id_token%20token&response_mode=fragment&client_id=" + node.clientId +
                                  "&redirect_uri=" + node.redirectURI +
                                  "&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server%20rs_post";
        window.open(authorizeUrl, '_blank');
    }

    loadFromSession(){
        let curSession = JSON.parse(window.sessionStorage.currentUser);
        this.signedInUser = curSession.first_name;
        this.role = curSession.role;
        this.tenantName = curSession.tenant;
        this.configService.currentUser = curSession;
        this.checkForSystemTenant();
    }

    setcontext(id_token, access_token, token_type, expires_in, state) {
        let decodedJwt:any = this.utilsService.decodeJWT(id_token);
        let decodedAccessJwt:any = this.utilsService.decodeJWT(access_token);
        let currentUser:any =  {};
        currentUser.server = {};
        currentUser.server.host = window.location.host;
        currentUser.server.port = window.location.port;
        currentUser.server.protocol = window.location.protocol;
        currentUser.tenant = decodedJwt.header.tenant;
        currentUser.username = decodedJwt.header.sub;
        currentUser.first_name = decodedJwt.header.given_name;
        currentUser.last_name = decodedJwt.header.family_name;
        currentUser.role = decodedJwt.header.admin_server_role;
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

    getAllTenants() {
        let tlist:any;
        let listingObj:any;
        this.vmdirService.getAllTenants(this.configService.currentUser.tenant)
        .subscribe(
            listing => {
               tlist = JSON.stringify(listing);
               listingObj = JSON.parse(tlist);
               this.utilsService.extractName(listingObj.result, false);
               this.allTenants = listingObj.result;
               let pos=0;
               for (let tenant of this.allTenants){
                   if(tenant.displayName === this.tenantName){
                       this.allTenants.splice(pos,1);
                   }
                   pos ++;
               }
               },
               error => this.error = <any>error);
     }

    getAllOIDCClients() {
        let tlist:any;
        let arrList:any[];
        this.vmdirService.getOIDCClients(this.configService.currentUser.tenant)
        .subscribe(
            listing => {
               tlist = JSON.stringify(listing);
               arrList = JSON.parse(tlist).result;
               arrList.shift();
               for(let oidcClient of arrList){
                   let attribsArr = oidcClient.attributes;
                   let oidcObj:any = {};
                   for (let attr of attribsArr){
                       if(attr.type == 'vmwOidcRedirectURIs'){
                           oidcObj.redirectURI = attr.value[0];
                       }
                       if(attr.type == 'vmwOidcClientID'){
                           oidcObj.clientId = attr.value[0];
                       }
                   }
                   let parts:string[] = oidcObj.redirectURI.split('/');
                   if(parts[parts.length - 1] == 'ui'){
                       oidcObj.host = parts[parts.length - 2];
                       this.postOIDCs.push(oidcObj);
                   }
               }
               },
               error => this.error = <any>error);
     }

     checkForSystemTenant() {
        let res;
        this.identitySourceService
            .GetAll(this.configService.currentUser)
            .subscribe(
                result => {
                       res = result;
                       let identitySources = res;
                       for (var i = 0; i < identitySources.length; i++) {
                           if (identitySources[i].domainType == 'LOCAL_OS_DOMAIN') {
                               this.isSystemTenant = true;
                               if(this.configService.currentUser.role == 'Administrator'){
                                   this.isSysTenantAdmin = true;
                               }
                               break;
                           }
                       }
                       if(this.isSystemTenant && this.isSysTenantAdmin){
                           this.getAllTenants();
                           this.getAllOIDCClients();
                       }
                 },
                 error => {
                     this.configService.errors = error.data;
                 });
     }

     handleLogout(){
        this.authService.logout();
     }
}
