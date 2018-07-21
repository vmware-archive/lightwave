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

import { Response } from '@angular/http';
import {Injectable} from '@angular/core';
import { Observable } from "rxjs/Rx";
import { ConfigService } from './config.service';

@Injectable()
export class UtilsService{
    currentUser;
    constructor(private configService:ConfigService){
    }
    decodeJWT(jwt:string):any {
        let sJWS = jwt;

        let components = sJWS.split(".");
        let uHeader = atob(components[1]).toString();

        let decodedJWT =
        {
            header: JSON.parse(uHeader)
            //claims: JSON.parse(uClaim)
        };

        return decodedJWT;
    }
    setIcon(cnObj: any){
       console.log(cnObj);
       if(cnObj.displayName.indexOf("Users") != -1 || cnObj.displayName.indexOf("Administrators") != -1 || cnObj.displayName.indexOf("Operators") != -1) {
           cnObj.iconShape = "user";
           cnObj.iconClass = "is-solid is-info";
       }
       else if(cnObj.displayName.indexOf("Computers") != -1)  {
           cnObj.iconShape="computer";
           cnObj.iconClass = "is-solid is-info";
       }
       else if(cnObj.displayName.indexOf("Security") != -1)  {
           cnObj.iconShape="shield";
           cnObj.iconClass = "is-solid is-info";
       }
       else if(cnObj.displayName.indexOf("Component") != -1) {
           cnObj.iconShape = "plugin";
           cnObj.iconClass = "is-solid is-info";
       }
       else if(cnObj.displayName.indexOf("Principals") != -1) {
           cnObj.iconShape = "list";
           cnObj.iconClass = "is-info";
       }
       else if(cnObj.displayName.indexOf("Services") != -1) {
           cnObj.iconShape = "cog";
           cnObj.iconClass = "is-solid is-info";
       }
       else if(cnObj.displayName.indexOf("Domain Controller") != -1) {
           cnObj.iconShape = "cluster";
           cnObj.iconClass = "is-info is-solid";
       }
       else if(cnObj.displayName.indexOf("Configuration") != -1) {
           cnObj.iconShape = "file-settings";
           cnObj.iconClass = "is-solid is-info";
       }
       else if(cnObj.displayName.indexOf("Service Accounts") != -1) {
           cnObj.iconShape = "users";
           cnObj.iconClass = "is-info is-solid";
       }
       else if(cnObj.displayName.indexOf("Zones") != -1) {
           cnObj.iconShape = "network-globe";
           cnObj.iconClass = "is-info";
       }
       else if(cnObj.displayName.indexOf("Builtin") != -1) {
           cnObj.iconShape = "connect";
           cnObj.iconClass = "is-info is-solid";
       }
       else if(cnObj.displayName.indexOf("Deleted") != -1) {
           cnObj.iconShape = "trash";
           cnObj.iconClass = "is-info is-solid";
           cnObj.active = 'true';
       }
       else if(cnObj.displayName.indexOf("policy") != -1) {
           cnObj.iconShape = "tasks";
           cnObj.iconClass = "is-info";
       }
       else {
           cnObj.iconShape = "folder";
           cnObj.iconClass = "is-solid is-info";
       }
    }

    extractName(dirList:any[], doSetIcon:boolean) {
        for (let cn of dirList) {
            let name = cn.dn;
            let id = name.indexOf(',');
            name = name.substr(0, id);
            id = name.indexOf('=');
            name = name.substr(id+1);
            console.log(name);
            cn.displayName = name;
            cn.active= 'false';
            if(doSetIcon){
                this.setIcon(cn);
            }
            cn.encodedDN = encodeURIComponent(cn.dn);
        }
    }

    strInsert(str:string, toInsert:string, position:number){
        str = [str.slice(0, position), toInsert, str.slice(position)].join('');
        return str;
    }

    toValidTimeStr(dateInput:string):string{
        dateInput = this.strInsert(dateInput, '-', 4);
        dateInput = this.strInsert(dateInput, '-', 7);
        dateInput = this.strInsert(dateInput, 'T', 10);
        dateInput = this.strInsert(dateInput, ':', 13);
        dateInput = this.strInsert(dateInput, ':', 16);
        dateInput = dateInput.replace('.0','');
        return dateInput;
    }

    getRootDnQuery(dmn:string) {
       let index = dmn.indexOf('.', 0);
       let finalStr:string = '';
       let prevIndex:number = -1;
       while(-1 != index){
           let dm1 = dmn.substr(0,index);
           finalStr += ('dc='+dm1+',');
           prevIndex = index;
           index = dmn.indexOf('.', index+1);
       }
       let dm2 = dmn.substr(prevIndex+1);
       finalStr += ('dc='+dm2);
       return encodeURIComponent(finalStr);
    }

    constructLogoutUrl(idpHost:string):string {
       let curUser = JSON.parse(window.sessionStorage.currentUser);
       let tenant = curUser.tenant;
       let postLogoutUrl = "https://" + document.location.host + "/lwraftui";
       let logoutUrl = "https://" + idpHost + "/openidconnect/logout/" + tenant;
       let token = curUser.token.id_token;
       let state = curUser.token.state;
       let args = "?id_token_hint=" + token +
                  "&post_logout_redirect_uri=" + postLogoutUrl + "&state=" + state;
       return logoutUrl + args;
    }

    handleError(error:any) {
        console.log(error);
        if((error) &&
        (error.status == 401 || (error.error && error.error.error == 'invalid_token'))){
                var redirectUri = '/lwraftui';
                sessionStorage.currentUser = 'logout';
                window.location.href = redirectUri;
        }else{
            return Observable.throw(error);
        }
    }

}
