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
       if(window.sessionStorage.currentUser && window.sessionStorage.currentUser != 'logout'){
            console.log(window.sessionStorage.currentUser);
            this.displayComponents = true;
            this.loadFromSession();
       }else{
           this.activatedRoute.fragment.subscribe((params: any) => {
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
        let idmUri = "/lightwaveui/app/#/ssohome"
        window.location.href = idmUri;
    }

    readQueryParams(params:string){
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
                this.setcontext(idToken, accessToken, tokenType, expiresIn, state);
            }else{
                this.redirectToSTSLogin();
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
                    this.setcontext(idToken, accessToken, tokenType, expiresIn, state);
                }else{ // No fragments present either.
                    this.redirectToSTSLogin();
                }
            }
        }
    }

    // Fetch the STS login URL & redirect there
    redirectToSTSLogin(){
        let resObj:any;
        let resStr:string; 
        this.authService.getSTSLoginUrl()
        .subscribe(
            result => {
                          resStr = JSON.stringify(result);
                          resObj = JSON.parse(resStr);
                          console.log(resObj.result)
                          this.loginURL = resObj.result;
                          window.location.href = this.loginURL;
                       },
              error => {
                  this.configService.errors = error.data;
                  this.showError = true;
              });
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
            // get the IDP host
            let resObj:any;
            let resStr:string;
            let idpHost:any;

            this.authService.getSTSLoginUrl()
            .subscribe(
                result => {
                          resStr = JSON.stringify(result);
                          resObj = JSON.parse(resStr);
                          idpHost = resObj.idp_host;
                          this.authService.logout(idpHost);
                       },
              error => {
                  this.configService.errors = error.data;
                  this.showError = true;
              });

        }
}
