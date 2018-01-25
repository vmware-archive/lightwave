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

import {StringFilter, DatagridFilter } from "clarity-angular";

export class AttributeFilter implements StringFilter<any> {
    accepts(attrObj: any, search: string):boolean {
        return attrObj.attrType.toLowerCase().indexOf(search) >= 0;
    }
}

export class UserAccountFilter implements StringFilter<any> {
    accepts(userObj: any, search: string):boolean {
         let str:string = userObj.cn;
         return str.toLowerCase().indexOf(search) >= 0;
    }
}

export class SimpleStringFilter implements StringFilter<any> {
    accepts(name: string, search: string):boolean {
         return name.toLowerCase().indexOf(search) >= 0;
    }
}
