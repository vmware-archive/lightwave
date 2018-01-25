"use strict";
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
Object.defineProperty(exports, "__esModule", { value: true });
var AttributeFilter = (function () {
    function AttributeFilter() {
    }
    AttributeFilter.prototype.accepts = function (attrObj, search) {
        return attrObj.attrType.toLowerCase().indexOf(search) >= 0;
    };
    return AttributeFilter;
}());
exports.AttributeFilter = AttributeFilter;
var UserAccountFilter = (function () {
    function UserAccountFilter() {
    }
    UserAccountFilter.prototype.accepts = function (userObj, search) {
        var str = userObj.cn;
        return str.toLowerCase().indexOf(search) >= 0;
    };
    return UserAccountFilter;
}());
exports.UserAccountFilter = UserAccountFilter;
var SimpleStringFilter = (function () {
    function SimpleStringFilter() {
    }
    SimpleStringFilter.prototype.accepts = function (name, search) {
        return name.toLowerCase().indexOf(search) >= 0;
    };
    return SimpleStringFilter;
}());
exports.SimpleStringFilter = SimpleStringFilter;
//# sourceMappingURL=customstringfilter.js.map