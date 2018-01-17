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

import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { VmdirComponent } from './vmdir.component';
import { HomeComponent }  from './home.component';

const appRoutes: Routes = [
    { path: '', component: HomeComponent},
    { path: 'lightwaveui/vmdir', component: VmdirComponent},
    // otherwise redirect to home
    { path: '**', redirectTo: 'lightwaveui/Home' }
];
@NgModule({
    imports:[RouterModule.forRoot(appRoutes)],
    exports:[RouterModule],
})
export class AppRoutingModule {}

export const routableComponents = [
    VmdirComponent,
    HomeComponent
]
