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
import {ConfigService} from './config.service';
import {UtilsService} from './utils.service';
import {HttpClient} from '@angular/common/http';
import { Response } from '@angular/http';
import { Observable } from "rxjs/Rx";
@Injectable()
export class IdentitySourceService
{
    constructor(private httpClient:HttpClient, private configService:ConfigService, private utilsService:UtilsService){
    }

    GetAll(context:any):Observable<any> {
        let res:any
        let endpoint:string = this.configService.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        let cType:string = 'application/json';
        let headers:any = this.configService.getHeaders(context.token,cType);
        return this.httpClient.get(endpoint, {headers})
               .share()
               .map((res: Response) => res)
               .catch(err => this.utilsService.handleError(err))
    }

    Add(context:any, identitySource:string):Observable<any> {
        let endpoint:string = this.configService.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        let cType:string = 'application/json';
        let headers:any = this.configService.getHeaders(context.token,cType);
        return this.httpClient.post(endpoint, identitySource, {headers})
               .map((res: Response) => res)
               .catch(err => this.utilsService.handleError(err))
    }

    Update(context:any, identitySource:any):Observable<any> {
        let endpoint = this.configService.getIdentitySourceEndpoint(context.server, context.tenant, identitySource.name);
        let cType = 'application/json';
        let headers = this.configService.getHeaders(context.token,cType);
        return this.httpClient.put(endpoint, identitySource, {headers})
               .map((res: Response) => res)
               .catch(err => this.utilsService.handleError(err))
    }

    Delete(context:any, name:string):Observable<any> {
        let endpoint = this.configService.getIdentitySourceEndpoint(context.server, context.tenant, name);
        let cType = 'application/json';
        let headers = this.configService.getHeaders(context.token,cType);
        return this.httpClient.delete(endpoint, {headers})
               .map((res: Response) => res)
               .catch(err => this.utilsService.handleError(err))
    }

    TestConnectivity(context:any, identitySource:string):Observable<any>{
        let endpoint = this.configService.getAllIdentitySourcesEndpoint(context.server, context.tenant);
        let cType = 'application/json';
        let headers = this.configService.getHeaders(context.token,cType);
        endpoint = endpoint + "?probe=true";
        return this.httpClient.post(endpoint, identitySource, {headers})
               .map((res: Response) => res)
               .catch(err => this.utilsService.handleError(err))
    }
}
