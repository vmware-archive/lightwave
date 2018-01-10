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

import { Component, Inject } from '@angular/core';
import { Router } from '@angular/router';
import { AuthService } from './auth.service';
import { HttpHeaders } from '@angular/common/http';
@Component({
  selector: 'login',
  templateUrl: './login.component.html',
})
export class LoginComponent {
  tokens:string ="";
  tokensObj: any ;
  authFailed: boolean = false;

  constructor(public router: Router, private authService: AuthService) {
  }

  login(event:any , tenant:string, username:string, password:string) {
    event.preventDefault();
    username = encodeURI(username);
    password = encodeURI(password);
    let id = username.indexOf('@');
    let tenantName = username.substr(id+1);
    console.log(tenantName);
    this.authService.getOIDCToken(tenant, username, password, tenantName)
        .subscribe(
            tokens => {
                    this.tokens = JSON.stringify(tokens);
                    console.log(this.tokens);
                    this.tokensObj = JSON.parse(this.tokens);
                    console.log(this.tokensObj);
                    this.router.navigate(['lightwaveui/vmdir']);
                    },
            error =>{
                   console.log(error);
                   this.authFailed = true;
                    }
                 );
  }
}
