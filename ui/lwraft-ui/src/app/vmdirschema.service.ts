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
import {ConfigService} from './config.service';
import { AuthService } from './auth.service';
import { UtilsService } from './utils.service';

@Injectable()
export class VmdirSchemaService {
    getUrl:string
    postServer:string = '';
    listing:any;
    error: any;
    postPort:number;
    schemaArr: any[];
    schema:string = '';
    constructor(private utilsService:UtilsService, private configService:ConfigService, private authService: AuthService, private httpClient:HttpClient) {}

    getSchema(rootDn:string): Observable<string[]> {
        this.postServer = this.authService.getPostServer();
        this.postPort = this.authService.getPostPort();
        let headers = this.authService.getAuthHeader();
        this.getUrl = 'https://' + this.postServer + ':' + this.postPort + '/v1/post/ldap';
        console.log("root DN:" + rootDn);
        this.getUrl += '?dn='+encodeURIComponent('cn='+rootDn+',cn=schemacontext');
        console.log(this.getUrl);
        return this.httpClient.get(this.getUrl, {headers})
               .share()
               .map((res: Response) => res)
               .do(listing => this.listing = listing)
               .catch(err => this.utilsService.handleError(err))
    }
}
