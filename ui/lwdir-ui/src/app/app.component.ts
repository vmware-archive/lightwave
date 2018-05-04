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

import { Component, OnInit, Inject, Injectable } from '@angular/core';
import { AuthService } from './auth.service';
@Component({
  selector: 'myapp',
  templateUrl: './app.component.html'
})

export class AppComponent  {
    private error:any = '';
    constructor(private authService: AuthService) {
       console.log("App component");
    }
    ngOnInit() {
    }
}

