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

import { NgModule, Renderer2, EventEmitter, Output }      from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ClarityModule } from 'clarity-angular';
import { AppComponent }  from './app.component';
import { HttpModule, JsonpModule }    from '@angular/http';
import { FormsModule } from '@angular/forms';
import { HttpClientModule } from '@angular/common/http';
import { AuthService } from './auth.service';
import { LazyLoadedLevel1Component } from './vmdir.component';
import { SdeditorComponent } from './sdeditor.component';
import { ObjectAddComponent } from './objectadd.component';
import { VmdirService }  from './vmdir.service';
import { IdentitySourceService }  from './identitysources.service';
import { ConfigService }  from './config.service';
import { UtilsService }  from './utils.service';
import { SDDLParser }  from './sddlparser';
import { VmdirUtils }  from './vmdir.utils';
import { VmdirSchemaService }  from './vmdirschema.service';
import { AppRoutingModule, routableComponents } from './app.routing.module';
 import { DateTimePickerModule } from 'ng-pick-datetime';
@NgModule({
  imports:      [
                 AppRoutingModule,
                 ClarityModule,
                 BrowserModule,
                 FormsModule,
                 BrowserAnimationsModule,
                 HttpModule,
                 HttpClientModule,
                 JsonpModule,
                 DateTimePickerModule
                ],
  declarations: [ AppComponent, routableComponents, LazyLoadedLevel1Component, SdeditorComponent, ObjectAddComponent],
  providers:    [ AuthService, VmdirService, VmdirUtils, VmdirSchemaService, ConfigService, UtilsService, IdentitySourceService, SDDLParser ],
  bootstrap:    [ AppComponent ],
})
export class AppModule { }
