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

import { Injectable, Inject } from '@angular/core'
import { Response } from '@angular/http';
import { HttpClient, HttpHeaders } from '@angular/common/http';
import {Jsonp} from '@angular/http'
import { Observable } from "rxjs/Rx";
import './rxjs-operators';
import { ConfigService } from './config.service';
import { AuthService } from './auth.service';
import { UtilsService } from './utils.service';
@Injectable()
export class VmdirService {
    getUrl:string
    listing:any;
    domain:string
    constructor(private utilsService:UtilsService, private configService:ConfigService, private authService: AuthService, private httpClient:HttpClient) {}
    getDirListing(rootDn:string): Observable<string[]> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        if(rootDn == null) {
            rootDn = this.authService.getRootDnQuery();
        }
        console.log("root DN:" + rootDn);
        this.getUrl += '?scope=one&dn='+rootDn+'&attrs=dn';
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, {headers})
               .share()
               .map((res: Response) => res)
               .do(listing => this.listing = listing)
               .catch(this.handleError)
    }
    getAllTenants(tenantName:string): Observable<string[]> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let rootDn = encodeURIComponent('cn=Tenants,cn=IdentityManager,cn=Services,') + this.utilsService.getRootDnQuery(tenantName)
        console.log("root DN:" + rootDn);
        this.getUrl += '?scope=one&dn='+rootDn+'&attrs=dn';
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, {headers})
               .share()
               .map((res: Response) => res)
               .do(listing => this.listing = listing)
               .catch(this.handleError)
    }

    getAttributes(rootDn:string): Observable<string[]> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        if(rootDn == null) {
            rootDn = this.authService.getRootDnQuery();
        }
        console.log("root DN:" + rootDn);
        this.getUrl += '?scope=base&dn='+rootDn+'&attrs='+encodeURIComponent('*,+');
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, {headers})
               .share()
               .map((res: Response) => res)
               .do(listing => this.listing = listing)
               .catch(this.handleError)
    }

    getACLString(rootDn:string): Observable<string[]> {
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let url = this.getUrl + '?scope=base&dn=' + rootDn + '&attrs=vmwaclstring';
        console.log(url);
        return this.httpClient.get(url, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError);
    }
    getJsonObjEntry(operation:string,attrType:string, attrValue:any):any{
        let jsonObjEntry:any={};
        jsonObjEntry.operation = operation;
        jsonObjEntry.attribute = {};
        jsonObjEntry.attribute.type = attrType;
        jsonObjEntry.attribute.value = [attrValue];
        return jsonObjEntry;
    }
    constructJsonBody(initialValMap:Map<string,any>, attrArr:string[], attrMap:Map<string,any>, schemaMap:Map<string,any>): string {
        let jsonOut:string;
        let jsonArrObj:any[] = [];
        for(let attrEntry of attrArr) {
           let attrType:string = attrEntry; //has the key
           let updatedVal:any = attrMap[attrType];
           let initVal = initialValMap[attrType];
           if(initialValMap[attrType] && schemaMap[attrType]['isSingleValued'][0] == 'FALSE'){ //handle list attribute update
               //create a map of Existing and new values
               let newMap = new Map<string, boolean>();
               for (var p = 0; p < updatedVal.value.length; p ++){
                   newMap[updatedVal.value[p]] = false;
               }
               for(var i = 0;i < initVal.length; i ++){
                   if(undefined == newMap[initVal[i]]) {// current value missing in new list, so delete it
                       jsonArrObj.push(this.getJsonObjEntry('delete', attrType, initVal[i]));
                   }else{
                       newMap[initVal[i]] = true; // new list[i] = old list[i]
                   }
               }
               for (var i = 0; i < updatedVal.value.length; i ++){
                   if(!newMap[updatedVal.value[i]]){ // If new list value, add!
                       jsonArrObj.push(this.getJsonObjEntry('add', attrType, updatedVal.value[i]));
                   }
               }
          }
          else if(!initialValMap[attrType] && schemaMap[attrType]['isSingleValued'][0] == 'FALSE'){ // handle list attribute addition
                for(var k = 0;k < updatedVal.value.length; k ++){
                       jsonArrObj.push(this.getJsonObjEntry('add', attrType, updatedVal.value[k]));
               }
          }
          else{ //handle single valued attr update(add/replace)
               let operation:string;
               if(!initVal || !initVal.length){
                   operation = 'add';
               }else{
                   operation = 'replace';
               }
               jsonArrObj.push(this.getJsonObjEntry(operation, attrType, updatedVal.value));
          }
       }
       return JSON.stringify(jsonArrObj);
    }

    updateAttributes(rootDn:string, originalValMap:Map<string,any>, attribsArr:string[], modifiedAttributesMap:Map<string,any>, schemaMap:Map<string,any>): Observable<string[]> {
        let headers = this.authService.getAuthHeader();
        let body = this.constructJsonBody(originalValMap, attribsArr, modifiedAttributesMap, schemaMap);
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let updateUrl = this.getUrl + '?dn='+rootDn;
        console.log(updateUrl);
        return this.httpClient.patch(updateUrl, body, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    constructSDJsonBody(aclString:string):string{
        let jsonOut = "[{\
\"operation\": \"" + "replace" +"\",\
\"attribute\": {\
\"type\":\"" + 'vmwaclstring' + "\",\
\"value\": [\"" + aclString +"\"]\
}\
}]"
        console.log(JSON.parse(jsonOut));
        return jsonOut;
    }
    createAttrObject(attr:string, value:string):any{
        let obj:any = {};
        obj.type=attr;
        obj.value=value.split(',');
        return obj;
    }
    createAttrObjArray(attrMap:any, attrsArr:string[],  jsonObjAttrs:any[]){
        for(let i = 0;i < attrsArr.length; i ++){
            if(attrMap[attrsArr[i]].length){
                jsonObjAttrs.push(this.createAttrObject(attrsArr[i], attrMap[attrsArr[i]]))
            }
        }
    }

    constructObjectAddJsonBody(mustAttrMap:any, mayAttrMap:any):any[]{
        let jsonObjAttrs:any[] = [];
        let mustAttrsArr:string[] = Object.keys(mustAttrMap);
        let mayAttrsArr:string[] = Object.keys(mayAttrMap);
        this.createAttrObjArray(mustAttrMap, mustAttrsArr, jsonObjAttrs);
        this.createAttrObjArray(mayAttrMap, mayAttrsArr, jsonObjAttrs);
//        jsonObjAttrs.push(this.createAttrObject('nTSecurityDescriptor', ''));
        return jsonObjAttrs;
    }

    addNewObject(rootDn:string, mustAttrMap:any, mayAttrMap:any): Observable<string[]> {
        let headers = this.authService.getAuthHeader();
        if(mustAttrMap['cn'] && mustAttrMap['cn'].length){
            mayAttrMap['cn'] = '';
        }
        let jsonObj:any = {};
        jsonObj.dn = rootDn;
        jsonObj.attributes = this.constructObjectAddJsonBody(mustAttrMap, mayAttrMap);
        let body = JSON.stringify(jsonObj);
        let addUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        console.log(addUrl);
        return this.httpClient.put(addUrl, body, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    updateAclString(rootDn:string, aclStrVal:string){
        let headers = this.authService.getAuthHeader();
        let body  = this.constructSDJsonBody(aclStrVal);
        let url = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        url += '?dn=' + encodeURIComponent(rootDn);
        console.log(url);
        return this.httpClient.patch(url, body, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    delete(rootDn:string) {
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap'
        let deleteUrl = this.getUrl +  '?dn='+rootDn;
        console.log(deleteUrl);
        return this.httpClient.delete(deleteUrl, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    getObjectBySID(objectSid:string): Observable<string[]> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let url = this.getUrl + '?scope=sub&dn=' + this.authService.getRootDnQuery() + '&filter=' + encodeURIComponent('objectsid='+objectSid);
        console.log(url);
        return this.httpClient.get(url, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    getAllStrObjectClasses():Observable<any> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let url = this.getUrl + '?scope=one&dn=' + encodeURIComponent('cn=schemacontext') + '&filter=' + encodeURIComponent('objectclass=classschema') + '&attrs='+encodeURIComponent('subClassOf,cn,systemMustContain,systemMayContain,auxiliaryClass,mayContain,mustContain,objectClassCategory');
        console.log(url);
        return this.httpClient.get(url, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    getAuxObjectClass(objectClass:string):Observable<string[]> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let url = this.getUrl + '?scope=base&dn=' + encodeURIComponent('cn='+objectClass+',cn=schemacontext') + '&attrs=auxiliaryClass';
        console.log(url);
        return this.httpClient.get(url, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    getAllUsersAndGroups(){
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let url = this.getUrl + '?scope=sub&dn=' + this.authService.getRootDnQuery() + '&filter=' + encodeURIComponent('objectsid=*')+
                  '&attrs=objectsid,cn,objectclass';
        console.log(url);
        return this.httpClient.get(url, {headers})
           .share()
           .map((res: Response) => res)
           .catch(this.handleError)
    }

    getObjectByGUID(objectGuid:string): Observable<string[]> {
        this.domain = this.authService.getDomain();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.domain + ':' + this.configService.API_PORT + '/v1/vmdir/ldap';
        let url = this.getUrl + '?scope=sub&dn=' + this.authService.getRootDnQuery() + '&filter=' + encodeURIComponent('objectguid='+objectGuid);
        console.log(url);
        return this.httpClient.get(url, {headers})
               .share()
               .map((res: Response) => res)
               .catch(this.handleError)
    }

    private handleError(error:any) {
        console.log(error);
        return Observable.throw(error.error);
    }
}
